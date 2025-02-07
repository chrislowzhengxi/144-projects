// ---------------------------------- No Hash -------------------------------
#include "encode.h"
#include "bit_shift.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXBITS 12  
#define MAX_CODES (1 << MAXBITS)   // 4096
#define ASCII_SIZE 256   // ASCII 0-255 
#define EMPTY -1 

typedef struct {
    int previous_index;          // Previous index in the sequence
    unsigned char character;              // Character in this entry
    int symbol_code;             // Code for the (previous_index, character) pair
    // int valid;
} DictStorage;

DictStorage dictionary[MAX_CODES];
static int dict_size = ASCII_SIZE;  // Start with ASCII size
static int next_symbol_code = ASCII_SIZE; 
static int highest_code_before_prune = ASCII_SIZE - 1;

// Initializes the dictionary with single-character ASCII entries
void initializeEncodeDictionary() { 
    for (int i = 0; i < ASCII_SIZE; i++) { 
        dictionary[i].previous_index = EMPTY;
        dictionary[i].character = (unsigned char)i;   // ASCII character 
        dictionary[i].symbol_code = i;       // ASCII index 
        // dictionary[i].valid = 1;
    }
}

// Searches for a (previous_index, character) pair in dictionary
// Returns the symbol code if found, -1 if not found
int findSymbolCode(int prev_index, unsigned char character) { 
    for (int i = 0; i < dict_size; i++) {
        if (dictionary[i].previous_index == prev_index && dictionary[i].character == character) {
            // Adding a valid check: dictionary[i].valid && 
            return dictionary[i].symbol_code;  // Found match
        }
    }
    return -1;  // Not found
}



// encode table 
void tableEncode(int iteration, const char *label) {
    char *dbg_env = getenv("DBG");
    if (dbg_env) {
        char filename[50];
        snprintf(filename, sizeof(filename), "DBG.encode.%s.iteration%d", label, iteration);
        
        FILE *dbg_file = fopen(filename, "w");
        if (!dbg_file) {
            perror("Error opening DBG file");
            return;
        }

        for (int code = 0; code < dict_size; code++) {
            // if (dictionary[code].valid) {  // Only print valid entries
                if (dictionary[code].character >= 32 && dictionary[code].character <= 126) {
                    fprintf(dbg_file, "Code: %d, Previous Index: %d, Character: %c\n",
                            dictionary[code].symbol_code, dictionary[code].previous_index, dictionary[code].character);
                } else {
                    fprintf(dbg_file, "Code: %d, Previous Index: %d\n",
                            dictionary[code].symbol_code, dictionary[code].previous_index);
                }
            // }
        }

        fclose(dbg_file);
    }
}



// New pruning 

int is_prefix(int code) {
    for (int i = 0; i < dict_size; i++) {
        if (dictionary[i].previous_index == code) {
            return 1;
        }
    }
    return 0;
}

int create_retained_dictionary(DictStorage *retained_list) { 
    int retained_count = 0;      // elements in retained 

    for (int i = 0; i < dict_size; i++) {
        int is_single_char = (dictionary[i].previous_index == EMPTY);
        if (is_single_char || is_prefix(i)) {
            /* Instead of making the retained list just the index i, I want to
            make it the dictionary element */ 
            retained_list[retained_count++] = dictionary[i]; 
        }
    }
    return retained_count; 
}

void pruneDictionary() { 
    DictStorage retained_list[MAX_CODES];
    int retained_count = create_retained_dictionary(retained_list);

    /* This array new_dict will be populated by elements in the retained list but shifted 
    
    Code: 261, Previous Index: 257, Character: 4 
    Code: 262, Previous Index: 52, Character: 0 
    Code: 263, Previous Index: 48, Character: 6 
    Code: 264, Previous Index: 54 
    Code: 265, Previous Index: 10, Character: 5 

    Code: 261, Previous Index: 257, Character: 4 
    Code: 262, Previous Index: 52, Character: 0 
    Code: 263, Previous Index: 54 
    Code: 264, Previous Index: 10, Character: 5 
    */ 
    DictStorage new_dict[MAX_CODES]; 

    /* This is where we keep the array of previous indices. It is initialized
    from 0-512: {0, 1, ....., 512}, where each element represents the previous
    index. When I update 264 to 263, the previous index of 264 is 263 now, so
    {0, 1, , ...., 262, 263, 263, 265}. So I can refernce 263 as a previous
    index (instead of 264) here. */
    int previous_indices[MAX_CODES];
    // Initialize it: 
    for (int i = 0; i < MAX_CODES; i++) {   // maxBits should be 2^9 when we enter -m 9. Check main function
        previous_indices[i] = i;    // Each corresponds to the element 
    }    

    // {260, 261, 262, 263, 264, 265, 266}
    // {260, 261, 262, 263, 263, 265, 266} 

    for (int i = 0; i < retained_count; i++) {
        int retained_element_code = retained_list[i].symbol_code;     
        DictStorage retained_element = retained_list[i];   // retained_list[263] = Dict(264) 
        new_dict[i] = retained_element;          // new_dict[263] = Dict(264)
        new_dict[i].symbol_code = i;  // That's where you update 264 to 263?

        /* update previous_indices. Because 264 is not 263, every element 
        that used to have 264 as a previons index would have 263 now. 
        This applies to only non-ASCII > 255. That's when pruning starts to
        occur. */
        if (retained_element_code >= ASCII_SIZE) {         // Changed from i 
            previous_indices[retained_element_code] = i;       // p[264] = 263, update the previous index 
        }

        /* You will need to update the previous index if the previous index
        is more than 255 (ASCII) */
        int new_dict_prefix = new_dict[i].previous_index;
        
        if (new_dict_prefix >= ASCII_SIZE) {       // needs to update to dict size as it grows
            new_dict[i].previous_index = previous_indices[new_dict_prefix];
        }        
    }

    memcpy(dictionary, new_dict, retained_count * sizeof(DictStorage));


    dict_size = retained_count;

    // Let the before iteration first element to be this ++ (encode()) 
    next_symbol_code = retained_count; 


}


// Main encoding function
void encode(FILE *input, FILE *output, int p, int max_bit_length) {
    // printf("%d\n", max_bit_length);  
    // printf("%d\n", p);  
    print_as_bits(output, max_bit_length, 8); 
    print_as_bits(output, p, 8);   
    initializeEncodeDictionary();  
    int before = EMPTY;  
    int current; 
    int current_bits = 9;  // Default to 9 (for 0-511)
    int prune_iteration = 0;
    char *dbg_env = getenv("DBG");
    while ((current = getchar()) != EOF) {
        int augmented = findSymbolCode(before, (unsigned char)current);
        // When does encode lookup a code?
        // Here: The `findSymbolCode` function looks up whether `before + current` exists in the table.    
        if (augmented != -1) { 
            // Code found, continue to next character
            before = augmented;  
        } else {
            // When does encode print a code?
            // Here: The `before` code is printed when `before + current` is not found in the table.
            // STAGE=1 
            // printf("%d\n", before);  
            // STAGE=3
            print_as_bits(output, before, current_bits);
            // MAX BIT DYNAMIC 
            // When does encode add a new entry to the table?
            // Here: A new entry is added to the dictionary if the table is not full.
            if (dict_size < (1 << max_bit_length)) {
                dictionary[dict_size].previous_index = before;
                dictionary[dict_size].character = (unsigned char)current;
                dictionary[dict_size].symbol_code = next_symbol_code++;  // dict_size
                dict_size++;
                if (next_symbol_code > (1 << current_bits) - 1 && current_bits < max_bit_length) {
                    fprintf(stderr, "DEBUG: Increasing bit length to %d\n", current_bits + 1);
                    current_bits++;
                }
            } 
            // When does encode prune?
            // Here: Pruning occurs if pruning is enabled (`p == 1`) and the dictionary is full.
            else if (p == 1) {  
                // if (dbg_env) tableEncode(prune_iteration, "before");
                pruneDictionary();
                if (before != EMPTY && dict_size < (1 << max_bit_length)) {
                // Add the new entry (before + output[0]) after pruning
                    dictionary[dict_size].previous_index = before;   // The previous index
                    dictionary[dict_size].character = (unsigned char)current;     // The first character of the current string
                    dictionary[dict_size].symbol_code = next_symbol_code++;  // Update symbol code (for pruning consistency)
                    dict_size++;
                    if (next_symbol_code > (1 << current_bits) - 1 && current_bits < max_bit_length) {
                        fprintf(stderr, "DEBUG: Increasing bit length to %d\n", current_bits + 1);
                        current_bits++;
                    }
                    // ADDED - test to see if pruning defaults (change decode first)
                }
                // if (dbg_env) tableEncode(prune_iteration, "after");
                prune_iteration++;
            }
            before = current;  // Set up for the next cycle
        }
    }
    // Output the remaining "before" code after loop
    if (before != EMPTY) { 
        // printf("%d\n", before);
        print_as_bits(output, before, current_bits);
    }
    print_out_redundant(output);
    
    if (dbg_env) tableEncode(prune_iteration, "final");
}