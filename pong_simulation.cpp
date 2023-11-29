/* Copyright 2023 Collin Bollinger 
 *
 * pong_simulation.cpp 
*/
 
#include "pong.h"

AnalogIn Left(PTB0);
AnalogIn Right(PTB1);
AnalogIn seed(PTB2);

// d_t: time elapsed since last time step in the simulation
float d_t = 0.0;

// Used to calculate d_t
float prev_time = 0.0;
float current_time = 0.0;

// timer used for meauring time differences
Timer timer;

// Ball and paddle instances
Ball *ball;
Paddle *leftPaddle, *rightPaddle;

// Mutex used for accessing the shared variables
Mutex mtx;

void updatePaddlePositions(){

    ScopedLock<Mutex> lock(mtx); // Lock mutex in this scope

    // Update the stored previous location
    leftPaddle->y_prev = leftPaddle->y_loc;
    rightPaddle->y_prev = rightPaddle->y_loc;

    // Read the analog voltage from potentiometer and map between the minimum
    // and maximum paddle positions [0, HEIGHT_PX - PADDLE_HEIGHT]. Store the 
    // result to the paddle structs
    leftPaddle->y_loc = mapValue(Left.read(), 0.0, 1.0, 0, HEIGHT_PX - PADDLE_HEIGHT);
    rightPaddle->y_loc = mapValue(Right.read(), 0.0, 1.0, 0, HEIGHT_PX - PADDLE_HEIGHT);
}

int mapValue(float x, float in_min, float in_max, int out_min, int out_max) {
    // Perform mapping using floating-point arithmetic
    return static_cast<int>((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min + 0.5);
}

float vectorMagnitude(float x, float y){
    // Calculate magnitude of vector
    return sqrt(pow(x, 2) + pow(y, 2));
}

void incrementSpeed(float inc){
    // Calculate a 2d vector of magnitude "inc" and the same direction a the ball,
    // and then add it to the ball's velocity
    ScopedLock<Mutex> lock(mtx);
    ball->x_slope += inc*(ball->x_slope/vectorMagnitude(ball->x_slope, ball->y_slope));
    ball->y_slope += inc*(ball->y_slope/vectorMagnitude(ball->x_slope, ball->y_slope));
}

uint8_t findPaddleIntercept(){

    ScopedLock<Mutex> lock(mtx); // Lock mutex in this scope

    // Find which paddle was collided with and then use y=mx+b to find the y intercept location
    if(ball->x_pos <= 1){ // Left side collision
        float b = ball->y_pos - (ball->y_slope/ball->x_slope) * ball->x_pos;
        return static_cast<uint8_t>((ball->y_slope/ball->x_slope) + b + 0.5); // Find y at x = 1 using y = mx + b form
    } else {
        float b = ball->y_pos - (ball->y_slope/ball->x_slope) * ball->x_pos;
        return static_cast<uint8_t>((ball->y_slope/ball->x_slope) * WIDTH_PX + b + 0.5); // Find y at x = 1 using y = mx + b form
    }
}

void reset(uint16_t waitFor){

    // Stop the timer so there isn't a massive d_t meaured after sleeping
    timer.stop();

    // Wait before resetting
    thread_sleep_for(waitFor); 

    ScopedLock<Mutex> lock(mtx); // Lock mutex in this scope

    // Reset ball color to white
    ball->color = COLOR_WHITE;

    // Place ball in center position
    ball->y_pos = static_cast<float>(HEIGHT_PX) / 2;
    ball->x_pos = static_cast<float>(WIDTH_PX) / 2;

    // Set new discrete coordinates for the ball 
	ball->x_int = (uint8_t)(ball->x_pos + 0.5);
	ball->y_int = (uint8_t)(ball->y_pos + 0.5);

    // Trying to generate random ball direction between -45 and 45 degrees. Might be a problem here.
    ball->y_slope = (rand() % static_cast<int>((2*sin(45) + 1) * 1000) - 1000*sin(45)) / 1000 * BEGIN_VELOCITY;

    // Calculate a corresponding x_slope
    ball->x_slope = sqrt(pow(BEGIN_VELOCITY, 2) - pow(ball->y_slope, 2));

    // Flip the x direction with a 50% chance to send the ball in either direction randomly
    if(rand() % 2 == 0) ball->x_slope = -ball->x_slope;

    // Read intial state of the sliders
    leftPaddle->y_loc = leftPaddle->y_prev = mapValue(Left.read(), 0.0, 1.0, 0, HEIGHT_PX - PADDLE_HEIGHT);
    rightPaddle->y_loc = rightPaddle->y_prev = mapValue(Right.read(), 0.0, 1.0, 0, HEIGHT_PX - PADDLE_HEIGHT);

    //ball->y_slope = 0.0;

    // Resume the timer 
    timer.start();
}

void updateBallState(){
	
    ScopedLock<Mutex> lock(mtx);

	// Move ball by the distance calculated from time step d_t and velocity vector
	ball->x_pos += d_t * ball->x_slope;
	ball->y_pos += d_t * ball->y_slope;

    if (ball->y_pos >= HEIGHT_PX - 1) { // Top out of bounds collision
        ball->y_pos = 2*HEIGHT_PX - ball->y_pos - 2;
        ball->y_slope *= -1;
    } else if (ball->y_pos <= 0) { // Bottom out of bounds collision
        ball->y_pos = abs(ball->y_pos);
        ball->y_slope *= -1;
    }

	// Check the coordinates and readjust them to stay within bounds
    if (ball->x_pos >= WIDTH_PX - 2) { // Right out of bounds
        
        uint8_t y = findPaddleIntercept(); // Get the y intercept location

        // Check if paddle is there
        if(y <= rightPaddle->y_loc + PADDLE_HEIGHT - 1 & y >= rightPaddle->y_loc){ // Paddle is there

            ball->x_pos = 2*WIDTH_PX - ball->x_pos - 4; // Bounce back normally
            ball->x_slope *= -1;
            incrementSpeed(2);
        } else { // Paddle isn't there
            ball->x_pos = WIDTH_PX - 1; // Set location to intercept and velocity to 0. change color to red
            ball->y_pos = y;
            ball->x_slope = ball->y_slope = 0;
            ball->color = COLOR_RED;
        }
    } else if (ball->x_pos <= 1) { // Left out of bounds

        uint8_t y = findPaddleIntercept(); // Get the y-intercept location

        // Check if paddle is there
        if(y <= leftPaddle->y_loc + PADDLE_HEIGHT - 1 & y >= leftPaddle->y_loc){ // Paddle is there

            ball->x_pos = abs(ball->x_pos) + 1;
            ball->x_slope *= -1;
            incrementSpeed(2);
        } else { // Paddle isn't there
            ball->x_pos = 0; 
            ball->y_pos = y;
            ball->x_slope = ball->y_slope = 0;
            ball->color = COLOR_RED;
        }
    }

	// Set new discrete coordinates for the ball 
	ball->x_int = (uint8_t)(ball->x_pos + 0.5);
	ball->y_int = (uint8_t)(ball->y_pos + 0.5);
}

void simulation_step(){
	
	// Evaluate d_t from current time and previous time
    current_time = timer.read();
    d_t = current_time - prev_time;
    prev_time = current_time;

    // Update the location of the paddles
    updatePaddlePositions();

    // Update the ball velocity, direction, position, and color
    updateBallState();
}

void simulate(){

    // Seed the random number generator
    int rnd_seed = 0;
    for (int i = 0; i < 8; i++) {
        int bit = static_cast<int>(mapValue(seed.read(), 0.0, 1.0, 0, 255));
        rnd_seed = (rnd_seed | ((1 << i) & bit));
    }
    srand(rnd_seed);
	
	// Begin the timer for time tracking
	timer.start();


    mtx.lock(); // Lock Mutex while accessing ball struct

    // Allocate the ball struct
    ball = (Ball*) malloc(sizeof(Ball));
    // Allocate Paddles
    leftPaddle = (Paddle*) malloc(sizeof(Paddle));
    rightPaddle = (Paddle*) malloc(sizeof(Paddle));
	
    mtx.unlock();
    

    reset(0);

	// Main loop of this thread: 
	while(true){

        mtx.lock(); // Lock mutex to access ball slope for conditional

        // Checks if the ball is motionless. The ball is set motionless whenever a point has
        // ended so this is just checking if the point is still going.
        if(ball->x_slope == 0 & ball->y_slope == 0) { // Point is over

            mtx.unlock(); // Unlock the mutex after conditional and before sleep

            // Reset the point after waiting 3 seconds
            reset(3000);
        } else { // Point isn't over

            mtx.unlock(); // Unlock mutex

            // Step the simulator forward
		    simulation_step();
        }
		
		// Wait a millisecond between time steps to reduce strobing effects on rgb matrix
		thread_sleep_for(1);
	}
}
