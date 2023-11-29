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

#define BEGIN_VELOCITY 10.0 // Starting velocity of the ball. Change as necessary

#define COLOR_RED 4
#define COLOR_BLUE 2
#define COLOR_GREEN 1
#define COLOR_YELLOW COLOR_RED | COLOR_GREEN
#define COLOR_TEAL COLOR_GREEN | COLOR_BLUE
#define COLOR_MAGENTA COLOR_BLUE | COLOR_RED
#define COLOR_WHITE COLOR_RED | COLOR_BLUE | COLOR_GREEN

#define GPIOC_BASE      0x400FF080UL // GPIO port register addresses for Port C

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
    uint8_t y_loc; 
    uint8_t y_prev;
};

// 2D array of pixels values
//extern uint8_t **rgb_matrix;

extern Mutex mtx; // Declaration of the mutex

extern Ball *ball;
extern Paddle *leftPaddle, *rightPaddle;

// 2D array of pixels values
//extern uint8_t **rgb_matrix;

//extern Mutex mtx; // Declaration of the mutex

/* The function simulation_step is used to refresh the pong simulation. This involves tracking
 * the delta time between refreshes, updating the properties of the ball, and updating the paddles.
 * All this function does internally is recalculated d_t and call the updateBallState function. This
 * is important because updateBallState relies on an accurate value of d_t.
 * This functions is meant to be called interally from pong_simulation.
 */
void simulation_step();


/* The function updateBallState is used to refresh the ball properties. This function 
 * calculates the location of the ball after time step d_t. Collisions with edges will flip
 * the balls direction on that axis. This function is meant to be called internally from pong_simulation.
 */
void updateBallState();


/* The function simulate will forever loop the simulation step and paddle adjustments.
 * This functions also handles resetting the game after points.
 */
void simulate();


/* The function updatePaddlePositions reads analog data from the sliders and will update
 * the paddleL and paddleR indexes.
 */
void updatePaddlePositions();

/* The function mapValue is used to scale a value x between a different set of bounds
 */
int mapValue(float x, float in_min, float in_max, int out_min, int out_max);

/* Return the magnitude of a 2D vector
 */
float vectorMagnitude(float x, float y);

/* Gets the y intercept location of the paddle against the ball
 */
uint8_t findPaddleIntercept();

/* Reset the game. The waitFor parameter is how long the thread
 * will sleep until resetting the game. The score does not get reset.
 */
 void reset(uint16_t waitFor);

#endif


#ifndef RGB_MATRIX_H
#define RGB_MATRIX_H

#include <climits>

// Rename
void thread_function();


#endif
