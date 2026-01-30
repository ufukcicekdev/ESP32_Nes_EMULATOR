#include "apu2A03.h"
#include "bus.h"
#include "cpu6502.h"

DMA_ATTR uint16_t Apu2A03::audio_buffer[AUDIO_BUFFER_SIZE * 2];

Apu2A03::Apu2A03() {
    memset(audio_buffer, 0, sizeof(audio_buffer));
    cycle_accumulator = 0.0f;
    buffer_index = 0;
}

Apu2A03::~Apu2A03() {}

void Apu2A03::reset() {
    pulse1_enable = pulse2_enable = triangle_enable = noise_enable = DMC_enable = false;
    IRQ = false;
    pulse1.len_counter.timer = pulse2.len_counter.timer = 0;
    triangle.len_counter.timer = noise.len_counter.timer = 0;
    DMC.output_unit.output_level = 0;
    DMC.output_unit.silence_flag = true;
    clock_counter = 0;
    cycle_accumulator = 0.0f;
}

IRAM_ATTR void Apu2A03::cpuWrite(uint16_t addr, uint8_t data) {
    switch (addr) {
    case 0x4000:
        pulse1.seq.duty_cycle = ((data & 0xC0) >> 6);
        pulse1.env.loop = ((data >> 5) & 0x01);
        pulse1.len_counter.halt = ((data >> 5) & 0x01);
        pulse1.env.constant_volume = ((data >> 4) & 0x01);
        pulse1.env.volume = (data & 0x0F);
        break;
    case 0x4001:
        pulse1.sweep.enable = (data >> 7);
        pulse1.sweep.reload = ((data >> 4) & 0x07) + 1;
        pulse1.sweep.negate = ((data >> 3) & 0x01);
        pulse1.sweep.shift_count = data & 0x07;
        pulse1.sweep.reload_flag = true;
        break;
    case 0x4002:
        pulse1.seq.reload = (pulse1.seq.reload & 0xFF00) | data;
        break;
    case 0x4003:
        pulse1.seq.cycle_position = 0;
        pulse1.seq.reload = (pulse1.seq.reload & 0x00FF) | (uint16_t)((data & 0x07) << 8);
        pulse1.seq.timer = pulse1.seq.reload;
        pulse1.env.start_flag = true;   
        if (pulse1_enable) pulse1.len_counter.timer = length_counter_lookup[data >> 3] + 1;
        pulse1.env.timer = pulse1.env.volume;
        pulse1.env.decay_level_counter = 15;
        break;
    case 0x4004:
        pulse2.seq.duty_cycle = ((data & 0xC0) >> 6);
        pulse2.env.loop = ((data >> 5) & 0x01);
        pulse2.len_counter.halt = pulse2.env.loop;
        pulse2.env.constant_volume = ((data >> 4) & 0x01);
        pulse2.env.volume = (data & 0x0F);
        break;
    case 0x4005:
        pulse2.sweep.enable = (data >> 7);
        pulse2.sweep.reload = ((data >> 4) & 0x07) + 1;
        pulse2.sweep.negate = ((data >> 3) & 0x01);
        pulse2.sweep.shift_count = data & 0x07;
        pulse2.sweep.reload_flag = true;
        break;
    case 0x4006:
        pulse2.seq.reload = (pulse2.seq.reload & 0xFF00) | data;
        break;
    case 0x4007:
        pulse2.seq.cycle_position = 0;
        pulse2.seq.reload = (pulse2.seq.reload & 0x00FF) | (uint16_t)((data & 0x07) << 8);
        pulse2.seq.timer = pulse2.seq.reload;
        pulse2.env.start_flag = true;
        if (pulse2_enable) pulse2.len_counter.timer = length_counter_lookup[data >> 3] + 1;
        pulse2.env.timer = pulse2.env.volume;
        pulse2.env.decay_level_counter = 15;
        break;
    case 0x4008:
        triangle.lin_counter.reload = data & 0x7F;
        triangle.len_counter.halt = data >> 7;
        triangle.lin_counter.control = data >> 7;
        break;
    case 0x400A:
        triangle.seq.reload = (triangle.seq.reload & 0xFF00) | data;
        break;
    case 0x400B:
        triangle.seq.reload = ((triangle.seq.reload & 0x00FF) | (uint16_t)((data & 0x07)) << 8) + 1;
        triangle.seq.timer = triangle.seq.reload;
        if (triangle_enable) triangle.len_counter.timer = length_counter_lookup[data >> 3] + 1;
        triangle.lin_counter.reload_flag = true;
        break;
    case 0x400C:
        noise.len_counter.halt = (data >> 5) & 0x01;
        noise.env.constant_volume = (data >> 4) & 0x01;
        noise.env.volume = data & 0x0F;
        break;
    case 0x400E:
        noise.mode = data >> 7;
        noise.reload = noise_period_lookup[data & 0x0F] / 2;
        break;
    case 0x400F:
        noise.env.start_flag = true;
        if (noise_enable) noise.len_counter.timer = length_counter_lookup[data >> 3] + 1;
        break;
    case 0x4010:
        DMC.IRQ_flag = data >> 7;
        DMC.loop_flag = (data & 0x40) == 0x40;
        DMC.reload = (DMC_rate_lookup[data & 0x0F] / 2) - 1;
        DMC.timer = DMC.reload;
        break;
    case 0x4011: DMC.output_unit.output_level = data & 0x7F; break;
    case 0x4012:
        DMC.sample_address = 0xC000 | ((uint32_t)data << 6);
        DMC.memory_reader.address = DMC.sample_address;
        break;
    case 0x4013:
        DMC.sample_length = (data << 4) | 0x0001;
        DMC.memory_reader.remaining_bytes = DMC.sample_length;
        break;
    case 0x4015:
        IRQ = false;
        pulse1_enable = (data & 0x01); if (!pulse1_enable) pulse1.len_counter.timer = 0;
        pulse2_enable = (data & 0x02); if (!pulse2_enable) pulse2.len_counter.timer = 0;
        triangle_enable = (data & 0x04); if (!triangle_enable) triangle.len_counter.timer = 0;
        noise_enable = (data & 0x08); if (!noise_enable) noise.len_counter.timer = 0;
        DMC_enable = (data & 0x10);
        break;
    case 0x4017:
        four_step_sequence_mode = ((data >> 7) == 0);
        interrupt_inhibit = ((data >> 6) & 0x01);
        if (interrupt_inhibit) IRQ = false;
        break;
    }
}

IRAM_ATTR uint8_t Apu2A03::cpuRead(uint16_t addr) {
    if (addr == 0x4015) { IRQ = false; return 0x00; }
    return 0x00;
}

// Bus.cpp veya Nes.cpp içinde olması gereken yapı:
// apu2A03.cpp içindeki clock() fonksiyonunu boşaltalım, sadece kanalları güncellesin
IRAM_ATTR void Apu2A03::clock() {
    // Sadece kanalları ilerlet
  pulseChannelClock(pulse1.seq, true); // Zorla true yap
    pulseChannelClock(pulse2.seq, true);
    triangleChannelClock(triangle, true);
    noiseChannelClock(noise, noise_enable);
    
    // generateSample() BURADA OLMAYACAK!
    clock_counter++;
}

// generateSample() fonksiyonunu olduğu gibi bırak, sadece gürlüğünü artırIRAM_ATTR void Apu2A03::generateSample() {
IRAM_ATTR void Apu2A03::generateSample() {
    if (buffer_index >= AUDIO_BUFFER_SIZE) buffer_index = 0;
    uint16_t index = (buffer_index << 1);

    int32_t val = 0;
    
    // KANALLARI ZORLA OKU
    // Sesi geri getirmek için en garantisi: Kanal sinyal üretiyorsa sabit değer ekle.
    if (pulse1.seq.output)   val += 15; 
    if (pulse2.seq.output)   val += 15;
    if (triangle.seq.output) val += 15;

    // Merkezi sıfırlama (DC Offset engellemek için toplamın yarısını çıkarıyoruz)
    // Bu işlem hoparlörün "vınlamasını" önler.
    int32_t centered = val - 22; 

    // SES SEVİYESİ (Güçlü çarpan)
    // globalVolume'un 0 olmadığını varsayıyoruz (min 50 olsun)
    int vol = (globalVolume < 10) ? 70 : globalVolume;
    int32_t final_val = centered * (vol * 5); 

    // 16-BIT SINIRLAMA
    if (final_val > 30000)  final_val = 30000;
    if (final_val < -30000) final_val = -30000;

    // I2S BUFFER YAZIMI (Signed 16-bit)
    audio_buffer[index]     = (int16_t)final_val;
    audio_buffer[index + 1] = (int16_t)final_val;
    
    buffer_index++;
}
void Apu2A03::processEnvelopes() {
    soundChannelEnvelopeClock(pulse1.env);
    soundChannelEnvelopeClock(pulse2.env);
    soundChannelEnvelopeClock(noise.env);
    linearCounterClock(triangle.lin_counter);
}

void Apu2A03::processLengthAndSweep() {
    soundChannelSweeperClock(pulse1);
    soundChannelLengthCounterClock(pulse1.len_counter);
    soundChannelSweeperClock(pulse2);
    soundChannelLengthCounterClock(pulse2.len_counter);
    soundChannelLengthCounterClock(triangle.len_counter);
    soundChannelLengthCounterClock(noise.len_counter);
}

// --- KANAL CLOCK FONKSIYONLARI ---
IRAM_ATTR void Apu2A03::pulseChannelClock(sequencerUnit& seq, bool enable) {
    if (!enable) return;
    seq.timer--;
    if (seq.timer == 0xFFFF) {
        seq.timer = seq.reload;
        seq.output = duty_sequences[seq.duty_cycle][seq.cycle_position];
        seq.cycle_position = (seq.cycle_position + 1) & 0x07;
    }
}

IRAM_ATTR void Apu2A03::triangleChannelClock(triangleChannel& triangle, bool enable) {
    if (!enable) return;
    triangle.seq.timer--;
    if (triangle.seq.timer == 0) {
        triangle.seq.timer = triangle.seq.reload;
        if (triangle.len_counter.timer > 0 && triangle.lin_counter.counter > 0 && triangle.seq.reload >= 2) {
            triangle.seq.output = triangle_sequence[triangle.seq.duty_cycle];
            triangle.seq.duty_cycle = (triangle.seq.duty_cycle + 1) & 0x1F;
        }
    }
}

IRAM_ATTR void Apu2A03::noiseChannelClock(noiseChannel& noise, bool enable) {
    if (!enable) return;
    noise.timer--;
    if (noise.timer == 0xFFFF) {
        noise.timer = noise.reload;
        uint16_t feedback = (noise.shift_register & 0x01) ^ (noise.mode ? (noise.shift_register >> 6 & 0x01) : (noise.shift_register >> 1 & 0x01));
        noise.shift_register = (noise.shift_register >> 1) | (feedback << 14);
    }
}

IRAM_ATTR void Apu2A03::DMCChannelClock(DMCChannel& DMC, bool enable) {
    if (!enable) return;
    DMC.timer--;
    if (DMC.timer == 0xFFFF) {
        DMC.timer = DMC.reload + 1;
        if (!DMC.output_unit.silence_flag) {
            if (DMC.output_unit.shift_register & 0x01) {
                if (DMC.output_unit.output_level <= 125) DMC.output_unit.output_level += 2;
            } else {
                if (DMC.output_unit.output_level >= 2) DMC.output_unit.output_level -= 2;
            }
            DMC.output_unit.shift_register >>= 1;
        }
        if (--DMC.output_unit.remaining_bits <= 0) {
            DMC.output_unit.remaining_bits = 8;
            if (DMC.sample_buffer_empty) DMC.output_unit.silence_flag = true;
            else {
                DMC.output_unit.silence_flag = false;
                DMC.output_unit.shift_register = DMC.sample_buffer;
                DMC.sample_buffer_empty = true;
            }
        }
    }
}

IRAM_ATTR void Apu2A03::soundChannelEnvelopeClock(envelopeUnit& envelope) {
    if (envelope.start_flag) {
        envelope.start_flag = false;
        envelope.decay_level_counter = 15;
        envelope.timer = envelope.volume + 1;
    } else {
        if (--envelope.timer == 0) {
            envelope.timer = envelope.volume + 1;
            if (envelope.decay_level_counter > 0) envelope.decay_level_counter--;
            else if (envelope.loop) envelope.decay_level_counter = 15;
        }
    }
    envelope.output = envelope.constant_volume ? envelope.volume : envelope.decay_level_counter;
}

IRAM_ATTR void Apu2A03::soundChannelSweeperClock(pulseChannel& channel) {
    channel.sweep.change = (channel.seq.reload >> channel.sweep.shift_count);
    if (channel.sweep.negate) channel.sweep.change = (channel.sweep.pulse_channel_number == 1) ? -channel.sweep.change - 1 : -channel.sweep.change;
    channel.sweep.target_period = channel.seq.reload + channel.sweep.change;
    channel.sweep.mute = (channel.seq.reload < 8 || channel.sweep.target_period > 0x7FF);
    if (--channel.sweep.timer == 0 || channel.sweep.reload_flag) {
        if (channel.sweep.enable && !channel.sweep.mute && channel.sweep.shift_count != 0) channel.seq.reload = channel.sweep.target_period;
        channel.sweep.timer = channel.sweep.reload;
        channel.sweep.reload_flag = false;
    }
}

IRAM_ATTR void Apu2A03::soundChannelLengthCounterClock(length_counter& len_counter) {
    if (len_counter.timer > 0 && !len_counter.halt) len_counter.timer--;
}

IRAM_ATTR void Apu2A03::linearCounterClock(linear_counter& lin_counter) {
    if (lin_counter.reload_flag) lin_counter.counter = lin_counter.reload;
    else if (lin_counter.counter > 0) lin_counter.counter--;
    if (!lin_counter.control) lin_counter.reload_flag = false;
}

void Apu2A03::setDMCBuffer() { /* Opsiyonel DMC buffer mantığı */ }