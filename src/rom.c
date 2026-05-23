#include <setup.h>
#include <rom.h>
#include <string.h>

bool LoadRom(Bus *bus, const char *filename) {
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

const char *select_rom_dialog() {
    static char filepath[1024];

    FILE *file = popen("zenity --file-selection --title=\"Select Game Boy ROM\" --file-filter=\"GB ROMs (*.gb *.gbc) | *.gb *.gbc\"", "r");

    if (file == NULL) {
	return NULL;
    }

    if (fgets(filepath, sizeof(filepath), file) != NULL) {
	filepath[strcspn(filepath, "\r\n")] = 0;
	pclose(file);
	return filepath;
    }

    pclose(file);
    return NULL;
}
