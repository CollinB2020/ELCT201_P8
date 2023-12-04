/* Copyright 2023 Collin Bollinger 
 *
 * pong_simulation.cpp 
*/
 
#include "pong.h"

AnalogIn Left(PTB0);
AnalogIn Right(PTB1);
AnalogIn seed(PTB2);

// States used by the simulation
bool running = true;
bool practiceMode = true;

// Used for finding a change in time between screens
float d_t = 0.0;
float prev_time = 0.0;
float current_time = 0.0;

Timer timer;

// Ball and paddle instances
Ball *ball;
Paddle *leftPaddle, *rightPaddle;

// Mutex used for accessing the shared variables
Mutex mtx;
int mapValue_i(float x, float in_min, float in_max, int out_min, int out_max) { return static_cast<int>((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min + 0.5); }

float mapValue_f(float x, float in_min, float in_max, float out_min, float out_max) { return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min; }

float vectorMagnitude(float x, float y) { return sqrt(pow(x, 2) + pow(y, 2)); }

void updatePaddlePositions(){

    ScopedLock<Mutex> lock(mtx); // Lock mutex in this scope

    // Update the stored previous location
    leftPaddle->y_prev = leftPaddle->y_idx;
    rightPaddle->y_prev = rightPaddle->y_idx;

    // Read the analog voltage from potentiometer and map between the minimum
    // and maximum paddle positions [0, HEIGHT_PX - PADDLE_HEIGHT]. Store the 
    // result to the paddle structs
    leftPaddle->y_idx = mapValue_i(Left.read(), 0.0, 1.0, 0, HEIGHT_PX - PADDLE_HEIGHT);
    
    if(!practiceMode) rightPaddle->y_idx = mapValue_i(Right.read(), 0.0, 1.0, 0, HEIGHT_PX - PADDLE_HEIGHT);
    else { 
        int paddle_idx = ball->y_pos - (PADDLE_HEIGHT/2.0); // Find the y_loc desired to set the midpoint of the paddle horizontal to the ball
        // Bound the paddle_idx between [0, HEIGHT_PX - PADDLE_HEIGHT]
        rightPaddle->y_idx = paddle_idx < 0 ? 0 : (paddle_idx > (HEIGHT_PX - PADDLE_HEIGHT) ? (HEIGHT_PX - PADDLE_HEIGHT) : paddle_idx);
    }
}

void incrementSpeed(float inc){
    // Calculate a 2d vector of magnitude "inc" and the same direction a the ball,
    // and then add it to the ball's velocity
    ScopedLock<Mutex> lock(mtx); // Lock mutex while accessing ball shared variable
    ball->x_slope += inc*(ball->x_slope/vectorMagnitude(ball->x_slope, ball->y_slope));
    ball->y_slope += inc*(ball->y_slope/vectorMagnitude(ball->x_slope, ball->y_slope));
}

float findBallIntercept(float x){

    ScopedLock<Mutex> lock(mtx); // Lock mutex in this scope

    // Use y=mx+b to find the y intercept location
    float b = ball->y_pos - (ball->y_slope/ball->x_slope) * ball->x_pos;
    return (ball->y_slope/ball->x_slope)*x + b + 0.5; // Find y at x = 1 using y = mx + b form
}

void reset_point(uint16_t waitFor){

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

    // Trying to generate random ball direction between -45 and 45 degrees
    ball->y_slope = mapValue_f(rand()%255, 0, 254, -0.5*BEGIN_VELOCITY, 0.5*BEGIN_VELOCITY);

    // Calculate a corresponding x_slope
    ball->x_slope = sqrt(pow(BEGIN_VELOCITY, 2) - pow(ball->y_slope, 2));

    // Flip the x direction with a 50% chance for random starting direction
    if(rand() % 2 == 0) ball->x_slope = -ball->x_slope;

    // Read intial state of the sliders after reset
    leftPaddle->y_idx = leftPaddle->y_prev = mapValue_i(Left.read(), 0.0, 1.0, 0, HEIGHT_PX - PADDLE_HEIGHT);
    rightPaddle->y_idx = rightPaddle->y_prev = mapValue_i(Right.read(), 0.0, 1.0, 0, HEIGHT_PX - PADDLE_HEIGHT);

    // Resume the timer 
    timer.start();
}

void updateBallState(){
	
    //ScopedLock<Mutex> lock(mtx); // Lock mutex in this scope
    mtx.lock();

	// Move ball by the distance calculated from time step d_t and velocity vector
	ball->x_pos += d_t * ball->x_slope;
	ball->y_pos += d_t * ball->y_slope;

    // Collision detection for the top and bottom walls in the simulation
    if (ball->y_pos >= HEIGHT_PX) { // Top out of bounds collision
        ball->y_pos = 2*HEIGHT_PX - ball->y_pos - 2;
        ball->y_slope *= -1;
    } else if (ball->y_pos <= 0) { // Bottom out of bounds collision
        ball->y_pos = abs(ball->y_pos);
        ball->y_slope *= -1;
    }

	// Check the coordinates and readjust them to stay within bounds
    if (ball->x_pos >= WIDTH_PX - 1) { // Right out of bounds
        
        float y = findBallIntercept(WIDTH_PX - 1); // Get the y intercept of the ball's path and paddle
        uint8_t y_idx = mapValue_i(y, 0, HEIGHT_PX, 0, HEIGHT_PX - 1); // Map the simulation coordinates to matrix indexes

        // Check if paddle is there or not. The ball will either bounce or the point will end.
        if(y_idx <= (rightPaddle->y_idx + PADDLE_HEIGHT - 1) & y_idx >= rightPaddle->y_idx){ // Paddle is there
            ball->x_pos = 2*WIDTH_PX - ball->x_pos - 2; // Move ball inounds

            // Recaulculate the ball direction based on intercept location
            float v = vectorMagnitude(ball->x_slope, ball->y_slope);
            float k = v * M_PI / 4; // |v| * sin(45) // Max and min y_slope value
            ball->y_slope = ((mapValue_f(y, 0, HEIGHT_PX, 0, HEIGHT_PX - 1) - rightPaddle->y_idx) + 1) / (PADDLE_HEIGHT + 1.0) * 2 * k - k;
            ball->x_slope = -sqrt(pow(v, 2) - pow(ball->y_slope, 2));

            // Increase speed by 2 after changing direction
            incrementSpeed(2);
        } else { // Paddle isn't there
            ball->x_pos = WIDTH_PX - 0.5; // Set location to intercept and velocity to 0. change color to red
            ball->y_pos = y;
            ball->x_slope = ball->y_slope = 0;
            ball->color = COLOR_RED;
        }
    } else if (ball->x_pos <= 1) { // Left out of bounds

        float y = findBallIntercept(1); // Get the y intercept of the ball's path and paddle
        uint8_t y_idx = mapValue_i(y, 0, HEIGHT_PX, 0, HEIGHT_PX - 1); // Map the simulation coordinates to matrix indexes

        // Check if paddle is there
        if(y_idx <= (leftPaddle->y_idx + PADDLE_HEIGHT - 1) & y_idx >= leftPaddle->y_idx){ // Paddle is there
            ball->x_pos = abs(ball->x_pos) + 1; // Move ball inbounds

            // Recaulculate the ball direction based on intercept location
            float v = vectorMagnitude(ball->x_slope, ball->y_slope);
            float k = v * M_PI / 4; // |v| * sin(45)
            ball->y_slope = ((mapValue_f(y, 0, HEIGHT_PX, 0, HEIGHT_PX - 1) - leftPaddle->y_idx) + 1) / (PADDLE_HEIGHT + 1.0) * 2 * k - k;
            ball->x_slope = sqrt(pow(v, 2) - pow(ball->y_slope, 2));

            // Increase speed by 2 after changing direction
            incrementSpeed(2);
        } else { // Paddle isn't there
            ball->x_pos = 0.5; 
            ball->y_pos = y;
            ball->x_slope = ball->y_slope = 0;
            ball->color = COLOR_RED;
        }
    }

	// Set new discrete coordinates for the ball 
    ball->x_int = mapValue_i(ball->x_pos, 0, WIDTH_PX, 0, WIDTH_PX - 1);
    ball->y_int = mapValue_i(ball->y_pos, 0, HEIGHT_PX, 0, HEIGHT_PX - 1);

    // Reset the point if the ball has a slope vector of 0. Unlock mutex before resetting, but after accessing ball in conditional.
    if(ball->x_slope == 0 & ball->y_slope == 0) {
        mtx.unlock();
        reset_point(3000);
    } else { mtx.unlock(); }
}

void simulation_step(){
	
	// Evaluate d_t from current time and previous time
    current_time = timer.read();
    d_t = current_time - prev_time;
    prev_time = current_time;

    if(!running) return; // Don't step the simulator if the game is paused

    // Update the location of the paddles
    updatePaddlePositions();

    // Update the ball velocity, direction, position, and color
    updateBallState();
}

void simulate(){

    // Seed the random number generator
    int rnd_seed = 0;
    for (int i = 0; i < 8; i++) {
        int bit = mapValue_i(seed.read(), 0.0, 1.0, 0, 255);
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
    
    // Perform a point reset with a delay of 0
    reset_point(0);

	// Main loop of this thread: 
	while(true){

		simulation_step();
		
		// Wait a millisecond between simulation steps
		thread_sleep_for(1);
	}
}
