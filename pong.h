/* Copyright 2023 Collin Bollinger 
 *
 * pong.h 
 */
 
#ifndef PONG_SIMULATION_H
#define PONG_SIMULATION_H

#include "mbed.h"
#include "platform/mbed_thread.h"
#include "stdio.h"
#include <math.h>
#include <random>
#include <ctime>

#define M_PI 3.14159265358979323846

#define BEGIN_VELOCITY 10.0 // Starting velocity of the ball. Change as necessary

#define HEIGHT_PX       32 // Height of the rgb matrix in pixels
#define WIDTH_PX        64 // Width of the rgb matrix in pixels

#define PADDLE_HEIGHT 8 // The number of pixels that make up the paddle. Change as necessary, cannot exceed HEIGHT / 2 [ceil]

// GPIO register structure
typedef struct {
    uint32_t PDOR; // GPIO output data
    uint32_t PSOR; // Set PDOR bits
    uint32_t PCOR; // Clear PDOR bits
    uint32_t PTOR; // Toggle PDOR bits
    uint32_t PDIR; // Data in from GPIO
    uint32_t PDDR; // Specifies pins as I/O
} GPIO_TypeDef;

// Ball struct stores ball properties
struct Ball {
  float x_pos; // x_pos and y_pos are the simulation coordinates of the ball
  float y_pos;
  float x_slope; // x_slope and y_slope make up the ball's velocity vector
  float y_slope;

  uint8_t x_int; // Rounded position coordinates
  uint8_t y_int;

  uint8_t color;
};

// Paddle struct store paddle properties
struct Paddle {
    uint8_t y_idx; 
    uint8_t y_prev;
};

// 2D array of pixels values
//extern uint8_t **rgb_matrix;

extern Mutex mtx; // Declaration of the mutex

extern Ball *ball;
extern Paddle *leftPaddle, *rightPaddle;


 /* Takes a floating point value in a floating point range and calculates a mapped integer value between
 * a new set of integer bounds */
int mapValue_i(float x, float in_min, float in_max, int out_min, int out_max);

/* Takes a floating point value in a floating point range and calculates a mapped floating point value between
 * a new set of floating point bounds */
float mapValue_f(float x, float in_min, float in_max, float out_min, float out_max);

/* Return the magnitude of a 2D vector */
float vectorMagnitude(float x, float y);

/* Reads analog data from the potentiometers and updates the location of the paddle in simulation.
 * If practiceMode == true, this function will not read from the right potentiometer. */
void updatePaddlePositions();

/* Increments the magnitude of the ball's direction vector by inc */
void incrementSpeed(float inc);

/* Gets the y-intercept of the ball's direction and x */
float findBallIntercept(float x);

/* This will reset the point after losing a point. Also called for the initial point to start as well.
 * Waits for waitFor milliseconds before resetting the point, as a way to allow you to see the end of the point */
void reset_point(uint16_t waitFor);

/* The function updateBallState is used to refresh the ball properties. This function 
 * calculates the location of the ball after time step d_t. Collisions with edges will flip
 * the balls direction on that axis. This function is meant to be called internally from simulation_step() function.
 * All collisions are handled inside of this function. Reset_point is called from here when the ball misses a 
 * paddle after a collision */
void updateBallState();

/* The function simulation_step is used to refresh the pong simulation. This involves tracking
 * the delta time between refreshes, updating the properties of the ball, and updating the paddles.
 * All this function does internally is recalculated d_t and call the updateBallState function. This
 * is important because updateBallState relies on an accurate value of d_t.
 * This functions is meant to be called interally from simulation() function */
void simulation_step();

/* The function simulate() loops the simulation step function. This function is run in its own thread from main. 
 * The simulate function is the only function that needs to be run in order for the whole thread to run on its own*/
void simulate();

#endif


#ifndef RGB_MATRIX_H
#define RGB_MATRIX_H

#include <climits>
#include <cstdint>

#define COLOR_RED 4
#define COLOR_BLUE 2
#define COLOR_GREEN 1
#define COLOR_YELLOW COLOR_RED | COLOR_GREEN
#define COLOR_TEAL COLOR_GREEN | COLOR_BLUE
#define COLOR_MAGENTA COLOR_BLUE | COLOR_RED
#define COLOR_WHITE COLOR_RED | COLOR_BLUE | COLOR_GREEN

#define GPIOC_BASE      0x400FF080UL // GPIO port register addresses for Port C

/* This function is used to write pixels to the screen. The screen can only display 2 rows at once. This function
 * loops through the rows and writes them to the screen. This is done is a separate thread so that the delay between
 * each row would be uniform. The rgb_matrix_function function is run in its own thread by main */
void rgb_matrix_function();

#endif
