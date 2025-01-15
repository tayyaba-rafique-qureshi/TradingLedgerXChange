#pragma once
#include <iostream>
#include <string>



class Asset {
public:
    int assetId;
    std::string assetName;
    double price;
    std::string date; // Add a date field

    // Constructor to initialize Asset with all fields
    Asset(int id, const std::string& name, double price, const std::string& date)
        : assetId(id), assetName(name), price(price), date(date) {
    }

    // Default constructor
    Asset() : assetId(0), assetName(""), price(0.0), date("") {}
};

class PriorityQueue {
private:
    struct Node {
        Asset data;
        Node* next;
    };
    Node* head;

    struct StackNode {
        Asset data;
        StackNode* next;
    };
    StackNode* stackTop; // Top of the stack

    void displayReverseHelper(Node* node, int& count); // Helper function for recursive reverse display

public:
    PriorityQueue();  // Constructor
    ~PriorityQueue(); // Destructor

    void insert(const Asset& asset);    // Insert asset into the priority queue
    void display();                     // Display priority queue from high to low
    void displayReverse();              // Display priority queue from low to high

    void push(const Asset& asset);      // Push order history into the stack
    void pop();                         // Remove top order history from the stack
    bool isStackEmpty() const;          // Check if the stack is empty
    void displayStack() const;          // Display all order history from the stack
};
