#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h> // Include for basename

#include "encode.h"
#include "decode.h"

// extern void freeCurrentNewHash(); 
// extern void freeHash();
// extern void freeActiveHash();
// extern void tableEncode(int iteration, const char *label);
// extern void tableDecode();

extern int pruning_activated;

int main(int argc, char *argv[])
{
    static char bin[64], bout[64];
    setvbuf(stdin, bin, _IOFBF, 64);
    setvbuf(stdout, bout, _IOFBF, 64);

    setbuf(stdout, NULL);  // Disable output buffering

    char *exec_name = basename(argv[0]); // Get the executable name
    char *dbg_env = getenv("DBG"); // Check if DBG is set

    int maxBits = 12;  // Was in the encode conditional
    int pruning = 0;

    if (strcmp(exec_name, "encode") == 0) {
        
        
        /* From Ed: 
        - maxBits can't be less than 8 (due to ASCII)
        - maxBits cannot exceed 20 - replaced by 12 (default)
        - maxBits is between 9-20, with 12 default 
        */
        // Parsing logic: 
        for (int i = 1; i < argc; i++) {
            // First one can either be -m (number) or -p 
            if ((i + 1 < argc) && (strcmp(argv[i], "-m") == 0)) {
                maxBits = atoi(argv[++i]);

                if (maxBits < 8) { 
                    fprintf(stderr, "Error, MAXBITS less than 8.\n");
                    exit(1);
                } else if (maxBits == 8 || maxBits > 20) {
                    maxBits = 12;      // per (D) on page 3 project pdf
                    printf("MAXBITS not valid. Setting default 12.\n");
                } 
            } else if (strcmp(argv[i], "-p") == 0) {
                pruning = 1;
            } else {
                fprintf(stderr, "encode: invalid input '%s'\n", argv[i]);
                exit(1);
            }
        }


        encode(stdin, stdout, pruning, maxBits);
        

    } else if (strcmp(exec_name, "decode") == 0) {
        decode(stdin);

    } else {
        fprintf(stderr, "Usage: %s [-m MAXBITS] [-p] < input > output\n", argv[0]);
        fprintf(stderr, "       %s < input > output\n", argv[0]);
        exit(1);
    }
    return 0;
}