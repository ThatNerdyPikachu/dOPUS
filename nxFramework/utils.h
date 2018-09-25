#pragma once

#include <common.h>

namespace NXFramework
{

char*   ClockGetCurrentTime(void);
bool    GetBatteryStatus(ChargerType& state, u32& percent);

}
