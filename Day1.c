#include <stdio.h>
#include <stdlib.h>

typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

i32 calculateFuel(i32 input) {
	i32 nextFuel = input/3 - 2;
	if (nextFuel <= 0) {
		return 0;
	} else {
		return nextFuel + calculateFuel(nextFuel);
	}
}

int part12() {
	FILE* filePointer = fopen("Day1_input.txt", "r");
	if (filePointer == NULL) {
		printf("Unable to read file!\n");
		return 1;
	}
	char *massString = NULL;
	size_t len = 0;
	ssize_t read;

	u32 basicFuel = 0;
	u32 totalFuel = 0;
	while ((read = getline(&massString, &len, filePointer)) != -1) {
		u32 mass = atoi(massString);
		basicFuel += mass/3 - 2;
        totalFuel += calculateFuel(mass);
    }
    printf("Part 1: Total fuel = %u\n", basicFuel);
    printf("Part 2: Total fuel = %u\n", totalFuel);
    fclose(filePointer);
    return 0;
}

int main(int argc, char const *argv[]) {
	part12();
	return 0;
}