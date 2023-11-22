/* Copyright 2023 Collin Bollinger */

/*
 * pong_simulation.h 
 *
 */
 
#ifndef PONG_SIMULATION_H
#define PONG_SIMULATION_H

#include "mbed.h"
#include "platform/mbed_thread.h"
#include "stdio.h"
#include <math.h>
#include <random>
#include <ctime>

#include "rgb_matrix.h"

#define WIDTH 64 // Simulation grid width
#define HEIGHT 32 // Simulation grid height
#define BEGIN_VELOCITY 100.0 // Starting velocity of the ball. Change as necessary
#define PADDLE_HEIGHT 3 // The number of pixels that make up the paddle. Change as necessary, cannot exceed HEIGHT / 2 [ceil]

// 2D array of pixels values
extern uint8_t **rgb_matrix;

/* The function simulation_step is used to refresh the pong simulation. This involves tracking
 * the delta time between refreshes, updating the properties of the ball, and updating the paddles.
 * All this function does internally is recalculated d_t and call the updateBallState function. This
 * is important because updateBallState relies on an accurate value of d_t.
 * This functions is meant to be called interally from pong_simulation.
 */
void simulation_step();


/* The function updateBallState is used to refresh the ball parameter's properties. This function 
 * calculates the location of the ball after time step d_t. Collisions with edges results in
 * the negation of the corresponding component of the velocity vector. Any new pixel locations
 * will be turned on and any pixels left will be turned off. 
 * This function is meant to be called internally from pong_simulation.
 */
void updateBallState();


/* The functions simulate will forever loop the simulation step and paddle adjustments.
 * This functions also handles any intialization steps for the pong_simulation 
 */
void simulate();

#endif