/* Copyright 2023 Collin Bollinger
 *
 *	rgb_matrix.cpp
 */
 
#include "pong.h"
 
// Set the inputs and outputs for the rgb_matrix header pins
DigitalOut R1(PTC13);
DigitalOut R2(PTC10);
DigitalOut G1(PTC12);
DigitalOut G2(PTC9);
DigitalOut B1(PTC11);
DigitalOut B2(PTC8);
DigitalOut A(PTC3);
DigitalOut B(PTC4);
DigitalOut C(PTC5);
DigitalOut D(PTC6);
DigitalOut E(PTC7);
DigitalOut OE(PTC1);
DigitalOut CLK(PTC2);
DigitalOut LAT(PTC0);
 
// GPIO port C instance
volatile GPIO_TypeDef *GPIOC = reinterpret_cast<GPIO_TypeDef*>(GPIOC_BASE);

void rgb_matrix_function(){

    // Sleep for 10 milliseconds to ensure that the pong simulation thread has
    // allocated the ball and paddle structs and everything
    thread_sleep_for(10);
	 
	// Clear 13 GPIOC bits being used except for OE'
    GPIOC->PCOR = 16381;

    // Counter used for generating rows
    uint32_t row = 0;



    // Main thread loop. This loops scans over rows and columns to output one full screen per loop.
    for (;;) {


        // The rgb matrix has 16 row select values. Each row select value displays 2 different rows. Row_counter counts from
        // 0 to 16. Row_counter is used to generate two rows numbers, row_1 and row_2.
        for(uint8_t row_counter = 0; row_counter < HEIGHT_PX/2; row_counter++){

            uint8_t row_1 = row_counter; // First row in simulation space being displayed
            uint8_t row_2 = row_counter + 16; // Second row in simulation space being displayed

            // the rgb matrix indexes rows from top to bottom, so the row counter must be flipped
            uint8_t row_out = HEIGHT_PX/2 - 1 - row_counter; 

            // Loop through every col in this row
            for(int col = 0; col < WIDTH_PX; col++){

                // Checks if the current column is the leftmost column
                if(col == 0){
                    ScopedLock<Mutex> lock(mtx); // Lock the mutex while accessing the paddle structs

                    // Check if row_simulation of RGB1 is occupied by paddle
                    if((row_1 >= leftPaddle->y_idx) & (row_1 <= leftPaddle->y_idx + PADDLE_HEIGHT - 1)){
                        GPIOC->PSOR = (COLOR_GREEN << 8);
                    }

                    // Check if row_select of RGB2 is occupied by paddle
                    if((row_2 >= leftPaddle->y_idx) & (row_2 <= leftPaddle->y_idx + PADDLE_HEIGHT - 1)){
                        GPIOC->PSOR = (COLOR_GREEN << 11);
                    }
                }

                // Checks if the current column is the rightmost column
                if(col == WIDTH_PX - 1){
                    ScopedLock<Mutex> lock(mtx); // Lock the mutex while accessing the paddle structs

                    // Check the RGB1 row for the paddle
                    if((row_1 >= rightPaddle->y_idx) & (row_1 <= rightPaddle->y_idx + PADDLE_HEIGHT - 1)){
                        GPIOC->PSOR = (COLOR_GREEN << 8);
                    }

                    // Check the RGB2 row for the paddle
                    if((row_2 >= rightPaddle->y_idx) & (row_2 <= rightPaddle->y_idx + PADDLE_HEIGHT - 1)){
                        GPIOC->PSOR = (COLOR_GREEN << 11);
                    }
                }


                mtx.lock(); // Lock mutex while accessing shared variable ball

                // Check the ball row against the row being displayed and write to the pixel if the ball exists there
                if(row_1 == ball->y_int){ // RGB1
                    if(col == ball->x_int) GPIOC->PSOR = (ball->color << 8);
                }

                if(row_2 == ball->y_int){ // RGB2
                    if(col == ball->x_int) GPIOC->PSOR = (ball->color << 11);
                }

                mtx.unlock();


                GPIOC->PSOR = 4; // Set CLK high
                GPIOC->PCOR = (7 << 11) | (7 << 8) | 4; // Clear CLK, RGB1, RGB2
            }

            GPIOC->PSOR = 2; // OE LOW
            GPIOC->PDOR = (row_out << 3) | 1; // Set row select and set LATCH
            GPIOC->PCOR = 3; // set OE high and clear latch
        }
    }
}
