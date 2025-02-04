#pragma once

#include "Image.hh"
#include "gltf/gltf.hh"

#include "adt/String.hh"
#include "adt/Pool.hh"
#include "adt/Arena.hh"

namespace asset
{

enum TYPE : adt::u8
{
    UNKNOWN, IMAGE, MODEL
};

struct Object
{
    adt::Arena m_arena {};
    union
    {
        Image img;
        gltf::Model model;
    } m_uData {};
    TYPE m_eType {};

    /* */

    Object() = default;
    Object(adt::ssize prealloc) : m_arena(prealloc) {};
};

extern adt::Pool<Object, 128> g_assets;

bool load(adt::String sFilePath);
[[nodiscard]] Object* search(adt::String sKey, TYPE eType); /* may be null */
[[nodiscard]] Image* searchImage(adt::String sKey);
[[nodiscard]] gltf::Model* searchModel(adt::String sKey);

} /* namespace asset */
