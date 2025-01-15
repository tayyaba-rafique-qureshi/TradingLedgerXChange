#pragma once
#pragma once
#include <iostream>
#include <string>

// Node structure for the BST
struct OrderNode {
    int orderId;
    double price;
    std::string date;
    std::string type;
    OrderNode* left;
    OrderNode* right;

    OrderNode(int id, double p, const std::string& d, const std::string& t)
        : orderId(id), price(p), date(d), type(t), left(nullptr), right(nullptr) {
    }
};

// Node for the linked list
struct ListNode {
    int orderId;
    double price;
    std::string date;
    std::string type;
    ListNode* next;

    ListNode(int id, double p, const std::string& d, const std::string& t)
        : orderId(id), price(p), date(d), type(t), next(nullptr) {
    }
};

// BST class
class OrderTree {
private:
    OrderNode* root;

    void insert(OrderNode*& node, int id, double price, const std::string& date, const std::string& type);
    void filterOrders(OrderNode* node, double threshold, bool greater, ListNode*& head, ListNode*& tail) const;

    void clearTree(OrderNode* node);
    void clearList(ListNode* head);

public:
    OrderTree() : root(nullptr) {}
    ~OrderTree();

    void insertOrder(int id, double price, const std::string& date, const std::string& type);
    ListNode* filter(double threshold, bool greater = true) const;
    void printList(ListNode* head) const;
};
