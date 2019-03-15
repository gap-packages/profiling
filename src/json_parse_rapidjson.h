//  Please refer to the COPYRIGHT file of the profiling package for details.
//  SPDX-License-Identifier: MIT

#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"
#include <iostream>
#include <string>
#include <map>

using namespace rapidjson;

//#define DEBUG_OUT(x) std::cerr << x << "\n";
#define DEBUG_OUT(x)

enum ArgType {
  arg_Fun,
  arg_File,
  arg_Type,
  arg_Line,
  arg_Ticks,
  arg_Execs,
  arg_FileId,
  arg_Version,
  arg_IsCover,
  arg_EndLine,
  arg_TimeType,
  arg_INVALID
};

ArgType getArgType(const char *str, SizeType length) {
  if (length == 3) {
    if (strncmp(str, "Fun", 3) == 0)
      return arg_Fun;
  }

  if (length == 4) {
    if (strncmp(str, "File", 4) == 0)
      return arg_File;
    if (strncmp(str, "Type", 4) == 0)
      return arg_Type;
    if (strncmp(str, "Line", 4) == 0)
      return arg_Line;
  }

  if (length == 5) {
    if (strncmp(str, "Ticks", 5) == 0)
      return arg_Ticks;
    if (strncmp(str, "Execs", 5) == 0)
      return arg_Execs;
  }

  if (length == 6) {
    if (strncmp(str, "FileId", 6) == 0)
      return arg_FileId;
  }

  if (length == 7) {
    if (strncmp(str, "Version", 7) == 0)
      return arg_Version;
    if (strncmp(str, "IsCover", 7) == 0)
      return arg_IsCover;
    if (strncmp(str, "EndLine", 7) == 0)
      return arg_EndLine;
  }

  if (length == 8) {
    if (strncmp(str, "TimeType", 8) == 0)
      return arg_TimeType;
  }

  return arg_INVALID;
}

struct MessageHandler {
  MessageHandler() : name_(arg_INVALID) {}

  bool StartObject() {
		DEBUG_OUT("SO:");
		return name_ == arg_INVALID;
  }

  bool String(const char *str, SizeType length, bool) {
		DEBUG_OUT("ST:" << std::string(str,length) << ":");
    switch (name_) {
#define FILL_STRING(x) case arg_##x: jp->x = std::string(str, length); break;
    case arg_Type:
      jp->Type = CharToProf(str[0]);
      break;
      FILL_STRING(TimeType);
      FILL_STRING(File);
      FILL_STRING(Fun);
    default:
      DEBUG_OUT( "!1:" << name_ << ":" << std::string(str, length));
      return false;
    }
		name_ = arg_INVALID;
		return true;
  }

  bool Int(int i) { return Int64(i); }
  bool Uint(unsigned u) { return Int64(u); }
  bool Uint64(uint64_t u) { return Int64(u); }

  bool Int64(int64_t i) {
		DEBUG_OUT("int:"<<i);
    switch (name_) {
#define FILL_INT(x) case arg_##x: jp->x = i; break;
    case arg_Version:
      if (i > 2) {
        ErrorMayQuit("This version of the 'profiling' package is too old "
                     "to read this file (only accepts version 1 or 2, this file"
                     " is version %d)",
                     (int)i, 0);
      }
      break;
      FILL_INT(FileId);
      FILL_INT(Line);
      FILL_INT(EndLine);
      FILL_INT(Ticks);
      FILL_INT(Execs);
    default:
      DEBUG_OUT("!I:" << name_ << ":" << i);
      return false;
    }
    name_ = arg_INVALID;
    return true;
  }

  bool Bool(bool b) {
		DEBUG_OUT("B"<<b);
    if (name_ == arg_IsCover) {
      jp->IsCover = b;
      name_ = arg_INVALID;
      return true;
    } else {
      DEBUG_OUT("!3" << name_ << ":" << b);
      return false;
    }
  }

  bool EndObject(SizeType) { return name_ = arg_INVALID; }

  bool Null() {
    DEBUG_OUT("NULL");
    return false;
  }
  bool Double(double d) {
		DEBUG_OUT("DOUBLE");
    return false;
  }
	
  bool Key(const char *str, SizeType length, bool copy) {
		DEBUG_OUT("K:"<<std::string(str,length));
		if(name_ != arg_INVALID)
			return false;
    name_ = getArgType(str, length);
    return true;
  }

  bool StartArray() {
		DEBUG_OUT("SA");
    return false;
  }
  bool EndArray(SizeType elementCount) {
    	DEBUG_OUT("EA");
    return false;
  }

  JsonParse *jp;
  ArgType name_;
};

bool ReadJson(char *json, JsonParse &ret) {
  Reader reader;
  MessageHandler handler;
  handler.jp = &ret;
  InsituStringStream ss(json);
	DEBUG_OUT("START_PARSE");
  try
  {
    if (reader.Parse<kParseInsituFlag>(ss, handler))
      return true;
  }
  catch(...) // catch any bad parsing and just throw it away
  { }
  return false;
}
