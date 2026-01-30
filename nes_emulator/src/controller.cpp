#include "controller.h"
#include "../config.h"
#include <Arduino.h>

uint16_t controllerRead() // uint16_t olarak güncellendi
{
    uint16_t state = 0;

#if CONTROLLER_TYPE == 0
    // Standart NES Butonları
    if (digitalRead(A_BUTTON)      == LOW) state |= CONTROLLER::A;
    if (digitalRead(B_BUTTON)      == LOW) state |= CONTROLLER::B;
    if (digitalRead(SELECT_BUTTON) == LOW) state |= CONTROLLER::Select;
    if (digitalRead(START_BUTTON)  == LOW) state |= CONTROLLER::Start;
    if (digitalRead(UP_BUTTON)     == LOW) state |= CONTROLLER::Up;
    if (digitalRead(DOWN_BUTTON)   == LOW) state |= CONTROLLER::Down;
    if (digitalRead(LEFT_BUTTON)   == LOW) state |= CONTROLLER::Left;
    if (digitalRead(RIGHT_BUTTON)  == LOW) state |= CONTROLLER::Right;

    // Yeni Fonksiyon Butonları
    if (digitalRead(MENU)       == LOW) state |= CONTROLLER::Menu;
    if (digitalRead(GAME_MENU)  == LOW) state |= CONTROLLER::GameMenu;
    if (digitalRead(GAME_MENU1) == LOW) state |= CONTROLLER::GameMenu1;
    if (digitalRead(GAME_MENU2) == LOW) state |= CONTROLLER::GameMenu2;

#elif CONTROLLER_TYPE == 1
    state = NESControllerRead(); // Gerekirse bu fonksiyonları da 16 bit yapabilirsin
#endif

    return state;
}

bool isDownPressed(CONTROLLER button)
{
    return (controllerRead() & button) != 0;
}

void initController()
{
#if CONTROLLER_TYPE == 0
    // Mevcut Pinler
    pinMode(A_BUTTON, INPUT_PULLUP);
    pinMode(B_BUTTON, INPUT_PULLUP);
    pinMode(LEFT_BUTTON, INPUT_PULLUP);
    pinMode(RIGHT_BUTTON, INPUT_PULLUP);
    pinMode(UP_BUTTON, INPUT_PULLUP);
    pinMode(DOWN_BUTTON, INPUT_PULLUP);
    pinMode(START_BUTTON, INPUT_PULLUP);
    pinMode(SELECT_BUTTON, INPUT_PULLUP);

    // Yeni Eklenen Fonksiyon Butonları
    pinMode(MENU, INPUT_PULLUP);
    pinMode(GAME_MENU, INPUT_PULLUP);
    pinMode(GAME_MENU1, INPUT_PULLUP);
    pinMode(GAME_MENU2, INPUT_PULLUP);
#endif
    // Diğer controller tipleri (NES, SNES, PSX) için gerekirse ekleme yapılabilir
}