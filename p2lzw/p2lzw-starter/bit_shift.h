#ifndef BIT_SHIFT_H
#define BIT_SHIFT_H

#include <stdio.h>
#include <stdint.h>

// Function declarations
void print_as_bits(FILE *output, uint16_t number, int total_bits);
void print_out_redundant(FILE *output);
int decode_printed_bits(FILE *input, int *output, int bit_size, int num_values); 
int read_bits(FILE *in, int bits);

#endif // BIT_SHIFT_H
