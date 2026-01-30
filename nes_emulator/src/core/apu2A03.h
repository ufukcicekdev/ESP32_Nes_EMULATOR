#ifndef APU2A03_H
#define APU2A03_H
#include "driver/i2s.h"
#include <Arduino.h>
#include <cstdint>
#include "config.h"

// Buffer boyutu 512, I2S akışının kesilmemesi için idealdir.
#define AUDIO_BUFFER_SIZE 512

class Bus;
class Cpu6502;

class Apu2A03
{
public:
    Apu2A03();
    ~Apu2A03();

    void connectBus(Bus* n) { bus = n; }
    void connectCPU(Cpu6502* n) { cpu = n; }
    void cpuWrite(uint16_t addr, uint8_t data);
    uint8_t cpuRead(uint16_t addr);
    void clock();
    void reset();

    // apuTask içinden erişebilmemiz için generateSample artık PUBLIC
    void generateSample(); 

    double output = 0.0; 
    double getOutput() { return output; } 

    // Statik buffer sayesinde I2S doğrudan bu belleğe erişebilir
    static uint16_t audio_buffer[AUDIO_BUFFER_SIZE * 2];
    bool IRQ = false;
    uint16_t buffer_index = 0;

private:
    Bus* bus = nullptr;
    Cpu6502* cpu = nullptr;
    
    uint32_t clock_counter = 0;
    float cycle_accumulator = 0.0f;
    
    bool four_step_sequence_mode = true;
    bool interrupt_inhibit = false;

    // --- SES KANALI YAPILARI ---
    struct sequencerUnit {
        uint8_t duty_cycle = 0x00;
        uint8_t cycle_position = 0x00;
        uint16_t timer = 0x0000;
        uint16_t reload = 0x0000;
        uint8_t output = 0x00;
    };
    struct linear_counter {
        bool control = false;
        bool reload_flag = false;
        uint8_t counter = 0x00;
        uint8_t reload = 0x00;
    };
    struct envelopeUnit {
        bool start_flag = false;
        bool loop = false;
        bool constant_volume = true;
        uint8_t volume = 0x00;
        uint8_t timer = 0x00;
        uint8_t output = 0x00;
        uint8_t decay_level_counter = 0x00;
    };
    struct sweepUnit {
        bool enable = false;
        bool negate = false;
        bool reload_flag = false;
        bool mute = false;
        uint8_t pulse_channel_number = 0;
        uint8_t shift_count = 0x00;
        int16_t change = 0x0000;
        uint16_t timer = 0x0000;
        uint16_t reload = 0x0000;
        int16_t target_period = 0x0000;
    };
    struct length_counter {
        bool enable = false;
        bool halt = false;
        uint8_t timer = 0x00;
    };
    struct memoryReader {
        uint16_t address = 0x0000;
        int16_t remaining_bytes = 0;
    };
    struct outputUnit {
        uint8_t shift_register = 0x00;
        int16_t remaining_bits = 0;
        uint8_t output_level = 0;
        bool silence_flag = false;
    };

    struct pulseChannel {
        sequencerUnit seq;
        envelopeUnit env;
        sweepUnit sweep;
        length_counter len_counter;
    };
    struct triangleChannel {
        sequencerUnit seq;
        length_counter len_counter;
        linear_counter lin_counter;
    };
    struct noiseChannel {
        envelopeUnit env;
        length_counter len_counter;
        uint16_t timer = 0x0000;
        uint16_t reload = 0x0000;
        uint16_t shift_register = 0x01;
        uint8_t output = 0x00;
        bool mode = false;
    };
    struct DMCChannel {
        bool IRQ_flag = false;
        bool loop_flag = false;
        bool sample_buffer_empty = true;
        uint8_t sample_buffer = 0x00;
        uint16_t sample_address = 0x0000;
        uint16_t sample_length = 0x0000;
        uint16_t timer = 0x0000;
        uint16_t reload = 0x0000;
        outputUnit output_unit;
        memoryReader memory_reader;
    };

    pulseChannel pulse1;
    bool pulse1_enable = false;
    pulseChannel pulse2;
    bool pulse2_enable = false;
    triangleChannel triangle;
    bool triangle_enable = false;
    noiseChannel noise;
    bool noise_enable = false;
    DMCChannel DMC;
    bool DMC_enable = false;

    // --- SABİTLER ---
    static constexpr uint8_t duty_sequences[4][8] = {
        {0, 1, 0, 0, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0, 0, 0},
        {0, 1, 1, 1, 1, 0, 0, 0}, {1, 0, 0, 1, 1, 1, 1, 1}
    };
    static constexpr uint8_t length_counter_lookup[32] = {
        10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14,
        12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
    };
    static constexpr uint8_t triangle_sequence[32] = {
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    static constexpr uint16_t noise_period_lookup[16] = {
        4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
    };
    static constexpr uint16_t DMC_rate_lookup[16] = {
        428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54
    };

    // void generateSample(); // <--- PRIVATE BÖLÜMÜNDEN SİLDİM
    void processEnvelopes(); 
    void processLengthAndSweep();
    void pulseChannelClock(sequencerUnit& seq, bool enable);
    void triangleChannelClock(triangleChannel& triangle, bool enable);
    void noiseChannelClock(noiseChannel& noise, bool enable);
    void DMCChannelClock(DMCChannel& DMC, bool enable);
    void soundChannelEnvelopeClock(envelopeUnit& envelope);
    void soundChannelSweeperClock(pulseChannel& channel);
    void soundChannelLengthCounterClock(length_counter& len_counter);
    void linearCounterClock(linear_counter& lin_counter);
    void setDMCBuffer();
};

#endif