/*-----------------------------------------------------------------------------

  Ini.cpp

  2009 Shamus Young


-------------------------------------------------------------------------------
  
  This takes various types of data and dumps them into a predefined ini file.

-----------------------------------------------------------------------------*/

#include "Win.h"

#define FORMAT_VECTOR       "%f %f %f"
#define MAX_RESULT          256
#define FORMAT_FLOAT        "%1.2f"

#ifdef WINDOWS
#define INI_FILE            ".\\" APP ".ini"
#else
#define INI_FILE            "./" APP ".ini"
#endif

#define SECTION             "Settings"

#ifdef WINDOWS
#include <windows.h>
#else
#include <string>
#include <fstream>
#include <map>
#include <sstream>
#endif

#include <stdio.h>
#include "glTypes.h"

#include "Ini.h"

static char                 result[MAX_RESULT];

#ifndef WINDOWS
// Windows provides nice {Get,Set}PrivateProfile[type] functions.  Emulate them.

// We'll be doing lots of gets.  Build this map the first time it's needed.
typedef std::map<std::string, std::string> ent_map;
static ent_map entries;

static void BuildEntriesMap()
{
  std::ifstream conf(INI_FILE);
  std::string line, val;
  std::string::size_type eq_pos;

  while(!(conf.eof() || !conf)) {
    std::getline(conf, line);

    // skip blank lines and comments
    if(line.length() == 0 || line[0] == '#')
      continue;

    // skip lines that don't contain = characters
    if((eq_pos = line.find('=')) == std::string::npos)
      continue;

    val = line.substr(eq_pos + 1, line.size());
    line.resize(eq_pos);
    entries[line] = val;
  }

  conf.close();
}

static void FlushEntriesMap()
{
  std::ofstream conf(INI_FILE, std::ios_base::out | std::ios_base::trunc);

  for(ent_map::iterator i = entries.begin(); i != entries.end(); ++i) {
    conf << i->first << '=' << i->second << std::endl;
  }

  conf.close();
}

template<typename T>
static T GetConfFileEntry(const std::string& entry, const T& default_value)
{
  ent_map::iterator pos;
  T value;

  if(entries.size() == 0)
    BuildEntriesMap();

  if((pos = entries.find(entry)) == entries.end())
    return default_value;

  std::istringstream val_stm(pos->second);
  val_stm >> value;
  return value;
}

template<typename T>
static void SetConfFileEntry(const std::string& entry, const T& value)
{
  std::ostringstream val_stm;

  if(entries.size() == 0)
    BuildEntriesMap();

  val_stm << value;
  entries[entry] = val_stm.str();

  FlushEntriesMap();
}

#endif /* !WINDOWS */

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

int IniInt (char* entry)
{

#ifdef WINDOWS
  int         result;

  result = GetPrivateProfileInt (SECTION, entry, 0, INI_FILE);
  return result;
#else
  return GetConfFileEntry<int>(entry, 0);
#endif

}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void IniIntSet (char* entry, int val)
{

#ifdef WINDOWS
  char        buf[20];

  sprintf (buf, "%d", val);
  WritePrivateProfileString (SECTION, entry, buf, INI_FILE);
#else
  SetConfFileEntry<int>(entry, val);
#endif

}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

float IniFloat (char* entry)
{

#ifdef WINDOWS
  float     f;

  GetPrivateProfileString (SECTION, entry, "", result, MAX_RESULT, INI_FILE);
  f = (float)atof (result);
  return f;
#else
  return GetConfFileEntry<float>(entry, 0);
#endif

}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void IniFloatSet (char* entry, float val)
{

#ifdef WINDOWS
  char        buf[20];
  
  sprintf (buf, FORMAT_FLOAT, val);
  WritePrivateProfileString (SECTION, entry, buf, INI_FILE);
#else
  SetConfFileEntry<float>(entry, val);
#endif

}


/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

char* IniString (char* entry)
{

#ifdef WINDOWS
  GetPrivateProfileString (SECTION, entry, "", result, MAX_RESULT, INI_FILE);
  return result;
#else
  /* why not just return the string?  because i have to either muck around
   * with the windows version, or with this one (one or the other).  sigh. */

  std::string res = GetConfFileEntry<std::string>(entry, "");
  std::string::size_type len = res.copy(result, sizeof(result) - 1);
  result[len] = 0;
  return result;
#endif

}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void IniStringSet (char* entry, char* val)
{

#ifdef WINDOWS
  WritePrivateProfileString (SECTION, entry, val, INI_FILE);
#else
  SetConfFileEntry<std::string>(entry, val);
#endif

}


/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void IniVectorSet (char* entry, GLvector v)
{

#ifdef WINDOWS
  sprintf (result, FORMAT_VECTOR, v.x, v.y, v.z);
  WritePrivateProfileString (SECTION, entry, result, INI_FILE);
#else
  SetConfFileEntry<GLvector>(entry, v);
#endif

}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

GLvector IniVector (char* entry)
{

#if WINDOWS
  GLvector  v;

  v.x = v.y = v.z = 0.0f;
  GetPrivateProfileString (SECTION, entry, "0 0 0", result, MAX_RESULT, INI_FILE);
  sscanf (result, FORMAT_VECTOR, &v.x, &v.y, &v.z);
  return v;
#else
  GLvector def = {0.0f, 0.0f, 0.0f};
  return GetConfFileEntry<GLvector>(entry, def);
#endif

}
