#pragma once

#include "set.hpp"
#include "std_set.hpp"

#include <mutex>

// A03: Copy your `FineSet` implementation from Lab 01 into this file and
// remove all references to the monitor. We want to benchmark the data
// structure and monitoring the performed operation would influence the
// results.

struct FineSetNode {
    // A04: You can add or remove fields as needed.
    int value;
    FineSetNode* next;
    std::mutex lock;
};


class FineSet: public Set {
private:
    FineSetNode* head;
public:
    FineSet() : head(nullptr)
    {
        // A04: Initiate the internal state
    }

    ~FineSet() override {
        // A04: Cleanup any memory that was allocated
        FineSetNode* current = head;
        while (current != nullptr) {
            FineSetNode* next = current->next;
            delete current;
            current = next;
        }
    }


    bool add(int elem) override {
        bool result = true;
        
        // First we check the head, to see if it's empty
        if (head == nullptr) {
            FineSetNode* addedNode = new FineSetNode{ elem, nullptr };
            addedNode->lock.lock();
            head = addedNode;
            head->next = nullptr;
            head->lock.unlock();
        }
        else {
            // we get to the last element of the list while checking if elem is already in the list
            FineSetNode* current = head;
            current->lock.lock();
            while (current->next != nullptr) {
                if (current->value == elem) {
                    result = false;
                    current->lock.unlock();
                    break;
                }
                FineSetNode* next = current->next;
                next->lock.lock();
                current->lock.unlock();
                current = next;
            }
            if (current->value == elem) {
                result = false;
                current->lock.unlock();
            }

            // if we got to the last node and elem is not in the list, we create and add a new node wiht
            // elem, and link the previous last node to this one
            if (result) {
                FineSetNode* addedNode = new FineSetNode{ elem, nullptr };
                addedNode->lock.lock();
                current->next = addedNode;
                current->lock.unlock();
                addedNode->lock.unlock();
            }
        }

        return result;
    }

    bool rmv(int elem) override {
        bool result = false;
        
        // First we need to check the head
        if (head != nullptr && head->value == elem) {
            head->lock.lock();
            if (head->next != nullptr) {
                head->next->lock.lock();
            }
            FineSetNode* temp = head;
            head = head->next;
            delete temp;
            if (head != nullptr) {
                head->lock.unlock();
            }
            result = true;
        }
        else {
            // we get to the last element of the list while checking if elem is already in the list
            FineSetNode* current = head;
            FineSetNode* previous = nullptr;
            while (current != nullptr) {
                current->lock.lock();
                if (current->value == elem) {
                    result = true;
                    if (previous != nullptr) {
                        previous->next = current->next;
                    }
                    if (current->next != nullptr) {
                        current->next->lock.lock();
                    }
                    if (previous != nullptr) {
                        previous->lock.unlock();
                    }
                    current->lock.unlock();
                    delete current;
                    if (current->next != nullptr) {
                        current->next->lock.unlock();
                    }
                    break;
                }
                if (previous != nullptr) {
                    previous->lock.unlock();
                }
                previous = current;
                current = current->next;
            }
            if (previous != nullptr && previous != current) {
                previous->lock.unlock();
            }
        }

        return result;
    }

    bool ctn(int elem) override {
        bool result = false;
        
         // We iterate over the linked list and check for elem
        FineSetNode* current = head;
        while (current != nullptr) {
            current->lock.lock();
            if (current->value == elem) {
                result = true;
                current->lock.unlock();
                break;
            }
            FineSetNode* next = current->next;
            current->lock.unlock();
            current = next;
        }

        return result;
    }

    void print_state() override {
        std::cout << "FineSet {...}";
    }
};

