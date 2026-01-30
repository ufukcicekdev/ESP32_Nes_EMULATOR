#ifndef UI_H
#define UI_H

#include <TFT_eSPI.h>
#include <string>
#include <vector>

extern TFT_eSPI screen;
extern uint16_t themeColor; 
extern int globalVolume;

class UI {
public:
    static void init();
    static void clearScreen();
    static void drawFPS(float fps); 
    static void drawPauseMenu(int selected);
    static void drawVolumeBar(int vol);
    static void drawDashboard(int selected);

    static void drawBootScreen();
    
    // Hata veren fonksiyon: Parametreleri tam olarak buradaki gibi olmalı
    static void drawStatusBar(const char* sol, const char* sag);
    
    static void drawList(const char* title, const std::vector<std::string>& fileList, int selectedIndex, int scrollOffset);
    static void drawSettings(int selected, int currentVol, bool fpsOn, uint16_t theme);

private:
    static void drawBars();
    // Önemli: borderColor parametresi burada da olmalı
    static void drawWindowBox(int x, int y, int w, int h, uint16_t borderColor);
    static void drawGameIcon(int x, int y, uint16_t color);
    static void drawFileIcon(int x, int y, uint16_t color);
    static void drawSettingsIcon(int x, int y, uint16_t color);
};

#endif