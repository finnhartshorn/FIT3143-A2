//
// Created by finnethen on 9/05/17.
//

#ifndef FIT3143_A2_SEND_RECEIVE_H
#define FIT3143_A2_SEND_RECEIVE_H

#include "main.h"

typedef enum {NORTH, EAST, SOUTH, WEST} Direction ;

void try_receive_neighbour(Direction, int *, MPI_Comm);
void send_neighbour(Direction direction, int *, MPI_Comm);

#endif //FIT3143_A2_SEND_RECEIVE_H
