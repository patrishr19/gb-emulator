#include <raylib.h>
#include <setup.h>
#include <emulator.h>
#include <rom.h>
#include <iogm.h>
#include <dma.h>
#include <ppu.h>
#include <lcd.h>

void print_cpu_status(Gameboy *gb) {
    printf("PC: 0x%04X | AF: 0x%02X%02X | BC: 0x%02X%02X | DE: 0x%02X%02X | HL: 0x%02X%02X | LY: %03d | Mode: %d\n",
    gb->cpu.pc, gb->cpu.a, gb->cpu.f, gb->cpu.b, gb->cpu.c, gb->cpu.d, gb->cpu.e, gb->cpu.h, gb->cpu.l, lcd_get_context()->ly, LCDS_MODE 
    );
}


int main(int argc, char *argv[]) {
    Gameboy gb = {0};
    gb.bus.current_bank = 1;
    gb.bus.internal_divider = 0;
    CPUInit(&gb.cpu);
    ppu_init();
    IOInit(&gb.bus.io);

    bool running = false;

    if (argc < 2) {
        return 1;
    }

    //WINDOW
    const int scale = 4;
    InitWindow(XRES * scale, YRES * scale, "gb-emulator");
    SetTargetFPS(60);

    //TMP
    Image screen_img = {
        .data = ppu_get_context()->video_buffer,
        .width = XRES,
        .height = YRES,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };
    Texture2D screen_texture = LoadTextureFromImage(screen_img);

    if (LoadRom(&gb.bus, argv[1])) {
        printf("Loaded: %s\n", argv[1]);
        printf("Game: ");
        for (uint16_t i = 0x0134; i <= 0x0143; i++) { // 16 chars
            printf("%c", gb.bus.memory[i]);
        }
        printf("\n");
        running = true;
    } else {
        printf("Failed to load: %s\n", argv[1]);
        running = false;
    }
    
    while (running && !WindowShouldClose()) {
        uint32_t prev_frame = ppu_get_context()->current_frame;

        while (prev_frame == ppu_get_context()->current_frame) {
            int cycles = CPUStep(&gb.cpu, &gb.bus);

            for (int i = 0; i < cycles; i += 4) {
                dma_tick(&gb.bus);
                TimerStep(&gb.bus, 4);
                ppu_tick(&gb.bus);
            }
        }

        //DRAW
        UpdateTexture(screen_texture, ppu_get_context()->video_buffer);
        
        BeginDrawing();

        ClearBackground(BLACK);
        DrawTextureEx(screen_texture, (Vector2){0, 0}, 0.0f, (float)scale, WHITE);
        DrawFPS(10, 10);

        EndDrawing();

        print_cpu_status(&gb);

    }
    UnloadTexture(screen_texture);
    CloseWindow();
    //free

    return 0;
}
