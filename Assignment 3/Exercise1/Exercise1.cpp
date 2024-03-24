#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string.h>
#include <cmath>
#include <string>
#include <chrono>
#include <omp.h>



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



/* 
Implementation using Parallel regions and a barriers as sinchronitzation
*/

void openMPEratosthenes(int max, int num_threads) {
	auto begin = std::chrono::high_resolution_clock::now();
	// Create a list of natural numbers: 1, 2, 3, . . . , Max
	std::vector<bool> primes(max + 1, true);


#pragma omp parallel num_threads(num_threads)
	{
	
	int id = omp_get_thread_num();
	int nthrds = omp_get_num_threads();

	// Split the working space among threads
	int w = (max + 1) / num_threads;
	int start = id * w;
	/* old code, changed for a simpler one-line sentence using C++ Ternary Operators
	int end = max;
	if (id != num_threads - 1) {
		end = start + w - 1;
	}*/
	int end = (id == nthrds - 1) ? max : start + w - 1;

	std::cout << "Thread " << id << ":  " << start << " to " << end << std::endl;
	std::cout << std::endl;

	// actual Sieve algorithm
	//Set k to 2, the first unmarked number in the list
	for (int k = 2; k * k <= max; k++) {
		// Find the smallest number greater than k that is still unmarked
		if (primes[k] == true) {
			//Mark all multiples of k between k^2 and Max, in the designated work region for this thread
			for (int i = (start < k * k) ? k * k : ((start + k - 1) / k) * k; i <= end; i = i + k) {
				primes[i] = false;
			}
		}

		#pragma omp barrier
	}
	}

	//The unmarked numbers are all prime.
	/*
	std::cout << "Prime numbers from 0 to " << max << " are: " << std::endl;
	for (int i = 0; i < primes.size(); i++) {
		if (primes[i] == true) {
			std::cout << i << ", ";
		}
	}
	std::cout << std::endl;
	*/

	auto end = std::chrono::high_resolution_clock::now();

	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
	std::cout << "Execution time: " << elapsed.count() << " nanoseconds" << std::endl;
	
}



int main(int argc, char* argv[]) {

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

	// paralelEratosthenes(max, num_threads);

	openMPEratosthenes(max, num_threads);


	std::cout << "done" << std::endl;
	return 0;
}