/* Copyright 2023 Collin Bollinger */

/*
 * pong_simulation.cpp 
 *
 */
 
#include "pong_simulation.h"
 
// Ball struct stores ball properties
struct Ball {
  float x_pos; // x_pos and y_pos are the simulation coordinates of the ball
  float y_pos;
  float x_slope; // x_slope and y_slope make up the ball's velocity vector
  float y_slope;

  uint8_t x_discrete; // x_discrete and y_discrete are the rgb_matrix coordinates of the ball
  uint8_t y_discrete;
};

// Delta Time: time elapsed since last time step in the simulation
float d_t = 0.0;

// Used to calculate d_t
float prev_time = 0.0;
float current_time = 0.0;

// Main timer used to find d_t
Timer timer;

// The ball structure
Ball *ball;

// Paddle positions
uint8_t paddleL = 0, paddleR = 0;

void updateBallState(){
	
	// Move ball by the distance calculated from time step d_t and velocity vector
	ball->x_pos += d_t * ball->x_slope;
	ball->y_pos += d_t * ball->y_slope;

	// Check the simulation coordinates and readjust them to stay within bounds
    if (ball->x_pos >= WIDTH - 1) { // Right out of bounds
        ball->x_pos = 2*WIDTH - ball->x_pos - 2;
        ball->x_slope *= -1;
    } else if (ball->x_pos <= 0) { // Left out of bounds
        ball->x_pos = abs(ball->x_pos);
        ball->x_slope *= -1;
    }
    if (ball->y_pos >= HEIGHT - 1) { // Top out of bounds
        ball->y_pos = 2*HEIGHT - ball->y_pos - 2;
        ball->y_slope *= -1;
    } else if (ball->y_pos <= 0) { // Bottom out of bounds
        ball->y_pos = abs(ball->y_pos);
        ball->y_slope *= -1;
    }

	// New x_discrete and y_discrete
	uint8_t x_dis = (uint8_t)(ball->x_pos + 0.5);
	uint8_t y_dis = (uint8_t)(ball->y_pos + 0.5);

	// Check if discrete pixel location changed, and reset old pixel in rgb_matrix
	if((x_dis != ball->x_discrete) | (y_dis != ball->y_discrete)) rgb_matrix[(ball->y_discrete)][ball->x_discrete] = 0;
    
	// Set new discrete coordinates for the ball 
	ball->x_discrete = x_dis;
	ball->y_discrete = y_dis;

	// Write pixel to rgb_matrix as green
	rgb_matrix[(ball->y_discrete)][ball->x_discrete] = 1;
}

void simulation_step(){
	
	// Evaluate d_t from current time and previous time
    current_time = timer.read();
    d_t = current_time - prev_time;
    prev_time = current_time;

    // Update the ball properties between refreshes using d_t
    updateBallState();
    //fault caused by update ball state
}

void simulate(){
	
	// Begin the timer for time tracking
	timer.start();

    // Allocate the ball struct
    ball = (Ball*) malloc(sizeof(Ball));
	
	// Seed the random number generator
    std::srand(std::time(nullptr));

    // Calculate slope vectors for a slope of 1 with the target velocity
    // sqrt(pow(BEGIN_VELOCITY, 2)/2.0) is a vector with one slope
    (*ball).y_slope = (rand() % static_cast<int>((sqrt(pow(BEGIN_VELOCITY, 2)/2.0))*100)) / 100.0; // Randomize y_slope between 0 deg and 45 deg. 

    // Calculate a corresponding x vector to acheive the begin velocity
    (*ball).x_slope = sqrt(pow(BEGIN_VELOCITY, 2) - pow((*ball).y_slope, 2));

    // Place ball in center position
    (*ball).y_pos = static_cast<float>(HEIGHT) / 2;
    (*ball).x_pos = static_cast<float>(WIDTH) / 2;
	
	// Loop the simulation continuously 
	while(true){
		
		// Step the simulator forward
		simulation_step();
		
		// Wait a millisecond between time steps
		thread_sleep_for(1);
	}
}