#include <gui/screen1_screen/Screen1View.hpp>

/* Bench-only ranges for the guide's Dummy phase, not a vehicle spec. */
static const int SPEED_GAUGE_MAX_KMH = 200;
static const int RPM_GAUGE_MAX = 8000;

Screen1View::Screen1View()
    : speedValueBuffer{}, rpmValueBuffer{}, gearValueBuffer{}
{

}

void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();

    textSpeedValue.setWildcard(speedValueBuffer);
    textRpmValue.setWildcard(rpmValueBuffer);
    textGear.setWildcard(gearValueBuffer);

    // Keep enough room for values, units, and signal status messages.
    textRpmValue.setPosition(17, 149, 176, 28);
    textSpeedValue.setPosition(286, 149, 176, 28);
    textGear.setPosition(216, 56, 48, 28);

    gaugeSpeed.setRange(0, SPEED_GAUGE_MAX_KMH);
    gaugeRpm.setRange(0, RPM_GAUGE_MAX);

    boxWarning.setVisible(false);
    textWarning.setVisible(false);
    textReady.setVisible(false);
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

static void FormatSignalText(touchgfx::Unicode::UnicodeChar* buffer, uint16_t bufferSize,
                              SignalStatus status, int value, const char* unit)
{
    switch (status)
    {
    case SIGNAL_VALID:
    {
        touchgfx::Unicode::UnicodeChar unicodeUnit[8] = {};
        touchgfx::Unicode::strncpy(unicodeUnit, unit, 7);
        touchgfx::Unicode::snprintf(buffer, bufferSize, "%d %s", value, unicodeUnit);
        break;
    }
    case SIGNAL_INVALID:
        touchgfx::Unicode::snprintf(buffer, bufferSize, "-- INVALID");
        break;
    case SIGNAL_TIMEOUT:
        touchgfx::Unicode::snprintf(buffer, bufferSize, "-- COMM LOST");
        break;
    case SIGNAL_NO_DATA:
    default:
        touchgfx::Unicode::snprintf(buffer, bufferSize, "--");
        break;
    }
}

void Screen1View::updateVehicleData(const VehicleDataSnapshot& data)
{
    FormatSignalText(speedValueBuffer, VALUE_BUFFER_SIZE, data.drive.status, (int)data.drive.speed_kmh, "km/h");
    gaugeSpeed.setValue(data.drive.status == SIGNAL_VALID ? (int)data.drive.speed_kmh : 0);
    textSpeedValue.invalidate();

    FormatSignalText(rpmValueBuffer, VALUE_BUFFER_SIZE, data.drive.status, (int)data.drive.rpm, "rpm");
    gaugeRpm.setValue(data.drive.status == SIGNAL_VALID ? (int)data.drive.rpm : 0);
    textRpmValue.invalidate();

    if (data.gear_ready.status == SIGNAL_VALID)
    {
        touchgfx::Unicode::snprintf(gearValueBuffer, GEAR_BUFFER_SIZE, "%c", data.gear_ready.gear);
    }
    else
    {
        touchgfx::Unicode::snprintf(gearValueBuffer, GEAR_BUFFER_SIZE, "-");
    }
    textGear.invalidate();

    bool readyVisible = (data.gear_ready.status == SIGNAL_VALID) && (data.gear_ready.ready != 0);
    textReady.setVisible(readyVisible);
    textReady.invalidate();

    textDemo.setVisible(data.demo_source != 0);
    textDemo.invalidate();

    bool warningVisible = (data.warning.active != 0);
    boxWarning.setVisible(warningVisible);
    textWarning.setVisible(warningVisible);
    boxWarning.invalidate();
    textWarning.invalidate();
}
