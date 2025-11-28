// haffman.c

#include "treelist.h"

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <encode|decode> <input_file>\n", argv[0]);
        return 1;
    }

    const char *mode = argv[1];
    const char *input = argv[2];

    int result;
    if (strcmp(mode, "--encode") == 0) {
        result = utite_fuctions_to_encode(input);
    } else if (strcmp(mode, "--decode") == 0) {
        result = decode(input);
    } else {
        printf("Unknown mode (use 'encode' or 'decode')\n");
        return 1;
    }

    return result;
}