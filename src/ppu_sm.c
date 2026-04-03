#include <bus.h>
#include <ppu.h>
#include <lcd.h>
#include <cpu.h>
#include <setup.h>
#include <iogm.h>

void increment_ly(Bus *bus) {
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