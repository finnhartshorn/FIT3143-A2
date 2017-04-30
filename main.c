#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "mpi.h"

#include "main.h"

#define MAXITERATIONS 10
#define XDIM 15
#define YDIM 4
#define SENSORTHRESHOLD 20


int main(int argc, char **argv) {
	int rank, size, source, destination, flag;
	int root = 60;

	int event_counter = 0;

	MPI_Comm comm;
	MPI_Request req;
//	MPI_Status status;
	int period[2], coord[2];
	int dim[2] = { XDIM, YDIM };

	int iteration = 1;

	int tuple[6];

//	int neighbours[5];

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	period[0] = 0, period[1] = 0;

	MPI_Cart_create(MPI_COMM_WORLD, 2, dim, period, 0, &comm);

	srand(time(NULL) + rank);    // Seed random number generation with current time + rank to make sure each process produces different random numbers

	while (iteration <= MAXITERATIONS) {
		memset(tuple, 0, sizeof(tuple));
		if (rank != root) {
			tuple[0] = rank;
			MPI_Cart_coords(comm, rank, 2, coord);
//			tuple[1] = coord[0];
//			tuple[2] = coord[1];
			tuple[1] = rand() % 100 + 1;		// Produces a random number from 1-100
			if (tuple[1] > SENSORTHRESHOLD) {
				if (coord[0] != XDIM) {
					MPI_Cart_shift(comm, 0, 1, &source, &destination);
					MPI_Isend(&tuple[1], 1, MPI_INT, destination, 0, MPI_COMM_WORLD, &req);
				}
				if (coord[0] != 0) {
					MPI_Cart_shift(comm, 0, -1, &source, &destination);
					MPI_Isend(&tuple[1], 1, MPI_INT, destination, 0, MPI_COMM_WORLD, &req);
				}

				if (coord[0] != YDIM) {
					MPI_Cart_shift(comm, 1, 1, &source, &destination);
					MPI_Isend(&tuple[1], 1, MPI_INT, destination, 0, MPI_COMM_WORLD, &req);
				}
				if (coord[1] != 0) {
					MPI_Cart_shift(comm, 1, -1, &source, &destination);
					MPI_Isend(&tuple[1], 1, MPI_INT, destination, 0, MPI_COMM_WORLD, &req);
				}
			}

			MPI_Barrier(MPI_COMM_WORLD);

			if (coord[0] != 0) {
				MPI_Cart_shift(comm, 0, 1, &source, &destination);
				attempt_receive_neighbour(destination, &tuple[2]);
			}

			if (coord[0] != XDIM) {
				MPI_Cart_shift(comm, 0, -1, &source, &destination);
				attempt_receive_neighbour(destination, &tuple[3]);
			}

			if (coord[0] != 0) {
				MPI_Cart_shift(comm, 1, 1, &source, &destination);
				attempt_receive_neighbour(destination, &tuple[4]);
			}
			if (coord[1] != YDIM) {
				MPI_Cart_shift(comm, 1, -1, &source, &destination);
				attempt_receive_neighbour(destination, &tuple[5]);
			}

			if (tuple[1] > SENSORTHRESHOLD && tuple[2] > SENSORTHRESHOLD && tuple[3] > SENSORTHRESHOLD && tuple[4] > SENSORTHRESHOLD && tuple[5] > SENSORTHRESHOLD) {
				MPI_Isend(tuple, 6, MPI_INT, root, 0, MPI_COMM_WORLD, &req);
			}
			MPI_Barrier(MPI_COMM_WORLD);
		} else if (rank == root) {
			MPI_Barrier(MPI_COMM_WORLD);
			MPI_Barrier(MPI_COMM_WORLD);
			fflush(stdout);
			event_counter = 0;

			MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);

			while (flag) {
				event_counter++;
				MPI_Recv(tuple, 6, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//				printf("%d: %d,%d,%d,%d, %d\n", tuple[0], tuple[1], tuple[2], tuple[3], tuple[4], tuple[5]);fflush(stdout);
				MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
			}

			printf("Finished iteration %d. %d events received.\n", iteration, event_counter);fflush(stdout);

		}

		MPI_Barrier(MPI_COMM_WORLD);
		iteration++;
	}

		MPI_Finalize();
		return 0;
}

void attempt_receive_neighbour(int destination, int *buffer) {
	MPI_Request req;
	int flag = 0;
	MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
	if (flag) {
		MPI_Irecv(buffer, 1, MPI_INT, destination, 0, MPI_COMM_WORLD, &req);
	}
}