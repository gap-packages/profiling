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

enum ProfType { Read = 1, Exec = 2, IntoFun = 3, OutFun = 4, StringId = 5, Info = 6, InvalidType = -1};

ProfType StringToProf(const std::string& s)
{
  if(s[0] == 'R') return Read;
  if(s[0] == 'E') return Exec;
  if(s[0] == 'I') return IntoFun;
  if(s[0] == 'O') return OutFun;
  if(s[0] == 'S') return StringId;
  if(s[0] == 'X') return Info;
  throw GAPException("Invalid Type in profile");
}

struct JsonParse
{
  ProfType Type;
  std::string Fun;
  int Ticks;
  int Line;
  int EndLine;
  std::string File;
  int FileId;

  JsonParse() : Type(InvalidType), Ticks(-1), Line(-1), EndLine(-1), FileId(-1)
    { }
};

bool ReadJson(char* str, JsonParse& ret)
{
  picojson::value v;
  std::string err;
  picojson::parse(v, str, str+strlen(str)+1, &err);
//Pr("0",0,0);
  if(!err.empty())
    return false;
//Pr("1",0,0);
  if(!v.is<picojson::object>())
    return false;
//Pr("2",0,0);
  if(!v.contains("Type") || !v.get("Type").is<std::string>())
    return false;
//Pr("3",0,0);
  ret.Type = StringToProf(v.get("Type").get<std::string>());

  if(ret.Type == StringId)
  {
    if(!v.contains("File") || !v.get("File").is<std::string>())
      return false;
    ret.File = v.get("File").get<std::string>();

    if(!v.contains("FileId") || !v.get("FileId").is<int64_t>())
      return false;
    ret.FileId = v.get("FileId").get<int64_t>();
    return true;
  }

  if(!v.contains("Line") || !v.get("Line").is<int64_t>())
    return false;
  ret.Line = v.get("Line").get<int64_t>();

  if(ret.Type == IntoFun || ret.Type == OutFun)
  {
    if(!v.contains("Fun") || !v.get("Fun").is<std::string>())
      return false;
    ret.Fun = v.get("Fun").get<std::string>();

    if(!v.contains("EndLine") || !v.get("EndLine").is<int64_t>())
      return false;
    ret.EndLine = v.get("EndLine").get<int64_t>();

    if(!v.contains("File") || !v.get("File").is<std::string>())
      return false;
    ret.File = v.get("File").get<std::string>();
    return true;
  }

  if(!v.contains("FileId") || !v.get("FileId").is<int64_t>())
    return false;
  ret.FileId = v.get("FileId").get<int64_t>();

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
  int endline;

  FullFunction() {}
  FullFunction(const std::string& _name, const std::string _file, int _line, int _endline)
    : name(_name), filename(_file), line(_line), endline(_endline)
  { }

  friend bool operator<(const FullFunction& lhs, const FullFunction& rhs)
  {
    if(lhs.line < rhs.line) return true;
    if(lhs.line > rhs.line) return false;
    if(lhs.endline < rhs.endline) return true;
    if(lhs.endline > rhs.endline) return true;
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
    r.set("endline", f.endline);
    r.set("name", f.name);
    r.set("filename", f.filename);
    return r.raw_obj();
  }
};
}

FullFunction buildFunctionName(const JsonParse& jp)
{ return FullFunction(jp.Fun, jp.File, jp.Line, jp.EndLine); }

// We lazily set up 'children' because we can't be bothered using
// a shared_ptr
struct StackTrace
{
    int runtime;
    int calls;
    std::map<FullFunction, StackTrace>* children;
    StackTrace* parent;

    StackTrace() : runtime(0), calls(0),
    children(NULL), parent(NULL)
    { }

    StackTrace(StackTrace* p) : runtime(0), calls(0),
    children(NULL), parent(p)
    { }

    void setupChildren()
    {
      if(!children)
        children = new std::map<FullFunction, StackTrace>;
    }

    ~StackTrace()
    {
      if(children)
        delete children;
    }

    StackTrace(const StackTrace& st) :
    runtime(st.runtime), calls(st.calls), children(st.children), parent(st.parent)
    { assert(!children); }

};


void dumpRuntimes_in(StackTrace* st,
                     std::vector<std::pair<std::vector<FullFunction>, Int > >& ret,
                     std::vector<FullFunction>& stack)
{
    ret.push_back(std::make_pair(stack, st->runtime));
    for(std::map<FullFunction, StackTrace>::iterator it = st->children->begin();
        it != st->children->end();
        ++it)
    {
        stack.push_back(it->first);
        dumpRuntimes_in(&(it->second), ret, stack);
        stack.pop_back();
    }
}

std::vector<std::pair<std::vector<FullFunction>, Int > > dumpRuntimes(StackTrace* st)
{
    std::vector<std::pair<std::vector<FullFunction>, Int > > ret;
    std::vector<FullFunction> stack;
    dumpRuntimes_in(st, ret, stack);
    return ret;
}

struct TimeStash
{
  Int runtime;
  Int runtime_with_children;
  Int total_ticks;

  TimeStash(Int _l, Int _cl, Int _tt) :
  runtime(_l), runtime_with_children(_cl),
  total_ticks(_tt) { }
};

Obj READ_PROFILE_FROM_STREAM(Obj self, Obj stream, Obj param2)
{
    GAPFunction readline("IO_ReadLine");
    int failedparse = 0;
    bool firstExec = true;

    std::map<Int, std::string> filename_map;

    std::map<Int, std::set<Int> > read_lines;
    std::map<Int, std::map<Int, Int> > exec_lines;
    std::map<Int, std::map<Int, Int> > runtime_lines;
    std::map<Int, std::map<Int, Int> > runtime_with_children_lines;

    std::map<Int, std::map<Int, std::set<FullFunction> > > called_functions;
    StackTrace stacktrace;
    stacktrace.setupChildren();
    StackTrace* current_stack = &stacktrace;

    // prev_exec is the last function executed, calling_exec is the statement which
    // we would currently say called a function. The only time when there differ
    // is immediately after returning from a function.
    JsonParse prev_exec;
    JsonParse calling_exec;

    // These keeps track of us going down our function stack
    std::vector<FullFunction> function_stack;
    std::vector<JsonParse> line_stack;
    // this stores various time values
    // when we call a function, so we can correct everything on return.
    std::vector<TimeStash> line_times_stack;

    long long total_ticks = 0;

    while(true)
    {
      Obj gapstr = GAP_callFunction(readline, stream);
      if(!IS_STRING(gapstr) || !IS_STRING_REP(gapstr))
        return Fail;
      char* str = (char*)CHARS_STRING(gapstr);
      if(*str == 0)
        break;

      JsonParse ret;
      if(ReadJson(str, ret))
      {
        switch(ret.Type)
        {
          case InvalidType: ErrorReturnVoid("Internal Error",0,0,""); break;
          case StringId:
            if(filename_map.count(ret.FileId) > 0) {
              ErrorReturnVoid("Invalid input - Reused file-id",0,0,""); break;
            }
            filename_map[ret.FileId] = ret.File;
          break;
          case IntoFun:
          {
            called_functions[calling_exec.FileId][calling_exec.Line].insert(buildFunctionName(ret));
            function_stack.push_back(buildFunctionName(ret));
            line_stack.push_back(calling_exec);
            line_times_stack.push_back(
              TimeStash(runtime_lines[calling_exec.FileId][calling_exec.Line],
                        runtime_with_children_lines[calling_exec.FileId][calling_exec.Line],
                        total_ticks));

            StackTrace* next_stack = &((*(current_stack->children))[function_stack.back()]);
            next_stack->setupChildren();

            if(!next_stack->parent)
                next_stack->parent = current_stack;
            assert(next_stack->parent == current_stack);
            current_stack = next_stack;
            (current_stack->calls)++;
          }
          break;
          case OutFun:
          {
            if(current_stack->parent)
            {
                current_stack = current_stack->parent;
                calling_exec = line_stack.back();
                TimeStash ts = line_times_stack.back();
                runtime_with_children_lines[calling_exec.FileId][calling_exec.Line] =
                  ts.runtime_with_children + (total_ticks - ts.total_ticks) -
                    (runtime_lines[calling_exec.FileId][calling_exec.Line] - ts.runtime);

                line_stack.pop_back();
                line_times_stack.pop_back();
            }
          }
          break;

          case Read:
          case Exec:

          if(ret.Type == Read)
          {
            read_lines[ret.FileId].insert(ret.Line);
          }
          else
          {
            exec_lines[ret.FileId][ret.Line]++;
            if(firstExec)
              firstExec = false;
            else
            {
              // The ticks are since the last executed line
              runtime_lines[prev_exec.FileId][prev_exec.Line]+=ret.Ticks;
              // Hard to know exactly where to charge these to --
              // this is easiest
              (current_stack->runtime) += ret.Ticks;
              total_ticks += ret.Ticks;
            }
          }

          case Info:; // ignored
        }
      }
      else
      {
        // We allow a couple of failed parses to deal with truncated files
        failedparse++;
        if(failedparse > 2) {
          return Fail;
        }
      }


      if(ret.Type == Exec) { prev_exec = ret; calling_exec = ret; }
      if(ret.Type == Info) { calling_exec = ret; }
    }


    // Now lets build a bunch of stuff which GAP will want back.
    // This stores the read, exec and runtime data.
    // vector of [filename, [ [read,exec,runtime] of line 1, [read,exec,runtime] of line 2, ... ] ]

    std::vector<std::pair<std::string, std::vector<std::vector<Int> > > > read_exec_data;

    std::vector<std::pair<std::string, std::vector<std::set<FullFunction> > > > called_functions_ret;

    // First gather all used filenames
    std::set<Int> filenameids;

    for(std::map<Int, std::set<Int> >::iterator it = read_lines.begin(); it != read_lines.end(); ++it)
      filenameids.insert(it->first);

    for(std::map<Int, std::map<Int,Int> >::iterator it = exec_lines.begin(); it != exec_lines.end(); ++it)
      filenameids.insert(it->first);

    for(std::map<Int, std::map<Int,Int> >::iterator it = runtime_lines.begin(); it != runtime_lines.end(); ++it)
      filenameids.insert(it->first);

    // Now we have all our filenames!
    // clear out a marker we use for start of file
    filenameids.erase(-1);

    for(std::set<Int>::iterator it = filenameids.begin(); it != filenameids.end(); ++it)
    {
      const std::set<Int>& read_set = read_lines[*it];
      std::map<Int,Int>& exec_set = exec_lines[*it];
      std::map<Int,Int>& runtime = runtime_lines[*it];
      std::map<Int,Int>& runtime_children = runtime_with_children_lines[*it];
      std::map<Int, std::set<FullFunction> >& functions = called_functions[*it];

      Int max_line = 0;

      if(!read_set.empty())
        max_line = std::max(max_line, *(read_set.rbegin()));

      if(!exec_set.empty())
        max_line = std::max(max_line, exec_set.rbegin()->first);

      if(!runtime.empty())
        max_line = std::max(max_line, runtime.rbegin()->first);

      if(!runtime_with_children_lines.empty())
        max_line = std::max(max_line, runtime_with_children_lines.rbegin()->first);

      if(!called_functions.empty())
        max_line = std::max(max_line, called_functions.rbegin()->first);

      std::vector<std::vector<Int> > line_data;
      std::vector<std::set<FullFunction> > called_data;
      for(int i = 1; i <= max_line; ++i)
      {
        std::vector<Int> data;
        data.push_back(read_set.count(i));
        data.push_back(exec_set[i]);
        data.push_back(runtime[i]);
        data.push_back(runtime_children[i]);
        line_data.push_back(data);

        called_data.push_back(functions[i]);
      }

      if(filename_map.count(*it) == 0)
      {
        Pr("Warning: damaged profile, cannot find a filename to match id %d", *it, 0L);
      }
      else
      {
        read_exec_data.push_back(std::make_pair(filename_map[*it], line_data));
        called_functions_ret.push_back(std::make_pair(filename_map[*it], called_data));
      }
    }

    std::vector<std::pair<std::vector<FullFunction>, Int> > function_stack_runtimes = dumpRuntimes(&stacktrace);

    GAPRecord r;

    r.set("line_info", read_exec_data);
    r.set("stack_runtimes", function_stack_runtimes);
    r.set("line_function_calls", called_functions_ret);

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
