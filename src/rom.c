#include <setup.h>
#include <rom.h>

bool LoadRom(Bus *bus, char *filename) {
    FILE *fpointer = fopen(filename, "rb");
    if (!fpointer) {
        printf("Failed at fopen");
        return false;
    }

    // https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
    fseek(fpointer, 0, SEEK_END);
    long fsize = ftell(fpointer);
    fseek(fpointer, 0, SEEK_SET);


    size_t bytesRead = fread(bus->memory, 1, fsize, fpointer); // dedicated to ROM
    
    printf("Loading %zu bytes from: %s \n", bytesRead, filename);

    fclose(fpointer);
    return true;
}