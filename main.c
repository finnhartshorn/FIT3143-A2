#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>

#include "mpi.h"

#include "main.h"


int main(int argc, char **argv) {
	int rank, size;
//	int root = 0;

	MPI_Comm comm;
	int dim[2], period[2];
//	int coord[2];

	int tuple[2];

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Barrier(MPI_COMM_WORLD);

	dim[0] = 4;		// y dimension
	dim[1] = 15;			// x dimension
	period[0] = 0, period[1] = 0;

	MPI_Cart_create(MPI_COMM_WORLD, 2, dim, period, 0, &comm);

	if (rank != 0) {
		srand(time(NULL) + rank);	// Seed random number generation with current time + rank to make sure each process produces different random numbers
		tuple[0] = rank;
		tuple[1] = rand() % 100 + 1;		// Produces a random number from 1-100
		MPI_Send(tuple, 2, MPI_INT, 0, 0, MPI_COMM_WORLD);

	} else {
		for (int i = 0; i < size-1; i++) {
			MPI_Recv(tuple, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			printf("%d: %d\n", tuple[0], tuple[1]);fflush(stdout);
		}
	}


	MPI_Barrier(MPI_COMM_WORLD);
		    
	MPI_Finalize();
	return 0;
}
