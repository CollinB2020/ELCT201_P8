/* Copyright 2023 Collin Bollinger */ 

/*
 *	rgb_matrix.cpp
 *
 */
 
#include "rgb_matrix.h"
 
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

// 2D array of pixels values
uint8_t **rgb_matrix;

void thread_function(){
	 
	// Clear 13 GPIOC bits being used (NOT OE')
    GPIOC->PCOR = 16381;
	 
	// Set Pin [13:0] GPIOC as output
    //GPIOC->PDDR |= 16383; //  2^14 - 1 = 16383
	 
	// 2D array for storing row pixel values. Thread_function will write these to the LED matrix continuously
    rgb_matrix = (uint8_t**) malloc(HEIGHT_PX*sizeof(uint8_t*));

    // Dynamically allocate each row of pixels 
    for(uint8_t row = 0; row < HEIGHT_PX; row++){
        rgb_matrix[row] = (uint8_t*) malloc(WIDTH_PX*sizeof(uint8_t));

        // Fill with off
        for(uint8_t col = 0; col < WIDTH_PX; col++){
            rgb_matrix[row][col] = 0;

            if(col == 0 | col == WIDTH_PX - 1 | row == 0 | row == HEIGHT_PX - 1) rgb_matrix[row][col] = 4;
        }
    }

    // Current row index
    uint32_t row = 0;

    // Set EDCBA for the row select to 0, CLK = 0, OE' = 1, LATCH = 0
    //GPIOC->PSOR = (row << 2) | 2;
    // Fix this line^

    while (true) {

        // Reset row  when out of bounds
        // Try using row as row_actual = row_counter % HEIGHT_PX/2
        if (row >= HEIGHT_PX/2) row = 0;

        // Clear 13 GPIOC bits being used except OE'
        //GPIOC->PCOR = 16381;

        // Loop through every col in this row
        for(int col = 0; col < WIDTH_PX; col++){

            // Set the RGB1 and RGB2 fields before writing 
            GPIOC->PSOR = (((int)rgb_matrix[row][WIDTH_PX - col - 1]) << 11) | (((int)rgb_matrix[row + 16][WIDTH_PX - col - 1]) << 8);
            // NOTE: Columns get reversed when they are written
            
            // Pulse only the CLK pin
            GPIOC->PSOR = 4;
            GPIOC->PCOR = 4;

            // CLEAR RGB
            GPIOC->PCOR = (7 << 11) | (7 << 8);
        }

        GPIOC->PSOR = 2; // OE LOW

        // Set EDCBA for the row select
        GPIOC->PCOR = (31 << 3); // Clear EDCBA
        GPIOC->PSOR = (row << 3); // Set EDCBA

        // Pulse LATCH
        GPIOC->PSOR = 1;
        GPIOC->PCOR = 1;

        GPIOC->PCOR = 2; // OE HIGH

        // Increment to next row
        row++;
        thread_sleep_for(1);
    }
}