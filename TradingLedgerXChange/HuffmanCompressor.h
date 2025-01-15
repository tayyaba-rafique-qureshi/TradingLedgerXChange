#pragma once
#ifndef HUFFMAN_COMPRESSOR_H
#define HUFFMAN_COMPRESSOR_H

#include <string>
#include <unordered_map>
#include <queue>
#include <vector>
#include <fstream>
#include <iostream>

// Huffman Node structure
struct HuffmanNode {
    char ch;
    int freq;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(char character, int frequency)
        : ch(character), freq(frequency), left(nullptr), right(nullptr) {
    }
};

// Comparator for priority queue
struct Compare {
    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        return a->freq > b->freq;
    }
};

class HuffmanCompressor {

public:
    // Compress data and return compressed string
    std::string compressData(const std::string& data, std::unordered_map<char, std::string>& huffmanCodes);

    // Decompress data and return original string
    std::string decompressData(const std::string& compressedData, HuffmanNode* root);

    // Save compressed data to file
    void saveCompressedToFile(const std::string& compressedData, const std::string& filePath);

    // Load compressed data from file
    std::string loadCompressedFromFile(const std::string& filePath);

    // Build Huffman tree
    HuffmanNode* buildHuffmanTree(const std::string& data);

    // Generate Huffman codes
    void generateCodes(HuffmanNode* root, const std::string& str, std::unordered_map<char, std::string>& huffmanCodes);

    // Compress an input file and save to an output file
    void compressFile(const std::string& inputFilePath, const std::string& outputFilePath);

    void freeTree(HuffmanNode* root);
};

#endif // HUFFMAN_COMPRESSOR_H
