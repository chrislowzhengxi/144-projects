#include "encode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHAR_BIT 8 
 

#define MAXBITS 12  
#define MAX_CODES (1 << MAXBITS)   // 4096
#define ASCII_SIZE 256   // ASCII 0-255 
#define EMPTY -1 
#define HASH_TABLE_SIZE (1 << MAXBITS)   // 4096


typedef struct Storage {
    int previous_index;          // For it's empty
    char character;              // Character in this entry
    int symbol_code;             // Code for the (previous_index, character) pair
    struct Storage *next;        
} DictStorage; 

DictStorage *hashTable[HASH_TABLE_SIZE];   
  


// Using suggested hash function 
static unsigned int hashFunction(int prefix, char character) { 
    return ((unsigned long)(prefix) << CHAR_BIT | (unsigned) character) % HASH_TABLE_SIZE;
}

static void initializeHashTable() { 
    for (int i = 0; i < HASH_TABLE_SIZE; i++) { 
        hashTable[i] = NULL; 
    }

    for (int i = 0; i < ASCII_SIZE; i++) { 
        DictStorage *storage = (DictStorage *)malloc(sizeof(DictStorage));
        storage->previous_index = EMPTY;
        char ascii_char = (char)i; 
        storage->character = ascii_char;   // ASCII character 
        storage->symbol_code = i;          // ASCII index 
        storage->next = NULL;    

        unsigned int index = hashFunction(EMPTY, ascii_char);
        storage->next = hashTable[index];
        hashTable[index] = storage;        // Insert to front  
    }
}

// We want to see if (previous_index, current charcater) - augmented is in dict
int findHash(int prev_index, char character) { 
    unsigned int index = hashFunction(prev_index, character);
    DictStorage *storage = hashTable[index]; 

    while (storage != NULL) { 
        if (storage->previous_index == prev_index && storage->character == character) { 
            return storage->symbol_code;   // Found match
        }
        storage = storage->next;
    }
    return -1;  // Not found
}


static int dict_size = ASCII_SIZE;
void tableEncode() {
    char *dbg_env = getenv("DBG");
    if (dbg_env) {
        FILE *dbg_file = fopen("DBG.encode", "w");
        if (!dbg_file) {
            perror("Error opening DBG.encode");
            return;
        }

        for (int code = 0; code < dict_size; code++) {
            for (int i = 0; i < HASH_TABLE_SIZE; i++) {
                DictStorage *storage = hashTable[i];
                while (storage != NULL) {
                    if (storage->symbol_code == code) {
                        fprintf(dbg_file, "Code: %d, Previous Index: %d, Character: %c\n", 
                                storage->symbol_code, storage->previous_index, \
                                storage->character);
                        break;
                    }
                    storage = storage->next;
                }
            }
        }


        fclose(dbg_file);
    }
}




void encode(int p, int max_bit_length) {
    initializeHashTable();      // Initialize dictionary with ASCII chars

    int before = EMPTY;          // Initialize the "previous index" as empty
    int current; 

    while ((current = getchar()) != EOF) {

        int augmented = findHash(before, (char)current);
        
        if (augmented != -1) { 
            before = augmented;  // Extend the match
        } else {
            printf("%d\n", before);  // Output the code for "before"
            
            // Add new entry for (before, current) to the dictionary
            if (dict_size < HASH_TABLE_SIZE) {
                DictStorage *storage = (DictStorage *)malloc(sizeof(DictStorage));
                storage->previous_index = before; 
                storage->character = (char)current;
                storage->symbol_code = dict_size;  
                storage->next = NULL; 

                unsigned int index = hashFunction(before, (char)current);
                storage->next = hashTable[index];
                hashTable[index] = storage;

                // Increment dict_size only after entry has been assigned code
                dict_size++; 
            }

            before = current;  // Set up for the next cycle
        }
    }

    // Output the remaining "before" code after loop
    if (before != EMPTY) { 
        printf("%d\n", before);
    }
}

void freeHash() { 
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        DictStorage *storage = hashTable[i]; 
        
        while (storage != NULL) { 
            DictStorage *tmp = storage;
            storage = storage->next; 
            free(tmp);
        }
    }
}




// ----------------------------------------------------------------------------
#include "encode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHAR_BIT 8
#define MAXBITS 12
#define MAX_CODES (1 << MAXBITS)      // 4096 maximum entries based on MAXBITS
#define ASCII_SIZE 256                // Standard ASCII characters
#define EMPTY -1
#define TABLE_SIZE (1 << MAXBITS)     // Hash table size, also 4096 entries

typedef struct Node {
    int parent_code;                  // Code of the prefix (EMPTY for single characters)
    char symbol;                      // The character at this node
    int code_value;                   // Code for (parent_code, symbol) pair
    struct Node *next_entry;          // Pointer to next entry in the hash chain
} DictionaryNode;

DictionaryNode *dictionary[TABLE_SIZE];    // The hash table

// Hash function for (parent_code, symbol) pair
static unsigned int computeHash(int prefix_code, char character) {
    return ((unsigned long)(prefix_code) << CHAR_BIT | (unsigned) character) % TABLE_SIZE;
}

// Initialize dictionary with ASCII characters
static void initializeDictionary() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        dictionary[i] = NULL;
    }

    for (int i = 0; i < ASCII_SIZE; i++) {
        DictionaryNode *node = (DictionaryNode *)malloc(sizeof(DictionaryNode));
        node->parent_code = EMPTY;
        node->symbol = (char)i;
        node->code_value = i;
        node->next_entry = NULL;

        unsigned int hash_index = computeHash(EMPTY, node->symbol);
        node->next_entry = dictionary[hash_index];
        dictionary[hash_index] = node;  // Insert at the beginning of the linked list
    }
}

// Search for a (parent_code, character) combination in the dictionary
int lookup(int parent_code, char character) {
    unsigned int hash_index = computeHash(parent_code, character);
    DictionaryNode *entry = dictionary[hash_index];

    while (entry != NULL) {
        if (entry->parent_code == parent_code && entry->symbol == character) {
            return entry->code_value;
        }
        entry = entry->next_entry;
    }
    return -1;  // Not found
}

// Function to add debug info about the dictionary to a file
static int dictionary_size = ASCII_SIZE;
void logDictionary() {
    char *debug_env = getenv("DBG");
    if (debug_env) {
        FILE *file = fopen("DBG.encode", "w");
        if (!file) {
            perror("Error opening DBG.encode");
            return;
        }

        for (int code = 0; code < dictionary_size; code++) {
            for (int i = 0; i < TABLE_SIZE; i++) {
                DictionaryNode *node = dictionary[i];
                while (node != NULL) {
                    if (node->code_value == code) {
                        fprintf(file, "Code: %d, Parent Code: %d, Symbol: %c\n",
                                node->code_value, node->parent_code, node->symbol);
                        break;
                    }
                    node = node->next_entry;
                }
            }
        }
        fclose(file);
    }
}

// Array to store pointers to nodes we want to keep during pruning
DictionaryNode *keep_list[MAX_CODES];
int keep_count = 0;

// Check if a node represents a single ASCII character
int is_single_character(DictionaryNode *node) {
    return node->parent_code == EMPTY;
}

// Check if a node is a prefix of another node in the dictionary
int is_prefix(DictionaryNode *node) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        DictionaryNode *entry = dictionary[i];
        while (entry != NULL) {
            if (entry->parent_code == node->code_value) {
                return 1;
            }
            entry = entry->next_entry;
        }
    }
    return 0;
}

// Mark entries to retain in keep_list based on pruning rules
void markForRetention() {
    keep_count = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        DictionaryNode *entry = dictionary[i];
        while (entry != NULL) {
            if (is_single_character(entry) || is_prefix(entry)) {
                keep_list[keep_count++] = entry;
            }
            entry = entry->next_entry;
        }
    }
}

// Clear the dictionary by freeing each node and resetting buckets
void clearDictionary() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        DictionaryNode *entry = dictionary[i];
        while (entry != NULL) {
            DictionaryNode *next_node = entry->next_entry;
            free(entry);
            entry = next_node;
        }
        dictionary[i] = NULL;
    }
}

// Reinsert retained entries into dictionary with sequential codes
void reinsertEntries() {
    int new_code = ASCII_SIZE;

    for (int i = 0; i < keep_count; i++) {
        DictionaryNode *entry = keep_list[i];
        entry->code_value = new_code++;

        unsigned int hash_index = computeHash(entry->parent_code, entry->symbol);
        entry->next_entry = dictionary[hash_index];
        dictionary[hash_index] = entry;
    }
    dictionary_size = new_code;
}

// Function to prune the dictionary
void pruneDictionary() {
    markForRetention();   // Step 1: Mark entries to keep
    clearDictionary();    // Step 2: Clear the dictionary
    reinsertEntries();    // Step 3: Reinsert retained entries
}

// Main encode function that includes pruning when the dictionary is full
void encode(int prune_enabled, int max_bit_length) {
    initializeDictionary();

    int previous = EMPTY;
    int current;

    while ((current = getchar()) != EOF) {
        int augmented = lookup(previous, (char)current);

        if (augmented != -1) {
            previous = augmented;
        } else {
            printf("%d\n", previous);

            if (dictionary_size < MAX_CODES) {
                DictionaryNode *node = (DictionaryNode *)malloc(sizeof(DictionaryNode));
                node->parent_code = previous;
                node->symbol = (char)current;
                node->code_value = dictionary_size;
                node->next_entry = NULL;

                unsigned int hash_index = computeHash(previous, (char)current);
                node->next_entry = dictionary[hash_index];
                dictionary[hash_index] = node;

                dictionary_size++;
            } else if (prune_enabled) {
                pruneDictionary();
            }

            previous = current;
        }
    }

    if (previous != EMPTY) {
        printf("%d\n", previous);
    }
}

// Function to free the dictionary at the end
void freeDictionary() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        DictionaryNode *entry = dictionary[i];
        while (entry != NULL) {
            DictionaryNode *temp = entry;
            entry = entry->next_entry;
            free(temp);
        }
    }
}



