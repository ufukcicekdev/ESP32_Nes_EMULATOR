#ifndef CPU6502_H
#define CPU6502_H

#include <Arduino.h>
#include <SD.h>
#include <stdint.h>

#include "apu2A03.h"
#include "cartridge.h"

#define GET_FLAG(f) ((status & (f)) != 0)
#define SET_FLAG(f, v) (status = (v) ? (status | (f)) : (status & ~(f)))
#define SET_ZN(v) (status = ((status & ~(Z | N)) | zn_table[(v)]))

class Bus;
class Cpu6502
{
public:
    Cpu6502();
    ~Cpu6502();

public:
    Apu2A03 apu;

    // Status Register Flags
    enum FLAGS
    {
        C = (1 << 0), // Carry Bit
        Z = (1 << 1), // Zero Bit
        I = (1 << 2), // Interrupt Bit
        D = (1 << 3), // Decimal Bit
        B = (1 << 4), // Break Bit
        U = (1 << 5), // Unused Bit
        V = (1 << 6), // Overflow Bit
        N = (1 << 7)  // Negative Bit
    };

    void apuWrite(uint16_t addr, uint8_t data);
    uint8_t apuRead(uint16_t addr);
    void clock(int i);
    void OAM_DMA(uint8_t page);
    void reset();

    void IRQ();
    void NMI();

    void dumpState(File& state);
    void loadState(File& state);

    void connectBus(Bus* n) { bus = n; }
    void connectCartridge(Cartridge* cartridge) { cart = cartridge; }

    // Registers 
    uint8_t A = 0x00; // Accumulator
    uint8_t X = 0x00; // X Index
    uint8_t Y = 0x00; // Y Index
    uint16_t PC = 0x0000; // Program Counter
    uint8_t SP = 0x00; // Stack Pointer
    uint8_t status = 0x00; // Status register

    uint8_t fetched = 0x00;
    uint16_t addr_abs = 0x0000;
    uint16_t addr_rel = 0x0000;
    uint8_t opcode = 0x00;
    uint16_t cycles = 0;
    uint16_t temp = 0x0000;

private: 
    Cartridge* cart = nullptr;
	Bus* bus = nullptr;
    bool addrmode_implied = false;
    uint8_t additional_cycle1 = 0;
    uint8_t additional_cycle2 = 0;
    uint8_t fetch();
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);
    void OAM_Write(uint8_t addr, uint8_t data);

	// Addressing Modes
	uint8_t ABS();	uint8_t IDX();
	uint8_t ABX();	uint8_t IDY();
	uint8_t ABY();	uint8_t REL();
	uint8_t IMM();	uint8_t ZPG();
	uint8_t IMP();	uint8_t ZPX();
	uint8_t IND();	uint8_t ZPY();		

	// Instructions
	uint8_t Instr_ADC(); uint8_t Instr_CLI(); uint8_t Instr_LDX(); uint8_t Instr_SED();
	uint8_t Instr_AND(); uint8_t Instr_CLV(); uint8_t Instr_LDY(); uint8_t Instr_SEI();
	uint8_t Instr_ASL(); uint8_t Instr_CMP(); uint8_t Instr_LSR(); uint8_t Instr_STA();
	uint8_t Instr_BCC(); uint8_t Instr_CPX(); uint8_t Instr_NOP(); uint8_t Instr_STX();
	uint8_t Instr_BCS(); uint8_t Instr_CPY(); uint8_t Instr_ORA(); uint8_t Instr_STY();
	uint8_t Instr_BEQ(); uint8_t Instr_DEC(); uint8_t Instr_PHA(); uint8_t Instr_TAX();
	uint8_t Instr_BIT(); uint8_t Instr_DEX(); uint8_t Instr_PHP(); uint8_t Instr_TAY();
	uint8_t Instr_BMI(); uint8_t Instr_DEY(); uint8_t Instr_PLA(); uint8_t Instr_TSX();
	uint8_t Instr_BNE(); uint8_t Instr_EOR(); uint8_t Instr_PLP(); uint8_t Instr_TXA();
	uint8_t Instr_BPL(); uint8_t Instr_INC(); uint8_t Instr_ROL(); uint8_t Instr_TXS();
	uint8_t Instr_BRK(); uint8_t Instr_INX(); uint8_t Instr_ROR(); uint8_t Instr_TYA();
	uint8_t Instr_BVC(); uint8_t Instr_INY(); uint8_t Instr_RTI(); uint8_t Instr_CLD();
	uint8_t Instr_BVS(); uint8_t Instr_JMP(); uint8_t Instr_RTS(); uint8_t Instr_LDA();
	uint8_t Instr_CLC(); uint8_t Instr_JSR(); uint8_t Instr_SBC(); uint8_t Instr_SEC();
	uint8_t Instr_XXX();

    // Instruction cycle count
    static const uint8_t instr_cycles[256];
    static const uint8_t zn_table[256];

    void DMC_DMA_Load();
    void DMC_DMA_Reload();

    uint16_t OAM_DMA_page = 0x00;
};

#endif