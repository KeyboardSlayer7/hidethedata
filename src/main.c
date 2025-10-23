#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "png.h"
#include "utils.h"

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        printf("[ERROR] No file or option provided\n");
        return EXIT_FAILURE;
    }

    const char* filename = argv[1];
    FILE* file = fopen(filename, "rb");
    
    if (file == NULL)
    {
        printf("[ERROR] Could not open file: %s\n", filename);
        return EXIT_FAILURE;
    }

    const char* option = argv[2];

    if (option[0] == 'h')
    {
        if (argc < 4)
        {
            printf("[ERROR] No data was provided\n");
            return EXIT_FAILURE;
        }
        
        const char* string = argv[4];
        H_processPNG(file, string);
    }
    else if (option[0] == 'e')
    {
        char* data = E_processPNG(file);
        printf("Data: %s\n", data);
    }
    else 
    {
        printf("[ERROR] No such option exists\n");
        return EXIT_FAILURE;
    }

    fclose(file);

    return 0;
}
