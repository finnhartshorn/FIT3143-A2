#include "main.h"

#define MAXITERATIONS 10000
#define XDIM 15
#define YDIM 4
#define SENSORTHRESHOLD 20
#define OUTPUTFILE "output.txt"
#define VERBOSE 0


int main(int argc, char **argv) {
	int rank, size, flag;
	int root = XDIM*YDIM;

	int event_counter, message_counter;
	FILE *fp;

	MPI_Comm comm;
	MPI_Request req;
	int period[2] = {0,0};
	int dim[2] = { YDIM, XDIM };
	int coord[2], temp[2];

	int iteration = 1;

	int result, north, east, south, west;

	double startTime, endTime;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Cart_create(MPI_COMM_WORLD, 2, dim, period, 0, &comm);

	if (rank == root) {
		if (VERBOSE) {
			int results[60] = {0};
		}
		fp = fopen(OUTPUTFILE, "w");
		if (fp == NULL) {
			printf("Error: Unable to open file\n");
			return 1;
		}
		startTime = MPI_Wtime();
	}

	// Uses PCG random number generator http://www.pcg-random.org/
	pcg32_random_t rng;
	uint64_t seeds[2];
	entropy_getbytes((void*)seeds, sizeof(seeds));			// Entropy gets some random bytes which are random enough for initialization

	pcg32_srandom_r(&rng, seeds[0]^rank, seeds[1]^rank);	// Xor with rank to get unique seed for each process



	srand(time(NULL) + rank);    // Seed random number generation with current time + rank to make sure each process produces different random numbers
	message_counter = event_counter = 0;

	while (iteration <= MAXITERATIONS) {

		result = north = east = south = west = 0;
		if (rank != root) {
			MPI_Cart_coords(comm, rank, 2, coord);
			result = pcg32_boundedrand_r(&rng, 100) + 1;			// Produces a random number from 1-100
			if (result > SENSORTHRESHOLD) {				// If a node generates a value above the threshold, send to all adjacent neighbours
				if (coord[0] < YDIM-1) {			// Edge and corner nodes don't attempt to send messages outside the array
					send_neighbour(SOUTH, &result, comm);	// Sends are non blocking to prevent deadlocks
					message_counter++;
				}
				if (coord[0] > 0) {
					send_neighbour(NORTH, &result, comm);
					message_counter++;
				}
				if (coord[1] < XDIM-1) {
					send_neighbour(EAST, &result, comm);
					message_counter++;
				}
				if (coord[1] > 0) {
					send_neighbour(WEST, &result, comm);
					message_counter++;
				}
			}

			MPI_Barrier(MPI_COMM_WORLD);				// Barriers are used to make sure all nodes are in sync

			if (coord[0] > 0) {					// Nodes only send messages if they generate an event, so each node checks to see if there are
				try_receive_neighbour(NORTH, &north, comm);	//   any messages from its neighbours by using MPI_probe
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

			if (coord[1] == XDIM-2) {							// Nodes that are next to the right edge
				if (coord[0] > 0 && north > SENSORTHRESHOLD) { 				// Sends north value east if it is over the threshold
					send_neighbour(EAST, &north, comm);
					message_counter++;
				} else if (coord[0] == 0 && south > SENSORTHRESHOLD) { 			// Uses south value if node has no north neighbour
					send_neighbour(EAST, &south, comm);
					message_counter++;
				}
			}

			if (coord[1] == 1) {								// Nodes that are next to the left edge
				if (coord[0] > 0 && north > SENSORTHRESHOLD) { 				// Sends north value west if it is over the threshold
					send_neighbour(WEST, &north, comm);
					message_counter++;
				} else if (coord[0] == 0 && south > SENSORTHRESHOLD) { 			// Sends south value west if it is over the threshold
					send_neighbour(WEST, &south, comm);
					message_counter++;
				}
			}

			if (coord[0] == YDIM-2) {							// Nodes that are next to the lower edge
				if ((coord[1] == XDIM-1 || coord[1] == 0) && north > SENSORTHRESHOLD) {	// The two outer nodes send their Northern nodes south
					send_neighbour(SOUTH, &north, comm);
					message_counter++;
				} else if (west > SENSORTHRESHOLD) {                    		// Sends west value south if it is over the threshold
					send_neighbour(SOUTH, &west, comm);
					message_counter++;
				}
			}

			if (coord[0] == 1) {        							// Nodes that are next to the upper edge
				if ((coord[1] == XDIM-1 || coord[1] == 0) && south > SENSORTHRESHOLD) {
					send_neighbour(NORTH, &south, comm);
					message_counter++;
				} else if (west > SENSORTHRESHOLD) {
					send_neighbour(NORTH, &west, comm);            			// Sends west value north if it is over the threshold
					message_counter++;
				}
			}
			MPI_Barrier(MPI_COMM_WORLD);
			// Edge and corner nodes receive results via proxy
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
			// If all neighbours and the nodes own result are above the threshold, send a report to the base station.
			if (result > SENSORTHRESHOLD && north > SENSORTHRESHOLD && east > SENSORTHRESHOLD && south > SENSORTHRESHOLD && west > SENSORTHRESHOLD) {
				event_counter++;
				int temp[2]={rank, result};
				MPI_Isend(temp, 2, MPI_INT, root, 0, MPI_COMM_WORLD, &req);
			}

			MPI_Barrier(MPI_COMM_WORLD);


		} else if (rank == root) {
			int results[60] = {0};
			MPI_Barrier(MPI_COMM_WORLD);			// The base station is idle whilst the nodes are confering
			MPI_Barrier(MPI_COMM_WORLD);
			MPI_Barrier(MPI_COMM_WORLD);
			MPI_Barrier(MPI_COMM_WORLD);
			fflush(stdout);
			event_counter = 0;

			MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);		// The number of events is unknown so the base station checks 
														//   if there are messages to receive and receives until none are left
			while (flag) {
				event_counter++;
				int temp[2];
				MPI_Recv(temp, 2, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
				if (VERBOSE) {
					results[temp[0]] = 1;
				}
			}
			if (VERBOSE) {										// If the VERBOSE flag is set to true, a graph of what nodes reported events is shown for each iteration
				for (int i = 0; i < 60; i++) {
					if (i % 15 == 0) {
						fprintf(fp, "\n");
					}
					fprintf(fp, "%d ", results[i]);
				}
				fprintf(fp, "\n");

				fprintf(fp, "Finished iteration %d. %d events received.\n", iteration, event_counter);
			}


		}

		MPI_Barrier(MPI_COMM_WORLD);
		iteration++;
	}
	// The rest of the codes is solely for calculating some statistics after all iterations are finished.
	if (rank == root) {
		endTime = MPI_Wtime();
	}

	if (rank != root) {
		temp[0] = message_counter;
		temp[1] = event_counter;
		MPI_Send(temp, 2, MPI_INT, root, 0, MPI_COMM_WORLD);
	}

	if (rank == root) {
		int stats[2] = {0,0};
		for (int i = 0; i < XDIM*YDIM; i++) {
			MPI_Recv(temp, 2, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			stats[0] += temp[0];
			stats[1] += temp[1];
		}
		// To avoid dividing by zero
		if (stats[1] == 0) {
			stats[1] = 1;
		}
		if (iteration == 0) {
			iteration = 1;
		}

		fprintf(fp, "Sensor threshold: %d\n", SENSORTHRESHOLD);
		fprintf(fp, "Total time: %f\nAverage time over %d iterations: %f\n", endTime-startTime, iteration-1, (endTime-startTime)/(iteration-1));
		fprintf(fp, "Total adjacent messages: %d\nTotal events: %d\nAverage messages per event: %d\n", stats[0], stats[1], stats[0]/stats[1]);
		fprintf(fp, "Messages per iteration: %d", stats[0]/(iteration-1));
	}

	MPI_Finalize();
	return 0;
}
