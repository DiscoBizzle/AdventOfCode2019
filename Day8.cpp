// AdventOfCode.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

int main()
{
	std::fstream fs("Day8_input.txt", std::fstream::in);

	std::string inputString;
	fs >> inputString;

	int layerw = 25;
	int layerh = 6;
	int layerSize = layerw * layerh;

	int numLayers = inputString.length() / layerSize;

	std::vector<std::string> layers;

	for (int layerIndex = 0; layerIndex < numLayers; layerIndex++) {
		layers.push_back(inputString.substr(layerIndex*layerSize, layerSize));
	}

	std::vector<int> numZeros;
	std::vector<int> numOnes;
	std::vector<int> numTwos;
	for (int layerIndex = 0; layerIndex < numLayers; layerIndex++) {
		std::cout << layers[layerIndex] << std::endl;
		numZeros.push_back(0);
		numOnes.push_back(0);
		numTwos.push_back(0);
		for (auto c : layers[layerIndex]) {
			switch (c) {
			case '0': {
				numZeros[layerIndex]++;
			} break;
			case '1': {
				numOnes[layerIndex]++;
			} break;
			case '2': {
				numTwos[layerIndex]++;
			} break;
			}
		}
		if (numZeros[layerIndex] + numOnes[layerIndex] + numTwos[layerIndex] != layerSize) {
			std::cout << "Error! total number of pixel values does not equal the layer size!" << std::endl;
			return -1;
		}
	}

	std::cout << "numzeros[0]" << numZeros[0] << std::endl;
	std::cout << "numOnes[0]" << numOnes[0] << std::endl;
	std::cout << "numTwos[0]" << numTwos[0] << std::endl;

	int minZeroIndex = std::min_element(numZeros.begin(), numZeros.end()) - numZeros.begin();

	std::cout << "Part 1: " << numOnes[minZeroIndex] * numTwos[minZeroIndex] << std::endl;

	std::string message = layers[0];
	for (int pixelIndex = 0; pixelIndex < layerSize; pixelIndex++) {
		for (auto currLayer : layers) {
			if (currLayer[pixelIndex] == '0') {
				message[pixelIndex] = ' ';
				break;
			} 
			else if (currLayer[pixelIndex] == '1') {
				message[pixelIndex] = 'X';
				break;
			}
			else if (currLayer[pixelIndex] == '2') {
				continue;
			}
		}
	}

	std::cout << "Part 2: " << std::endl;
	for (int rowIndex = 0; rowIndex < layerh; rowIndex++) {
		std::cout << message.substr(rowIndex*layerw, layerw) << std::endl;
	}
}
