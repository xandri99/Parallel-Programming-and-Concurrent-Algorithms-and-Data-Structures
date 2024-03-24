#pragma once

#include "set.hpp"
#include "std_set.hpp"

#include <mutex>

#define MIN -999999
#define MAX 999999

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
    EventMonitor<OptimisticSet, StdSet, SetOperator>* monitor;
public:
    OptimisticSet(EventMonitor<OptimisticSet, StdSet, SetOperator>* monitor) : monitor(monitor)
    {
        head = new OptimisticSetNode{ MIN, nullptr };
        head->next = new OptimisticSetNode{ MAX, nullptr };
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
    /*
    bool validate(OptimisticSetNode* p, OptimisticSetNode* c) {
        // A01: Implement the `validate` function used during
        // optimistic synchronization.

        // Corner case when adding the first element of the list
        OptimisticSetNode* vNode = head;
        if (!p) {
            return (head == c);
        }
        else {
            // Validate predecessor and current to check for no changes.
            while (vNode && vNode != p) {
                vNode = vNode->next;
            }
            if (vNode == p) {
                if (vNode->next == c) {
                    return true;
                }
            }
        }
        
        return false;
    }
    */
    bool validate(OptimisticSetNode* pred, OptimisticSetNode* curr) {
        // Traverse from the head to pred to ensure it's still part of the list
        OptimisticSetNode* node = head;
        while (node != nullptr && node->value < pred->value) {
            node = node->next;
        }
        return node == pred && pred->next == curr;
    }

public:
    /*
    bool add(int elem) override {
        bool result = false;
        // A01: Add code to insert the element into the set and update `result`.

        // If the list was empty
        if (!head) {
            OptimisticSetNode* addedNode = new OptimisticSetNode{ elem, nullptr };
            head = addedNode;
            head->next = nullptr;
            result = true;
            this->monitor->add(SetEvent(SetOperator::Add, elem, result));
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
                result = false;
                this->monitor->add(SetEvent(SetOperator::Add, elem, result));
                return result;
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



        this->monitor->add(SetEvent(SetOperator::Add, elem, result));
        return result;
    }*/

    bool add(int elem) override {
        while (true) {
            OptimisticSetNode* pred = head;
            OptimisticSetNode* curr = pred->next;
            while (curr->value < elem) {
                pred = curr;
                curr = curr->next;
            }

            pred->lock.lock();
            curr->lock.lock();

            if (validate(pred, curr)) {
                if (curr->value != elem) { // Ensure elem is not already present
                    OptimisticSetNode* node = new OptimisticSetNode{ elem, curr };
                    pred->next = node;
                    pred->lock.unlock();
                    curr->lock.unlock();
                    return true;
                }
                pred->lock.unlock();
                curr->lock.unlock();
                return false;
            }

            pred->lock.unlock();
            curr->lock.unlock();
        }
    }

    /*
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

        this->monitor->add(SetEvent(SetOperator::Remove, elem, result));
        return result;
    }
    */
    bool rmv(int elem) override {
        while (true) {
            OptimisticSetNode* pred = head;
            OptimisticSetNode* curr = pred->next;
            while (curr->value < elem) {
                pred = curr;
                curr = curr->next;
            }

            pred->lock.lock();
            curr->lock.lock();

            if (validate(pred, curr)) {
                if (curr->value == elem) { // Found the element
                    pred->next = curr->next;
                    delete curr; // Safe delete as it's no longer in the list
                    pred->lock.unlock();
                    curr->lock.unlock();
                    return true;
                }
                pred->lock.unlock();
                curr->lock.unlock();
                return false;
            }

            pred->lock.unlock();
            curr->lock.unlock();
        }
    }

    /*
    bool ctn(int elem) override {
        bool result = false;
        // A01: Add code to check if the element is inside the set and update `result`.

        OptimisticSetNode* c = head;
        while (c && c->value != elem) {
            c = c->next;
        }
        if (c && c->value == elem) {
            result = true;
        }

        this->monitor->add(SetEvent(SetOperator::Contains, elem, result));
        return result;
    }
    */
    bool ctn(int elem) override {
        while (1)
        {
            OptimisticSetNode* pred = head;
            OptimisticSetNode* curr = pred->next;

            while (curr->value < elem)
            {
                pred = curr;
                curr = curr->next;
            }

            pred->lock.lock();
            curr->lock.lock();

            if (validate(pred, curr))
            {
                bool retVal = (curr->value == elem);
                pred->lock.unlock();
                curr->lock.unlock();
                this->monitor->add(SetEvent(SetOperator::Contains, elem, retVal));
                return retVal;
            }
            pred->lock.unlock();
            curr->lock.unlock();
        }
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

