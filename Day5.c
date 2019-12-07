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

i32 *compute(i32 *memory_in_copy, i32 input, i32 *output) {
	i32 *memory = NULL;
	for (int i = 0; i < buf_len(memory_in_copy); i++) {
		buf_push(memory, memory_in_copy[i]);
	}

	i32 cursor = 0;
	bool running = true;
	while (running) {
		i32 opcode = memory[cursor] % 100;
		i32 mode1 = (memory[cursor] / 100)%10;
		i32 mode2 = (memory[cursor] / 1000)%10;
		i32 mode3 = (memory[cursor] / 10000)%10;
		// print_memory(memory,0,buf_len(memory)+1);
		// printf("memory[%d] = %d, opcode = %d, modes = {%d, %d, %d}\n", cursor, memory[cursor], opcode, mode1, mode2, mode3);
		switch(opcode) {
			case 1: { // add
				if (mode3 == IMMEDIATE_MODE) {
					printf("Error! tried to write in immediate mode\n");
					return NULL;
				}
				i32 read1 = memory[cursor+1];
				i32 read2 = memory[cursor+2];
				i32 write = memory[cursor+3];

				memory[write] = get_value(read1, memory, mode1) + get_value(read2, memory, mode2);
				cursor += 4;
			} break;

			case 2: { // multiply
				if (mode3 == IMMEDIATE_MODE) {
					printf("Error! tried to write in immediate mode\n");
					return NULL;
				}
				i32 read1 = memory[cursor+1];
				i32 read2 = memory[cursor+2];
				i32 write = memory[cursor+3];

				memory[write] = get_value(read1, memory, mode1) * get_value(read2, memory, mode2);
				cursor += 4;
			} break;
			case 3: { // input
				if (mode1 == IMMEDIATE_MODE) {
					printf("Error! tried to write in immediate mode\n");
					return NULL;
				}
				i32 write = memory[cursor+1];
				memory[write] = input;
				cursor += 2;
			} break;
			case 4: { // output
				i32 read = memory[cursor+1];
				*output = get_value(read, memory, mode1);
				printf("output = %d\n", *output);
				cursor +=2;
			} break;
			case 5: { // jump-if-true
				i32 read = memory[cursor+1];
				i32 jump = memory[cursor+2];
				if (get_value(read, memory, mode1) != 0) {
					cursor = get_value(jump, memory, mode2);
				} else {
					cursor += 3; 
				}
			} break; 
			case 6: { // jump if false 
				i32 read = memory[cursor+1];
				i32 jump = memory[cursor+2];
				if (get_value(read, memory, mode1) == 0) {
					cursor = get_value(jump, memory, mode2);
				} else {
					cursor += 3; 
				}
			} break;	
			case 7: { // less than
				i32 read1 = memory[cursor+1];
				i32 read2 = memory[cursor+2];
				i32 write = memory[cursor+3];
				if (get_value(read1, memory, mode1) < get_value(read2, memory, mode2)) {
					memory[write] = 1;
				} else {
					memory[write] = 0;
				}
				cursor += 4;
			} break;
			case 8: { // equals
				i32 read1 = memory[cursor+1];
				i32 read2 = memory[cursor+2];
				i32 write = memory[cursor+3];
				if (get_value(read1, memory, mode1) == get_value(read2, memory, mode2)) {
					memory[write] = 1;
				} else {
					memory[write] = 0;
				}
				cursor += 4;
			} break;
			case 99: {
				running = false;
			} break;
			default: {
				printf("error! expected opcode 1, 2, 3, 4 or 99, got %d\n", opcode);
				return NULL;
			}
		}
	}
	return memory;
}

bool test_computer(i32 *memory, i32 *expectedOutput) {
	i32 input = 0; 
	i32 output = 0;
	i32 *actualOutput = compute(memory, input, &output);
	if (!actualOutput) {
		return false; 
	}

	for (i32 i = 0; i < buf_len(memory); i++) {
		printf("input[%d] = %d, expected_output[%d] = %d\n", i, memory[i], i, expectedOutput[i]);
		if (actualOutput[i] != expectedOutput[i]) {
			return false;
		}
	}

	return true;
}

void test() {
	i32 *memory_in = parseArray("Day5_test1.txt");
	i32 input = 5;
	i32 output = 0;
	i32 *memory_out = compute(memory_in, input, &output);

	i32 *memory_in2 = parseArray("Day5_test2.txt");
	i32 input2 = 5;
	i32 output2 = 0;
	i32 *memory_out2 = compute(memory_in, input, &output);
}

int main(int argc, char const *argv[]) {
	i32 *memory_in = parseArray("Day5_input.txt");
	i32 output = 0;
	// part 1
	i32 *memory_out = compute(memory_in, 1, &output);
	buf_free(memory_out);

	// part 2
	memory_out = compute(memory_in, 5, &output);
	buf_free(memory_out);
	buf_free(memory_in);
	return 0;
}