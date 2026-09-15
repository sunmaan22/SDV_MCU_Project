#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>
#include <touchgfx/Unicode.hpp>
#include "vehicle_data.h"

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    void updateVehicleData(const VehicleDataSnapshot& data);

protected:
private:
    static const uint16_t VALUE_BUFFER_SIZE = 20;
    static const uint16_t GEAR_BUFFER_SIZE = 4;
    touchgfx::Unicode::UnicodeChar speedValueBuffer[VALUE_BUFFER_SIZE];
    touchgfx::Unicode::UnicodeChar rpmValueBuffer[VALUE_BUFFER_SIZE];
    touchgfx::Unicode::UnicodeChar gearValueBuffer[GEAR_BUFFER_SIZE];
};

#endif // SCREEN1VIEW_HPP
