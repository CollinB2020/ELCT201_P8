/* Copyright 2023 Collin Bollinger */ 

/*
 *	rgb_matrix.h
 *
 */
 
#ifndef RGB_MATRIX_H
#define RGB_MATRIX_H

#include "mbed.h"

#define GPIOC_BASE      0x400FF080UL // GPIO port register addresses for Port C

#define HEIGHT_PX       32 // Height of the rgb matrix in pixels
#define WIDTH_PX        64 // Width of the rgb matrix in pixels

// GPIO register structure
typedef struct {
    uint32_t PDOR; // GPIO output data
    uint32_t PSOR; // Set PDOR bits
    uint32_t PCOR; // Clear PDOR bits
    uint32_t PTOR; // Toggle PDOR bits
    uint32_t PDIR; // Data in from GPIO
    uint32_t PDDR; // Specifies pins as I/O
} GPIO_TypeDef;

// 2D array of pixels values
extern uint8_t **rgb_matrix;

// Rename
void thread_function();


#endif