#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string.h>
#include <cmath>
#include <string>
#include <chrono>



void usage(char* program) {
	std::cout << "Usage: " << program << " <number of threads> <maximum positive integer>" << std::endl;
	exit(1);
}

void normalEratosthenes(int max) {
	// Create a list of natural numbers: 1, 2, 3, . . . , Max
	std::vector<bool> primes(max + 1, true);

	//Set k to 2, the first unmarked number in the list
	for (int k = 2; k * k <= max; k++) {
		// Find the smallest number greater than k that is still unmarked.
		if (primes[k] == true) {
			//Mark all multiples of k between k^2 and Max
			for (int i = k * k; i <= max; i = i + k) {
				primes[i] = false;
			}
		}
	}
	//The unmarked numbers are all prime.
	std::cout << "Prime numbers from 0 to " << max << " are: " << std::endl;
	for (int i = 0; i < primes.size(); i++) {
		if (primes[i] == true) {
			std::cout << i << ", ";
		}
	}
	std::cout << std::endl;
}




void Eratosthenes(int min, int max, int realMax, std::vector<bool>& primes) {
	//std::cout << "min max" << min << "   " << max << std::endl;
	//Set k to the first unmarked number in the list
	for (int k = min; k * k <= max; k++) {
		// Find the smallest number greater than k that is still unmarked.
		if (primes[k] == true) {
			//Mark all multiples of k between k^2 and Max
			for (int i = k * k; i <= realMax; i = i + k) {
				primes[i] = false;
				//debug
				 //std::cout << k << "   " << i << std::endl;
			}
		}
	}
}

void paralelEratosthenes(int max,  int num_threads) {
	auto begin = std::chrono::high_resolution_clock::now();

	int sqrtMax = (int)(std::sqrt(max));

	// Create a list of natural numbers: 1, 2, 3, . . . , Max
	std::vector<bool> primes(max + 1, true);

	//First sequentially compute primes up to ?Max .
	Eratosthenes(2, sqrtMax, max, std::ref(primes));

	// Given p cores, build p chunks of roughly equal length covering the range from ?Max + 1 to Max
	int w = (max - sqrtMax + 1) / num_threads;
	std::vector<std::thread> threads;

	// and allocate a thread for each chunk
	for (int i = 0; i < num_threads; i++) {
		int end = max;
		int start = sqrtMax + 1 + i * w;
		if (i != num_threads - 1) {
			end = start + w - 1;
		}
		std::cout << "thread " << i << "  :  start end " << start << "   " << end << std::endl;
		// Each thread uses the sequentially computed “seeds” to mark the numbers in its chunk.
		threads.emplace_back(Eratosthenes, start, end, end, std::ref(primes));
	}

	// The master waits for all threads to finish and collects the unmarked numbers.
	for (auto& thread : threads) {
		thread.join();
	}


	//The unmarked numbers are all prime.
	std::cout << "Prime numbers from 0 to " << max << " are: " << std::endl;
	for (int i = 0; i < primes.size(); i++) {
		if (primes[i] == true) {
			//std::cout << i << ", ";
		}
	}
	//std::cout << std::endl;

	auto end = std::chrono::high_resolution_clock::now();

	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
	std::cout << "Execution time: " << elapsed.count() << " nanoseconds" << std::endl;
}



int main(int argc, char* argv[]) {
	//auto begin = std::chrono::high_resolution_clock::now();

	if (argc != 3) {
		usage(argv[0]);
	}

	int num_threads = std::stoi(argv[1]);
	int max = std::stoi(argv[2]);

	if (num_threads <= 0 || max <= 0) {
		std::cout << "These should be positive integers, bigger than 0." << std::endl;
		exit(1);
	}

	//normalEratosthenes(max);

	paralelEratosthenes(max, num_threads);
	std::cout << "done" << std::endl;
	return 0;
}