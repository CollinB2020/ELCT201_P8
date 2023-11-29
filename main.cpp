/* Copyright 2023 Collin Bollinger 
 *
 *	main.cpp
 */
 
#include "pong.h"

int main()
{

    // Create the pong simulation thread and start it
    Thread thread1(osPriorityAboveNormal, 1152, NULL, "pong_simulation");
    thread1.start(simulate);

    // Create the rgb matrix thread and start it
    Thread thread(osPriorityAboveNormal, 512, NULL, "rgb_matrix");
    thread.start(thread_function);

    // Ensure the main thread does not exit prematurely
    while (true) {
        
        // Idle
        ThisThread::sleep_for(UINT16_MAX);
    }
}
