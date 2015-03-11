#ifndef PROTOTYPES_HPP_ZLALA
#define PROTOTYPES_HPP_ZLALA

#include "include_gap_headers.hpp"

namespace GAPdetail
{
template<typename T>
struct GAP_getter;
}

template<typename T>
T GAP_get(Obj rec);

template<typename T>
bool GAP_isa(Obj rec);

namespace GAPdetail
{
template<typename T>
struct GAP_maker;
}

template<typename T>
Obj GAP_make(const T& t);

#endif


