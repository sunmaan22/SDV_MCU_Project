#include "KeySampler.hpp"
#include "main.h"

using namespace touchgfx;

void KeySampler::init()
{
}

bool KeySampler::sample(uint8_t& key)
{
    bool buttonPressed = false;
    bool newState = HAL_GPIO_ReadPin(USER_BUTTON_GPIO_Port, USER_BUTTON_Pin) == GPIO_PIN_SET;
    if (newState && !previousState)
    {
        buttonPressed = true;
        key = 1; // This is the "hardware button key" reffered in the TouchGFX Designer Interactions
    }
    previousState = newState;

    return buttonPressed;
}
