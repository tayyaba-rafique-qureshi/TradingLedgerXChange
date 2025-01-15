#include "OrderTree.h"

// Destructor to clean up tree and linked list
OrderTree::~OrderTree() {
    clearTree(root);
}

// Recursive function to clear the BST
void OrderTree::clearTree(OrderNode* node) {
    if (node) {
        clearTree(node->left);
        clearTree(node->right);
        delete node;
    }
}

// Function to clear the linked list
void OrderTree::clearList(ListNode* head) {
    while (head) {
        ListNode* temp = head;
        head = head->next;
        delete temp;
    }
}

// Insert a new order into the BST
void OrderTree::insert(OrderNode*& node, int id, double price, const std::string& date, const std::string& type) {
    if (!node) {
        node = new OrderNode(id, price, date, type);
    }
    else if (price <= node->price) {
        insert(node->left, id, price, date, type);
    }
    else {
        insert(node->right, id, price, date, type);
    }
}

// Public method to insert an order
void OrderTree::insertOrder(int id, double price, const std::string& date, const std::string& type) {
    insert(root, id, price, date, type);
}

// Recursive function to filter orders and build a linked list
void OrderTree::filterOrders(OrderNode* node, double threshold, bool greater, ListNode*& head, ListNode*& tail) const {
    if (!node) return;

    if ((greater && node->price > threshold) || (!greater && node->price < threshold)) {
        ListNode* newNode = new ListNode(node->orderId, node->price, node->date, node->type);
        if (!head) {
            head = tail = newNode;
        }
        else {
            tail->next = newNode;
            tail = newNode;
        }
    }

    filterOrders(node->left, threshold, greater, head, tail);
    filterOrders(node->right, threshold, greater, head, tail);
}

// Public method to filter orders
ListNode* OrderTree::filter(double threshold, bool greater) const {
    ListNode* head = nullptr;
    ListNode* tail = nullptr;
    filterOrders(root, threshold, greater, head, tail);
    return head;
}

// Utility function to print the linked list
void OrderTree::printList(ListNode* head) const {
    ListNode* current = head;
    while (current) {
        std::cout << "Order ID: " << current->orderId
            << ", Price: $" << current->price
            << ", Date: " << current->date
            << ", Type: " << current->type << std::endl;
        current = current->next;
    }
}
