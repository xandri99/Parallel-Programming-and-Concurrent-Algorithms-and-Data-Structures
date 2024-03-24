#pragma once

#include "set.hpp"
#include "std_set.hpp"

#include <mutex>

/// The node used for the linked list implementation of a set in the [`CoarseSet`]
/// class. This struct is used for task 3
struct CoarseSetNode {
    // A03: You can add or remove fields as needed.
    int value;
    CoarseSetNode* next;
};

/// A set implementation using a linked list with coarse grained locking.
class CoarseSet: public Set {
private:
    // A03: You can add or remove fields as needed. Just having the `head`
    // pointer and the `lock` should be sufficient for task 3
    CoarseSetNode* head;
    std::mutex lock;
    EventMonitor<CoarseSet, StdSet, SetOperator>* monitor;
public:
    CoarseSet(EventMonitor<CoarseSet, StdSet, SetOperator>* monitor) :
        head(nullptr), monitor(monitor)
    {
        // A03: Initiate the internal state
    }

    ~CoarseSet() override {
        // A03: Cleanup any memory that was allocated

        CoarseSetNode* current = head;
        while (current != nullptr) {
            CoarseSetNode* next = current->next;
            delete current;
            current = next;
        }
    }

    bool add(int elem) override {
        lock.lock();
        bool result = true;
        // A03: Add code to insert the element into the set and update `result`.
        //      Also make sure, to insert the event inside the locked region of
        //      the linearization point.

        // First we check the head, to see if it's empty
        if (head == nullptr) {
            CoarseSetNode* addedNode = new CoarseSetNode{ elem, nullptr };
            head = addedNode;
            head->next = nullptr;
        }
        else {
            // we get to the last element of the list while checking if elem is already in the list
            CoarseSetNode* current = head;
            CoarseSetNode* previous = head;
            while (current != nullptr) {
                if (current->value == elem) {
                    result = false;
                    break;
                }
                previous = current;
                current = current->next;
            }

            // if we got to the last node and elem is not in the list, we create and add a new node wiht
            // elem, and link the previous last node to this one
            if (result) {
                current = new CoarseSetNode{ elem, nullptr };
                previous->next = current;
            }
        }

        this->monitor->add(SetEvent(SetOperator::Add, elem, result));
        lock.unlock();
        return result;
    }

    bool rmv(int elem) override {
        lock.lock();
        bool result = false;
        // A03: Add code to remove the element from the set and update `result`.
        //      Also make sure, to insert the event inside the locked region of
        //      the linearization point.

        // First we need to check the head
        if (head != nullptr && head->value == elem) {
            CoarseSetNode* temp = head;
            head = head->next;
            delete temp;
            result = true;
        }
        else {
            // we get to the last element of the list while checking if elem is already in the list
            CoarseSetNode* current = head;
            CoarseSetNode* previous = head;
            while (current != nullptr) {
                if (current->value == elem) {
                    result = true;
                    previous->next = current->next;
                    delete current;
                    break;
                }
                previous = current;
                current = current->next;
            }
        }


        this->monitor->add(SetEvent(SetOperator::Remove, elem, result));
        lock.unlock();
        return result;
    }

    bool ctn(int elem) override {
        lock.lock();
        bool result = false;
        // A03: Add code to check if the element is inside the set and update `result`.
        //      Also make sure, to insert the event inside the locked region of
        //      the linearization point.

        // We iterate over the linked list and check for elem
        CoarseSetNode* current = head;
        while (current != nullptr) {
            if (current->value == elem) {
                result = true;
                break;
            }
            current = current->next;
        }

        this->monitor->add(SetEvent(SetOperator::Contains, elem, result));
        lock.unlock();
        return result;
    }

    void print_state() override {
        // A03: Optionally, add code to print the state. This is useful for debugging,
        // but not part of the assignment
        std::cout << "CoarseSet {...}";
    }
};

