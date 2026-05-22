#include "gamepad.h"
#include <raylib.h>
#include <setup.h>
#include <emulator.h>
#include <rom.h>
#include <iogm.h>
#include <dma.h>
#include <ppu.h>
#include <lcd.h>
#include "raygui.h"
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
	printf("Emulator needs a rom path argument\nexample: ./emulator path/to/rom/game.gb\n");
        return 1;
    }

    //WINDOW
    const int scale = 6;
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
	

	//input
	int gamepad_id = 0;	
	if (IsGamepadAvailable(gamepad_id)) {
	    gamepad_get_state()->up = IsKeyDown(KEY_UP) || IsGamepadButtonDown(gamepad_id, GAMEPAD_BUTTON_LEFT_FACE_UP); 
	    gamepad_get_state()->down = IsKeyDown(KEY_DOWN) || IsGamepadButtonDown(gamepad_id, GAMEPAD_BUTTON_LEFT_FACE_DOWN); 
	    gamepad_get_state()->left = IsKeyDown(KEY_LEFT) || IsGamepadButtonDown(gamepad_id, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
	    gamepad_get_state()->right = IsKeyDown(KEY_RIGHT) || IsGamepadButtonDown(gamepad_id, GAMEPAD_BUTTON_LEFT_FACE_RIGHT); 
	    gamepad_get_state()->b = IsKeyDown(KEY_Z) || IsGamepadButtonDown(gamepad_id, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT); 
	    gamepad_get_state()->a = IsKeyDown(KEY_X) || IsGamepadButtonDown(gamepad_id, GAMEPAD_BUTTON_RIGHT_FACE_DOWN); 
	    gamepad_get_state()->start = IsKeyDown(KEY_ENTER) || IsGamepadButtonDown(gamepad_id, GAMEPAD_BUTTON_MIDDLE_RIGHT); 
	    gamepad_get_state()->select = IsKeyDown(KEY_TAB) || IsGamepadButtonDown(gamepad_id, GAMEPAD_BUTTON_MIDDLE_LEFT);
	} else {
	    gamepad_get_state()->up = IsKeyDown(KEY_UP); 
	    gamepad_get_state()->down = IsKeyDown(KEY_DOWN); 
	    gamepad_get_state()->left = IsKeyDown(KEY_LEFT);
	    gamepad_get_state()->right = IsKeyDown(KEY_RIGHT); 
	    gamepad_get_state()->b = IsKeyDown(KEY_Z); 
	    gamepad_get_state()->a = IsKeyDown(KEY_X); 
	    gamepad_get_state()->start = IsKeyDown(KEY_ENTER); 
	    gamepad_get_state()->select = IsKeyDown(KEY_TAB);
	}

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
    UnloadImage(screen_img);
    CloseWindow();
    //free

    return 0;
}
