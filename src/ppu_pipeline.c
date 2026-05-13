#include <ppu.h>
#include <bus.h>
#include <cpu.h>
#include <setup.h>
#include <lcd.h>
#include <stdint.h>
#include <string.h>


void pixel_fifo_push(uint32_t value) {
    fifo_entry *next = malloc(sizeof(fifo_entry));
    next->next = NULL;
    next->value = value;

    if (!ppu_get_context()->pfc.pixel_fifo.head) {
        //first entry
        ppu_get_context()->pfc.pixel_fifo.head = ppu_get_context()->pfc.pixel_fifo.tail = next;
    } else {
        ppu_get_context()->pfc.pixel_fifo.tail->next = next;
        ppu_get_context()->pfc.pixel_fifo.tail = next;
    }

    ppu_get_context()->pfc.pixel_fifo.size++;
}

uint32_t pixel_fifo_pop() {
    if (ppu_get_context()->pfc.pixel_fifo.size <= 0) {
        fprintf(stderr, "ERROR in pixel fifo\n");
        exit(-8);
    }

    fifo_entry *popped = ppu_get_context()->pfc.pixel_fifo.head;
    ppu_get_context()->pfc.pixel_fifo.head = popped->next;
    ppu_get_context()->pfc.pixel_fifo.size--;

    uint32_t value = popped->value;
    free(popped);

    return value;
}

uint32_t fetch_sprite_pixels(int bit, uint32_t color, uint8_t bg_color) {
    for (int i=0; i<ppu_get_context()->fetched_entry_count; i++) {
	int sprite_x = (ppu_get_context()->fetched_entries[i].x - 8) + (lcd_get_context()->scroll_x % 8);

	if (sprite_x + 8 < ppu_get_context()->pfc.fifo_x) {
	    //past
	    continue;
	}

	int offset = ppu_get_context()->pfc.fifo_x - sprite_x;

	if (offset < 0 || offset > 7) {
	    continue;
	}

	bit = (7 - offset);

	if (ppu_get_context()->fetched_entries[i].f_x_flip) {
	    bit = offset;
	}

	uint8_t hi = !!(ppu_get_context()->pfc.fetch_entry_data[i * 2] & (1 << bit));
	uint8_t lo = !!(ppu_get_context()->pfc.fetch_entry_data[(i * 2) + 1] & (1 << bit)) << 1;

	bool bg_priority = ppu_get_context()->fetched_entries[i].f_bgp;

	if (!(hi|lo)) {
	    //bg transparent
	    continue;
	}

	if (!bg_priority || bg_color == 0) {
	    color = (ppu_get_context()->fetched_entries[i].f_pn) ? lcd_get_context()->sp2_colors[hi|lo] : lcd_get_context()->sp1_colors[hi|lo];

	    if (hi|lo) {
		break;
	    }
	}
    }
    return color;
}

bool pipeline_fifo_add() {
    if (ppu_get_context()->pfc.pixel_fifo.size > 8) {
        //full
        return false;
    }

    int x = ppu_get_context()->pfc.fetch_x - (8 - (lcd_get_context()->scroll_x % 8));

    for (int i=0; i<8; i++) {
        int bit = 7 - i;
        uint8_t lo = !!(ppu_get_context()->pfc.bgw_fetch_data[1] & (1 << bit));
        uint8_t hi = !!(ppu_get_context()->pfc.bgw_fetch_data[2] & (1 << bit)) << 1;
        uint32_t color = lcd_get_context()->bg_colors[hi | lo];
	
	if (!LCDC_BGW_ENABLE) {
	    color = lcd_get_context()->bg_colors[0];
	}

	if (LCDC_OBJ_ENABLE) {
	    color = fetch_sprite_pixels(bit, color, hi | lo);
	}

        if (x >= 0) {
            pixel_fifo_push(color);
            ppu_get_context()->pfc.fifo_x++;
        }
    }

    return true;
}

void pipeline_load_sprite_tile() {
    oam_line_entry *le = ppu_get_context()->line_sprites;

    while (le) {
	int sprite_x = (le->entry.x - 8) + (lcd_get_context()->scroll_x % 8);

	if ((sprite_x >= ppu_get_context()->pfc.fetch_x && sprite_x < ppu_get_context()->pfc.fetch_x + 8) || ((sprite_x + 8) >= ppu_get_context()->pfc.fetch_x
	&& (sprite_x + 8) < ppu_get_context()->pfc.fetch_x + 8)) {
	    //add
	    ppu_get_context()->fetched_entries[ppu_get_context()->fetched_entry_count++] = le->entry;
	}

	le = le->next;

	if (!le || ppu_get_context()->fetched_entry_count >= 3) {
	    //max 3
	    break;
	}
    }
}
void pipeline_load_sprite_data(uint8_t offset, Bus *bus) {
    int current_y = lcd_get_context()->ly;
    uint8_t sprite_height = LCDC_OBJ_HEIGHT;

    for (int i=0; i<ppu_get_context()->fetched_entry_count; i++) {
	uint8_t ty = ((current_y + 16) - ppu_get_context()->fetched_entries[i].y) * 2;

	if (ppu_get_context()->fetched_entries[i].f_y_flip) {
	    ty = ((sprite_height * 2) - 2) - ty;
	}

	uint8_t tile_index = ppu_get_context()->fetched_entries[i].tile;

	if (sprite_height == 16) {
	    tile_index &= ~(1);
	}

	ppu_get_context()->pfc.fetch_entry_data[(i * 2) + offset] = BusRead16(bus, (0x8000 + (tile_index * 16) + ty + offset));
    }
}

void pipeline_fetch(Bus *bus) {
    switch(ppu_get_context()->pfc.cur_fetch_state) {
        case FS_TILE: {
	    ppu_get_context()->fetched_entry_count = 0;
            if (LCDC_BGW_ENABLE) {
                ppu_get_context()->pfc.bgw_fetch_data[0] = BusRead(bus, (LCDC_BG_MAP_AREA + 
                (ppu_get_context()->pfc.map_x / 8) + 
                ((ppu_get_context()->pfc.map_y / 8) * 32)));
                
                if (LCDC_BGW_DATA_AREA == 0x8800) {
                    ppu_get_context()->pfc.bgw_fetch_data[0] += 128;
                    // int8_t signed_int = (int8_t)ppu_get_context()->pfc.bgw_fetch_data[0];
                    // ppu_get_context()->pfc.bgw_fetch_data[0] = (uint8_t)(signed_int + 128);
                    // ppu_get_context()->pfc.bgw_fetch_data[0] += 28;
                }

            }

	    if (LCDC_OBJ_ENABLE && ppu_get_context()->line_sprites) {
		pipeline_load_sprite_tile();
	    }
            ppu_get_context()->pfc.cur_fetch_state = FS_DATA0;
            ppu_get_context()->pfc.fetch_x += 8;
        } break;
        case FS_DATA0: {
            ppu_get_context()->pfc.bgw_fetch_data[1] = BusRead(bus, (LCDC_BGW_DATA_AREA + (ppu_get_context()->pfc.bgw_fetch_data[0] * 16) + ppu_get_context()->pfc.tile_y));

	    pipeline_load_sprite_data(0, bus);

            ppu_get_context()->pfc.cur_fetch_state = FS_DATA1;
        } break;
        case FS_DATA1: {
            ppu_get_context()->pfc.bgw_fetch_data[2] = BusRead(bus, (LCDC_BGW_DATA_AREA + (ppu_get_context()->pfc.bgw_fetch_data[0] * 16) + ppu_get_context()->pfc.tile_y + 1));

	    pipeline_load_sprite_data(1, bus);

            ppu_get_context()->pfc.cur_fetch_state = FS_IDLE;
        } break;
        case FS_IDLE: {
            ppu_get_context()->pfc.cur_fetch_state = FS_PUSH;
        } break;
        case FS_PUSH: {
            if (pipeline_fifo_add()) {
                ppu_get_context()->pfc.cur_fetch_state = FS_TILE;
            }
        } break;
    }
}

void pipeline_push_pixel() {
    if (ppu_get_context()->pfc.pixel_fifo.size > 8) {
        uint32_t pixel_data = pixel_fifo_pop();

        if (ppu_get_context()->pfc.line_x >= (lcd_get_context()->scroll_x % 8)) {
            ppu_get_context()->video_buffer[ppu_get_context()->pfc.pushed_x + (lcd_get_context()->ly * XRES)] = pixel_data;

            ppu_get_context()->pfc.pushed_x++;
        }

        ppu_get_context()->pfc.line_x++;
    }
}

void pipeline_process(Bus *bus) {
    ppu_get_context()->pfc.map_y = (lcd_get_context()->ly + lcd_get_context()->scroll_y) % 256;
    ppu_get_context()->pfc.map_x = (ppu_get_context()->pfc.fetch_x + lcd_get_context()->scroll_x) % 256;


    ppu_get_context()->pfc.tile_y = ((lcd_get_context()->ly + lcd_get_context()->scroll_y) % 8) * 2;

    if (!(ppu_get_context()->line_ticks & 1)) {
        pipeline_fetch(bus);
    }

    pipeline_push_pixel();

}

void pipeline_fifo_reset() {
    while(ppu_get_context()->pfc.pixel_fifo.size) {
        pixel_fifo_pop();
    }

    ppu_get_context()->pfc.pixel_fifo.head = 0;
}
