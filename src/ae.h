#pragma once

#include <map>
#include "platform.h"
#include "ae_string.h"
#include "ae_logging.h"
#include "ae_threading_spsc_queue.h"
#include "ae_input.h"

#define internal static
#define persist static
#define global static

using namespace std;

// services platform layer offers to game layer
internal void AeBusySleep(TDuration _duration);

// services game layer offers to platform layer
internal void AppUpdateAndRender(map<int, ControllerState> _gamepadIdx_to_GamepadState);

