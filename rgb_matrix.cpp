/* Copyright 2023 Collin Bollinger
 *
 *	rgb_matrix.cpp
 */
 
#include "pong.h"
 
// Set the inputs and outputs
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

void thread_function(){

    // Sleep for 100 milliseconds to ensure that the pong simulation thread has
    // allocated the ball and paddle structs and everything
    thread_sleep_for(100);
	 
	// Clear 13 GPIOC bits being used except for OE'
    GPIOC->PCOR = 16381;

    // Counter used for generating rows
    uint32_t row = 0;

    // Main thread loop
    while (true) {

        // Row%(HEIGHT_PX/2) is the current rgb matrix row being displayed. Row zero is the top row.
        // Becasue the simulation uses an origin in the bottom left, the rows will have to be flipped.
        // row_actual represents what the current reg matrix row is equivlent to in the simulation space.
        uint8_t row_actual = HEIGHT_PX/2 - 1 - row%(HEIGHT_PX/2); // row_actual just flip 0,1,...2 to ...2,1,0

        // Loop through every col in this row
        for(int col = 0; col < WIDTH_PX; col++){

            // Display Left paddle pixels on the first column
            if(col == 0){
                ScopedLock<Mutex> lock(mtx); // Lock the mutex while accessing the paddle structs

                // Check RGB1
                if((row_actual >= leftPaddle->y_loc) & (row_actual <= leftPaddle->y_loc + PADDLE_HEIGHT - 1)){
                    GPIOC->PSOR = (COLOR_GREEN << 8);
                }

                // Check RGB2
                if((row_actual + 16 >= leftPaddle->y_loc) & (row_actual + 16 <= leftPaddle->y_loc + PADDLE_HEIGHT - 1)){
                    GPIOC->PSOR = (COLOR_GREEN << 11);
                }
            }

            // Display right paddle pixels on the last column
            if(col == WIDTH_PX - 1){
                ScopedLock<Mutex> lock(mtx); // Lock the mutex while accessing the paddle structs

                // Check RGB1
                if((row_actual >= rightPaddle->y_loc) & (row_actual <= rightPaddle->y_loc + PADDLE_HEIGHT - 1)){
                    GPIOC->PSOR = (COLOR_GREEN << 8);
                }

                // Check RGB2
                if((row_actual + 16 >= rightPaddle->y_loc) & (row_actual + 16 <= rightPaddle->y_loc + PADDLE_HEIGHT - 1)){
                    GPIOC->PSOR = (COLOR_GREEN << 11);
                }
            }

            // Ball RGB2
            if(row_actual == ball->y_int){ // RGB1
                if(col == ball->x_int) GPIOC->PSOR = (ball->color << 8);
            }

            // Ball RGB1
            if(row_actual + 16 == ball->y_int){ // RGB2
                if(col == ball->x_int) GPIOC->PSOR = (ball->color << 11);
            }

            // Set CLK high
            GPIOC->PSOR = 4;

            // Clear CLK and RGB1, RGB2
            GPIOC->PCOR = (7 << 11) | (7 << 8) | 4;
        }

        GPIOC->PSOR = 2; // OE LOW
        GPIOC->PDOR = (row%(HEIGHT_PX/2) << 3) | 1; // Set row select and set LATCH

        // Clear LATCH and set OE high
        GPIOC->PCOR = 3;

        // Increment to next row
        row++;
    }
}
