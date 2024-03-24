#pragma once

#include "set.hpp"
#include "std_set.hpp"

#include <mutex>

/// The node used for the linked list implementation of a set in the [`OptimisticSet`]
/// class. This struct is used for task 3
struct OptimisticSetNode {
    // A01: You can add or remove fields as needed.
    int value;
    OptimisticSetNode* next;
    std::mutex lock;
};

/// A set implementation using a linked list with optimistic syncronization.
class OptimisticSet: public Set {
private:
    // A01: You can add or remove fields as needed. Just having the `head`
    // pointer should be sufficient for this task
    OptimisticSetNode* head;
public:
    OptimisticSet() : head(nullptr)
    {
        // A01: Initiate the internal state
    }

    ~OptimisticSet() override {
        // A01: Cleanup any memory that was allocated
        OptimisticSetNode* current = head;
        while (current != nullptr) {
            OptimisticSetNode* next = current->next;
            delete current;
            current = next;
        }
    }

private:
    bool validate(OptimisticSetNode* p, OptimisticSetNode* c) {
        // A01: Implement the `validate` function used during
        // optimistic synchronization.

        // Corner case when adding the first element of the list
        OptimisticSetNode* vNode = head;
        if (!p) {
            return (head == c);
        }

        // Validate predecessor and current to check for no changes.
        while (vNode && vNode != p) {
            vNode = vNode->next;
        }
        if (vNode == p) {
            if (vNode->next == c) {
                return true;
            }
        }
        return false;
    }


public:
    bool add(int elem) override {
        bool result = false;
        // A01: Add code to insert the element into the set and update `result`.

        // If the list was empty
        if (!head) {
            OptimisticSetNode* addedNode = new OptimisticSetNode{ elem, nullptr };
            head = addedNode;
            head->next = nullptr;
            result = true;
            return result;
        }

        // First check to see if the element can be added, and if it's not in the list, we add it at the end
        while (true) {
            OptimisticSetNode* p = nullptr;
            OptimisticSetNode* c = head;
            while (c && c->value < elem) {
                p = c;
                c = c->next;
            }
            if (c && c->value == elem) {
                break;
            }

            if (p) p->lock.lock();
            if (c) c->lock.lock();

            if (validate(p, c)) {
                OptimisticSetNode* addedNode = new OptimisticSetNode{ elem, nullptr };
                if (p) p->next = addedNode;
                if (!p) head = addedNode;
                addedNode->next = c;
                result = true;
                if (p) p->lock.unlock();
                if (c) c->lock.unlock();
                break;
            }

            if (p) p->lock.unlock();
            if (c) c->lock.unlock();
        }

        return result;
    }

    
    bool rmv(int elem) override {
        bool result = false;
        // A01: Add code to remove the element from the set and update `result`.
        
        // Three cases, elem is head, elem is anything in between or elem is tail
        // elem is head:
        if (head && head->value == elem) {
            OptimisticSetNode* c = head;
            c->lock.lock();
            if (validate(nullptr, c)) {
                head = c->next;
                c->lock.unlock();
                delete c;
                result = true;
            }
            c->lock.unlock();
        }

        //elem is anything in between or elem is tail
        while (true) {
            OptimisticSetNode* p = nullptr;
            OptimisticSetNode* c = head;
            while (c && c->value != elem) {
                p = c;
                c = c->next;
            }
            
            if (!c) {
                break; // we finish going through the list and the element has not been found
            }
            if (p) p->lock.lock();
            if (c) c->lock.lock();
            if (validate(p, c)) {
                p->next = c->next;
                c->lock.unlock();
                delete c;
                if (p) p->lock.unlock();
                result = true;
                break;
            }
            if (p) p->lock.unlock();
            if (c) c->lock.unlock();
        }

        return result;
    }



    bool ctn(int elem) override {
        bool result = false;
        // A01: Add code to check if the element is inside the set and update `result`.

        OptimisticSetNode* c = head;

        while (c) {
            c->lock.lock();

            if (c->value == elem) {
                result = true;
                c->lock.unlock();
                break;
            }
            c->lock.unlock();
            c = c->next;
        }

        return result;
    }

    void print_state() override {
        // A01: Optionally, add code to print the state. This is useful for debugging,
        // but not part of the assignment
        OptimisticSetNode* c = head;
        std::cout << "OptimisticSet { ";
        while (c != nullptr) {
            std::cout << c->value << " ";
            c = c->next;
        }
        std::cout << "}" << std::endl;
    }
};

