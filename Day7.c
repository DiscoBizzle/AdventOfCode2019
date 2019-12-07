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

i32 *parseArray(const char *filePath) {
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

	i32 *array = NULL;

	while (*end != 0) {
		if (isdigit(*end) || *end == '-') {
			end++;
		} else {
			strncpy(temp, start, end-start);
			temp[end-start] = 0;
			buf_push(array, atoi(temp));

			start = end+1;
			end = start;
		}
	}
	free(inputString);
	return array;
}

#define POSITION_MODE 0
#define IMMEDIATE_MODE 1
i32 get_value(i32 value, i32 *memory, i32 mode) {
	if (mode == POSITION_MODE) {
		return memory[value];
	} else if (mode == IMMEDIATE_MODE) {
		return value;
	} else {
		printf("Error! unrecognized mode %d\n", mode);
		return ~0;
	}

}

void print_memory(i32 *memory, i32 min_index, i32 max_index) {
	for (i32 i = min_index; i <= max_index; i++) {
		printf("%d] \t %d \n", i, memory[i]);
	}
}

typedef struct {
	i32 *memory;
	i32 cursor;
	bool running;
	i32 *inputQueue;
	i32 inputCursor;
	i32 *outputQueue;
} computer;

void compute(computer *c) {

	while (c->running) {
		i32 opcode = c->memory[c->cursor] % 100;
		i32 mode1 = (c->memory[c->cursor] / 100)%10;
		i32 mode2 = (c->memory[c->cursor] / 1000)%10;
		i32 mode3 = (c->memory[c->cursor] / 10000)%10;
		// print_memory(memory,0,buf_len(memory)+1);
		// printf("memory[%d] = %d, opcode = %d, modes = {%d, %d, %d}\n", cursor, memory[cursor], opcode, mode1, mode2, mode3);
		switch(opcode) {
			case 1: { // add
				if (mode3 == IMMEDIATE_MODE) {
					printf("Error! tried to write in immediate mode\n");
					return;
				}
				i32 read1 = c->memory[c->cursor+1];
				i32 read2 = c->memory[c->cursor+2];
				i32 write = c->memory[c->cursor+3];

				c->memory[write] = get_value(read1, c->memory, mode1) + get_value(read2, c->memory, mode2);
				c->cursor += 4;
			} break;

			case 2: { // multiply
				if (mode3 == IMMEDIATE_MODE) {
					printf("Error! tried to write in immediate mode\n");
					return;
				}
				i32 read1 = c->memory[c->cursor+1];
				i32 read2 = c->memory[c->cursor+2];
				i32 write = c->memory[c->cursor+3];

				c->memory[write] = get_value(read1, c->memory, mode1) * get_value(read2, c->memory, mode2);
				c->cursor += 4;
			} break;
			case 3: { // input
				if (mode1 == IMMEDIATE_MODE) {
					printf("Error! tried to write in immediate mode\n");
					return;
				}
				i32 write = c->memory[c->cursor+1];
				c->memory[write] = c->inputQueue[c->inputCursor++];
				c->cursor += 2;
			} break;
			case 4: { // output
				i32 read = c->memory[c->cursor+1];
				i32 output = get_value(read, c->memory, mode1);
				buf_push(c->outputQueue, output);
				// printf("output = %d\n", output);
				c->cursor +=2;
			} break;
			case 5: { // jump-if-true
				i32 read = c->memory[c->cursor+1];
				i32 jump = c->memory[c->cursor+2];
				if (get_value(read, c->memory, mode1) != 0) {
					c->cursor = get_value(jump, c->memory, mode2);
				} else {
					c->cursor += 3; 
				}
			} break; 
			case 6: { // jump if false 
				i32 read = c->memory[c->cursor+1];
				i32 jump = c->memory[c->cursor+2];
				if (get_value(read, c->memory, mode1) == 0) {
					c->cursor = get_value(jump, c->memory, mode2);
				} else {
					c->cursor += 3; 
				}
			} break;	
			case 7: { // less than
				i32 read1 = c->memory[c->cursor+1];
				i32 read2 = c->memory[c->cursor+2];
				i32 write = c->memory[c->cursor+3];
				if (get_value(read1, c->memory, mode1) < get_value(read2, c->memory, mode2)) {
					c->memory[write] = 1;
				} else {
					c->memory[write] = 0;
				}
				c->cursor += 4;
			} break;
			case 8: { // equals
				i32 read1 = c->memory[c->cursor+1];
				i32 read2 = c->memory[c->cursor+2];
				i32 write = c->memory[c->cursor+3];
				if (get_value(read1, c->memory, mode1) == get_value(read2, c->memory, mode2)) {
					c->memory[write] = 1;
				} else {
					c->memory[write] = 0;
				}
				c->cursor += 4;
			} break;
			case 99: {
				c->running = false;
			} break;
			default: {
				printf("error! expected opcode 1, 2, 3, 4 or 99, got %d\n", opcode);
				return;
			}
		}
	}
	return;
}

i32 *buf_copy(i32 *input) {
	i32 *copy = NULL;
	for (i32 i = 0; i < buf_len(input); i++) {
		buf_push(copy, input[i]);
	}
	return copy;
}

bool 

i32 runAmplifierCircuit(i32 *program, i32 *phaseSettings, i32 numAmps) {
	i32 currInput = 0;
	for (i32 i = 0; i < numAmps; i++) {
		computer A = {};
		A.memory = buf_copy(program);
		buf_push(A.inputQueue, phaseSettings[i]);
		buf_push(A.inputQueue, currInput);
		A.running = true;

		compute(&A);

		currInput = A.outputQueue[0];
		buf_free(A.memory);
		buf_free(A.inputQueue);
		buf_free(A.outputQueue);
	}

	i32 output = currInput;
	return output;
} 

void swap(i32 *a, i32 *b) {
	i32 temp = *a;
	*a = *b;
	*b = temp;
}

bool nextPermutation(i32 *array, i32 *length, i32 *c, i32 *i) {
	bool newPerm = false;
	if (c[*i] < *i) {
		if (*i%2 == 0) {
			swap(&array[0], &array[*i]);
		} else {
			swap(&array[c[*i]], &array[*i]);
		}
		newPerm = true;
		c[*i]++;
		*i = 0;
	} else {
		c[*i] = 0;
		*i = *i + 1;
	}
	return newPerm;
}

void printPermuations(i32 *array) {
	i32 length = 5;
	i32 c[] = {0,0,0,0,0};
	i32 i = 0;
	i32 numPermutations = 0;
	while (i < length) {
		if (nextPermutation(array, &length, c, &i)) {
			numPermutations++;
			for (i32 j = 0; j < length; j++) {
				printf("%d,", array[j]);
			}
			printf("\n");
		}
	}
	printf("num permutations found: %d\n", numPermutations);
}

i32 optimisePhaseSettings(i32 *program) {
	i32 numAmps = 5;
	i32 phaseSettings[] = {0,1,2,3,4};
	i32 maxOutput = runAmplifierCircuit(program, phaseSettings, numAmps);
		
	i32 c[] = {0,0,0,0,0};
	i32 i = 0;
	while (i < numAmps) {
		if (nextPermutation(phaseSettings, &numAmps, c, &i)) {
			i32 nextOutput = runAmplifierCircuit(program, phaseSettings, numAmps);
			if (nextOutput > maxOutput) {
				maxOutput = nextOutput;
			}
		}
	}
	
	return maxOutput;
}

void test() {
	i32 *program1 = parseArray("Day7_test1.txt");
	i32 phaseSettings[] = {4,3,2,1,0};
	runAmplifierCircuit(program1, phaseSettings, 5);

	i32 maxOutput1 = optimisePhaseSettings(program1);
	printf("max output for program 1: %d\n", maxOutput1);

	i32 *program2 = parseArray("Day7_test2.txt");
	i32 phaseSettings2[] = {0,1,2,3,4};
	runAmplifierCircuit(program2, phaseSettings2, 5);

	i32 maxOutput2 = optimisePhaseSettings(program2);
	printf("max output for program 2: %d\n", maxOutput2);

	i32 *program3 = parseArray("Day7_test3.txt");
	i32 phaseSettings3[] = {1,0,4,3,2};
	runAmplifierCircuit(program3, phaseSettings3, 5);

	i32 maxOutput3 = optimisePhaseSettings(program3);
	printf("max output for program 2: %d\n", maxOutput3);
}

int main(int argc, char const *argv[]) {
	test();

	i32 *program = parseArray("Day7_input.txt");
	i32 maxOutput = optimisePhaseSettings(program);
	printf("Part 1: %d\n", maxOutput);

	return 0;
}