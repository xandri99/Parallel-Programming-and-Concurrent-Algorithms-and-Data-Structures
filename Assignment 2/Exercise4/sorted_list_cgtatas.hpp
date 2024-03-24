#ifndef lacpp_sorted_list_hpp
#define lacpp_sorted_list_hpp lacpp_sorted_list_hpp

#include <atomic>

/* a sorted list implementation by David Klaftenegger, 2015
 * please report bugs or suggest improvements to david.klaftenegger@it.uu.se
 */

 /* struct for list nodes */
template<typename T>
struct node {
	T value;
	node<T>* next;
};

/* non-concurrent sorted singly-linked list */
template<typename T>
class sorted_list {
	node<T>* first = nullptr;
	std::atomic<bool> lock{false};

public:
	/* default implementations:
	 * default constructor
	 * copy constructor (note: shallow copy)
	 * move constructor
	 * copy assignment operator (note: shallow copy)
	 * move assignment operator
	 *
	 * The first is required due to the others,
	 * which are explicitly listed due to the rule of five.
	 */
	sorted_list() = default;
	sorted_list(const sorted_list<T>& other) = default;
	sorted_list(sorted_list<T>&& other) = default;
	sorted_list<T>& operator=(const sorted_list<T>& other) = default;
	sorted_list<T>& operator=(sorted_list<T>&& other) = default;
	~sorted_list() {
		while (first != nullptr) {
			remove(first->value);
		}
	}
	/* insert v into the list */


	void set_lock() {
		// exchange function swaps the current value of lock with true and returns the previous value.
		while (lock.exchange(true)) {
			while (lock.load()) {}
		}
	}

	void set_unlock() {
		lock.store(false);
	}


	void insert(T v) {
		set_lock();
		/* first find position */
		node<T>* pred = nullptr;
		node<T>* succ = first;
		while (succ != nullptr && succ->value < v) {
			pred = succ;
			succ = succ->next;
		}

		/* construct new node */
		node<T>* current = new node<T>();
		current->value = v;

		/* insert new node between pred and succ */
		current->next = succ;
		if (pred == nullptr) {
			first = current;
		}
		else {
			pred->next = current;
		}
		set_unlock();
	}

	void remove(T v) {
		set_lock();
		/* first find position */
		node<T>* pred = nullptr;
		node<T>* current = first;
		while (current != nullptr && current->value < v) {
			pred = current;
			current = current->next;
		}
		if (current == nullptr || current->value != v) {
			/* v not found */
			return;
		}
		/* remove current */
		if (pred == nullptr) {
			first = current->next;
		}
		else {
			pred->next = current->next;
		}
		delete current;

		set_unlock();
	}

	/* count elements with value v in the list */
	std::size_t count(T v) {
		set_lock();
		std::size_t cnt = 0;
		/* first go to value v */
		node<T>* current = first;
		while (current != nullptr && current->value < v) {
			current = current->next;
		}
		/* count elements */
		while (current != nullptr && current->value == v) {
			cnt++;
			current = current->next;
		}

		set_unlock();
		return cnt;

	}
};

#endif // lacpp_sorted_list_hpp
