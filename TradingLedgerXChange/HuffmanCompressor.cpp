#include "HuffmanCompressor.h"

std::string HuffmanCompressor::compressData(const std::string& data, std::unordered_map<char, std::string>& huffmanCodes) {
    // Build Huffman tree
    HuffmanNode* root = buildHuffmanTree(data);

    // Generate codes
    generateCodes(root, "", huffmanCodes);

    // Encode the data
    std::string compressedData;
    for (char ch : data) {
        compressedData += huffmanCodes[ch];
    }

    // Free tree memory
    freeTree(root);

    return compressedData;
}
void HuffmanCompressor::freeTree(HuffmanNode* root) {
    if (!root) return; // Base case: if the node is null, return

    // Recursively free the left and right subtrees
    freeTree(root->left);
    freeTree(root->right);

    // Delete the current node
    delete root;
}


std::string HuffmanCompressor::decompressData(const std::string& compressedData, HuffmanNode* root) {
    std::string decompressedData;
    HuffmanNode* current = root;
    for (char bit : compressedData) {
        current = (bit == '0') ? current->left : current->right;

        if (!current->left && !current->right) { // Leaf node
            decompressedData += current->ch;
            current = root;
        }
    }
    return decompressedData;
}

void HuffmanCompressor::saveCompressedToFile(const std::string& compressedData, const std::string& filePath) {
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Unable to open file for writing.");
    }
    outFile << compressedData;
    outFile.close();
}

std::string HuffmanCompressor::loadCompressedFromFile(const std::string& filePath) {
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) {
        throw std::runtime_error("Unable to open file for reading.");
    }
    std::string compressedData((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();
    return compressedData;
}

HuffmanNode* HuffmanCompressor::buildHuffmanTree(const std::string& data) {
    // Frequency map
    std::unordered_map<char, int> freqMap;
    for (char ch : data) {
        freqMap[ch]++;
    }

    // Priority queue
    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, Compare> pq;
    for (auto& pair : freqMap) {
        pq.push(new HuffmanNode(pair.first, pair.second));
    }

    // Build tree
    while (pq.size() > 1) {
        HuffmanNode* left = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();
        HuffmanNode* newNode = new HuffmanNode('\0', left->freq + right->freq);
        newNode->left = left;
        newNode->right = right;
        pq.push(newNode);
    }
    return pq.top();
}

void HuffmanCompressor::generateCodes(HuffmanNode* root, const std::string& str, std::unordered_map<char, std::string>& huffmanCodes) {
    if (!root) return;

    if (!root->left && !root->right) { // Leaf node
        huffmanCodes[root->ch] = str;
    }

    generateCodes(root->left, str + "0", huffmanCodes);
    generateCodes(root->right, str + "1", huffmanCodes);
}

#include "HuffmanCompressor.h"
#include <fstream>
#include <stdexcept>

void HuffmanCompressor::compressFile(const std::string& inputFilePath, const std::string& outputFilePath) {
    // Read the input file
    std::ifstream inputFile(inputFilePath, std::ios::binary);
    if (!inputFile) {
        throw std::runtime_error("Unable to open input file for compression.");
    }
    std::string data((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    inputFile.close();

    // Generate Huffman codes
    std::unordered_map<char, std::string> huffmanCodes;
    std::string compressedData = compressData(data, huffmanCodes);

    // Save the compressed data to the output file
    saveCompressedToFile(compressedData, outputFilePath);
}
