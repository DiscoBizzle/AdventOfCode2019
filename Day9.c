#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "StretchyBuffer.h"

typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

void printIntBuf(i64 *buf) {
	for (u64 i = 0; i < buf_len(buf)-1; i++) {
		printf("%lld, ", buf[i]);
	}
	printf("%lld\n", buf[buf_len(buf)-1]);
}

i64 *parseArray(const char *filePath) {
	FILE* filePointer = fopen(filePath, "r");
	if (filePointer == NULL) {
		printf("Unable to read file!\n");
		return NULL;
	}
	char *inputString = NULL;
	size_t len = 0;
	ssize_t read;

	if ((read = getline(&inputString, &len, filePointer)) == -1) {
		printf("Unable to read line from file!\n");
		return NULL;
	}
	fclose(filePointer);

	char temp[40]; 
	char *start = inputString;
	char *end = inputString;

	i64 *array = NULL;

	while (*end != 0) {
		if (isdigit(*end) || *end == '-') {
			end++;
		} else {
			strncpy(temp, start, end-start);
			temp[end-start] = 0;
			buf_push(array, atol(temp));

			start = end+1;
			end = start;
		}
	}
	free(inputString);
	return array;
}

typedef struct {
	i64 *memory;
	i64 cursor;
	bool running;
	bool waiting;
	i64 *inputQueue;
	i64 inputCursor;
	i64 *outputQueue;
	i64 relativeBase;
} computer;

#define POSITION_MODE 0
#define IMMEDIATE_MODE 1
#define RELATIVE_MODE 2

i64 get_value(i64 value, computer c, i64 mode) {
	switch (mode) {
		case POSITION_MODE: {
			return c.memory[value];
		} break;
		case IMMEDIATE_MODE: {
			return value;
		} break;
		case RELATIVE_MODE: {
			return c.memory[c.relativeBase + value];
		} break;
		default: {
			printf("Error! unrecognized mode %lld\n", mode);
			return ~0;
		}
	}
}

void put_value(i64 value, computer *c, i64 param, i64 mode) {
	i64 address = -1;
	switch (mode) {
		case POSITION_MODE: {
			address = param;
		} break;
		case IMMEDIATE_MODE: {
			printf("Error! tried to write in immediate mode\n");
			return;
		} break;
		case RELATIVE_MODE: {
			address = c->relativeBase+param;
		} break;
		default: {
			printf("Error! unrecognized mode %lld\n", mode);
			return;
		}
	}

	i64 paddingNeeded = address - buf_len(c->memory) + 1;

	for (i64 padIndex = 0; padIndex < paddingNeeded; padIndex++) {
		buf_push(c->memory, 0);
	}

	c->memory[address] = value;
}

void print_memory(i64 *memory, i64 min_index, i64 max_index) {
	for (i64 i = min_index; i <= max_index; i++) {
		printf("%lld] \t %lld \n", i, memory[i]);
	}
}

void compute(computer *c) {

	while (c->running && !c->waiting) {
		i64 opcode = c->memory[c->cursor] % 100;
		i64 mode1 = (c->memory[c->cursor] / 100)%10;
		i64 mode2 = (c->memory[c->cursor] / 1000)%10;
		i64 mode3 = (c->memory[c->cursor] / 10000)%10;

		switch(opcode) {
			case 1: { // add
				if (mode3 == IMMEDIATE_MODE) {
					printf("Error! tried to write in immediate mode\n");
					return;
				}
				i64 read1 = c->memory[c->cursor+1];
				i64 read2 = c->memory[c->cursor+2];
				i64 write = c->memory[c->cursor+3];

				i64 sum = get_value(read1, *c, mode1) + get_value(read2, *c, mode2);
				put_value(sum, c, write, mode3);
				c->cursor += 4;
			} break;

			case 2: { // multiply
				if (mode3 == IMMEDIATE_MODE) {
					printf("Error! tried to write in immediate mode\n");
					return;
				}
				i64 read1 = c->memory[c->cursor+1];
				i64 read2 = c->memory[c->cursor+2];
				i64 write = c->memory[c->cursor+3];

				i64 product = get_value(read1, *c, mode1) * get_value(read2, *c, mode2);
				put_value(product, c, write, mode3);
				c->cursor += 4;
			} break;
			case 3: { // input
				if (mode1 == IMMEDIATE_MODE) {
					printf("Error! tried to write in immediate mode\n");
					return;
				}
				i64 write = c->memory[c->cursor+1];
				if (c->inputCursor < buf_len(c->inputQueue)) {
					i64 val = c->inputQueue[c->inputCursor++];
					put_value(val, c, write, mode1);
					c->cursor += 2;
				} else {
					// if we don't have any more inputs in the queue,
					// go to wait mode.  
					c->waiting = true;
					return;
				}
			} break;
			case 4: { // output
				i64 read = c->memory[c->cursor+1];
				i64 output = get_value(read, *c, mode1);
				buf_push(c->outputQueue, output);
				// printf("output = %d\n", output);
				c->cursor +=2;
			} break;
			case 5: { // jump-if-true
				i64 read = c->memory[c->cursor+1];
				i64 jump = c->memory[c->cursor+2];
				if (get_value(read, *c, mode1) != 0) {
					c->cursor = get_value(jump, *c, mode2);
				} else {
					c->cursor += 3; 
				}
			} break; 
			case 6: { // jump if false 
				i64 read = c->memory[c->cursor+1];
				i64 jump = c->memory[c->cursor+2];
				if (get_value(read, *c, mode1) == 0) {
					c->cursor = get_value(jump, *c, mode2);
				} else {
					c->cursor += 3; 
				}
			} break;	
			case 7: { // less than
				i64 read1 = c->memory[c->cursor+1];
				i64 read2 = c->memory[c->cursor+2];
				i64 write = c->memory[c->cursor+3];
				if (get_value(read1, *c, mode1) < get_value(read2, *c, mode2)) {
					put_value(1, c, write, mode3);
					// c->memory[write] = 1;
				} else {
					put_value(0, c, write, mode3);
					// c->memory[write] = 0;
				}
				c->cursor += 4;
			} break;
			case 8: { // equals
				i64 read1 = c->memory[c->cursor+1];
				i64 read2 = c->memory[c->cursor+2];
				i64 write = c->memory[c->cursor+3];
				if (get_value(read1, *c, mode1) == get_value(read2, *c, mode2)) {
					put_value(1, c, write, mode3);
					// c->memory[write] = 1;
				} else {
					put_value(0, c, write, mode3);
					// c->memory[write] = 0;
				}
				c->cursor += 4;
			} break;
			case 9: { // change relative base
				i64 read1 = c->memory[c->cursor+1];
				c->relativeBase += get_value(read1, *c, mode1);
				c->cursor += 2;
			} break;
			case 99: {
				c->running = false;
			} break;
			default: {
				printf("error! expected opcode 1-8 or 99, got %lld\n", opcode);
				return;
			}
		}
	}
	return;
}

i64 *buf_copy(i64 *input) {
	i64 *copy = NULL;
	for (i64 i = 0; i < buf_len(input); i++) {
		buf_push(copy, input[i]);
	}
	return copy;
}

computer initComputer(i64 *program) {
	computer C = {};
	C.memory = buf_copy(program);
	C.running = true;
	C.waiting = false;
	
	return C;
}

void test1() {
	i64 *program1 = parseArray("Day9_test1.txt");
	
	computer A = initComputer(program1);

	compute(&A);

	i64 expectedOutput[] = {109,1,204,-1,1001,100,1,100,1008,100,16,101,1006,101,0,99};
	bool actualEqualsExpected = true;
	for(i32 i = 0; i < buf_len(A.outputQueue); i++) {
		if (A.outputQueue[i] != expectedOutput[i]) {
			actualEqualsExpected = false;
		}
	}
	assert(actualEqualsExpected);

	i64 *program2 = parseArray("Day9_test2.txt");

	computer B = initComputer(program2);

	compute(&B);

	assert(buf_len(B.outputQueue) == 1);
	assert(B.outputQueue[0] == 1219070632396864ll);

	i64 *program3 = parseArray("Day9_test3.txt");

	computer C = initComputer(program3);

	compute(&C);

	assert(buf_len(C.outputQueue) == 1);
	assert(C.outputQueue[0] == 1125899906842624ll);

	printf("test1 passed\n");
}

int main(int argc, char const *argv[]) {
	test1();
	i64 *program = parseArray("Day9_input.txt");

	computer C1 = initComputer(program);
	buf_push(C1.inputQueue, 1);
	compute(&C1);

	printf("Part 1: ");
	printIntBuf(C1.outputQueue);

	computer C2 = initComputer(program);
	buf_push(C2.inputQueue, 2);
	compute(&C2);

	printf("Part 2: ");
	printIntBuf(C2.outputQueue);

	return 0;
}