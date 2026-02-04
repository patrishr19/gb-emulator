#include <setup.h>
#include <emulator.h>
#include <rom.h>
int main() {
    Bus bus = {0};

    if (LoadRom(&bus, "testRoms/tetris.gb")) {
        printf("Game: ");
        for (uint16_t i = 0x0134; i <= 0x0143; i++) {
            printf("%c", bus.memory[i]);
        }
        printf("\n");
    }
    // FILE *fpointer;
    // // fpointer = fopen("log.txt", "w");


    // // fprintf(fpointer, "hello");
    
    // // fpointer = fopen("log.txt", "a");
    // // fprintf(fpointer, "hello2\n");
    // fpointer = fopen("log.txt", "r");
    // char string[100];

    // while(fgets(string, 100, fpointer)) {
    //     printf("%s", string);
    // }
   
    

    // fclose(fpointer);


    // uint8_t a = 255;

    // printf("%u\n", a);
    return 0;
}