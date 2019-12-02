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
		if (isdigit(*end)) {
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

i32 *compute(i32 *input_copy) {
	i32 *memory = NULL;
	for (int i = 0; i < buf_len(input_copy); i++) {
		buf_push(memory, input_copy[i]);
	}
	i32 cursor = 0;
	bool running = true;
	while (running) {
		switch(memory[cursor]) {
			case 1: {
				i32 read1 = memory[cursor+1];
				i32 read2 = memory[cursor+2];
				i32 write = memory[cursor+3];

				memory[write] = memory[read1] + memory[read2];
				cursor += 4;
			} break;

			case 2: {
				i32 read1 = memory[cursor+1];
				i32 read2 = memory[cursor+2];
				i32 write = memory[cursor+3];

				memory[write] = memory[read1] * memory[read2];
				cursor += 4;
			} break;
			case 99: {
				running = false;
			} break;
			default: {
				printf("error! expected opcode 1, 2 or 99, got %d\n", memory[cursor]);
				return NULL;
			}
		}
	}
	return memory;
}

bool test_computer(i32 *input, i32 *expectedOutput) {
	i32 *actualOutput = compute(input);
	if (!actualOutput) {
		return false; 
	}

	for (i32 i = 0; i < buf_len(input); i++) {
		printf("input[%d] = %d, expected_output[%d] = %d\n", i, input[i], i, expectedOutput[i]);
		if (actualOutput[i] != expectedOutput[i]) {
			return false;
		}
	}

	return true;
}

bool test() {
	i32 *input = NULL;	//1,0,0,0,99
	buf_push(input, 1);
	buf_push(input, 0);
	buf_push(input, 0);
	buf_push(input, 0);
	buf_push(input, 99);
	int output[] = {2,0,0,0,99};

	bool success = true;
	success = test_computer(input, output);
	printf("test1 = %d\n", success);
	return success;
}

void part1() {
	i32 *input = parseArray("Day2_input.txt");
	input[1] = 12;
	input[2] = 2;
    i32 *output = compute(input);

    printf("value at position 0 is %d\n", output[0]);
    buf_free(input);
    buf_free(output);
}

void part2() {
	i32 goalOutput = 19690720;
	i32 *input = parseArray("Day2_input.txt"); 
	for (i32 noun = 0; noun <= 99; noun++) {
		for (i32 verb = 0; verb <= 99; verb++) {
			input[1] = noun;
			input[2] = verb;
			i32 *output = compute(input);
			if (output[0] == goalOutput) {
				printf("goal reached, ans = %d\n", 100*noun+verb);
				return;
			}
		}
	}
}

i32 predictor(i32 noun, i32 verb) {
	return (243000*noun + verb + 250702);
}

void part11() {
	printf("result = %d\n", predictor(12, 2));
}

void part22() {
	i32 goal = 19690720;
	// 243000*noun + verb = 19690720 - 250702;
	i32 verb = (19690720 - 250702)%243000;
	i32 noun = (19690720 - 250702)/243000;
	printf("ans = %d\n", 100*noun+verb);
}

int main(int argc, char const *argv[]) {

	test();
	
	part1();
	part2();

	part11();
	part22();
	return 0;
}