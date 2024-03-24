#pragma once

#include "set.hpp"
#include "std_set.hpp"

#include <mutex>

/// The node used for the linked list implementation of a set in the [`FineSet`]
/// class. This struct is used for task 4.
struct FineSetNode {
    // A04: You can add or remove fields as needed.
    int value;
    FineSetNode* next;
    std::mutex lock;
};

/// A set implementation using a linked list with fine grained locking.
class FineSet: public Set {
private:
    // A04: You can add or remove fields as needed. Just having the `head`
    // pointer should be sufficient for task 4
    FineSetNode* head;
    EventMonitor<FineSet, StdSet, SetOperator>* monitor;
public:
    FineSet(EventMonitor<FineSet, StdSet, SetOperator>* monitor) :
        head(nullptr), monitor(monitor)
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
        // A04: Add code to insert the element into the set and update `result`.
        //      Also make sure, to insert the event inside the locked region of
        //      the linearization point.


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

        this->monitor->add(SetEvent(SetOperator::Add, elem, result));
        return result;
    }

    bool rmv(int elem) override {
        bool result = false;
        // A04: Add code to remove the element from the set and update `result`.
        //      Also make sure, to insert the event inside the locked region of
        //      the linearization point.

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

        this->monitor->add(SetEvent(SetOperator::Remove, elem, result));
        return result;
    }

    bool ctn(int elem) override {
        bool result = false;
        // A04: Add code to check if the element is inside the set and update `result`.
        //      Also make sure, to insert the event inside the locked region of
        //      the linearization point.

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

        this->monitor->add(SetEvent(SetOperator::Contains, elem, result));
        return result;
    }

    void print_state() override {
        // A04: Optionally, add code to print the state. This is useful for debugging,
        // but not part of the assignment
        std::cout << "FineSet {...}";
    }
};

