#include "decode.h"
#include "bit_shift.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAXBITS 12
#define MAX_CODES (1 << MAXBITS)    // 4096
#define INITIAL_DICT 256            // ASCII 0-255
#define EMPTY -1

typedef struct {
    int previous_index;       
    unsigned char character;    
    int symbol_code;          // Just for pruning           
} DictStorage;

DictStorage symbolTable[MAX_CODES];


int dict_size = INITIAL_DICT;
int current = INITIAL_DICT;
static int next_symbol_code = INITIAL_DICT;  // ?

void initializeDictionary() {
    // Initialize ASCII entries
    for (int i = 0; i < INITIAL_DICT; i++) {
        symbolTable[i].previous_index = EMPTY;
        symbolTable[i].character = (unsigned char)i;        // Change everything to unsigned
        symbolTable[i].symbol_code = i;    // ASCII, just for prune 
    }
}

// Updates output with the symbol code
void make_code_string(int code, char *output, int *length) { 
    if (code == EMPTY) {
        return;  // Base case
    }
    make_code_string(symbolTable[code].previous_index, output, length);
    output[(*length)++] = symbolTable[code].character;  
}

// Returns length to print "output", also calls make_code_string
int get_code_length(int code, char *output) {
    int length = 0;
    make_code_string(code, output, &length);
    output[length] = '\0';  
    return length;
}

//////////////////////////////// Pruning logic ////////////////////////////////


int is_prefix_decode(int code) {
    for (int i = 0; i < dict_size; i++) {
        if (symbolTable[i].previous_index == code) {
            return 1;
        }
    }
    return 0;
}


int create_retained_symbol_table(DictStorage *retained_list) { 
    int retained_count = 0;      // elements in retained 

    for (int i = 0; i < dict_size; i++) {
        int is_single_char = (symbolTable[i].previous_index == EMPTY);
        if (is_single_char || is_prefix_decode(i)) {
            /* Instead of making the retained list just the index i, I want to
            make it the dictionary element */ 
            retained_list[retained_count++] = symbolTable[i]; 
        }
    }
    return retained_count; 
}


void pruneSymbolTable() { 
    DictStorage retained_list[MAX_CODES];
    int retained_count = create_retained_symbol_table(retained_list);

    DictStorage new_dict[MAX_CODES]; 


    int previous_indices[MAX_CODES];
    // Initialize it: 
    for (int i = 0; i < MAX_CODES; i++) {   // maxBits should be 2^9 when we enter -m 9. Check main function
        previous_indices[i] = i;    // Each corresponds to the element 
    }    


    for (int i = 0; i < retained_count; i++) {
        int retained_element_code = retained_list[i].symbol_code;     
        DictStorage retained_element = retained_list[i];   // retained_list[263] = Dict(264) 
        new_dict[i] = retained_element;          // new_dict[263] = Dict(264)
        new_dict[i].symbol_code = i;  // That's where you update 264 to 263?

        if (retained_element_code >= INITIAL_DICT) {         // Changed from i 
            previous_indices[retained_element_code] = i;       // p[264] = 263, update the previous index 
        }

        /* You will need to update the previous index if the previous index
        is more than 255 (ASCII) */
        int new_dict_prefix = new_dict[i].previous_index;
        
        if (new_dict_prefix >= INITIAL_DICT) {       // needs to update to dict size as it grows
            new_dict[i].previous_index = previous_indices[new_dict_prefix];
        }        
    }

    memcpy(symbolTable, new_dict, retained_count * sizeof(DictStorage));


    dict_size = retained_count;

    // Let the before iteration first element to be this ++ (encode()) 
    next_symbol_code = retained_count; 


}





//////////////////////////////// Pruning logic ////////////////////////////////


// Debugging function for symbol table with iteration support
void tableDecode(int iteration, const char *label) {
    char *dbg_env = getenv("DBG");

    if (dbg_env) {
        char filename[50];
        snprintf(filename, sizeof(filename), "DBG.decode.%s.iteration%d", label, iteration);
        
        FILE *dbg_file = fopen(filename, "w");
        if (!dbg_file) {
            perror("Error opening DBG.decode file");
            return;
        }

        for (int i = 0; i < dict_size; i++) {
            if (symbolTable[i].character >= 32 && symbolTable[i].character <= 126) {
                fprintf(dbg_file, "Code: %d, Previous Index: %d, Character: %c\n",
                        i, symbolTable[i].previous_index, symbolTable[i].character);
            } else {
                fprintf(dbg_file, "Code: %d, Previous Index: %d\n",
                        i, symbolTable[i].previous_index);
            }
        }

        fclose(dbg_file);
    }
}




void decode(FILE *input) {
    int maxBits, pruning_enabled;
    // Read (maxBits and pruning flag)
    if (decode_printed_bits(input, &maxBits, 8, 1) == -1 || 
        decode_printed_bits(input, &pruning_enabled, 8, 1) == -1) {
        fprintf(stderr, "Error: Failed to read metadata from encoded input\n");
        return;
    }

    initializeDictionary();  // Initialize ASCII entries in symbolTable
    int before = EMPTY;
    int current;
    int current_bit_size = 9; // Default bit size
    int prune_iteration = 0;
    char *dbg_env = getenv("DBG");

    // Read the first code
    if (decode_printed_bits(input, &current, current_bit_size, 1) == -1) {
        fprintf(stderr, "Error: Failed to read the first code.\n");
        return;
    }

    char output[4096];
    int length = get_code_length(current, output);
    for (int i = 0; i < length; i++) {
        putchar(output[i]);
    }
    before = current;

    // Process remaining codes
    while (1) {
        int result = decode_printed_bits(input, &current, current_bit_size, 1);
        if (result == -1) {
            break; // Exit loop on EOF
        }

        // if (current == 10) {
        //     return; // Stop decoding when EOF-like condition is encountered
        // }

        int length;
        if (current < dict_size) {
            length = get_code_length(current, output);
        } else if (current == dict_size && before != EMPTY) {
            length = get_code_length(before, output);
            output[length++] = output[0];
            output[length] = '\0';
        } else {
            fprintf(stderr, "ERROR: Invalid code %d encountered\n", current);
            return;
        }

        for (int i = 0; i < length; i++) {
            putchar(output[i]);
        }

        if (dict_size < (1 << maxBits) && before != EMPTY) {
            symbolTable[dict_size].previous_index = before;
            symbolTable[dict_size].character = output[0];
            symbolTable[dict_size].symbol_code = next_symbol_code++;
            dict_size++;
        }

        // if (next_symbol_code > (1 << current_bit_size) - 1 && current_bit_size < maxBits) {
        //     fprintf(stderr, "DEBUG: Increasing decode bit length to %d\n", current_bit_size + 1);
        //     current_bit_size++;
        // }
        if (next_symbol_code > (1 << current_bit_size) - 1) {
            if (current_bit_size < maxBits) {
                current_bit_size++; // Increase bit size
                fprintf(stderr, "DEBUG: Increased decode bit size to %d\n", current_bit_size);
            } else {
                fprintf(stderr, "WARNING: Reached maxBits limit (%d). No further growth.\n", maxBits);
            }
        }

        if (pruning_enabled && dict_size >= (1 << maxBits)) {
            pruneSymbolTable();
            prune_iteration++;
        }

        before = current;
    }

    if (dbg_env) {
        tableDecode(prune_iteration, "final");
    }

}