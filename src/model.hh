#pragma once

#include "adt/String.hh"
#include "adt/Vec.hh"

namespace model
{

struct Mesh
{
};

struct Node
{
    adt::String sName {};
    adt::VecBase<adt::u16> aMeshIdxs {};
};

struct Object
{
    adt::VecBase<Node> m_aNodes {};
};

} /* namespace model */
