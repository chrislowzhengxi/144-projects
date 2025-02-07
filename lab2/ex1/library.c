#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Takes two NULL-terminated strings as arguments and
 * determines the sum of their lengths.
 */
int multiStrlen(char *first, char *second) {
    char *p, *q;

    p = first;
    while (*p++ != '\0') ;
    q = second;
    while (*q++ != '\0') ;

    return (p - first - 1) + (q - second - 1);
}

/*
 * Sorts two lists of characters in two steps:
 * 1) Allocate a result array and copy the two strings over
 * 2) For each character, if there is a larger character
 * later in the string, perform a swap
 */ 
char *twoFingerSort(char *first, char *second)
{
    int i, j;
    int length = multiStrlen(first, second);
    char *result = (char*)malloc(length + 1); // Allocate with space for NULL terminator
    char *p, *q, temp;

    q = result;
    p = first;
    while ((*q++ = *p++) != '\0') ;  // Copy first string
    q--;  // Move q back by one to overwrite the null terminator

    p = second;
    while ((*q++ = *p++) != '\0') ;  // Copy second string

    for (i = 0; i < length - 1; i++) {
        for (j = i + 1; j < length; j++) {
            if (result[i] < result[j]) {  // Descending order
                temp = result[i];
                result[i] = result[j];
                result[j] = temp;
            }
        }
    }

    return result;
}
