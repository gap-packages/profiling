//  Please refer to the COPYRIGHT file of the profiling package for details.
//  SPDX-License-Identifier: MIT
#ifndef _GAP_WRAP_HPP_AQD
#define _GAP_WRAP_HPP_AQD

#include "src/compiled.h"   // GAP headers

#include "gap_prototypes.hpp"
#include "gap_exception.hpp"

class GAPRecord
{
  Obj record;
public:
  GAPRecord()
  { record = NEW_PREC(0); }

  GAPRecord(Obj o) : record(o)
  {
    if(!IS_REC(o))
      throw GAPException("Not a record");
  }

  bool has(const char* c)
  {
    UInt n = RNamName(c);
    return ISB_REC(record, n);
  }

  Obj get(const char* c)
  {
    UInt n = RNamName(c);
    if(!has(c))
      throw GAPException("field not in record");

    return ELM_REC(record, n);
  }

  template<typename T>
  void set(const char* c, const T& t)
  {
    UInt n = RNamName(c);
    AssPRec(record, n, GAP_make(t));
  }

  Obj raw_obj() const
  { return record; }
};


#endif

