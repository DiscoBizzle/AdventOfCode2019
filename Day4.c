#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

i32 getNumDigits(i32 n) {
	i32 numDigits = 0;
	while (n > 0) {
		numDigits++;
		n /= 10; 
	}
	return numDigits;
}

bool hasAtLeastOneExactPair(i32 n) {
	i32 numDigits = getNumDigits(n);
	i32 currMod = 10;
	i32 currDiv = 1;
	i32 currDigit = -1;
	i32 prevDigit = -1;
	i32 runLength = 1;
	for (i32 i=0; i<numDigits; i++) {
		prevDigit = currDigit;
		currDigit = (n%currMod)/currDiv;
		// printf("currDigit = %d, prevDigit = %d\n", currDigit, prevDigit);
		if (prevDigit == currDigit) {
			runLength++;
		} else {
			if (runLength == 2) {
				// printf("%d has pair\n", n);
				return true; 
			}
			runLength = 1; 
		}
		currMod *= 10;
		currDiv *= 10;
	}
	// if the last two digits are a pair
	return runLength == 2;
}

bool hasIdenticalAdjacentDigits(i32 n) {
	i32 numDigits = getNumDigits(n);
	
	i32 currMod = 10;
	i32 currDiv = 1;
	i32 currDigit = -1;
	i32 prevDigit = -1;

	for (i32 i=0; i<numDigits; i++) {
		prevDigit = currDigit;
		currDigit = (n%currMod)/currDiv;
		// printf("currDigit = %d, prevDigit = %d\n", currDigit, prevDigit);
		if (prevDigit == currDigit) {
			return true;
		}
		if (prevDigit == currDigit) {
			return false;
		}
		currMod *= 10;
		currDiv *= 10;
	}

	return false;
}

bool isAscending(i32 n) {
	i32 numDigits = getNumDigits(n);
	
	i32 currMod = 10;
	i32 currDiv = 1;
	i32 currDigit = 100;
	i32 prevDigit = 100;

	for (i32 i=0; i<numDigits; i++) {
		prevDigit = currDigit;
		currDigit = (n%currMod)/currDiv;
		// printf("currDigit = %d, prevDigit = %d\n", currDigit, prevDigit);
		if (prevDigit < currDigit) {
			return false;
		}
		currMod *= 10;
		currDiv *= 10;
	}
	return true;
}

int main(int argc, char const *argv[]) {
	i32 low = 246540;
	i32 high = 787419;
	
	i32 part1 = 0;
	i32 part2 = 0;
	for (i32 n = low; n <= high; n++) {
		if (hasIdenticalAdjacentDigits(n) && isAscending(n)) {
			printf("%d ", n);
			part1++;
			if (hasAtLeastOneExactPair(n)) {
				printf("X");
				part2++;
			}
			printf("\n");
		}
	}
	printf("numPossiblePasswords = %d\n", part1);
	printf("Part 2 = %d\n", part2);
	return 0;
}