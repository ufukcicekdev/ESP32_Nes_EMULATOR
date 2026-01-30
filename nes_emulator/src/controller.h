#ifndef CONTROLLER_H
#define CONTROLLER_H
#include <stdint.h>

enum CONTROLLER : uint16_t // 16-bit genişliğine çıkardık
{
    A      = (1 << 0), 
    B      = (1 << 1), 
    Select = (1 << 2), 
    Start  = (1 << 3), 
    Up     = (1 << 4), 
    Down   = (1 << 5), 
    Left   = (1 << 6), 
    Right  = (1 << 7),
    // Yeni Fonksiyon Butonları
    Menu      = (1 << 8),  // Ana menü (Exit)
    GameMenu  = (1 << 9),  // Oyun içi Pause menüsü
    GameMenu1 = (1 << 10), // Hızlı Kayıt (Quick Save)
    GameMenu2 = (1 << 11)  // Hızlı Yükleme (Quick Load)
};

void initController();
uint16_t controllerRead(); // Dönüş tipini uint16_t yaptık
bool isDownPressed(CONTROLLER button);



#endif