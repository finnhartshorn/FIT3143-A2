//
// Created by finnethen on 9/05/17.
//

#include "send_receive.h"
#include "main.h"

void try_receive_neighbour(Direction direction, int *buffer, MPI_Comm comm) {
	MPI_Request req;
	int source, destination;
	if (direction == NORTH) {
		MPI_Cart_shift(comm, 0, -1, &source, &destination);
	} else if (direction == SOUTH) {
		MPI_Cart_shift(comm, 0, 1, &source, &destination);
	} else if (direction == EAST) {
		MPI_Cart_shift(comm, 1, 1, &source, &destination);
	} else if (direction == WEST) {
		MPI_Cart_shift(comm, 1, -1, &source, &destination);
	}

	int flag = 0;
	MPI_Iprobe(destination, 0, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
	if (flag) {
		MPI_Irecv(buffer, 1, MPI_INT, destination, 0, MPI_COMM_WORLD, &req);
	}
}

void send_neighbour(Direction direction, int *buffer, MPI_Comm comm) {
	MPI_Request req;
	int source, destination;
	if (direction == NORTH) {
		MPI_Cart_shift(comm, 0, -1, &source, &destination);
	} else if (direction == SOUTH) {
		MPI_Cart_shift(comm, 0, 1, &source, &destination);
	} else if (direction == EAST) {
		MPI_Cart_shift(comm, 1, 1, &source, &destination);
	} else if (direction == WEST) {
		MPI_Cart_shift(comm, 1, -1, &source, &destination);
	}
	MPI_Isend(buffer, 1, MPI_INT, destination, 0, MPI_COMM_WORLD, &req);
}
