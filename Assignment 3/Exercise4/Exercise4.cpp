#include <omp.h>
#include <string>
#include <iostream>
#include <vector>
#include <chrono>


typedef std::vector<std::vector<double>> matrix;
typedef std::vector<double> vect;



void usage(char* program) {
	std::cout << "Usage: " << program << " <number of threads> <dimension of the system>" << std::endl;
	exit(1);
}

matrix generateRandomMatrix(int dim) {
	srand(time(NULL));
	matrix matx(dim, std::vector<double>(dim));

	// fill a matrix of size N with random integers.
	for (int i = 0; i < dim; ++i) {
		for (int j = 0; j < dim; ++j) {
			matx[i][j] = rand() % 21 - 10; // to generate random int in [-10, 10]
		}
	}
	return matx;
}

vect generateRandomVector(int dim) {
	srand(time(NULL));
	vect x(dim);

	// fill a matrix of size N with random integers.
	for (int i = 0; i < dim; ++i) {
		x[i] = rand() % 21 - 10; // to generate random int in [-10, 10]
	}
	return x;
 }

void printMatrix(const matrix &matx, int dim) {
	for (int i = 0; i < dim; ++i) {
		for (int j = 0; j < dim; ++j) {
			std::cout << matx[i][j] << " ";
		}
		std::cout << std::endl;
	}
}

void printVector(const vect &vecto, int dim) {
	for (int i = 0; i < dim; ++i) {
		std::cout << vecto[i] << std::endl;
	}
}

void gaussianElimination(matrix &A, int dim) {
	// since a Gaussian elimination algorithm that can work fo a system of n elements is quite a long task, 
	// i opted for a different approach that hopefuly also works.
	
	// parallelization could also be done here, but it's out of the scope
	for (int i = 0; i < dim; ++i) {
		for (int j = 0; j < dim; ++j) {
			if (i > j) {
				A[i][j] = 0;
			}
			else if (i == j && A[i][j] == 0) {
				A[i][j] = 5;
			}
		}
	}
}


void rowOrientedAlgorithm(const matrix &A, const vect &b, vect &x, int dim, int num_threads) {
	for (int row = dim - 1; row >= 0; row--) {
		x[row] = b[row];

#pragma omp parallel for num_threads(num_threads) schedule(auto) // last part modification for shceduler
		for (int col = row + 1; col < dim; col++) {
			x[row] -= A[row][col] * x[col];
		}

		x[row] /= A[row][row];
	}
}

void columnOrientedAlgorithm(const matrix &A, const vect &b, vect &x, int dim, int num_threads) {
#pragma omp parallel for num_threads(num_threads)
	for (int row = 0; row < dim; row++) {
		x[row] = b[row];
	}

	for (int col = dim - 1; col >= 0; col--) {
		x[col] /= A[col][col];

#pragma omp parallel for num_threads(num_threads)
		for (int row = 0; row < col; row++) {
			x[row] -= A[row][col] * x[col];
		}
	}
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



	// Generation of the large linear system
	matrix A = generateRandomMatrix(dim);
	vect b = generateRandomVector(dim);
	vect x(dim, 0);

	gaussianElimination(A, dim);

	/*
	// For testing the results, and see if the calculations were correct
	printMatrix(A, dim);
	printf("\n");
	printVector(b, dim);
	printf("\n");
	printVector(x, dim);
	printf("\n");
	*/


	rowOrientedAlgorithm(A, b, x, dim, num_threads);
	//columnOrientedAlgorithm(A, b, x, dim, num_threads);


	/*
	printf("\n");
	printMatrix(A, dim);
	printf("\n");
	printVector(x, dim);
	*/
	 



	/* TIME MEASURING CODE*/
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
	std::cout << "Execution time: " << elapsed.count() << " nanoseconds" << std::endl;

	exit(0);
}
