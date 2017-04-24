#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include "mpi.h"
#include "main.h"


int main(int argc, char **argv) {
	int rank, size;
//	int root = 0;

	MPI_Comm comm;
	int dim[2], period[2];
//	int coord[2];

	int neighbours[5];

	//int intInput;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Barrier(MPI_COMM_WORLD);

	dim[0] = 4;		// X dimension
	dim[1] = 15;			// Y dimension
	period[0] = 0, period[1] = 0;

	MPI_Cart_create(MPI_COMM_WORLD, 2, dim, period, 0, &comm);

	if (rank != 0) {
//		printf("%d:\t", rank);

		sendNeighbours(comm, neighbours);
	} else {
		for (int i = 0; i < size-1; i++) {
			MPI_Recv(neighbours, 5, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			printf("%d: %d %d %d %d\n", neighbours[0], neighbours[1], neighbours[2], neighbours[3], neighbours[4]);fflush(stdout);
		}
	}


	MPI_Barrier(MPI_COMM_WORLD);
		    
	MPI_Finalize();
	return 0;
}

void sendNeighbours(MPI_Comm communicator, int destination[5]) {
	int source;

	MPI_Comm_rank(MPI_COMM_WORLD, &destination[0]);

	MPI_Cart_shift(communicator, 0, 1, &source, &destination[1]);

	MPI_Cart_shift(communicator, 1, 1, &source, &destination[2]);

	MPI_Cart_shift(communicator, 0, -1, &source, &destination[3]);

	MPI_Cart_shift(communicator, 1, -1, &source, &destination[4]);

	MPI_Send(destination, 5, MPI_INT, 0, 0, MPI_COMM_WORLD);

}
//void printNeighbours(MPI_Comm communicator) {
//	int x, y;
//	for(x = 0; x<2; x++) {
//		for(y = 0; y<2; y++) {
//			int source, destination;
//			MPI_Cart_shift(communicator, x, y, &source, &destination);
//			printf("%d\t", destination);
//		}
//	}
//	printf("\n");
//
//}
