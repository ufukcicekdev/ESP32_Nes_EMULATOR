#ifndef MAPPER002_H
#define MAPPER002_H

#include "../mapper.h"

#define MAPPER002_NUM_PRG_BANKS_16K 7
struct Mapper002_state
{
    Cartridge* cart;
    uint8_t number_PRG_banks;
    uint8_t number_CHR_banks;
    uint8_t* ptr_16K_PRG_banks[2];
    Bank prg_banks[MAPPER002_NUM_PRG_BANKS_16K];
    BankCache prg_cache;
    uint8_t PRG_bank[16*1024];
    uint8_t CHR_bank[8*1024];
};

Mapper createMapper002(uint8_t PRG_banks, uint8_t CHR_banks, Cartridge* cart);

#endif