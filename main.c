#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "mpi.h"


int main(int argc, char **argv) {
	int rank, size;
	int root = 0;

	int intInput;


	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Barrier(MPI_COMM_WORLD);

	if (rank == 0) {
		for (int j = 0; j < (size-1); j++) {
			MPI_Recv(&intInput, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			printf("%d\n", intInput);
			fflush(stdout);
	        }
	} else {
		MPI_Send(&rank, 1, MPI_INT, root, 0, MPI_COMM_WORLD);
		fflush(stdout);
	}
	MPI_Barrier(MPI_COMM_WORLD);
		    
	MPI_Finalize();
	return 0;
}
