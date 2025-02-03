#pragma once

#include "Image.hh"

#include "adt/String.hh"
#include "adt/Pool.hh"

namespace asset
{

enum TYPE : adt::u8
{
    UNKNOWN, IMAGE
};

struct Object
{
    union
    {
        Image img {};
    } uData {};
    TYPE eType {};
};

extern adt::Pool<Object, 128> g_assets;

bool load(adt::String sFilePath);
[[nodiscard]] Object* search(adt::String sKey); /* may be null */

} /* namespace asset */
