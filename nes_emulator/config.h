#ifndef CONFIG_H
#define CONFIG_H


// config.h içine uygun bir yere ekle
extern int globalVolume;       // 0-100 arası
extern bool showFPS;           // FPS sayacı açık mı?
extern uint16_t themeColor;    // Seçili tema rengi

// Kontrolcü Tipi (Buton kullanmadığın için 0 kalsın)
#define CONTROLLER_TYPE 0

// MicroSD kart hızı (Eski kartlar için 4MHz en güvenlisidir)
#define SD_FREQ 4000000

// Ekran Konfigürasyonu
#define SCREEN_ROTATION 3
#define SCREEN_SWAP_BYTES false



// --- SD KART FSPI PİNLERİ (Ayırdığın Hat) ---
#define SD_MOSI_PIN 14
#define SD_MISO_PIN 3
#define SD_SCLK_PIN 41
#define SD_CS_PIN   42

// Hata veren kontrolcü pinlerini etkisiz hale getiriyoruz
#define CONTROLLER_NES_LATCH -1
#define CONTROLLER_NES_DATA  -1
#define CONTROLLER_NES_CLK   -1
#define CONTROLLER_SNES_LATCH -1
#define CONTROLLER_SNES_DATA  -1
#define CONTROLLER_SNES_CLK   -1
#define CONTROLLER_PSX_ATTENTION -1
#define CONTROLLER_PSX_COMMAND   -1
#define CONTROLLER_PSX_CLK       -1
#define CONTROLLER_PSX_DATA      -1

// Butonlar (Şimdilik devre dışı)
#define A_BUTTON 0
#define B_BUTTON 20
#define UP_BUTTON 16
#define DOWN_BUTTON 38
#define LEFT_BUTTON 39
#define RIGHT_BUTTON 40
#define START_BUTTON 1
#define SELECT_BUTTON 2
#define MENU 5
#define GAME_MENU 6
#define GAME_MENU1 7
#define GAME_MENU2 47




// --- I2S SES PİNLERİ (MAX98357A) ---
#define I2S_BCLK_PIN 17   // Bit Clock
#define I2S_LRC_PIN  18   // Left/Right Clock (WS)
#define I2S_DOUT_PIN 21  // Data Out


#define SAMPLE_RATE 44100
#define FRAMESKIP
#define DEBUG true
#endif