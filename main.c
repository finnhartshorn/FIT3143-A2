#include "main.h"

#define MAXITERATIONS 10
#define XDIM 15
#define YDIM 4
#define SENSORTHRESHOLD 20


int main(int argc, char **argv) {
	int rank, size, flag;
	int root = 60;

	int event_counter;

	MPI_Comm comm;
	MPI_Request req;
//	MPI_Status status;
	int period[2] = {0,0};
	int coord[2];
	int dim[2] = { YDIM, XDIM };

	int iteration = 1;

	int result, north, east, south, west;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Cart_create(MPI_COMM_WORLD, 2, dim, period, 0, &comm);

	// TODO: Replace with better random number generator
	srand(time(NULL) + rank);    // Seed random number generation with current time + rank to make sure each process produces different random numbers

	while (iteration <= MAXITERATIONS) {
		event_counter = 0;
		result = north = east = south = west = 0;
		if (rank != root) {
			MPI_Cart_coords(comm, rank, 2, coord);
			result = rand() % 100 + 1;			// Produces a random number from 1-100//
			if (result > SENSORTHRESHOLD) {
				if (coord[0] < YDIM-1) {
					send_neighbour(SOUTH, &result, comm);
				}
				if (coord[0] > 0) {
					send_neighbour(NORTH, &result, comm);
				}
				if (coord[1] < XDIM-1) {
					send_neighbour(EAST, &result, comm);
				}
				if (coord[1] > 0) {
					send_neighbour(WEST, &result, comm);
				}
			}

			MPI_Barrier(MPI_COMM_WORLD);

			if (coord[0] > 0) {
				try_receive_neighbour(NORTH, &north, comm);
			}
			if (coord[0] < YDIM-1) {
				try_receive_neighbour(SOUTH, &south, comm);
			}
			if (coord[1] < XDIM-1) {
				try_receive_neighbour(EAST, &east, comm);
			}
			if (coord[1] > 0) {
				try_receive_neighbour(WEST, &west, comm);
			}

			MPI_Barrier(MPI_COMM_WORLD);

			if (coord[1] == XDIM-2) {											// Nodes that are next to the right edge
				if (coord[0] > 0 && north > SENSORTHRESHOLD) { 					// Sends north value east if it is over the threshold
					send_neighbour(EAST, &north, comm);
				} else if (coord[0] == 0 && south > SENSORTHRESHOLD) { 			// Uses south value if node has no north neighbour
					send_neighbour(EAST, &south, comm);
				}
			}

			if (coord[1] == 1) {												// Nodes that are next to the left edge
				if (coord[0] > 0 && north > SENSORTHRESHOLD) { 					// Sends north value west if it is over the threshold
					send_neighbour(WEST, &north, comm);
				} else if (coord[0] == 0 && south > SENSORTHRESHOLD) { 			// Sends south value west if it is over the threshold
					send_neighbour(WEST, &south, comm);
				}
			}

			if (coord[0] == YDIM-2) {											// Nodes that are next to the lower edge
				if ((coord[1] == XDIM-1 || coord[1] == 0) && north > SENSORTHRESHOLD) {			// The two outer nodes send their Northern nodes south
					send_neighbour(SOUTH, &north, comm);
				} else if (west > SENSORTHRESHOLD) {                    		// Sends west value south if it is over the threshold
					send_neighbour(SOUTH, &west, comm);
				}
			}

			if (coord[0] == 1) {        										// Nodes that are next to the upper edge
				if ((coord[1] == XDIM-1 || coord[1] == 0) && south > SENSORTHRESHOLD) {
					send_neighbour(NORTH, &south, comm);
				} else if (west > SENSORTHRESHOLD) {
					send_neighbour(NORTH, &west, comm);            					// Sends west value north if it is over the threshold
				}
			}
			MPI_Barrier(MPI_COMM_WORLD);
			// Receive
			if (coord[1] == 0) {
				try_receive_neighbour(EAST, &west, comm);
			}
			if (coord[1] == XDIM-1) {
				try_receive_neighbour(WEST, &east, comm);
			}
			if (coord[0] == 0) {
				try_receive_neighbour(SOUTH, &north, comm);
			}
			if (coord[0] == YDIM-1) {
				try_receive_neighbour(NORTH, &south, comm);
			}

			if (result > SENSORTHRESHOLD && north > SENSORTHRESHOLD && east > SENSORTHRESHOLD && south > SENSORTHRESHOLD && west > SENSORTHRESHOLD) {
				int temp[2]={rank, result};
				MPI_Isend(temp, 2, MPI_INT, root, 0, MPI_COMM_WORLD, &req);
			}

			MPI_Barrier(MPI_COMM_WORLD);


		} else if (rank == root) {
			int results[60] = {0};
			MPI_Barrier(MPI_COMM_WORLD);
			MPI_Barrier(MPI_COMM_WORLD);
			MPI_Barrier(MPI_COMM_WORLD);
			MPI_Barrier(MPI_COMM_WORLD);
			fflush(stdout);
			event_counter = 0;

			MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);

			while (flag) {
				event_counter++;
				int temp[2];
				MPI_Recv(temp, 2, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
				results[temp[0]] = 1;
			}
			for (int i = 0; i < 60; i++) {
				if (i % 15 == 0) {
					printf("\n");
				}
				printf("%d ", results[i]);
			}
			printf("\n");

			printf("Finished iteration %d. %d events received.\n", iteration, event_counter);fflush(stdout);

		}

		MPI_Barrier(MPI_COMM_WORLD);
		iteration++;
		// TODO: Gather more statistics on each iteration
	}

		MPI_Finalize();
		return 0;
}