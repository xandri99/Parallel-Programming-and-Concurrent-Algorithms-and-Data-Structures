#pragma once

#include "set.hpp"
#include "std_set.hpp"

#include <mutex>

/// The node used for the linked list implementation of a set in the [`LazySet`]
/// class. This struct is used for task 3
struct LazySetNode {
    // A02: You can add or remove fields as needed.
    int value;
    bool mark;
    LazySetNode* next;
    std::mutex lock;
};

/// A set implementation using a linked list with optimistic syncronization.
class LazySet: public Set {
private:
    // A02: You can add or remove fields as needed. Just having the `head`
    // pointer should be sufficient for this task
    LazySetNode* head;
public:
    LazySet() : head(nullptr)
    {
        // A02: Initiate the internal state
    }

    ~LazySet() override {
        // A02: Cleanup any memory that was allocated
        LazySetNode* current = head;
        while (current != nullptr) {
            LazySetNode* next = current->next;
            delete current;
            current = next;
        }
    }

private:
    std::pair<LazySetNode*, LazySetNode*> locate(int value) {
        // A02: Implement the `locate` function used for lazy synchronization.

        while (true) {
            LazySetNode* p = nullptr;
            LazySetNode* c = head;
            // empty list case, we return null for both.
            if (c == nullptr) {       
                return std::make_pair(nullptr, nullptr);
            }
            // the value is in the head or before, so p is null.
            else if (c->value >= value){
                c->lock.lock();
                if (!c->mark) {
                    return std::make_pair(nullptr, c);
                }
                c->lock.unlock();
            }
            // the value is anywhere else:
            else {
                while (c && c->value < value) {
                    p = c;
                    c = c->next;
                }
                // iterate over all list, two cases: we find the elem (c = value), or we finish (c=null)

                // The value is not in the list
                if (!c) {
                    p->lock.lock();
                    if (!p->mark && p->next == nullptr) {
                        return std::make_pair(p, nullptr);
                    }
                    p->lock.unlock();
                }
                else {
                    p->lock.lock();
                    c->lock.lock();
                    if (!p->mark && !c->mark && p->next == c) {
                        return std::make_pair(p, c);
                    }
                    p->lock.unlock();
                    c->lock.unlock();
                }
            }
        }
    }

public:
    bool add(int elem) override {
        bool result = false;
        // A02: Add code to insert the element into the set and update `result`.

        LazySetNode* p = nullptr;
        LazySetNode* c = nullptr;
        std::tie(p, c) = locate(elem);
        if (c && c->value == elem) {
            if (p) p->lock.unlock();
            c->lock.unlock();
            return result;
        }
        else {
            result = true;
            LazySetNode* newNode = new LazySetNode{ elem, false, nullptr };
            if (!p && !c) {
                head = newNode;
            }
            else {
                if (p) p->next = newNode;
                if (c) newNode->next = c;
                if (!p) head = newNode;
                if (p) p->lock.unlock();
                if (c) c->lock.unlock();
            }
        }

        return result;
    }

    bool rmv(int elem) override {
        bool result = false;
        // A02: Add code to remove the element from the set and update `result`.

        LazySetNode* p = nullptr;
        LazySetNode* c = nullptr;
        std::tie(p, c) = locate(elem);
        if (c && c->value == elem) {
            c->mark = true;
            if (c == head) {
                head = head->next;
            }
            else {
                p->next = c->next;
                p->lock.unlock();
            }
            c->lock.unlock();
            //delete c;
            result = true;
        }
        else {
            if (p) p->lock.unlock();
            if (c) c->lock.unlock();
        }

        return result;
    }

    bool ctn(int elem) override {
        bool result = false;
        // A02: Add code to check if the element is inside the set and update `result`.

        LazySetNode* c = head;
        while (c && c->value != elem) {
            c = c->next;
        }
        if (c && c->value == elem && !c->mark) {
            result = true;
        }

        return result;
    }

    void print_state() override {
        // A02: Optionally, add code to print the state. This is useful for debugging,
        // but not part of the assignment
        std::cout << "LazySet {...}";
    }
};

