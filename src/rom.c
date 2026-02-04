#include <setup.h>
#include <rom.h>

bool LoadRom(Bus *bus, char *filename) {
    FILE *fpointer = fopen(filename, "rb");
    if (!fpointer) {
        printf("Failed at fopen");
        return false;
    }

    size_t bytesRead = fread(bus->memory, 1, 0x8000, fpointer);

    printf("Loading %zu bytes from: %s \n", bytesRead, filename);

    fclose(fpointer);
    return true;
}