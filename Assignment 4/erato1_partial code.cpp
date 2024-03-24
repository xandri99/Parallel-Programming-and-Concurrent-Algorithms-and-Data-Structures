#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string.h>
#include <cmath>
#include <string>
#include <chrono>
#include <mpi.h>



void usage(char* program) {
	std::cout << "Usage: " << program << " <maximum positive integer>" << std::endl;
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
Implementation using MPI Send() and MPI Recv()
*/

void EratosthenesMPIsr(int max) {
	auto begin = std::chrono::high_resolution_clock::now();


	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);


	// Each process creates a list of natural numbers: 1, 2, 3, . . . , Max
	std::vector<int> primes(max + 1, 1);

	// Split the working space among Processes.
	int w = (max + 1) / size;
	int start = rank * w;
	int end = (rank == size - 1) ? max : start + w - 1;

	// std::cout << "Process " << rank << ":  " << start << " to " << end << std::endl;
	// std::cout << std::endl;

	// actual Sieve algorithm
	//Set k to 2, the first unmarked number in the list
	for (int k = 2; k * k <= max; k++) {
		// Find the smallest number greater than k that is still unmarked
		if (primes[k] == 1) {
			//Mark all multiples of k between k^2 and Max, in the designated work region for this thread
			for (int i = (start < k * k) ? k * k : ((start + k - 1) / k) * k; i <= end; i = i + k) {
				primes[i] = 0;
			}
		}
		// MPI_Barrier :Blocks the caller until all processes in the communicator have called it
		// The equivalent to the #pragma omp barrier in OpenMP
		MPI_Barrier(MPI_COMM_WORLD);
	}

	// Since each Process works with it's own memory, we need to gather the results in the 
	// main Process.

	// only the master does the gathering (not tree reduce, but easier to code)
	if (rank == 0) {
		for (int id = 1; id < size; id = id + 1) {
			int g_start = id * w;
			int g_end = (id == size - 1) ? max : g_start + w - 1;
			// we create a vector to recieve the partial primes from the other processes
			std::vector<int> recived_partial_primes(g_end - g_start + 1);

			// actual recieve for the data
			MPI_Recv(recived_partial_primes.data(), g_end - g_start + 1, MPI_INT, id, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			// adding the data to the master vector
			for (int j = g_start; j <= g_end; j = j + 1) {
				primes[j] = recived_partial_primes[j - g_start]; //j is counter for master, and offset for sender.
			}
		}
	}
	else {
		// all non master processes will send, from start to end, their part of the primes, to the master (0)
		std::vector<int> send_primes(primes.begin() + start, primes.begin() + end + 1);
		MPI_Send(send_primes.data(), end - start + 1, MPI_INT, 0, 11, MPI_COMM_WORLD);
	}



	if (rank == 0) {
		/*
		//The unmarked numbers are all prime.
		// Print it in master
		std::cout << "Prime numbers from 0 to " << max << " are: " << std::endl;
		for (int i = 0; i < primes.size(); i++) {
			if (primes[i] == 1) {
				std::cout << i << ", ";
			}
		}
		std::cout << std::endl;
		*/

		auto end = std::chrono::high_resolution_clock::now();

		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
		std::cout << "Execution time: " << elapsed.count() << " nanoseconds" << std::endl;
	}
}




/*
Implementation using MPI Bcast() and MPI Reduce()
*/

void EratosthenesMPIbr(int max) {
	auto begin = std::chrono::high_resolution_clock::now();

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	// calculate sqrt(max)
	int sqrtmax = (int)sqrt(max);

	// Each process creates a list of natural numbers: 1, 2, 3, . . . , Max
	std::vector<int> primes(max + 1, 1);

	// Folowing the instructions of Assignment 2
	// 1. First sequentially compute primes up to sqrt  (only done by the master process)
	std::vector<int> master_primes;
	if (rank == 0) {
		for (int k = 2; k <= sqrtmax; k = k + 1) {
			if (primes[k] == 1) {
				// saves the prime in the list to broadcast
				master_primes.push_back(k);
				for (int i = k * k; i <= sqrtmax; i = i + k) {
					primes[i] = 0;
				}
			}
		}
	}
	

	// broadcast the number of primes found so that the receptors can allocate memory for it
	int allocation_size = master_primes.size();
	MPI_Bcast(&allocation_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

	// all the processes that are not the master reseize the vector to fit the data that will be broadcasted
	if (rank != 0) {
		master_primes.resize(allocation_size);
	}

	// Broadcast the primes found by the master for all processes to have
	MPI_Bcast(master_primes.data(), allocation_size, MPI_INT, 0, MPI_COMM_WORLD);

	// Given p cores, build p chunks of roughly equal length covering the range from sqrtMax + 1 to Max
	int w = (max - sqrtmax) / size;
	int start = sqrtmax + 1 + rank * w;
	int end = (rank == size - 1) ? max : start + w - 1;

	// Each thread uses the sequentially computed “seeds” (work region) to mark the numbers in its chunk
	for (int prime : master_primes) {
		for (int i = (start < prime * prime) ? prime * prime : ((start + prime - 1) / prime) * prime; i <= end; i = i + prime) {
			primes[i] = 0;
		}
	}
	
	// The master waits for all threads to finish and collects the unmarked numbers.
	// To collect all the partial results, we can use reduce. All processes do Reduce, since it's a synchronitzated method
	std::vector<int> primes_total(max + 1, 0);
	// The operations should be an OR, since we want 1 if any of the compared has a 1, and 0 only if everyone has a 0.
	MPI_Reduce(primes.data(), primes_total.data(), max + 1, MPI_INT, MPI_LAND, 0, MPI_COMM_WORLD);


	// Only the master prints the primes and the execution time
	if (rank == 0) {

		std::cout << "Prime numbers from 0 to " << max << " are: " << std::endl;
		for (int i = 0; i < primes_total.size(); i++) {
			if (primes_total[i] == 1) {
				std::cout << i << ", ";
			}
		}
		std::cout << std::endl;
		

		auto end = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
		std::cout << "Execution time: " << elapsed.count() << " nanoseconds" << std::endl;
	}
}



int main(int argc, char* argv[]) {

	MPI_Init(&argc, &argv);

	if (argc != 2) {
		usage(argv[0]);
	}

	int max = std::stoi(argv[1]);

	if (max <= 0) {
		std::cout << "These should be positive integers, bigger than 0." << std::endl;
		exit(1);
	}


	//EratosthenesMPIsr(max);
	EratosthenesMPIbr(max);


	MPI_Finalize();


	// std::cout << "done" << std::endl;
	return 0;
}
