#include "ui.h"
#include <Arduino.h>

TFT_eSPI screen = TFT_eSPI();

// --- MODERN RENK PALETİ ---
#define COLOR_NEON_CYAN   0x07FF // Oyunlar için
#define COLOR_NEON_PINK   0xF81F // Dosyalar için
#define COLOR_NEON_GOLD   0xFDE0 // Ayarlar için
#define COLOR_DARK_PURPLE 0x2008 // Modern gölge rengi
#define COLOR_SOFT_WHITE  0xDEFB // Yumuşak yazı rengi

void UI::init() {
    screen.begin();
    screen.setRotation(3);
    screen.setSwapBytes(true);
    screen.fillScreen(TFT_BLACK);
}
void UI::clearScreen() {
    screen.fillScreen(TFT_BLACK);
}

// --- MODERN ÇERÇEVE SİSTEMİ ---
// Bu fonksiyon artık menüye göre farklı renklerde çerçeve çizer
void UI::drawWindowBox(int x, int y, int w, int h, uint16_t borderColor){
    // 1. Derinlik efekti (Gölge)
    screen.drawRect(x + 2, y + 2, w, h, COLOR_DARK_PURPLE);
    // 2. Ana neon kenarlık
    screen.drawRect(x, y, w, h, borderColor);
    // 3. İç neon parlama (Çift katmanlı efekt)
    screen.drawRect(x + 1, y + 1, w - 2, h - 2, borderColor);
    // 4. İç dolgu (Tam siyah, oyunun görünmesi için)
    screen.fillRect(x + 2, y + 2, w - 4, h - 4, TFT_BLACK);
}

void UI::drawBars() {
    // Üst ve alt modern dekoratif çizgiler
    screen.drawFastHLine(0, 32, screen.width(), COLOR_NEON_GOLD);
    screen.drawFastHLine(0, screen.height() - 22, screen.width(), COLOR_NEON_GOLD);
}

void UI::drawStatusBar(const char* sol, const char* sag) {
    screen.fillRect(0, screen.height() - 20, screen.width(), 20, COLOR_DARK_PURPLE);
    screen.setTextColor(TFT_SILVER);
    screen.drawString(sol, 10, screen.height() - 15, 1);
    screen.drawRightString(sag, screen.width() - 10, screen.height() - 15, 1);
}

// --- OYUN İÇİ BİLGİLER ---
void UI::drawFPS(float fps) {
    // Sadece rakam değişirse yazdır, her döngüde değil!
    static int lastFPS = 0;
    if ((int)fps == lastFPS) return; 
    lastFPS = (int)fps;

    screen.setTextColor(TFT_GREEN, TFT_BLACK);
    screen.setCursor(5, 5);
    screen.printf("%d ", lastFPS); // Sadece rakamı güncelle
}

void UI::drawVolumeBar(int vol) {
    int x = screen.width() - 65;
    int y = 5;
    screen.drawRoundRect(x, y, 60, 12, 3, TFT_WHITE);
    screen.fillRect(x + 2, y + 2, 56, 8, TFT_BLACK);
    screen.fillRect(x + 2, y + 2, (vol * 56 / 100), 8, COLOR_NEON_GOLD);
}

// --- ANA DASHBOARD ---
void UI::drawDashboard(int selected) {
    screen.fillScreen(TFT_BLACK);
    
    // Header
    screen.fillRect(0, 0, screen.width(), 35, COLOR_DARK_PURPLE);
    screen.drawFastHLine(0, 35, screen.width(), COLOR_NEON_CYAN);
    screen.setTextColor(TFT_WHITE);
    screen.drawCentreString("@withufuk GAMING OS", screen.width()/2, 10, 2);

    const char* lbls[] = {"OYUNLAR", "DOSYALAR", "AYARLAR"};
    uint16_t cols[] = {COLOR_NEON_CYAN, COLOR_NEON_PINK, COLOR_NEON_GOLD};

    for(int i=0; i<3; i++) {
        uint16_t c = (selected == i) ? cols[i] : 0x4208; // Seçili olmayanı sönük yap
        int x = 15 + (i * 100);
        
        // Seçili olana parlama efekti
        if(selected == i) {
            screen.drawRoundRect(x-2, 58, 94, 104, 12, TFT_WHITE);
        }
        screen.drawRoundRect(x, 60, 90, 100, 10, c);
        
        if (i == 0) drawGameIcon(x, 60, c);
        else if (i == 1) drawFileIcon(x, 60, c);
        else drawSettingsIcon(x, 60, c);
        
        screen.setTextColor(c); 
        screen.drawCentreString(lbls[i], x+45, 175, 1);
    }
    drawStatusBar("YON: SEC", "A: GIRIS");
}

// --- OYUN LİSTESİ (Neon Cyan Çerçeve) ---
void UI::drawList(const char* title, const std::vector<std::string>& fileList, int selectedIndex, int scrollOffset) {
    screen.fillScreen(TFT_BLACK);
    
    // Modern Base Çerçeve (Cyan)
    drawWindowBox(5, 30, screen.width()-10, screen.height()-55, COLOR_NEON_CYAN);
    
    screen.setTextColor(COLOR_NEON_CYAN);
    screen.drawCentreString(title, screen.width()/2, 8, 2);

    for (int i = 0; i < 9; i++) {
        int idx = i + scrollOffset;
        if (idx < (int)fileList.size()) {
            int y = 45 + (i * 18);
            if (idx == selectedIndex) {
                screen.fillRect(12, y-1, screen.width()-24, 17, COLOR_NEON_CYAN);
                screen.setTextColor(TFT_BLACK);
            } else screen.setTextColor(COLOR_SOFT_WHITE);
            screen.drawString(fileList[idx].c_str(), 20, y);
        }
    }
    drawStatusBar("YON: ARA", "A: OYNA | B: GERI");
}

// --- AYARLAR MENÜSÜ (Neon Gold Çerçeve) ---
void UI::drawSettings(int selected, int currentVol, bool fpsOn, uint16_t theme) {
    screen.fillScreen(TFT_BLACK);
    
    // Modern Base Çerçeve (Gold)
    drawWindowBox(5, 30, screen.width()-10, screen.height()-55, COLOR_NEON_GOLD);
    
    screen.setTextColor(COLOR_NEON_GOLD);
    screen.drawCentreString("SISTEM AYARLARI", screen.width()/2, 8, 2);

    const char* opts[] = {"SES SEVIYESI", "FPS SAYACI", "TEMA RENGI", "CIKIS"};
    
    for(int i=0; i<4; i++) {
        uint16_t c = (selected == i) ? COLOR_NEON_GOLD : COLOR_SOFT_WHITE; 
        int y = 55 + (i * 30);
        
        if(selected == i) {
            screen.fillRect(12, y-2, screen.width()-24, 22, 0x3186);
        }
        
        screen.setTextColor(c);
        screen.drawString(opts[i], 30, y+4);
        
        if(i == 0) screen.drawRightString(String(currentVol).c_str(), screen.width()-45, y+4, 1);
        if(i == 1) screen.drawRightString(fpsOn ? "ACIK" : "KAPALI", screen.width()-45, y+4, 1);
    }
    drawStatusBar("YON: DEGISTIR", "B: KAYDET");
}

void UI::drawPauseMenu(int selected) {
    int mw = 180; int mh = 110;
    int mx = (screen.width() - mw) / 2;
    int my = (screen.height() - mh) / 2;
    
    // Duraklatma menüsü için beyaz/nötr çerçeve
    drawWindowBox(mx, my, mw, mh, TFT_WHITE);
    
    screen.setTextColor(TFT_YELLOW);
    screen.drawCentreString("DURAKLATILDI", screen.width()/2, my + 10, 2);
    
    const char* opts[] = {"DEVAM ET", "KAYDET", "YUKLE", "CIKIS"};
    for(int i=0; i<4; i++) {
        uint16_t color = (selected == i) ? TFT_BLACK : TFT_WHITE;
        uint16_t bg = (selected == i) ? COLOR_NEON_CYAN : COLOR_DARK_PURPLE;
        screen.fillRect(mx+10, my+35+(i*17), mw-20, 15, bg);
        screen.setTextColor(color);
        screen.drawCentreString(opts[i], screen.width()/2, my+37+(i*17), 1);
    }
}

// --- İKON TASARIMLARI ---
void UI::drawGameIcon(int x, int y, uint16_t color) {
    screen.drawRoundRect(x+20, y+35, 50, 30, 5, color);
    screen.fillRect(x+28, y+48, 10, 4, color);
    screen.fillRect(x+31, y+45, 4, 10, color);
    screen.fillCircle(x+58, y+50, 3, color);
    screen.fillCircle(x+65, y+50, 3, color);
}

void UI::drawFileIcon(int x, int y, uint16_t color) {
    screen.drawRect(x+30, y+35, 30, 40, color);
    screen.drawFastHLine(x+35, y+45, 20, color);
    screen.drawFastHLine(x+35, y+55, 20, color);
}

void UI::drawSettingsIcon(int x, int y, uint16_t color) {
    screen.drawCircle(x+45, y+55, 12, color);
    for(int i=0; i<8; i++) {
        float a = i * 45 * PI / 180;
        screen.drawLine(x+45+cos(a)*12, y+55+sin(a)*12, x+45+cos(a)*16, y+55+sin(a)*16, color);
    }
}


void UI::drawBootScreen() {
    screen.fillScreen(TFT_BLACK);
    uint16_t neon = 0x07FF; // Neon Cyan
    int cx = screen.width() / 2;
    int cy = screen.height() / 2;

    // 1. HIZLI IŞIK PATLAMASI (Hücum Eden Çizgiler)
    for (int i = 0; i < 360; i += 15) {
        float rad = i * PI / 180;
        int x2 = cx + cos(rad) * 150;
        int y2 = cy + sin(rad) * 150;
        screen.drawLine(cx, cy, x2, y2, COLOR_DARK_PURPLE);
        delay(5);
    }

    // 2. LOGO PARLAMASI (Yavaşça beliren yazı efekti)
    for (int i = 0; i < 255; i += 15) {
        // Renk tonunu griden beyaza doğru açıyoruz
        uint16_t grey = screen.color565(i, i, i);
        screen.setTextColor(grey);
        screen.drawCentreString("@withufuk", cx, cy - 25, 4);
        delay(10);
    }

    // 3. ALT ÇİZGİ VE İMZA (Neon Efekti)
    screen.drawFastHLine(cx - 60, cy + 5, 120, neon);
    screen.drawFastHLine(cx - 60, cy + 6, 120, neon); // Kalın çizgi
    screen.setTextColor(neon);
    screen.drawCentreString("BY @WITHUFUK", cx, cy + 15, 2);

    // 4. "SİSTEM HAZIR" SCANLINE (Yukarıdan aşağı inen bir tarama çizgisi)
    for (int y = 0; y < screen.height(); y += 4) {
        screen.drawFastHLine(0, y, screen.width(), 0x0101); // Çok silik bir mor çizgi
        delay(2);
    }

    delay(300); // Havalı duruş
}