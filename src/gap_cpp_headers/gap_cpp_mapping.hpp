//  Please refer to the COPYRIGHT file of the profiling package for details.
//  SPDX-License-Identifier: MIT
#ifndef GAP_HELPER_PNFRE
#define GAP_HELPER_PNFRE

#include <stdexcept>
#include <string>
#include <exception>
#include <vector>
#include <deque>
#include <list>
#include <utility>
#include <set>

#include "gap_all.h"   // GAP headers

#include "gap_prototypes.hpp"
#include "gap_exception.hpp"
#include "gap_function.hpp"
#include "gap_wrapping.hpp"

#include "vec1.hpp"
#include "optional.hpp"

#if !defined(GAP_KERNEL_MAJOR_VERSION) || GAP_KERNEL_MAJOR_VERSION < 7
#define CONST_CSTR_STRING(x) CSTR_STRING(x)
#endif

namespace GAPdetail
{
template<typename T>
struct GAP_getter
{ };

// Yes, this is useful. It lets someone turn a GAP vector into a
// vec1<Obj>, without having to worry about GAP vector functions
// any more.
template<>
struct GAP_getter<Obj>
{
    bool isa(Obj) const
    { return true; }

    Obj operator()(Obj recval) const
    { return recval; }
};

template<>
struct GAP_getter<char*>
{
    bool isa(Obj recval) const
    { return IS_STRING(recval) && IS_STRING_REP(recval); }

    char* operator()(Obj recval) const
    {
        if(!isa(recval))
            throw GAPException("Invalid attempt to read string");
        return CSTR_STRING(recval);
    }
};

template<>
struct GAP_getter<std::string>
{
    bool isa(Obj recval) const
    { return IS_STRING(recval) && IS_STRING_REP(recval); }

    std::string operator()(Obj recval) const
    {
        if(!isa(recval))
            throw GAPException("Invalid attempt to read string");
        return std::string(CONST_CSTR_STRING(recval));
    }
};


template<>
struct GAP_getter<bool>
{
    bool isa(Obj recval) const
    { return (recval == True) || (recval == False); }

    bool operator()(Obj recval) const
    {
        if(recval == True)
            return true;
        if(recval == False)
            return false;
        if(recval == Fail)
            throw GAPException("Got 'fail' as a Boolean");
        throw GAPException("Not a bool!");
    }
};


template<>
struct GAP_getter<Int>
{
    bool isa(Obj recval) const
    { return IS_INTOBJ(recval); }

    Int operator()(Obj recval) const
    {
        if(!isa(recval))
            throw GAPException("Invalid attempt to read Int");
        return INT_INTOBJ(recval);
    }
};

template<typename Con>
Con fill_container(Obj rec)
{
    if(!(IS_SMALL_LIST(rec)))
        throw GAPException("Invalid attempt to read list");
    int len = LEN_LIST(rec);

    Con v;
    typedef typename Con::value_type T;
    GAP_getter<T> getter;
    for(int i = 1; i <= len; ++i)
    {
        v.push_back(getter(ELM_LIST(rec, i)));
    }
    return v;
}

template<typename T, typename U>
struct GAP_getter<std::pair<T, U> >
{
    bool isa(Obj recval) const
    { return IS_SMALL_LIST(recval) && LEN_LIST(recval) == 2; }

    std::pair<T,U> operator()(Obj rec) const
    {
      if(!isa(rec))
        throw GAPException("Invalid attempt to read pair");
      GAP_getter<T> get_T;
      GAP_getter<U> get_U;
      std::pair<T,U> p(get_T(ELM_LIST(rec, 1)), get_U(ELM_LIST(rec, 2)));
      return p;
    }
};

// This case, and next one, handle arrays with and without holes
template<typename T>
struct GAP_getter<vec1<T> >
{
    bool isa(Obj recval) const
    { return IS_SMALL_LIST(recval); }

    vec1<T> operator()(Obj rec) const
    { return fill_container<vec1<T> >(rec); }
};

template<typename T>
struct GAP_getter<std::vector<T> >
{
    bool isa(Obj recval) const
    { return IS_SMALL_LIST(recval); }

    std::vector<T> operator()(Obj rec) const
    { return fill_container<std::vector<T> >(rec); }
};


template<typename T>
struct GAP_getter<std::deque<T> >
{
    bool isa(Obj recval) const
    { return IS_SMALL_LIST(recval); }

    std::deque<T> operator()(Obj rec) const
    { return fill_container<std::deque<T> >(rec); }
};

template<typename T>
struct GAP_getter<std::list<T> >
{
    bool isa(Obj recval) const
    { return IS_SMALL_LIST(recval); }

    std::list<T> operator()(Obj rec) const
    { return fill_container<std::list<T> >(rec); }
};


template<typename Con, typename T>
Con fill_optional_container(Obj rec)
{
  if(!(IS_SMALL_LIST(rec)))
      throw GAPException("Invalid attempt to read list");
  int len = LEN_LIST(rec);

  Con v;
  GAP_getter<T> getter;
  for(int i = 1; i <= len; ++i)
  {
      if(ISB_LIST(rec, i))
      { v.push_back(getter(ELM_LIST(rec, i))); }
      else
      { v.push_back(optional<T>()); }
  }
  return v;
}

template<typename T>
struct GAP_getter<vec1<optional<T> > >
{
    bool isa(Obj recval) const
    { return IS_SMALL_LIST(recval); }

    vec1<optional<T> > operator()(Obj rec) const
    { return fill_optional_container<vec1<optional<T> >, T>(rec); }
};

template<typename T>
struct GAP_getter<std::vector<optional<T> > >
{
    bool isa(Obj recval) const
    { return IS_SMALL_LIST(recval); }

    std::vector<optional<T> > operator()(Obj rec) const
    { return fill_optional_container<std::vector<optional<T> >, T>(rec); }
};

template<typename T>
struct GAP_getter<std::deque<optional<T> > >
{
    bool isa(Obj recval) const
    { return IS_SMALL_LIST(recval); }

    std::deque<optional<T> > operator()(Obj rec) const
    { return fill_optional_container<std::deque<optional<T> >, T>(rec); }
};

template<typename T>
struct GAP_getter<std::list<optional<T> > >
{
    bool isa(Obj recval) const
    { return IS_SMALL_LIST(recval); }

    std::list<optional<T> > operator()(Obj rec) const
    { return fill_optional_container<std::list<optional<T> >, T>(rec); }
};


template<>
struct GAP_getter<GAPRecord>
{
  bool isa(Obj recval) const
  { return IS_REC(recval); }

  GAPRecord operator()(Obj rec) const
  {
    if(!isa(rec))
      throw GAPException("Not a record");

    return GAPRecord(rec);
  }
};

}

template<typename T>
T GAP_get(Obj rec)
{
    GAPdetail::GAP_getter<T> getter;
    return getter(rec);
}

template<typename T>
bool GAP_isa(Obj rec)
{
  GAPdetail::GAP_getter<T> getter;
  return getter.isa(rec);
}

Obj GAP_get_rec(Obj rec, UInt n)
{
    if(!IS_REC(rec))
        throw GAPException("Invalid attempt to read record");
    if(!ISB_REC(rec, n))
        throw GAPException(std::string("Unable to read value from rec"));
    return ELM_REC(rec, n);
}

// This is a special method. It gets a boolean from a record, and assumes
// it is 'false' if not present
bool GAP_get_maybe_bool_rec(Obj rec, UInt n)
{
    if(!IS_REC(rec))
        throw GAPException("Invalid attempt to read record");
    if(!ISB_REC(rec, n))
        return false;
    Obj b = ELM_REC(rec, n);
    if(b == True)
        return true;
    if(b == False)
        return false;
    throw GAPException("Record element is not a boolean");
}

namespace GAPdetail
{
template<typename T>
struct GAP_maker
{ };

template<>
struct GAP_maker<Int>
{
    Obj operator()(Int i)
    { return INTOBJ_INT(i); }
};

template<>
struct GAP_maker<bool>
{
    Obj operator()(bool b) const
    {
        if(b)
            return True;
        else
            return False;
    }
};

template<typename T>
Obj CopyContainerToGap(const T& v)
{
    size_t s = v.size();
    if(s == 0)
    {
      Obj l = NEW_PLIST(T_PLIST_EMPTY, 0);
      SET_LEN_PLIST(l, 0);
      CHANGED_BAG(l);
      return l;
    }
    Obj list = NEW_PLIST(T_PLIST_DENSE, s);
    SET_LEN_PLIST(list, s);
    CHANGED_BAG(list);
    GAP_maker<typename T::value_type> m;
    int pos = 1;
    for(typename T::const_iterator it = v.begin(); it != v.end(); ++it, ++pos)
    {
        SET_ELM_PLIST(list, pos, m(*it));
        CHANGED_BAG(list);
    }

    return list;
}

template<typename T>
struct GAP_maker<vec1<T> >
{
    Obj operator()(const vec1<T>& v) const
    {
        return CopyContainerToGap(v);
    }
};

template<typename T>
struct GAP_maker<std::vector<T> >
{
    Obj operator()(const std::vector<T>& v) const
    {
        return CopyContainerToGap(v);
    }
};

template<typename T>
struct GAP_maker<std::set<T> >
{
    Obj operator()(const std::set<T>& v) const
    {
        return CopyContainerToGap(v);
    }
};

template<>
struct GAP_maker<std::string>
{
    Obj operator()(const std::string& s) const
    {
      Obj o;
      size_t len = s.length();
      o = NEW_STRING(len);
      memcpy(CSTR_STRING(o), s.c_str(), len);
      return o;
    }
};

template<typename T, typename U>
struct GAP_maker<std::pair<T,U> >
{
    Obj operator()(const std::pair<T,U>& v) const
    {
        Obj list = NEW_PLIST(T_PLIST_DENSE, 2);
        SET_LEN_PLIST(list, 2);

        GAP_maker<T> m_t;
        SET_ELM_PLIST(list, 1, m_t(v.first));
        CHANGED_BAG(list);

        GAP_maker<U> m_u;
        SET_ELM_PLIST(list, 2, m_u(v.second));
        CHANGED_BAG(list);

        return list;
    }
};

template<>
struct GAP_maker<GAPRecord>
{
  Obj operator()(GAPRecord r) const
  { return r.raw_obj(); }
};

}

template<typename T>
Obj GAP_make(const T& t)
{
    GAPdetail::GAP_maker<T> m;
    return m(t);
}

Obj GAP_getGlobal(const char* name)
{
    UInt i = GVarName(name);
    Obj o =  VAL_GVAR(i);
    if(!o)
        throw GAPException("Missing global : " + std::string(name));
    return o;
}

// We would use CALL_0ARGS and friends here, but in C++
// we have to be more explicit with the types of our functions.
Obj GAP_callFunction(GAPFunction fun)
{
    return CALL_0ARGS(fun.getObj());
}

Obj GAP_callFunction(GAPFunction fun, Obj arg1)
{
    return CALL_1ARGS(fun.getObj(), arg1);
}

Obj GAP_callFunction(GAPFunction fun, Obj arg1, Obj arg2)
{
    return CALL_2ARGS(fun.getObj(), arg1, arg2);
}

Obj GAP_callFunction(GAPFunction fun, Obj arg1, Obj arg2, Obj arg3)
{
    return CALL_3ARGS(fun.getObj(), arg1, arg2, arg3);
}

struct GAP_convertor
{
    Obj o;

    GAP_convertor(Obj _o) : o(_o) { }

    template<typename T>
    operator T()
    {
        if(!GAP_isa<T>(o))
            throw GAPException("Failed to map GAP object to C++");
        return GAP_get<T>(o);
    }
};

// Register and deregister objects so they do not get garbage collected


void GAP_addRef(Obj o)
{
    static GAPFunction addRef("_YAPB_addRef");
    GAP_callFunction(addRef, o);
}

bool GAP_checkRef(Obj o)
{
    static GAPFunction checkRef("_YAPB_checkRef");
    return GAP_get<bool>(GAP_callFunction(checkRef, o));
}

void GAP_clearRefs()
{
    static GAPFunction clearRefs("_YAPB_clearRefs");
    GAP_callFunction(clearRefs);
}

void GAP_print(const std::string& s)
{ Pr(s.c_str(), 0, 0); }

#endif
