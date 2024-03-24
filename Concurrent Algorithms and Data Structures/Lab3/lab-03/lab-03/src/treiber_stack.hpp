#pragma once

#include "adt.hpp"

#include <atomic>


#define DEBUG false


struct TreiberStackNode {
    // A01: You can add or remove fields as needed.
    int value;
    TreiberStackNode* next = nullptr;

    TreiberStackNode(int value) : value(value), next(nullptr) {}
};

class TreiberStack: public Stack {
private:
    // A04: You can add or remove fields as needed.
    std::atomic<TreiberStackNode*> top;

    EventMonitor<TreiberStack, StdStack, StackOperator>* monitor;
    /// This lock can be used around the CAS operation, to insert the
    /// operation into the monitor at the linearization point. This is
    /// just one way to do it, you can also try alternative options.
    std::mutex cas_lock;
public:
    TreiberStack(EventMonitor<TreiberStack, StdStack, StackOperator>* monitor)
        : monitor(monitor)    {
        // A01: Initiate the internal state
        top = nullptr;
    }

    ~TreiberStack() {
        // A01: Cleanup any memory that was allocated
        while (pop() != EMPTY_STACK_VALUE) {}
    }

    /* Taken from the slides
    int push(int value) override {
        if (DEBUG) {
            std::cout << "Before push: ";
            print_state();
        }
        

        int result = true;
        // A01: Add code to insert the element at the top of the stack.
        //      Make sure, to insert the event at the linearization point.
        //      You can use the `cas_lock` to ensure that the event is
        //      inserted at the linearization point.

        TreiberStackNode* newNode = new TreiberStackNode(value);
        while (true) {
            TreiberStackNode* t = top.load();
            newNode->next = t;
            if (top.compare_exchange_weak(t, newNode)) {
                break;
            }
        }

        if (DEBUG) {
            std::cout << "After push: ";
            print_state();
        }

        this->monitor->add(StackEvent(StackOperator::StackPush, value, result));
        return result;
    }
    */


    bool tryPush(TreiberStackNode* node) {
        TreiberStackNode* oldTop = top.load();
        node->next = oldTop;
        return(top.compare_exchange_strong(oldTop, node));
    }
    int push(int value) override { // taken from the book
        TreiberStackNode* newNode = new TreiberStackNode(value);
        while (true) {
            if (tryPush(newNode)) {
                this->monitor->add(StackEvent(StackOperator::StackPush, value, true));
                return true;
            }
            else {
                continue;
            }
        }
    
    }

    /* Taken from the slides
    int pop() override {
        if (DEBUG) {
            std::cout << "Before pop: ";
            print_state();
        }

        int result = EMPTY_STACK_VALUE;
        // A01: Add code to pop the element at the top of the stack.
        //      Make sure, to insert the event at the linearization point.

        while (true) {
            TreiberStackNode* t = top.load();
            if (t == nullptr) {
                break;
            }
            if (top.compare_exchange_weak(t, t->next)) {
                result = t->value;
                delete t;
                break;
            }
        }

        if (DEBUG) {
            std::cout << "After pop: ";
            print_state();
        }

        this->monitor->add(StackEvent(StackOperator::StackPop, NO_ARGUMENT_VALUE, result));
        return result;
    }
    */

    int pop() override { // from the book
        retry:
        TreiberStackNode* oldTop = top.load();
        if (oldTop == nullptr) {
            this->monitor->add(StackEvent(StackOperator::StackPop, NO_ARGUMENT_VALUE, false));
            return EMPTY_STACK_VALUE;
        }
        while (true) {
            TreiberStackNode* newTop = oldTop->next;
            if (top.compare_exchange_strong(oldTop, newTop)) {
                this->monitor->add(StackEvent(StackOperator::StackPop, NO_ARGUMENT_VALUE, true));
                return oldTop->value;
            }
            else {
                goto retry;
            }
        }
    }


    int size() override {
        int result = 0;
        // A01: Add code to get the size of the stack.
        //      Make sure, to insert the event at the linearization point.

        TreiberStackNode* c = top.load();
        while (c != nullptr) {
            result = result + 1;
            c = c->next;
        }


        this->monitor->add(StackEvent(StackOperator::StackPop, NO_ARGUMENT_VALUE, result));
        return result;
    }

    void print_state() override {
        std::cout << "TreiberStack { ";
        TreiberStackNode* current = top.load();
        while (current != nullptr) {
            std::cout << current->value << " ";
            current = current->next;
        }
        std::cout << "}" << std::endl;
    }
};