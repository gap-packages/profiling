//  Please refer to the COPYRIGHT file of the profiling package for details.
//  SPDX-License-Identifier: MIT
#ifndef PROTOTYPES_HPP_ZLALA
#define PROTOTYPES_HPP_ZLALA

#include "compiled.h"   // GAP headers

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


