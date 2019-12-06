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

// note: we have to use indices rather than pointers to refer to nodes,
// because the pointers themselves are not stable, but the indices relative to
// the starting node are. 
typedef struct _node {
	char name[4];
	i32 *children;
	i32 parent;
} node; 

i32 containsName(node *list, const char *name) {
	for (i32 i = 0; i < buf_len(list); i++) {
		if (strcmp(list[i].name, name) == 0) {
			return i;
		}
	}
	return -1; 
}

bool containsIndex(i32 *list, i32 index) {
	for (i32 i = 0; i < buf_len(list); i++) {
		if (list[i] == index) {
			return true;
		}
	}
	return false; 
} 

node createNode(char *name, i32 parent) {
	node new;
	strncpy(new.name, name, 4);
	new.children = NULL;
	new.parent = parent;
	return new;
}

void freeTree(node *tree) {
	// free the arrays of child pointers.
	for (i32 i = 0; i < buf_len(tree); i++) {
		buf_free(tree[i].children);
	}
	// free the tree.
	buf_free(tree);
}

node *addNamesToTree(char *parentName, char*childName, node *currTree) {
	i32 foundParentIndex = containsName(currTree, parentName);
	if (foundParentIndex < 0) {
		foundParentIndex = buf_len(currTree);
		buf_push(currTree, createNode(parentName, -1)); 
	}

	i32 foundChildIndex = containsName(currTree, childName);
	if (foundChildIndex < 0) {
		foundChildIndex = buf_len(currTree);
		buf_push(currTree, createNode(childName, -1));
	}
	node *foundParent = &currTree[foundParentIndex];
	node *foundChild = &currTree[foundChildIndex];

	// at this point we have unique pointers to nodes in the tree and can add the connection.
	// (note that foundParent and foundChild pointers are only guaranteed to be stable for the 
	// scope of this function, so use the indices if we need the information to persist)

	if (foundParentIndex == foundChildIndex) {
		printf("Error! found parent (%s) cannot be its own child (%s) \n", 
				foundParent->name, foundChild->name);
		freeTree(currTree);
		return NULL;
	}

	if (currTree[foundChildIndex].parent >= 0) {
		printf("Error! child %s already has a parent (%s), cannot add %s!\n",
						foundChild->name, foundParent->name, parentName);
		freeTree(currTree);
		return NULL;
	} 
	foundChild->parent = foundParentIndex;
	
	if (containsIndex(foundParent->children, foundChildIndex)) {
		printf("Error! parent %s already has the child %s \n", foundParent->name, childName);
		freeTree(currTree);
		return NULL;
	}
	buf_push(foundParent->children, foundChildIndex);

	return currTree;
}

void parseLine(char *inputString, char *parentName, char *childName) {

	char *start = inputString;
	char *end = inputString;

	char directionChar = 0;

	while (*end != 0) {
		if (isalnum(*end)) {
			end++;
		} else if (*end == ')') {
			strncpy(parentName, start, end-start);
			parentName[3] = 0;

			start = end+1;
			end = start;
		} else if (*end == '\n' || *end == 0) {
			strncpy(childName, start, end-start);
			childName[3] = 0;

			start = end+1;
			end = start;
		} else {
			printf("ERROR in parseLine: unrecognised character %c\n", *end);
			return;
		}
	}
	return;
}

node *parseInput(const char *filePath) {
	FILE* filePointer = fopen(filePath, "r");
	if (filePointer == NULL) {
		printf("Unable to read file!\n");
		return NULL;
	}
	char *inputString = NULL;
	size_t len = 0;
	ssize_t read;

	node *tree = NULL;
	while ((read = getline(&inputString, &len, filePointer)) != -1) {
		char parentName[4]; 
		char childName[4];

		parseLine(inputString, parentName, childName);
		tree = addNamesToTree(parentName, childName, tree);
	}

	fclose(filePointer);
	free(inputString);

	return tree;
}

int findTopNodeIndex(node *tree) {
	for (i32 i = 0; i < buf_len(tree); i++) {
		if (tree[i].parent == -1) {
			return i;
		}
	}
	return -1;
}

int countTreeScore(node *tree, node *currNode, i32 depth) {
	if (!currNode->children) {
		return depth;
	} else {
		i32 sumOfChildren = 0;
		for (i32 i = 0; i < buf_len(currNode->children); i++) {
			node *currChild = &tree[currNode->children[i]];
			sumOfChildren += countTreeScore(tree, currChild, depth+1);
		}
		return depth + sumOfChildren;
	}
}

int breadthFirstSearch(node *tree, i32 startIndex, i32 goalIndex) {
	i32 *queue = NULL;
	i32 *distanceFromStart = NULL;
	i32 *visited = NULL;

	buf_push(queue, startIndex);
	buf_push(distanceFromStart, 0);

	for (int i = 0; i < buf_len(queue); i++) {
		if (queue[i] == goalIndex) {
			return distanceFromStart[i];
		}
		
		for (int j = 0; j < buf_len(tree[queue[i]].children); j++) {
			if (!containsIndex(queue, tree[queue[i]].children[j])) {
				buf_push(queue, tree[queue[i]].children[j]);
				buf_push(distanceFromStart, distanceFromStart[i]+1);
			}
		}
		if (!containsIndex(queue, tree[queue[i]].parent) && tree[queue[i]].parent >= 0) {
			buf_push(queue, tree[queue[i]].parent);
			buf_push(distanceFromStart, distanceFromStart[i]+1);
		}
	}
	return -1;
}

int main(int argc, char const *argv[]) {
	
	node *tree = parseInput("Day6_input.txt");
	// for (i32 i = 0; i < buf_len(tree); i++) {
	// 	printf("node: %s | children: ", tree[i].name);
	// 	i32 j;
	// 	for (j = 0; j < buf_len(tree[i].children); j++) {
	// 		i32 cj = tree[i].children[j];
	// 		printf("%s,", tree[cj].name);
	// 	}
	// 	printf("\n");
	// }

	i32 topIndex = findTopNodeIndex(tree);
	i32 part1 = countTreeScore(tree, &tree[topIndex], 0);
	printf("part1 = %d\n", part1);

	i32 youIndex = containsName(tree, "YOU");
	i32 startIndex = tree[youIndex].parent;
	i32 sanIndex = containsName(tree, "SAN");
	i32 goalIndex = tree[sanIndex].parent;

	i32 part2 = breadthFirstSearch(tree, startIndex, goalIndex);
	printf("part2 = %d\n", part2);

	return 0;
}
