/* Copyright 2023 Collin Bollinger */ 

/*
 *	main.cpp
 */
 
#include "pong_simulation.h"
#include "rgb_matrix.h"

int main()
{

	// Create a new thread
    Thread thread(osPriorityAboveNormal, 512);
    //Thread simulationThread(osPriorityAboveNormal, 2048);
	
	// Start the thrad
    thread.start(thread_function);

    thread_sleep_for(200);


	// Pass execution over to the simulation 
    simulate();
}