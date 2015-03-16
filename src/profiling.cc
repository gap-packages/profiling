/*
 * profiling: Line by line profiling and code coverage for GAP
 */

#include "gap_cpp_headers/gap_cpp_mapping.hpp"
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>

#define PICOJSON_USE_INT64
#include "picojson/picojson.h"



Obj TestCommand(Obj self)
{
    return INTOBJ_INT(42);
}

enum ProfType { Read = 1, Exec = 2, IntoFun = 3, OutFun = 4 };

ProfType StringToProf(const std::string& s)
{
  if(s == "R") return Read;
  if(s == "E") return Exec;
  if(s == "I") return IntoFun;
  if(s == "O") return OutFun;
  throw GAPException("Invalid Type in profile");
}

struct JsonParse
{
  ProfType Type;
  std::string Fun;
  int Ticks;
  int Line;
  std::string File;
};

bool ReadJson(const std::string& str, JsonParse& ret)
{
  std::istringstream iss(str);
  picojson::value v;
  iss >> v;
//Pr("0",0,0);
  if(iss.fail())
    return false;
//Pr("1",0,0);
  if(!v.is<picojson::object>())
    return false;
//Pr("2",0,0);
  if(!v.contains("Type") || !v.get("Type").is<std::string>())
    return false;
//Pr("3",0,0);
  ret.Type = StringToProf(v.get("Type").get<std::string>());

  if(ret.Type == IntoFun || ret.Type == OutFun)
  {
    if(!v.contains("Fun") || !v.get("Fun").is<std::string>())
      return false;

    ret.Fun = v.get("Fun").get<std::string>();
    return true;
  }

  if(!v.contains("Line") || !v.get("Line").is<int64_t>())
    return false;
  ret.Line = v.get("Line").get<int64_t>();
      
  if(!v.contains("File") || !v.get("File").is<std::string>())
    return false;
  ret.File = v.get("File").get<std::string>();

  ret.Ticks = 0;

  // this one is optional
  if(v.contains("Ticks") && v.get("Ticks").is<int64_t>())
  {
    ret.Ticks = v.get("Ticks").get<int64_t>();
  }

  return true;
}

struct FullFunction
{
  std::string name;
  std::string filename;
  int line;

  FullFunction() {}
  FullFunction(const std::string& _name, const std::string _file, int _line)
    : name(_name), filename(_file), line(_line)
  { }

  friend bool operator<(const FullFunction& lhs, const FullFunction& rhs)
  {
    if(lhs.line < rhs.line) return true;
    if(lhs.line > rhs.line) return false;
    if(lhs.name < rhs.name) return true;
    if(lhs.name > rhs.name) return false;
    if(lhs.filename < rhs.filename) return true;
    if(lhs.filename > rhs.filename) return false;

    return false;
  }
};

namespace GAPdetail {
template<>
struct GAP_maker<FullFunction>
{
  Obj operator()(const FullFunction& f)
  {
    GAPRecord r;
    r.set("line", f.line);
    r.set("name", f.name);
    r.set("filename", f.filename);
    return r.raw_obj();
  }
};
}

FullFunction buildFunctionName(const std::string& s, const JsonParse& jp)
{
  return FullFunction(s, jp.File, jp.Line);
}

Obj READ_PROFILE_FROM_STREAM(Obj self, Obj stream, Obj param2)
{
    GAPFunction readline("IO_ReadLine");
 
    bool firstExec = true;
    bool setupFunctionName = false;

    std::map<std::string, std::set<int> > read_lines;
    std::map<std::string, std::set<int> > exec_lines;
    std::map<std::string, std::map<int, int> > runtime_lines;

    std::map<FullFunction, int> runtime_in_function_map;
    
    std::map<std::vector<FullFunction>, int> runtime_stack_trace_map;
    // We cache where we are in this map, as doing the lookup is expensive
    int* runtime_stack_trace_lookup = 0;

    JsonParse prev_exec;

    std::vector<FullFunction> function_stack;

    std::string short_function_name;

    while(true)
    {
      Obj gapstr = GAP_callFunction(readline, stream); 
      if(!IS_STRING(gapstr) || !IS_STRING_REP(gapstr))
        return Fail;
      std::string str((char*)CHARS_STRING(gapstr));
      if(str.length() == 0)
        break;
       
      JsonParse ret;
      if(ReadJson(str, ret))
      {
        switch(ret.Type)
        {
          case IntoFun:
          {
            short_function_name = ret.Fun;
            setupFunctionName = true;
          }
          break;
          case OutFun:
          {
            if(setupFunctionName)
              setupFunctionName = false;
            else if(!function_stack.empty())
              function_stack.pop_back();  
          }break;

          case Read:
          case Exec:

          if(setupFunctionName)
          {
            function_stack.push_back(buildFunctionName(short_function_name, ret));
            runtime_stack_trace_lookup = &(runtime_stack_trace_map[function_stack]);
            setupFunctionName = false;
          }
          
          if(ret.Type == Read)
          {
            read_lines[ret.File].insert(ret.Line);
          }
          else
          {
            exec_lines[ret.File].insert(ret.Line);
            if(firstExec)
              firstExec = false;
            else
            {

              // The ticks are since the last executed line
              runtime_lines[prev_exec.File][prev_exec.Line]+=ret.Ticks;
              // Hard to know exactly where to charge these to --
              // this is easiest
              if(!function_stack.empty())
                runtime_in_function_map[function_stack.back()]+=ret.Ticks;
              if(runtime_stack_trace_lookup)
                *runtime_stack_trace_lookup+=ret.Ticks;
            }
          }
        }
      }

      if(ret.Type == Exec) prev_exec = ret;
    }
    

    // Now lets build a bunch of stuff which GAP will want back.
    // This stores the read, exec and runtime data.
    // vector of [filename, [ [read,exec,runtime] of line 1, [read,exec,runtime] of line 2, ... ] ]
    
    std::vector<std::pair<std::string, std::vector<std::vector<int> > > > read_exec_data;
    
    // First gather all used filenames
    std::set<std::string> filenames;
    for(std::map<std::string, std::set<int> >::iterator it = read_lines.begin(); it != read_lines.end(); ++it)
      filenames.insert(it->first);
    
    for(std::map<std::string, std::set<int> >::iterator it = exec_lines.begin(); it != exec_lines.end(); ++it)
      filenames.insert(it->first);

    for(std::map<std::string, std::map<int,int> >::iterator it = runtime_lines.begin(); it != runtime_lines.end(); ++it)
      filenames.insert(it->first);

    // Now we have all out filenames!
    
    for(std::set<std::string>::iterator it = filenames.begin(); it != filenames.end(); ++it)
    {
      const std::set<int>& read_set = read_lines[*it];
      const std::set<int>& exec_set = exec_lines[*it];
      std::map<int,int>& runtime = runtime_lines[*it];

      int max_line = 0;

      if(!read_set.empty())
        max_line = std::max(max_line, *(read_set.rbegin()));

      if(!exec_set.empty())
        max_line = std::max(max_line, *(exec_set.rbegin()));

      if(!runtime.empty())
        max_line = std::max(max_line, runtime.rbegin()->first);

      std::vector<std::vector<int> > line_data;
      for(int i = 1; i <= max_line; ++i)
      {
        std::vector<int> data;
        data.push_back(read_set.count(i));
        data.push_back(exec_set.count(i));
        data.push_back(runtime[i]);
        line_data.push_back(data); 
      }

      read_exec_data.push_back(std::make_pair(*it, line_data));
    }


    std::vector<std::pair<FullFunction, int> > function_runtimes;
    for(std::map<FullFunction, int>::iterator it = runtime_in_function_map.begin();
        it != runtime_in_function_map.end(); ++it)
    {
      function_runtimes.push_back(*it);
    }

    std::vector<std::pair<std::vector<FullFunction>, int> > function_stack_runtimes;
    for(std::map<std::vector<FullFunction>, int>::iterator it = runtime_stack_trace_map.begin(); 
        it != runtime_stack_trace_map.end(); ++it)
    {
      function_stack_runtimes.push_back(*it);
    }


    GAPRecord r;

    r.set("line_info", read_exec_data);
    r.set("fun_runtimes", function_runtimes);
    r.set("stack_runtimes", function_stack_runtimes);

    return GAP_make(r);
}


typedef Obj (* GVarFunc)(/*arguments*/);

#define GVAR_FUNC_TABLE_ENTRY(srcfile, name, nparam, params) \
  {#name, nparam, \
   params, \
   (GVarFunc)name, \
   srcfile ":Func" #name }

// Table of functions to export
static StructGVarFunc GVarFuncs [] = {
    GVAR_FUNC_TABLE_ENTRY("profiling.c", TestCommand, 0, ""),
    GVAR_FUNC_TABLE_ENTRY("profiling.c",READ_PROFILE_FROM_STREAM, 2, "param, param2"),

	{ 0 } /* Finish with an empty entry */

};

/******************************************************************************
*F  InitKernel( <module> )  . . . . . . . . initialise kernel data structures
*/
static Int InitKernel( StructInitInfo *module )
{
    /* init filters and functions                                          */
    InitHdlrFuncsFromTable( GVarFuncs );

    /* return success                                                      */
    return 0;
}

/******************************************************************************
*F  InitLibrary( <module> ) . . . . . . .  initialise library data structures
*/
static Int InitLibrary( StructInitInfo *module )
{
    /* init filters and functions */
    InitGVarFuncsFromTable( GVarFuncs );

    /* return success                                                      */
    return 0;
}

/******************************************************************************
*F  InitInfopl()  . . . . . . . . . . . . . . . . . table of init functions
*/
static StructInitInfo module = {
 /* type        = */ MODULE_DYNAMIC,
 /* name        = */ "profiling",
 /* revision_c  = */ 0,
 /* revision_h  = */ 0,
 /* version     = */ 0,
 /* crc         = */ 0,
 /* initKernel  = */ InitKernel,
 /* initLibrary = */ InitLibrary,
 /* checkInit   = */ 0,
 /* preSave     = */ 0,
 /* postSave    = */ 0,
 /* postRestore = */ 0
};

extern "C"
StructInitInfo * Init__Dynamic ( void )
{
  return &module;
}
