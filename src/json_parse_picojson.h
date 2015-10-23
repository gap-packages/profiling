#define PICOJSON_USE_INT64
#include "picojson/picojson.h"


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

  if(ret.Type == Info)
  {
      if(!v.contains("Version") || !v.get("Version").is<int64_t>()) {
        ErrorMayQuit("Do not understand version of file format "
                     "(no Version in the Info line)", 0, 0);
      }
      Int version = v.get("Version").get<int64_t>();
      if(version > 1) {
        ErrorMayQuit("This version of the 'profiling' package is too old "
                     "to read this file (only accepts version 1, this file"
                     " is version %d)", version, 0);
      } 
                     
      if(!v.contains("IsCover") || !v.get("IsCover").is<bool>()) {
        return false;
      }
      ret.IsCover = v.get("IsCover").get<bool>();

      if(!v.contains("TimeType") || !v.get("TimeType").is<std::string>()) {
        return false;
      }

      ret.TimeType = v.get("TimeType").get<std::string>();
      return true;
  }

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
