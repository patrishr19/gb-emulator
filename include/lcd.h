#ifndef LCD_H
#define LCD_H

#include <setup.h>

typedef struct {
    //registers
    uint8_t lcdc;
    uint8_t lcds;
    uint8_t scroll_y;
    uint8_t scroll_x;
    uint8_t ly;
    uint8_t ly_compare;
    uint8_t dma;
    uint8_t bg_palette;
    uint8_t obj_palette[2];
    uint8_t win_y;
    uint8_t win_x;

    //other 
    uint32_t bg_colors[4];
    uint32_t sp1_colors[4];
    uint32_t sp2_colors[4];
} lcd_context;

typedef enum {
    MODE_HBLANK,
    MODE_VBLANK,
    MODE_OAM,
    MODE_XFER
} lcd_mode;

lcd_context *lcd_get_context();

#define LCDC_BGW_ENABLE (BIT(lcd_get_context()->lcdc, 0))
#define LCDC_OBJ_ENABLE (BIT(lcd_get_context()->lcdc, 1))
#define LCDC_OBJ_HEIGHT (BIT(lcd_get_context()->lcdc, 2) ? 16 : 8)
#define LCDC_BG_MAP_AREA (BIT(lcd_get_context()->lcdc, 3) ? 0x9C00 : 0x9800)
#define LCDC_BGW_DATA_AREA (BIT(lcd_get_context()->lcdc, 4) ? 0x8000 : 0x8800)
#define LCDC_WIN_ENABLE (BIT(lcd_get_context()->lcdc, 5))
#define LCDC_WIN_MAP_AREA (BIT(lcd_get_context()->lcdc, 6) ? 0x9C00 : 0x9800)
#define LCDC_LCD_ENABLE (BIT(lcd_get_context()->lcdc, 7))

#define LCDS_MODE ((lcd_mode)(lcd_get_context()->lcds & 0b11))
#define LCDS_MODE_SET(mode) {lcd_get_context()->lcds &= ~0b11; lcd_get_context()->lcds |= mode}

#define LCDS_LYC (BIT(lcd_get_context()->lcds, 2))
#define LCDS_LYC_SET(b) (BIT_SET(lcd_get_context()->lcds, 2, b))

typedef enum {
    SS_HBLANK = (1 << 3),
    SS_VBLANK = (1 << 4),
    SS_OAM = (1 << 5),
    SS_LYC = (1 << 6)
} stat_src;

#define LCDS_STAT_INT(src) (lcd_get_context()->lcds & src)

void lcd_init();

uint8_t lcd_read(uint16_t address);

void lcd_write(uint16_t address, uint8_t value);


#endif