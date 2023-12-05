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

bool measurePerformance = false;

uint16_t pongTextMatrix[5] = { 0x8EAF, 0x8AA9, 0xEEEB, 0xA008, 0xE00F  };
uint8_t text_origin[2] = { WIDTH_PX/2 - 8 - 1, HEIGHT_PX/2 - 3 };

void rgb_matrix_function(){
    
    // Sleep for 10 milliseconds to ensure that the pong simulation thread has
    // allocated the ball and paddle structs and everything
    thread_sleep_for(10);
	 
	// Clear 13 GPIOC bits being used except for OE'
    GPIOC->PCOR = 16381;

    // Counter used for generating rows
    uint32_t row = 0;

    // Performance measuring
    float framePeriod = 0;
    uint16_t sampleSize = 1000;
    uint16_t sampleSizeCounter = 0;
    float d_t = 0.0;
    float prev_time = 0.0;
    float current_time = 0.0;

    Timer timer;
    timer.start();

    // Main thread loop. This loops scans over every row. For each row, every column pixel must be calculated.
    // It is more efficient to check the current locations of the paddles, ball, etc every time they are displayed than 
    // to store a 2d array of pixel values that must be managed. 
    for (;;) {

        // Evaluate d_t from current time and previous time. This time change is the period of one frame being displayed
        current_time = timer.read();
        d_t = current_time - prev_time;
        prev_time = current_time;

        framePeriod += d_t;
        sampleSizeCounter++;
        if (sampleSizeCounter >= sampleSize) { 
            framePeriod /= sampleSizeCounter;
            if(measurePerformance) { printf("\n\rAverage frame period over %i samples: %f ms", sampleSize, framePeriod); }
            sampleSizeCounter = framePeriod = 0;
        }

        // The rgb matrix has 16 row select values. Each row select value displays 2 different rows. Row_counter counts from
        // 0 to 16. Row_counter is used to generate two rows numbers, row_1 and row_2.
        for(uint8_t row_counter = 0; row_counter < HEIGHT_PX/2; row_counter++){

            uint8_t row_1 = row_counter; // First row in simulation space being displayed
            uint8_t row_2 = row_counter + 16; // Second row in simulation space being displayed

            // the rgb matrix indexes rows from top to bottom, so the row counter must be flipped
            uint8_t row_out = HEIGHT_PX/2 - 1 - row_counter; 


            // HUGE improvement with local variables:
            mtx.lock(); // Store extern variables once locally per frame so that the mutex is only needed once per frame
            uint8_t leftPaddleRow = leftPaddle->y_idx; // Row of the lowest pixel in the left paddle
            uint8_t rightPaddleRow = rightPaddle->y_idx; // Row of the lowest pixel in the right paddle
            uint16_t barHeights = display_score; // Heights of the left and right score bars
            uint8_t ballColor = ball->color;
            uint8_t ball_x = ball->x_int;
            uint8_t ball_y = ball->y_int;
            bool showText = showPongText;
            mtx.unlock();

            // Loop through every col in this row
            for(int col = 0; col < WIDTH_PX; col++){

                // Checks if the current column is the leftmost column
                if(col == 0){

                    // Check if row_1 of RGB1 is occupied by paddle
                    if((row_1 >= leftPaddleRow) & (row_1 <= leftPaddleRow + PADDLE_HEIGHT - 1)){ GPIOC->PSOR = (COLOR_GREEN << 8); }
                    // Check if row_2 of RGB2 is occupied by paddle
                    if((row_2 >= leftPaddleRow) & (row_2 <= leftPaddleRow + PADDLE_HEIGHT - 1)){
                        GPIOC->PSOR = (COLOR_GREEN << 11);
                    }
                } else if(col == WIDTH_PX - 1){ // Checks if the current column is the rightmost column

                    // Check if row_1 of RGB1 is occupied by paddle
                    if((row_1 >= rightPaddleRow) & (row_1 <= rightPaddleRow + PADDLE_HEIGHT - 1)){ GPIOC->PSOR = (COLOR_GREEN << 8); }
                    // Check if row_2 of RGB2 is occupied by paddle
                    if((row_2 >= rightPaddleRow) & (row_2 <= rightPaddleRow + PADDLE_HEIGHT - 1)){ GPIOC->PSOR = (COLOR_GREEN << 11); }
                }else if(col == WIDTH_PX / 2 + 11 | col == WIDTH_PX / 2 + 12){ // Checks if the current column is the right score columns

                    // Display the right score bar
                    if(row_1 < (barHeights & UINT8_MAX)){ GPIOC->PSOR = (SCORE_COLOR << 8); }
                    if(row_2 < (barHeights & UINT8_MAX)){ GPIOC->PSOR = (SCORE_COLOR << 11); }
                }else if(col == WIDTH_PX / 2 - 12 | col == WIDTH_PX / 2 - 13){ // Checks if the current column is the left score columns

                    // Display the left score bar
                    if(row_1 < (barHeights >> 8)){ GPIOC->PSOR = (SCORE_COLOR << 8); }
                    if(row_2 < (barHeights >> 8)){ GPIOC->PSOR = (SCORE_COLOR << 11); }
                }

                // Display the ball
                if(col == ball_x) {  // This code displays the ball once its coordinate is reached
                    if(row_1 == ball_y) { GPIOC->PSOR = (ballColor << 8); }
                    if(row_2 == ball_y) { GPIOC->PSOR = (ballColor << 11); }
                }

                // Display pong text
                if(col >= text_origin[0] & col <= text_origin[0] + 15 & showText) {
                    if (row_1 >= text_origin[1] & row_1 <= text_origin[1] + 4) { if(pongTextMatrix[row_1-13] & (1 << (38 - col))) { GPIOC->PSOR = (ballColor << 8); } }
                    if (row_2 >= text_origin[1] & row_2 <= text_origin[1] + 4) { if(pongTextMatrix[row_2-13] & (1 << (38 - col))) { GPIOC->PSOR = (ballColor << 11); } }
                }

                GPIOC->PSOR = 4; // Set CLK high
                GPIOC->PCOR = (7 << 11) | (7 << 8) | 4; // Clear CLK, RGB1, RGB2
            }

            GPIOC->PSOR = 2; // OE LOW
            GPIOC->PDOR = (row_out << 3) | 1; // Set row select and set LATCH
            GPIOC->PCOR = 1; // Clear latch
        }
    }
}
