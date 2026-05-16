#include <bus.h>
#include <ppu.h>
#include <lcd.h>
#include <cpu.h>
#include <setup.h>
#include <iogm.h>
#include <stdint.h>


bool window_visible();

void increment_ly(Bus *bus) {
    if (window_visible() && lcd_get_context()->ly >= lcd_get_context()->win_y &&
	lcd_get_context()->ly < lcd_get_context()->win_y + YRES
    ) {
	ppu_get_context()->window_line++;
    }


    lcd_get_context()->ly++;

    if (lcd_get_context()->ly == lcd_get_context()->ly_compare) {
        LCDS_LYC_SET(1);

        if (LCDS_STAT_INT(SS_LYC)) {
            request_interrupt(bus, 2);
        }
    } else {
        LCDS_LYC_SET(0);
    }
}

void load_lines_sprites() {
    int current_y = lcd_get_context()->ly;

    uint8_t sprite_height = LCDC_OBJ_HEIGHT;
    memset(ppu_get_context()->line_entry_array, 0, sizeof(ppu_get_context()->line_entry_array));

    for (int i=0; i<40; i++) {
	oam_entry o = ppu_get_context()->oam_ram[i];

	if (!o.x) {
	    //x = 0 not visible
	    continue;
	}

	if (ppu_get_context()->line_sprite_count >= 10) {
	    break;
	}

	if (o.y <= current_y + 16 && o.y + sprite_height > current_y + 16) {
	    //sprite on current line_

	    oam_line_entry *entry = &ppu_get_context()->line_entry_array[
		ppu_get_context()->line_sprite_count++
	    ];

	    entry->entry = o;
	    entry->next = NULL;

	    if (!ppu_get_context()->line_sprites || ppu_get_context()->line_sprites->entry.x > o.x ) {
		entry->next = ppu_get_context()->line_sprites;
		ppu_get_context()->line_sprites = entry;
		continue;
	    }

	    //sort

	    oam_line_entry *le = ppu_get_context()->line_sprites;
	    oam_line_entry *prev = le;

	    while(le) {
		if (le->entry.x > o.x) {
		    prev->next = entry;
		    entry->next = le;
		    break;
		}

		if (!le->next) {
		    le->next = entry;
		    break;
		}

		prev = le;
		le = le->next;
	    }
	}
    }
}

void ppu_mode_oam() {
    // printf("OAM ticks=%d\n", ppu_get_context()->line_ticks);

    if (ppu_get_context()->line_ticks >= 80) {
        LCDS_MODE_SET(MODE_XFER);

        ppu_get_context()->pfc.cur_fetch_state = FS_TILE;
        ppu_get_context()->pfc.line_x = 0;
        ppu_get_context()->pfc.fetch_x = 0;
        ppu_get_context()->pfc.pushed_x = 0;
        ppu_get_context()->pfc.fifo_x = 0;
    }

    if (ppu_get_context()->line_ticks == 1) {
	// read oam on the first tick
	ppu_get_context()->line_sprites = 0;
	ppu_get_context()->line_sprite_count = 0;

	load_lines_sprites();
    }
}
void ppu_mode_xfer(Bus *bus) {
    pipeline_process(bus);

    if (ppu_get_context()->pfc.pushed_x >= XRES) {
        pipeline_fifo_reset();
        LCDS_MODE_SET(MODE_HBLANK);

        if (LCDS_STAT_INT(SS_HBLANK)) {
            request_interrupt(bus, 2);
        }
    }
}
void ppu_mode_vblank(Bus *bus) {
    if (ppu_get_context()->line_ticks >= TICKS_PER_LINE) {
        increment_ly(bus);
        if (lcd_get_context()->ly >= 154) {
            LCDS_MODE_SET(MODE_OAM);
            lcd_get_context()->ly = 0;
	    ppu_get_context()->window_line = 0;
        }

        ppu_get_context()->line_ticks = 0;
    }
}

void ppu_mode_hblank(Bus *bus) {
    if (ppu_get_context()->line_ticks >= TICKS_PER_LINE) {
        increment_ly(bus);

        if (lcd_get_context()->ly >= YRES) {
            LCDS_MODE_SET(MODE_VBLANK);

            request_interrupt(bus, 1); //vblank

            if (LCDS_STAT_INT(SS_VBLANK)) {
                request_interrupt(bus, 2);
            }
            ppu_get_context()->current_frame++;

            //todo RAYLIB

        } else {
            LCDS_MODE_SET(MODE_OAM);
        }

        ppu_get_context()->line_ticks = 0;
    }
}
