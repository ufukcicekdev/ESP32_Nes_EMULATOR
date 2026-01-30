#ifndef PPU2C02_H
#define PPU2C02_H

#include <Arduino.h>
#include <stdint.h>

#include "cartridge.h"

#define BUFFER_SIZE 256 + 8 + 8
#define SCANLINE_SIZE 256
#define SCANLINES_PER_BUFFER 8
#define TILES_PER_SCANLINE 32
#define PIXELS_PER_TILE 8

class Bus;
class Ppu2C02
{
public:
    Ppu2C02();
    ~Ppu2C02();

public:
    void ppuWrite(uint16_t addr, uint8_t data);
    uint8_t ppuRead(uint16_t addr);
    void cpuWrite(uint16_t addr, uint8_t data);
    uint8_t cpuRead(uint16_t addr);

    void renderScanline(uint16_t scanline);
    void fakeSpriteHit(uint16_t scanline);
    void setVBlank();
    void clearVBlank();
    void reset();

    void connectBus(Bus* n) { bus = n; }
    void connectCartridge(Cartridge* cartridge);
    void setMirror(Cartridge::MIRROR mirror);
    Cartridge::MIRROR getMirror();

    void dumpState(File& state);
    void loadState(File& state);
private:
    Cartridge* cart = nullptr;
    Bus* bus = nullptr;

    void renderBackground();
    void renderSprites(uint16_t scanline);
    void transferScroll(uint16_t scanline);
    void incrementY();
    void finishScanline(uint16_t scanline);
    uint16_t scanline_buffer[BUFFER_SIZE];
    uint8_t scanline_metadata[BUFFER_SIZE];
    static uint16_t display_buffer[SCANLINE_SIZE * SCANLINES_PER_BUFFER];
    uint8_t nametable[2048];
    uint8_t* ptr_nametable[4];
    uint8_t palette_table[32];
    uint8_t scanline_counter = 0;

    static constexpr uint8_t palette_mirror[32] =
    {
        0x00,0x01,0x02,0x03, 0x04,0x05,0x06,0x07,
        0x08,0x09,0x0A,0x0B, 0x0C,0x0D,0x0E,0x0F,
        0x00,0x11,0x12,0x13, 0x04,0x15,0x16,0x17,
        0x08,0x19,0x1A,0x1B, 0x0C,0x1D,0x1E,0x1F
    };

    // PPU Registers
    // PPUCTRL
    union
    {
        struct
        {
            uint8_t nametable_x : 1;
            uint8_t nametable_y : 1;
            uint8_t VRAM_addr_increment : 1;
            uint8_t sprite_table_addr : 1;
            uint8_t background_table_addr : 1;
            uint8_t sprite_size : 1;
            uint8_t PPU_master_select : 1;
            uint8_t Vblank_NMI : 1;
        };
        uint8_t reg = 0x00;
    } control;

    // PPUMASK
    union
    {
        struct
        {
            uint8_t grayscale : 1;
            uint8_t render_background_left : 1;
            uint8_t render_sprite_left : 1;
            uint8_t render_background : 1;
            uint8_t render_sprite : 1;
            uint8_t emphasize_red : 1;
            uint8_t emphasize_green : 1;
            uint8_t emphasize_blue : 1;
        };
        uint8_t reg = 0x00;
    } mask;

    // PPUSTATUS
    union
    {
        struct
        {
            uint8_t unused : 5;
            uint8_t sprite_overflow : 1;
            uint8_t sprite_zero_hit : 1;
            uint8_t VBlank : 1;
        };
        uint8_t reg = 0x00;
    } status;

    // OAMADDR
    uint8_t OAMADDR = 0x00;
    // OAMDATA
    uint8_t OAMDATA = 0x00;

    // Internal register 
    typedef union
    {
        struct
        {
            uint16_t coarse_x : 5;
            uint16_t coarse_y : 5;
            uint16_t nametable_x : 1;
            uint16_t nametable_y : 1;
            uint16_t fine_y: 3;
            uint16_t unused : 1;
        };
        uint16_t reg = 0x00;
    } internal_register;

    // OAM
    typedef struct
    {
        uint8_t y;
        uint8_t index;
        uint8_t attribute;
        uint8_t x;
    } OAM;

    OAM sprite[64];
    internal_register v;
    internal_register t;
    uint8_t x;
    uint8_t w;
    uint8_t PPUDATA_buffer = 0x00;

    // Rendering
    uint16_t offset = 0x0000;
    uint8_t nametable_index = 0x00;
    uint16_t nametable_byte_base = 0x00;
    uint16_t attribute_byte_base = 0x00;
    uint8_t nametable_byte = 0x00;
    uint8_t attribute_byte = 0x00;
    uint8_t x_tile = 0x00;
    uint8_t y_tile = 0x00;
    uint8_t attribute_shift = 0x00;
    uint8_t attribute = 0x00;
    uint8_t tile_index = 0x00;
    uint8_t* ptr_tile = nullptr;
    uint8_t* ptr_attribute = nullptr;
    uint8_t* ptr_pattern_tile = nullptr;
    uint8_t* ptr_scanline_meta = nullptr;

    uint8_t sprite_count = 0;
public:
    uint8_t* ptr_sprite = (uint8_t*)sprite;
    uint16_t* ptr_buffer = scanline_buffer;
    static constexpr uint16_t* ptr_display = display_buffer;
};

#endif