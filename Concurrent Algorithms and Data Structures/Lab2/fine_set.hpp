#pragma once

#include <climits>
#include <mutex>

#include "set.hpp"

// A03: Copy your `FineSet` implementation from Lab 01 into this file and
// remove all references to the monitor. We want to benchmark the data
// structure and monitoring the performed operation would influence the
// results.
struct FineSetNode {
    // A04: You can add or remove fields as needed.
    int value;
    FineSetNode* next;
    std::mutex lock;
    FineSetNode(int elem, FineSetNode* next_node = NULL) {
        value = elem;
        next = next_node;
    }
};
class FineSet : public Set {
private:
    FineSetNode* head;
public:
    FineSet() {
        head = new FineSetNode(INT_MIN);
        head->next = new FineSetNode(INT_MAX);
    }

    ~FineSet() {
        while (head != NULL) {
            FineSetNode* temp = head;
            head = head->next;
            delete temp;
        }
    }

    bool add(int elem) override {
        bool result = false;
        head->lock.lock();
        FineSetNode* prev = head, * curr = head->next;
        curr->lock.lock();
        while (curr->value < elem) {
            prev->lock.unlock();
            prev = curr;
            curr = curr->next;
            curr->lock.lock();
        }
        if (curr->value != elem) {
            prev->next = new FineSetNode(elem, curr);
            result = true;
        }
        prev->lock.unlock();
        curr->lock.unlock();
        return result;
    }

    bool rmv(int elem) override {
        bool result = false;
        head->lock.lock();
        FineSetNode* prev = head, * curr = head->next;
        curr->lock.lock();
        while (curr->value < elem) {
            prev->lock.unlock();
            prev = curr;
            curr = curr->next;
            curr->lock.lock();
        }
        if (curr->value == elem) {
            prev->next = curr->next;
            delete curr;
            result = true;
        }
        else curr->lock.unlock();
        prev->lock.unlock();
        return result;
    }

    bool ctn(int elem) override {
        bool result = false;
        head->lock.lock();
        FineSetNode* prev = head, * curr = head->next;
        curr->lock.lock();
        while (curr->value < elem) {
            prev->lock.unlock();
            prev = curr;
            curr = curr->next;
            curr->lock.lock();
        }
        if (curr->value == elem) result = true;
        prev->lock.unlock();
        curr->lock.unlock();
        return result;
    }

    void print_state() override {
    }
};
