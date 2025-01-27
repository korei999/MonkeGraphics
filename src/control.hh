#pragma once

#include "adt/Arr.hh"
#include "adt/types.hh"

namespace control
{

constexpr int MAX_KEYBINDS = 128;
constexpr int MAX_KEYS = 256;

enum REPEAT_KEY : adt::u8 { ONCE, REPEAT };

struct Keybind
{
    REPEAT_KEY eRepeat;
    adt::u8 key;
    void (*pfn)();
};

extern bool g_aPressed[MAX_KEYS];
extern adt::Arr<Keybind, MAX_KEYBINDS> g_aKeybinds;
extern adt::Arr<Keybind, MAX_KEYBINDS> g_aModbinds; /* exec after g_aKeybinds */

void procKeys();

} /* namespace control */
