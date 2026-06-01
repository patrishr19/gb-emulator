#include "cpu.h"
#include "gamepad.h"
#include <raylib.h>
#include <setup.h>
#include <emulator.h>
#include <rom.h>
#include <iogm.h>
#include <dma.h>
#include <ppu.h>
#include <lcd.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#undef RAYGUI_IMPLEMENTATION

#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "gui_window_file_dialog.h"

#define MENU_HEIGHT 24

#if defined(_WIN32) || defined(_WIN64)
    #define PATH_SEPARATOR "\\"
#else
    #define PATH_SEPARATOR "/"
#endif

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

    //WINDOW
    int scale = 4;

    InitWindow(XRES * scale, (YRES * scale) + MENU_HEIGHT, "gb-emulator");
    SetTargetFPS(60);
    
    Image screen_img = {
        .data = ppu_get_context()->video_buffer,
        .width = XRES,
        .height = YRES,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };
    Texture2D screen_texture = LoadTextureFromImage(screen_img);


    bool rom_loaded = false;
    int active_dropdown_menu = -1;
    
    if (argc >= 2) {
	if (LoadRom(&gb.bus, argv[1])) {
	    printf("Loaded ROM: %s\n", argv[1]);
	    rom_loaded = 1;
	}
    }

    GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());

    while (!WindowShouldClose()) {
	if (IsWindowResized()) {
	    int current_width = GetScreenWidth();
	    if (current_width > 0 && current_width != (XRES * scale)) {
		int new_scale = current_width / XRES;
		if (new_scale >= 1) {
		    scale = new_scale;
		    SetWindowMinSize(XRES * scale, (YRES * scale) + MENU_HEIGHT);
		}
	    }

	    scale = GetScreenWidth() / XRES;
	    if (scale < 1) scale = 1;
	}

	if (fileDialogState.SelectFilePressed) {
	    char selected_rom[1024];
	    if (IsFileExtension(fileDialogState.fileNameText, ".gb")) {
		strcpy(selected_rom, TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText));
	    }

	    if (selected_rom[0] != '\0') {
		if (LoadRom(&gb.bus, selected_rom)) {
		    gb.bus.current_bank = 1;
		    gb.bus.internal_divider = 0;
		    CPUInit(&gb.cpu);
		    IOInit(&gb.bus.io);
		    rom_loaded = true;
		    printf("Loaded ROM: %s\n", selected_rom);
		} else {
		    printf("Failed to load ROM: %s\n", selected_rom);
		}
	    }
	    fileDialogState.SelectFilePressed = false;		
	}

	if (rom_loaded && !fileDialogState.windowActive) {
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

	    uint32_t prev_frame = ppu_get_context()->current_frame;
	    while (prev_frame == ppu_get_context()->current_frame) {
		int cycles = CPUStep(&gb.cpu, &gb.bus);

		for (int i = 0; i < cycles; i += 4) {
		    dma_tick(&gb.bus);
		    TimerStep(&gb.bus, 4);
		    ppu_tick(&gb.bus);
		}
	    }

	    UpdateTexture(screen_texture, ppu_get_context()->video_buffer);
	    print_cpu_status(&gb);
	}

	if (active_dropdown_menu != -1 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
	    int mouse_x = GetMouseX();
	    int mouse_y = GetMouseY();
	    bool is_in_menu = false;

	    if (active_dropdown_menu == 0) {
		is_in_menu = (mouse_x >= 0 && mouse_x <= 80 && mouse_y >= 0 && mouse_y <= (MENU_HEIGHT + 56));
	    } else if (active_dropdown_menu == 1) {
		is_in_menu = (mouse_x >= 80 && mouse_x <= 160 && mouse_y >= 0 && mouse_y <= (MENU_HEIGHT + 168));
	    }

	    if (!is_in_menu) {
		active_dropdown_menu = -1;
	    }
	}

	BeginDrawing();
	ClearBackground(BLACK);

	if (rom_loaded) {
	    DrawTextureEx(screen_texture, (Vector2){0, (float)MENU_HEIGHT}, 0.0f, (float)scale, WHITE);
	} else {
	    const char *text = "No ROM loaded";
	    DrawText(text, (GetScreenWidth() / 2) - (MeasureText(text, 20) / 2), (GetScreenHeight() / 2), 20, WHITE);
	}
	
	if (fileDialogState.windowActive) GuiLock();

	GuiPanel((Rectangle){ 0, 0, (float)GetScreenWidth(), MENU_HEIGHT }, NULL);
	
	if (GuiButton((Rectangle){0, 0, 80, MENU_HEIGHT}, "File")) {
	    active_dropdown_menu = (active_dropdown_menu == 0) ? -1 : 0;
	}

	if (GuiButton((Rectangle){80, 0, 80, MENU_HEIGHT}, "View")) {
	    active_dropdown_menu = (active_dropdown_menu == 1) ? -1 : 1;
	}

	if (active_dropdown_menu == 0) {
	    if (GuiButton((Rectangle){ 0, MENU_HEIGHT, 80, 28 }, "Load ROM")) {
		fileDialogState.windowActive = true;
		active_dropdown_menu = -1;
	    }
	    if (GuiButton((Rectangle){ 0, MENU_HEIGHT + 28, 80, 28 }, "Exit")) {
		UnloadTexture(screen_texture);
		CloseWindow();
		return 0;
	    }
	}
	
	if (active_dropdown_menu == 1) {
	    if (GuiButton((Rectangle){80, MENU_HEIGHT, 80, 28}, "1X")) {
		scale = 1;
		SetWindowMinSize(XRES * scale, (YRES * scale) + MENU_HEIGHT);
		SetWindowSize(XRES * scale, (YRES * scale) + MENU_HEIGHT);
		active_dropdown_menu = -1;
	    }
	    if (GuiButton((Rectangle){80, MENU_HEIGHT + 28, 80, 28}, "2X")) {
		scale = 2;
		SetWindowMinSize(XRES * scale, (YRES * scale) + MENU_HEIGHT);
		SetWindowSize(XRES * scale, (YRES * scale) + MENU_HEIGHT);
		active_dropdown_menu = -1;
	    }
	    if (GuiButton((Rectangle){80, MENU_HEIGHT + 56, 80, 28}, "3X")) {
		scale = 3;
		SetWindowMinSize(XRES * scale, (YRES * scale) + MENU_HEIGHT);
		SetWindowSize(XRES * scale, (YRES * scale) + MENU_HEIGHT);
		active_dropdown_menu = -1;
	    }
	    if (GuiButton((Rectangle){80, MENU_HEIGHT + 84, 80, 28}, "4X")) {
		scale = 4;
		SetWindowMinSize(XRES * scale, (YRES * scale) + MENU_HEIGHT);
		SetWindowSize(XRES * scale, (YRES * scale) + MENU_HEIGHT);
		active_dropdown_menu = -1;
	    }
	    if (GuiButton((Rectangle){80, MENU_HEIGHT + 112, 80, 28}, "5X")) {
		scale = 5;
		SetWindowMinSize(XRES * scale, (YRES * scale) + MENU_HEIGHT);
		SetWindowSize(XRES * scale, (YRES * scale) + MENU_HEIGHT);
		active_dropdown_menu = -1;
	    }
	    if (GuiButton((Rectangle){80, MENU_HEIGHT + 140, 80, 28}, "6X")) {
		scale = 6;
		SetWindowMinSize(XRES * scale, (YRES * scale) + MENU_HEIGHT);
		SetWindowSize(XRES * scale, (YRES * scale) + MENU_HEIGHT);
		active_dropdown_menu = -1;
	    }
	}
	
	GuiUnlock();

	GuiWindowFileDialog(&fileDialogState);

	DrawFPS(GetScreenWidth() - 80, 4);
	EndDrawing();
    }

    UnloadTexture(screen_texture);
    CloseWindow();
    return 0;
}
