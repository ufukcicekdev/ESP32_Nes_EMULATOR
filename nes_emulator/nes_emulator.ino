#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <string>
#include <vector>

#include "src/core/bus.h"
#include "src/controller.h"
#include "config.h"
#include "driver/i2s.h"
#include "src/ui.h"

// --- SİSTEM AYARLARI (GLOBAL) ---
int globalVolume = 70;      // 0-100 arası
bool showFPS = true;        // FPS Göstergesi
uint16_t themeColor = TFT_CYAN;
float currentFPS = 0;

// --- DURUMLAR ---
enum AppState { STATE_DASHBOARD, STATE_FILE_LIST, STATE_BROWSER, STATE_SETTINGS, STATE_EMULATING };
AppState currentState = STATE_DASHBOARD;

SPIClass SD_SPI(HSPI);
static Bus nes; 
static Cartridge* cart = nullptr; 

std::vector<std::string> fileList;
std::string currentPath = "/";
int selectedIndex = 0;
int scrollOffset = 0;
int dashSelected = 0; 
int settingsSelected = 0; // Ayarlar menüsü seçimi
bool needsRedraw = true;

unsigned long lastInputTime = 0;
const int inputDelay = 150;

// --- 1. SES MOTORU (APU) ---
void setupI2SExternal() {
    i2s_driver_uninstall(I2S_NUM_0);
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1 | ESP_INTR_FLAG_IRAM,
        // --- BURASI KRİTİK ---
        .dma_buf_count = 12,   // Buffer sayısını artırarak işlemciye nefes aldır
        .dma_buf_len = 256,   // Buffer boyunu küçülterek gecikmeyi (jitter) azalt
        .use_apll = false,
        .tx_desc_auto_clear = true
    };
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    
    i2s_pin_config_t pin_config = { 
        .bck_io_num = I2S_BCLK_PIN, 
        .ws_io_num = I2S_LRC_PIN, 
        .data_out_num = I2S_DOUT_PIN, 
        .data_in_num = I2S_PIN_NO_CHANGE 
    };
    i2s_set_pin(I2S_NUM_0, &pin_config);
    i2s_start(I2S_NUM_0); 
    i2s_zero_dma_buffer(I2S_NUM_0);
}

void apuTask(void* param) {
    Apu2A03* apu = (Apu2A03*)param;
    size_t bytes_written;
    i2s_zero_dma_buffer(I2S_NUM_0);

    while (true) {
        for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
            // NES Clock Hızı: Eğer ses çok cızırtılıysa 38-42 arasını dene.
            // Bu sayı saniyede kaç NES cycle'ının bir ses örneğine (sample) 
            // dönüşeceğini belirler.
            for(int j = 0; j < 40; j++) {
                apu->clock(); 
            }
            apu->generateSample(); 
        }

        // I2S'e yazarken portMAX_DELAY yerine küçük bir timeout ver
        i2s_write(I2S_NUM_0, 
                  Apu2A03::audio_buffer, 
                  sizeof(Apu2A03::audio_buffer), 
                  &bytes_written, 
                  pdMS_TO_TICKS(10)); 
        
        yield(); 
    }
}
// --- 2. DOSYA YÖNETİMİ ---
void scanDirectory(std::string path, bool sadeceOyun) {
    fileList.clear();
    selectedIndex = 0; scrollOffset = 0;
    if (path != "/") fileList.push_back("..");
    File root = SD.open(path.c_str());
    if(!root) return;
    File file = root.openNextFile();
    while (file) {
        std::string fn = file.name();
        if (fn[0] != '.' && fn.find("_.") == std::string::npos) {
            if (sadeceOyun) { if (fn.find(".nes") != std::string::npos) fileList.push_back(fn); }
            else { fileList.push_back(file.isDirectory() ? "[" + fn + "]" : fn); }
        }
        file = root.openNextFile();
    }
    root.close();
}

// --- 3. EMÜLASYON VE KONTROL ---
void forceExitEmulation(TaskHandle_t &apu_h) {
    if (apu_h != NULL) { vTaskDelete(apu_h); apu_h = NULL; }
    i2s_zero_dma_buffer(I2S_NUM_0);
    if (cart != nullptr) { delete cart; cart = nullptr; }
    digitalWrite(SD_CS_PIN, HIGH);
    lastInputTime = millis(); 
    needsRedraw = true;
}

void emulate(std::string fullPath) {
    cart = new Cartridge(fullPath.c_str());
    if (!cart) return;
    
    UI::clearScreen(); 
    nes.connectScreen(&screen); 
    nes.insertCartridge(cart);
    nes.reset();

    TaskHandle_t apu_h = NULL;
    xTaskCreatePinnedToCore(apuTask, "apuTask", 2028, &nes.cpu.apu, 10, &apu_h, 0);

    unsigned long lastFrameTime = micros();

    while (true) {
        uint16_t in = controllerRead();
        nes.controller = (uint8_t)in;

        // --- OYUN İÇİ SES KONTROLÜ (SELECT + UP/DOWN) ---
        if (in & CONTROLLER::Select) {
            if (in & CONTROLLER::Up && globalVolume < 100) { globalVolume += 2; delay(20); }
            if (in & CONTROLLER::Down && globalVolume > 0) { globalVolume -= 2; delay(20); }
        }

        // FPS Hesapla
        unsigned long now = micros();
        currentFPS = 1000000.0 / (now - lastFrameTime);
        lastFrameTime = now;

        if (in & CONTROLLER::GameMenu) {
            vTaskSuspend(apu_h);
            int mSel = 0; 
            UI::drawPauseMenu(mSel); 
            delay(250);
            bool menuOn = true;
            while(menuOn) {
                uint16_t mi = controllerRead();
                if (mi & CONTROLLER::Up) { mSel--; if(mSel<0) mSel=3; UI::drawPauseMenu(mSel); delay(150); }
                if (mi & CONTROLLER::Down) { mSel++; if(mSel>3) mSel=0; UI::drawPauseMenu(mSel); delay(150); }
                if (mi & CONTROLLER::A) {
                    if (mSel == 0) menuOn = false;
                    if (mSel == 1) { nes.saveState(); menuOn = false; }
                    if (mSel == 2) { nes.loadState(); menuOn = false; }
                    if (mSel == 3) { forceExitEmulation(apu_h); currentState = STATE_FILE_LIST; return; }
                }
                yield();
            }
            UI::clearScreen(); 
            vTaskResume(apu_h);
        }

        if (in & CONTROLLER::Menu) { forceExitEmulation(apu_h); currentState = STATE_DASHBOARD; return; }
        
        nes.clock(); 
        
        if(showFPS) UI::drawFPS(currentFPS);
    }
}

// --- 4. HANDLER FONKSİYONLARI ---
void handleDashboard() {
    uint16_t input = controllerRead();
    if (millis() - lastInputTime < inputDelay) return;
    if (input & CONTROLLER::Left && dashSelected > 0) { dashSelected--; needsRedraw = true; lastInputTime = millis(); }
    else if (input & CONTROLLER::Right && dashSelected < 2) { dashSelected++; needsRedraw = true; lastInputTime = millis(); }
    
    if (needsRedraw) { UI::drawDashboard(dashSelected); needsRedraw = false; }
    
    if (input & CONTROLLER::A) {
        lastInputTime = millis();
        if (dashSelected == 0) { currentPath = "/NES/"; scanDirectory(currentPath, true); currentState = STATE_FILE_LIST; }
        else if (dashSelected == 1) { currentPath = "/"; scanDirectory(currentPath, false); currentState = STATE_BROWSER; }
        else { currentState = STATE_SETTINGS; settingsSelected = 0; }
        needsRedraw = true;
    }
}

void handleSettings() {
    uint16_t in = controllerRead();
    if (millis() - lastInputTime < inputDelay) return;

    if (in & CONTROLLER::Up && settingsSelected > 0) { settingsSelected--; needsRedraw = true; lastInputTime = millis(); }
    if (in & CONTROLLER::Down && settingsSelected < 3) { settingsSelected++; needsRedraw = true; lastInputTime = millis(); }

    // Değer değiştirme (Sağ/Sol)
    if (settingsSelected == 0) { // Ses
        if (in & CONTROLLER::Right && globalVolume < 100) { globalVolume += 5; needsRedraw = true; lastInputTime = millis(); }
        if (in & CONTROLLER::Left && globalVolume > 0) { globalVolume -= 5; needsRedraw = true; lastInputTime = millis(); }
    }
    if (settingsSelected == 1) { // FPS
        if ((in & CONTROLLER::Right || in & CONTROLLER::Left)) { showFPS = !showFPS; needsRedraw = true; lastInputTime = millis(); }
    }

    if (in & CONTROLLER::B || (settingsSelected == 3 && in & CONTROLLER::A)) {
        currentState = STATE_DASHBOARD; needsRedraw = true; lastInputTime = millis(); return;
    }

    if (needsRedraw) {
        UI::drawSettings(settingsSelected, globalVolume, showFPS, themeColor);
        needsRedraw = false;
    }
}

void handleCommonList(bool isGameList) {
    uint16_t in = controllerRead();
    if (millis() - lastInputTime < inputDelay) return;
    if (in & CONTROLLER::Up && selectedIndex > 0) { selectedIndex--; if (selectedIndex < scrollOffset) scrollOffset = selectedIndex; needsRedraw = true; lastInputTime = millis(); }
    else if (in & CONTROLLER::Down && selectedIndex < (int)fileList.size()-1) { selectedIndex++; if (selectedIndex >= scrollOffset + 9) scrollOffset++; needsRedraw = true; lastInputTime = millis(); }
    if (in & CONTROLLER::B) { currentState = STATE_DASHBOARD; needsRedraw = true; lastInputTime = millis(); return; }
    
    if (in & CONTROLLER::A) {
        lastInputTime = millis();
        std::string sel = fileList[selectedIndex];
        if (sel == "..") {
            size_t last = currentPath.find_last_of('/', currentPath.length()-2);
            currentPath = currentPath.substr(0, last+1); scanDirectory(currentPath, isGameList);
            needsRedraw = true;
        } else if (sel[0] == '[') {
            currentPath += sel.substr(1, sel.length()-2) + "/"; scanDirectory(currentPath, isGameList);
            needsRedraw = true;
        } else if (isGameList && sel.find(".nes") != std::string::npos) {
            emulate(currentPath + sel);
            needsRedraw = true;
        }
    }
    
    if (needsRedraw) { 
        UI::drawList(isGameList ? "@withufuk OYUNLAR" : "@withufuk DOSYALAR", fileList, selectedIndex, scrollOffset); 
        needsRedraw = false; 
    }
}

// --- 5. SETUP VE LOOP ---
void setup() {
    Serial.begin(115200); 
    psramInit();
    UI::init(); 
    UI::drawBootScreen();
    setupI2SExternal();
    SD_SPI.begin(SD_SCLK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
    SD.begin(SD_CS_PIN, SD_SPI, 40000000);
    initController();
}

void loop() {
    switch(currentState) {
        case STATE_DASHBOARD: handleDashboard(); break;
        case STATE_FILE_LIST: handleCommonList(true); break;
        case STATE_BROWSER:   handleCommonList(false); break;
        case STATE_SETTINGS:  handleSettings(); break;
    }
}