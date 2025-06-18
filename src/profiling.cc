//  Please refer to the COPYRIGHT file of the profiling package for details.
//  SPDX-License-Identifier: MIT
/*
 * profiling: Line by line profiling and code coverage for GAP
 */

#include "gap_cpp_headers/gap_cpp_mapping.hpp"
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>


#include "md5.h"


enum ProfType { Read = 1, Exec = 2, IntoFun = 3, OutFun = 4, StringId = 5, Info = 6, InvalidType = -1};

ProfType CharToProf(char c)
{
  if(c == 'R') return Read;
  if(c == 'E' || c == 'X') return Exec;
  if(c == 'I') return IntoFun;
  if(c == 'O') return OutFun;
  if(c == 'S') return StringId;
  if(c == '_') return Info;
  throw GAPException("Invalid 'Type' in profile");
}

struct JsonParse
{
  // Type of line
  ProfType Type;
  // Name of function (for function start/end)
  std::string Fun;
  // Time spent on line
  int Ticks;
  // Number of line executed (1 is ommitted from JSON)
  int Execs;
  // Line executed
  int Line;
  // End line of function (for function start)
  int EndLine;
  std::string File;
  int FileId;

  bool IsCover;
  std::string TimeType;
  JsonParse() : Type(InvalidType), Ticks(0), Execs(1), Line(-1), EndLine(-1), FileId(-1),
                IsCover(false)
    { }
};

#include "json_parse_rapidjson.h"


struct FullFunction
{
  std::string name;
  std::string filename;
  Int line;
  Int endline;

  FullFunction() {}
  FullFunction(const std::string& _name, const std::string _file, Int _line, Int _endline)
    : name(_name), filename(_file), line(_line), endline(_endline)
  { }

  friend bool operator<(const FullFunction& lhs, const FullFunction& rhs)
  {
    if(lhs.line < rhs.line) return true;
    if(lhs.line > rhs.line) return false;
    if(lhs.endline < rhs.endline) return true;
    if(lhs.endline > rhs.endline) return false;
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

struct Location
{
  std::string filename;
  std::string name;
  Int line;

  Location() {}
  Location(const std::string _name, const std::string _file, Int _line)
    :  name(_name), filename(_file), line(_line)
  { }

  friend bool operator<(const Location& lhs, const Location& rhs)
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
struct GAP_maker<Location>
{
  Obj operator()(const Location& f)
  {
    GAPRecord r;
    r.set("line", f.line);
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

static int endsWithgz(char* s)
{
  s = strrchr(s, '.');
  if(s)
    return strcmp(s, ".gz") == 0;
  else
    return 0;
}

struct Stream {
  FILE* stream;
  bool wasPopened;
  Stream(char* name) {
    char popen_buf[4096];
    if(endsWithgz(name) && strlen(name) < 3000)
    {
      strcpy(popen_buf, "gzip -d -c < ");
      strcat(popen_buf, name);
      stream = popen(popen_buf, "r");
      wasPopened = true;
    }
    else
    {
      stream = fopen(name, "r");
      wasPopened = false;
    }
  }

  bool fail()
  { return stream == 0; }

  ~Stream() {
      if(wasPopened)
        pclose(stream);
      else
        fclose(stream);
    }
};



Obj READ_PROFILE_FROM_STREAM(Obj self, Obj filename, Obj param2)
{
try{
    bool isCover = false;
    std::string timeType;
    int failedparse = 0;
    bool firstExec = true;

    std::map<Int, std::string> filename_map;
    std::map<std::string, Int> filename_map_inverse;

    std::map<Int, std::set<Int> > read_lines;
    std::map<Int, std::map<Int, Int> > exec_lines;
    std::map<Int, std::map<Int, Int> > runtime_lines;
    std::map<Int, std::map<Int, Int> > runtime_with_children_lines;

    std::map<Int, std::map<Int, std::set<FullFunction> > > called_functions;
    std::map<Int, std::map<Int, std::set<Location> > > calling_functions;
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

    if(!(IS_STRING(filename))) {
      ErrorMayQuit("Filename must be a string", 0, 0);
    }
    Obj filenamestr = CopyToStringRep(filename);
    Stream infile(CSTR_STRING(filenamestr));
    if(infile.fail()) {
      ErrorMayQuit("Unable to open file %s", (Int)CSTR_STRING(filenamestr), 0);
      return Fail;
    }

    long line_number = 0;

    while(true)
    {
      if(feof(infile.stream)) {
        break;
      }
      line_number++;
      char str[10000];
      if(fgets(str, 10000, infile.stream) == 0)
      {
        if(feof(infile.stream)) {
          break;
        }
        else {
          return Fail;
        }
      }
      JsonParse ret;
      if(ReadJson(str, ret))
      {
        switch(ret.Type)
        {
          case InvalidType: ErrorReturnVoid("Internal Error",0,0,""); break;
          case StringId:
            if(filename_map.count(ret.FileId) > 0) {
              ErrorMayQuit("Invalid input - Reused fileId. Did you try concatenating files?"
                           "Use ConcatenateLineByLineProfiles",0,0); break;
            }
            filename_map[ret.FileId] = ret.File;
            filename_map_inverse[ret.File] = ret.FileId;
          break;
          case IntoFun:
          {
            FullFunction retfunc = buildFunctionName(ret);
            // Record which line called this function
            called_functions[calling_exec.FileId][calling_exec.Line].insert(retfunc);
            // Record we called this function from here
            if(!function_stack.empty()) {
              // This '!= 0' is to support older GAP's which don't provide this field
              if(ret.FileId != 0) {
                std::string filename = function_stack.back().filename;
                calling_functions[ret.FileId][ret.Line].insert(Location(filename, filename_map[calling_exec.FileId], calling_exec.Line));
              }
            }
            // Add this function to the stack of executing functions
            function_stack.push_back(retfunc);

            static int deepest_depth = 0;
            if(function_stack.size() > deepest_depth) {
              deepest_depth = function_stack.size();
            }
            // And to stack of executed files/line numbers
            line_stack.push_back(calling_exec);
            // We also store the amount of time spent in this stack as well.
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
                function_stack.pop_back();
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
            exec_lines[ret.FileId][ret.Line]+=ret.Execs;
            if(firstExec)
              firstExec = false;
            else
            {
              // The ticks are since the last executed line
              if(ret.Ticks > 0) {
                runtime_lines[prev_exec.FileId][prev_exec.Line]+=ret.Ticks;
                // Hard to know exactly where to charge these to --
                // this is easiest
                (current_stack->runtime) += ret.Ticks;
                total_ticks += ret.Ticks;
              }
            }
          }
          break;
          case Info:
            isCover = ret.IsCover;
            timeType = ret.TimeType;
          break;
        }
      }
      else
      {
        // We allow a few failed parses to deal with truncated files
        failedparse++;
        Pr("Warning: damaged profile at %g:%d",  (Int)filenamestr, (Int)line_number);
        if(failedparse > 4) {
          throw GAPException("Malformed profile");
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
    std::vector<std::pair<std::string, std::vector<std::set<Location> > > > calling_functions_ret;

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
      std::map<Int, std::set<Location> >& functions_calling = calling_functions[*it];

      Int max_line = 0;
      if(!read_set.empty())
        max_line = std::max(max_line, *(read_set.rbegin()));

      if(!exec_set.empty())
        max_line = std::max(max_line, exec_set.rbegin()->first);

      if(!runtime.empty())
        max_line = std::max(max_line, runtime.rbegin()->first);

      if(!runtime_children.empty())
        max_line = std::max(max_line, runtime_children.rbegin()->first);

      if(!functions.empty())
        max_line = std::max(max_line, functions.rbegin()->first);

      if(!functions_calling.empty())
        max_line = std::max(max_line, functions_calling.rbegin()->first);

      std::vector<std::vector<Int> > line_data;
      std::vector<std::set<FullFunction> > called_data;
      std::vector<std::set<Location> > calling_data;
      for(int i = 1; i <= max_line; ++i)
      {
        std::vector<Int> data;
        data.push_back(read_set.count(i));
        data.push_back(exec_set[i]);
        data.push_back(runtime[i]);
        data.push_back(runtime_children[i]);
        line_data.push_back(data);

        called_data.push_back(functions[i]);
        calling_data.push_back(functions_calling[i]);
      }

      if(filename_map.count(*it) == 0)
      {
        Pr("Warning: damaged profile, cannot find a filename to match id %d", *it, 0L);
      }
      else
      {
        read_exec_data.push_back(std::make_pair(filename_map[*it], line_data));
        called_functions_ret.push_back(std::make_pair(filename_map[*it], called_data));
        calling_functions_ret.push_back(std::make_pair(filename_map[*it], calling_data));
      }
    }

    std::vector<std::pair<std::vector<FullFunction>, Int> > function_stack_runtimes = dumpRuntimes(&stacktrace);

    GAPRecord info;
    info.set("is_cover", isCover);
    info.set("time_type", timeType);

    GAPRecord r;

    r.set("line_info", read_exec_data);
    r.set("stack_runtimes", function_stack_runtimes);
    r.set("line_function_calls", called_functions_ret);
    r.set("line_calling_function_calls", calling_functions_ret);
    r.set("info", info);

    return GAP_make(r);
} catch (const GAPException& exp) {
  ErrorMayQuit(exp.what(), 0, 0);
}
return Fail;
}

Obj HTMLEncodeString(Obj self, Obj param)
{
  if(!IS_STRING_REP(param))
  {
    ErrorMayQuit("<arg> must satisfy IsStringRep",0L,0L);
  }

  Int len = GET_LEN_STRING(param);
  // Make long enough there is no chance a
  // resize will be needed. - 8 * 6 to allow for tabs
  Obj outstring = NEW_STRING(len * 8 * 6);
  char* ptr = CSTR_STRING(param);
  char* outptr = CSTR_STRING(outstring);
  Int outpos = 0;
  for(Int i = 0; i < len; ++i)
  {
    switch(ptr[i]) {
      case '&':
        outptr[outpos++] = '&';
        outptr[outpos++] = 'a';
        outptr[outpos++] = 'm';
        outptr[outpos++] = 'p';
        outptr[outpos++] = ';';
        break;
      case '<':
        outptr[outpos++] = '&';
        outptr[outpos++] = 'l';
        outptr[outpos++] = 't';
        outptr[outpos++] = ';';
        break;
      case ' ':
        outptr[outpos++] = '&';
        outptr[outpos++] = 'n';
        outptr[outpos++] = 'b';
        outptr[outpos++] = 's';
        outptr[outpos++] = 'p';
        outptr[outpos++] = ';';
        break;
      case '\t':
        for(int i = 0; i < 8; ++i) {
          outptr[outpos++] = '&';
          outptr[outpos++] = 'n';
          outptr[outpos++] = 'b';
          outptr[outpos++] = 's';
          outptr[outpos++] = 'p';
          outptr[outpos++] = ';';
        }
        break;
      default:
        outptr[outpos++] = ptr[i];
    }
  }
  SET_LEN_STRING(outstring, outpos);
  SHRINK_STRING(outstring);
  return outstring;
}

Obj MD5File(Obj self, Obj filename)
{
  if (!IsStringConv(filename))
  {
    ErrorQuit("MD5File: <filename> must be a string", 0, 0);
  }

  MD5Context ctx;

  MD5Init(&ctx);

  int fd = open(CSTR_STRING(filename), O_RDONLY);
  if (fd < 0)
  {
    ErrorQuit("MD5File: failed to open file %g", (Int)filename, 0);
  }

  // get the file length
  struct stat sb;
  if (fstat(fd, &sb) == -1 || sb.st_size < 0)
  {
    close(fd);
    ErrorQuit("MD5File: failed to determine size of file %g", (Int)filename, 0);
  }
  off_t len = sb.st_size;

  while (len > 0)
  {
    uint8_t buffer[4096];
    size_t amount = (len < 4096 ? len : 4096);
    ssize_t bytesRead = read(fd, buffer, amount);
    if (bytesRead < 0) {
      close(fd);
      ErrorQuit("MD5File: error reading from file %g", (Int)filename, 0);
    }
    MD5Update(&ctx, buffer, (size_t)bytesRead);
    len -= bytesRead;
  }

  close(fd);

  uint8_t digest[16];
  MD5Final(digest, &ctx);

  char str[33];
  static const char hex[] = "0123456789abcdef";
  for (int i = 0; i < 16; i++)
  {
    str[2*i] = hex[digest[i] >> 4];
    str[2*i + 1] = hex[digest[i] & 0x0f];
  }
  str[32] = '\0';

  return MakeImmString(str);
}

#define GVAR_FUNC_TABLE_ENTRY(srcfile, name, nparam, params) \
  {#name, nparam, \
   params, \
   (ObjFunc)name, \
   srcfile ":Func" #name }

// Table of functions to export
static StructGVarFunc GVarFuncs [] = {
    GVAR_FUNC_TABLE_ENTRY("profiling.c",READ_PROFILE_FROM_STREAM, 2, "param, param2"),
    GVAR_FUNC_TABLE_ENTRY("profiling.c",HTMLEncodeString, 1, "param"),
    GVAR_FUNC_TABLE_ENTRY("profiling.c",MD5File, 1, "filename"),

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
