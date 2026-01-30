#ifndef MAPPER000_H
#define MAPPER000_H

#include "../mapper.h"

struct Mapper000_state
{
    Cartridge* cart;
    uint8_t number_PRG_banks;
    uint8_t number_CHR_banks;
    uint8_t PRG_ROM[32*1024];
    uint8_t CHR_ROM[8*1024];
    uint8_t* CHR_bank;
    uint8_t* PRG_banks[2];
};

Mapper createMapper000(uint8_t PRG_banks, uint8_t CHR_banks, Cartridge* cart);

#endif