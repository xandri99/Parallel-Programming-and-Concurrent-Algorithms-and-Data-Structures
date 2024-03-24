#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string.h>
#include <cmath>
#include <string>
#include <chrono>



void usage(char* program)
{
	std::cout << "Usage: " << program << " <number of threads> <number of trapezes>\n-h for help\n" << std::endl;
	exit(1);
}

double integralFunction(double x) {
	return 4 / (1 + std::pow(x, 2));
}

double integration(int vector_position, int num_threads, double a, double b, int num_trapezes) {
	double w = (b - a) / num_trapezes;
	double sum = 0;

	for (int i = vector_position; i < num_trapezes; i = i + num_threads) {
		double x0 = a + w * i;
		double x1 = x0 + w;

		double y0 = integralFunction(x0);
		double y1 = integralFunction(x1);

		sum = sum + 0.5 * (y0 + y1) * w;
	}
	return sum;
}

void threadFunction(int vector_position, int num_threads, double a, double b, int num_trapezes, std::vector<double>& partial_integrals) {
	partial_integrals[vector_position] = integration(vector_position, num_threads, a, b, num_trapezes);
}


int main(int argc, char* argv[]) {
	auto begin = std::chrono::high_resolution_clock::now();

	if (argc != 3) {
		if (argc == 2 && strcmp(argv[1], "-h") == 0) {
			std::cout << "HELP: This program accepts the total number of threads and trapezes with appropriate command-line"
				<< "arguments" << std::endl;
		}
		usage(argv[0]);
	}

	int num_threads = std::stoi(argv[1]);
	int num_trapezes = std::stoi(argv[2]);

	if (num_threads <= 0 || num_trapezes <= 0) {
		std::cout << "These should be positive integers, bigger than 0." << std::endl;
		exit(1);
	}

	// Integral description
	double a = 0.0;
	double b = 1.0;
	double total = 0.0;

	std::vector<std::thread> threads;
	std::vector<double> partial_integrals(num_threads);

	for (int i = 0; i < num_threads; i++) {
		threads.emplace_back(threadFunction, i, num_threads, a, b, num_trapezes, std::ref(partial_integrals));
	}

	for (auto& thread : threads) {
		thread.join();
	}

	for (double partial : partial_integrals) {
		total = total + partial;
	}
	
	std::cout << "The estimated integral with " << num_trapezes << " trapezes is: " << total << std::endl;

	auto end = std::chrono::high_resolution_clock::now();

	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
	std::cout << "Execution time: " << elapsed.count() << " nanoseconds" << std::endl;

	return 0;
}