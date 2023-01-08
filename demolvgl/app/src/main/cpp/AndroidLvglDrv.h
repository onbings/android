#pragma once

#include "Renderer.h"

bool AndroidLvlgDrvInit(Renderer *_pRenderer, uint16_t _Width_U16, uint16_t _Height_U16);
bool AndroidLvlgDrvRun();
bool AndroidLvlgDrvShutdown();