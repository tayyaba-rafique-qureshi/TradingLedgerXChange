#include "LinkedList.h"

// Constructor
PriorityQueue::PriorityQueue() : head(nullptr), stackTop(nullptr) {}

// Destructor
PriorityQueue::~PriorityQueue() {
    // Free the linked list
    Node* current = head;
    while (current != nullptr) {
        Node* temp = current;
        current = current->next;
        delete temp;
    }

    // Free the stack
    while (!isStackEmpty()) {
        pop();
    }
}

// Insert asset into the priority queue sorted by price (descending)
void PriorityQueue::insert(const Asset& asset) {
    Node* newNode = new Node{ asset, nullptr };

    // Insert at the head if the list is empty or the new price is higher
    if (!head || asset.price > head->data.price) {
        newNode->next = head;
        head = newNode;
    }
    else {
        // Traverse to find the correct position
        Node* current = head;
        while (current->next && current->next->data.price >= asset.price) {
            current = current->next;
        }
        newNode->next = current->next;
        current->next = newNode;
    }
}

// Display priority queue in descending order (Top Gainers)
void PriorityQueue::display() {
    Node* current = head;
    int count = 0; // Counter to limit to top 5 gainers
    std::cout << "Priority Queue (Top Gainers):\n";
    std::cout << "--------------------------------\n";
    while (current != nullptr && count < 5) { // Stop after 5 entries
        std::cout << "Asset ID: " << current->data.assetId
            << ", Asset Name: " << current->data.assetName
            << ", Price: $" << current->data.price
            << ", Date: " << current->data.date << "\n";
        current = current->next;
        count++; // Increment counter
    }
    std::cout << "--------------------------------\n";
}

// Helper function for recursive reverse display with counter
void PriorityQueue::displayReverseHelper(Node* node, int& count) {
    if (!node || count >= 5) return; // Stop recursion after 5 entries
    displayReverseHelper(node->next, count);
    if (count < 5) { // Only display if counter is within limit
        std::cout << "Asset ID: " << node->data.assetId
            << ", Asset Name: " << node->data.assetName
            << ", Price: $" << node->data.price
            << ", Date: " << node->data.date << "\n";
        count++; // Increment counter
    }
}

// Display priority queue in ascending order (Top Losers)
void PriorityQueue::displayReverse() {
    int count = 0; // Counter to limit to top 5 losers
    std::cout << "Priority Queue (Top Losers):\n";
    std::cout << "--------------------------------\n";
    displayReverseHelper(head, count);
    std::cout << "--------------------------------\n";
}


// Push an asset into the stack
void PriorityQueue::push(const Asset& asset) {
    StackNode* newNode = new StackNode{ asset, stackTop };
    stackTop = newNode;
}

// Pop the top asset from the stack
void PriorityQueue::pop() {
    if (isStackEmpty()) {
        std::cerr << "Stack is empty. Cannot pop.\n";
        return;
    }
    StackNode* temp = stackTop;
    stackTop = stackTop->next;
    delete temp;
}

// Check if the stack is empty
bool PriorityQueue::isStackEmpty() const {
    return stackTop == nullptr;
}

// Display all assets in the stack
void PriorityQueue::displayStack() const {
    if (isStackEmpty()) {
        std::cout << "Order history is empty.\n";
        return;
    }
    StackNode* current = stackTop;
    std::cout << "Order History:\n";
    std::cout << "----------------------------------------\n";
    while (current != nullptr) {
        std::cout << "Asset ID: " << current->data.assetId
            << ", Asset Name: " << current->data.assetName
            << ", Price: $" << current->data.price
            << ", Date: " << current->data.date << "\n";
        current = current->next;
    }
    std::cout << "----------------------------------------\n";
}
