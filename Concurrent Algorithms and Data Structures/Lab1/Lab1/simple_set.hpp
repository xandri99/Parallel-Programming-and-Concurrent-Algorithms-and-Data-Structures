#pragma once

#include "set.hpp"
#include "std_set.hpp"

/// The node used for the linked list implementation of a set in the [`SimpleSet`]
/// class. This struct is used for task 2
struct SimpleSetNode {
    // A02: You can add or remove fields as needed.
    int value;
    SimpleSetNode* next;
};

/// A simple set implementation using a linked list. This class shouldn't have
// any synchronization yet.
class SimpleSet: public Set {
private:
    // A02: You can add or remove fields as needed. Just having the `head`
    // pointer should be sufficient for task 2
    SimpleSetNode* head;
    EventMonitor<SimpleSet, StdSet, SetOperator>* monitor;
public:
    SimpleSet(EventMonitor<SimpleSet, StdSet, SetOperator>* monitor) :
        head(nullptr), monitor(monitor)
    {
        // A02: Initiate the internal state
        // we need to add the head as an empty pointer, since the list is empty at the initialitzation
    }

    ~SimpleSet() override {
        // A02: Cleanup any memory that was allocated
        
        // we need to iterate over all nodes in the linked list, and eliminate them one by one
        // until the current is an empty pointer, which implies no more nodes in the list.
        SimpleSetNode* current = head;
        while (current != nullptr) {
            SimpleSetNode* next = current->next;
            delete current;
            current = next;
        }

    }

    bool add(int elem) override {
        bool result = true;
        // A02: Add code to insert the element into the set and update `result`.
        
        // First we check the head, to see if it's empty
        if (head == nullptr) {
            SimpleSetNode* addedNode = new SimpleSetNode{ elem, nullptr };
            head = addedNode;
            head->next = nullptr;
        }
        else {
            // we get to the last element of the list while checking if elem is already in the list
            SimpleSetNode* current = head;
            SimpleSetNode* previous = head;
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
                current = new SimpleSetNode{ elem, nullptr };
                previous->next = current;
            }
        }


        this->monitor->add(SetEvent(SetOperator::Add, elem, result));
        return result;
    }


    bool rmv(int elem) override {
        bool result = false;
        // A02: Add code to remove the element from the set and update `result`.
        // First we need to check the head
        if (head != nullptr && head->value == elem) {
            SimpleSetNode* temp = head;
            head = head->next;
            delete temp;
            result = true;
        }
        else {
            // we get to the last element of the list while checking if elem is already in the list
            SimpleSetNode* current = head;
            SimpleSetNode* previous = head;
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
        return result;
    }

    bool ctn(int elem) override {
        bool result = false;
        // A02: Add code to check if the element is inside the set and update `result`.

        // We iterate over the linked list and check for elem
        SimpleSetNode* current = head;
        while (current != nullptr) {
            if (current->value == elem) {
                result = true;
                break;
            }
            current = current->next;
        }


        this->monitor->add(SetEvent(SetOperator::Contains, elem, result));
        return result;
    }

    void print_state() override {
        // A02: Optionally, add code to print the state. This is useful for debugging,
        // but not part of the assignment.
        std::cout << "SimpleSet {...}";
    }
};

