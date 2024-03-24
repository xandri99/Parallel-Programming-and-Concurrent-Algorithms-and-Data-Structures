#include <omp.h>
#include <string>
#include <iostream>
#include <vector>
#include <chrono>



void usage(char* program) {
	std::cout << "Usage: " << program << " <number of threads> <dimension of the matrixes>" << std::endl;
	exit(1);
}

std::vector<std::vector<int>> generateRandomMatrix(int dim) {
	srand(time(NULL));
	std::vector<std::vector<int>> matrix(dim, std::vector<int>(dim));

	// fill a matrix of size N with random integers.
	for (int i = 0; i < dim; ++i) {
		for (int j = 0; j < dim; ++j) {
			matrix[i][j] = rand() % 100;
		}
	}
	return matrix;
}

void printMatrix(const std::vector<std::vector<int>> &matrix, int dim) {
	for (int i = 0; i < dim; ++i) {
		for (int j = 0; j < dim; ++j) {
			std::cout << matrix[i][j] << " ";
		}
		std::cout << std::endl;
	}
}

/* 
static scheduling of matrix multiplication loops 
This is the version provided in slide 16 of Lecture 7 (07-OpenMP.pdf). Only the function encapsulation has been added.
Only the outermost loop is parallelized.
*/
std::vector<std::vector<int>> matrixProductFirst(const std::vector<std::vector<int>> &a, \
												const std::vector<std::vector<int>> &b, int dim, int num_threads) {
	// Matrix to return
	std::vector<std::vector<int>> c(dim, std::vector<int>(dim));


	omp_set_num_threads(num_threads);
#pragma omp parallel default(private) shared (a, b, c, dim)
	{
#pragma omp for schedule(static)
		for (int i = 0; i < dim; i++) {
			for (int j = 0; j < dim; j++) {
				c[i][j] = 0;
				for (int k = 0; k < dim; k++) {
					c[i][j] += a[i][k] * b[k][j];
				}
			}
		}
	}
	return c;
}

/*
First modification, so that the outer two loops are parallelized.
*/
std::vector<std::vector<int>> matrixProductSecond(const std::vector<std::vector<int>>& a, \
												 const std::vector<std::vector<int>>& b, int dim, int num_threads) {
	// Matrix to return
	std::vector<std::vector<int>> c(dim, std::vector<int>(dim));


	omp_set_num_threads(num_threads);
#pragma omp parallel default(private) shared (a, b, c, dim)
	{
#pragma omp for schedule(static) collapse(2)
		for (int i = 0; i < dim; i++) {
			for (int j = 0; j < dim; j++) {
				c[i][j] = 0;
				for (int k = 0; k < dim; k++) {
					c[i][j] += a[i][k] * b[k][j];
				}
			}
		}
	}
	return c;
}

/*
First modification, so that all three loops are parallelized.
*/
std::vector<std::vector<int>> matrixProductThird(const std::vector<std::vector<int>>& a, \
												 const std::vector<std::vector<int>>& b, int dim, int num_threads) {
	// Matrix to return
	std::vector<std::vector<int>> c(dim, std::vector<int>(dim));


	omp_set_num_threads(num_threads);
#pragma omp parallel default(private) shared (a, b, c, dim)
	{
#pragma omp for schedule(static) collapse(2)
		for (int i = 0; i < dim; i++) {
			for (int j = 0; j < dim; j++) {
				c[i][j] = 0;
				for (int k = 0; k < dim; k++) {
					c[i][j] += a[i][k] * b[k][j];
				}
			}
		}
	}
	return c;
}






int main(int argc, char* argv[]) {
	/* TIME MEASURING CODE*/
	auto begin = std::chrono::high_resolution_clock::now();


	// Input variables
	if (argc != 3) {
		usage(argv[0]);
	}

	int num_threads = std::stoi(argv[1]);
	int dim = std::stoi(argv[2]);


	if (num_threads <= 0) {
		std::cout << "These should be positive integers, bigger than 0." << std::endl;
		exit(1);
	}



	// Actual matrix-matrix product code
	std::vector<std::vector<int>> a = generateRandomMatrix(dim);
	std::vector<std::vector<int>> b = generateRandomMatrix(dim);
	
	std::vector<std::vector<int>> c = matrixProductThird(a, b, dim, num_threads);

	//printMatrix(c, dim);

	
	/* TIME MEASURING CODE*/
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
	std::cout << "Execution time: " << elapsed.count() << " nanoseconds" << std::endl;

	exit(0);
}
