#include <setup.h>
#include <emulator.h>
#include <bus.h>
#include <ppu.h>
#include <dma.h>


typedef struct {
    bool active;
    uint8_t byte;
    uint8_t value;
    uint8_t start_delay;
} dma_context;

static dma_context ctx;

void dma_start(uint8_t start) {
    ctx.active = true;
    ctx.byte = 0;
    ctx.start_delay = 2;
    ctx.value = start;
}
void dma_tick(Bus *bus) {
    if (!ctx.active) {
        return;
    }

    if (ctx.start_delay) {
        ctx.start_delay--;
        return;
    }

    ppu_oam_write(ctx.byte, BusRead(bus, ((ctx.value * 0x100) + ctx.byte)));

    ctx.byte++;
    ctx.active = ctx.byte < 0xA0;

    if (!ctx.active) {
        // printf("DMA DONE\n");
        // sleep(2);
    }
}

bool dma_transfering() {
    return ctx.active;
}

