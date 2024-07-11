#include <bareio.h>
#include <barelib.h>


int strcmp(char* string1, char* string2){
    int i;
    for(i = 0; string1[i] != '\0' && string2[i] != '\0'; i++) {
        if(string1[i] != string2[i])
            return 0;
    }
    if(string1[i] == string2[i])
        return 1;
    else
        return 0;
}

// Function to copy the source string to the destination string
char *strcpy(char *dest, const char *src) {
    // Pointer to the destination string
    char *originalDest = dest;

    // Copy characters from source to destination until '\0' (null terminator) is encountered
    while ((*dest++ = *src++) != '\0');

    // Return the original destination pointer
    return originalDest;
}