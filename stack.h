#ifndef STACK_H
#define STACK_H

#include <iostream>
#include <cstring>
using namespace std;

template <typename T>
struct StackNode {
    T data;
    StackNode* next;
    StackNode(T val) : data(val), next(nullptr) {}
};

template <typename T>
class Stack {
private:
    StackNode<T>* top;
    int count;

public:
    Stack() : top(nullptr), count(0) {}

    void push(T val) {
        StackNode<T>* newNode = new StackNode<T>(val);
        newNode->next = top;
        top = newNode;
        count++;
    }

    T pop() {
        if (top == nullptr) {
            cout << "Stack is empty, cannot pop.\n";
            T empty;
            return empty;
        }
        T val = top->data;
        StackNode<T>* temp = top;
        top = top->next;
        delete temp;
        count--;
        return val;
    }

    T peek() {
        if (top == nullptr) {
            T empty;
            return empty;
        }
        return top->data;
    }

    bool isEmpty() {
        return top == nullptr;
    }

    int size() {
        return count;
    }

    void printAll() {
        StackNode<T>* curr = top;
        int i = 1;
        while (curr != nullptr) {
            cout << "  " << i++ << ". " << curr->data << "\n";
            curr = curr->next;
        }
    }

    ~Stack() {
        while (top != nullptr) {
            StackNode<T>* temp = top;
            top = top->next;
            delete temp;
        }
    }
};

#endif
