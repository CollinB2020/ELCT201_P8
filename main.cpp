/* Copyright 2023 Collin Bollinger 
 *
 *	main.cpp
 */
 
#include "pong.h"

int main()
{

    // Create the pong simulation thread and start it
    Thread thread1(osPriorityHigh, 1024, NULL, "pong_simulation");
    thread1.start(simulate);

    // Create the rgb matrix thread and start it
    Thread thread(osPriorityHigh, 1024, NULL, "rgb_matrix");
    thread.start(rgb_matrix_function);

    // Idle the main thread while other threads run
    while (true) {
        
        // Idle
        thread_sleep_for(UINT8_MAX);
    }
}
