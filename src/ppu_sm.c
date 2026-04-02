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
    if (ppu_get_context()->line_ticks >= 80) {
        LCDS_MODE_SET(MODE_XFER);
    }
}
void ppu_mode_xfer() {
    if (ppu_get_context()->line_ticks >= 80 + 172) {
        LCDS_MODE_SET(MODE_HBLANK);
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