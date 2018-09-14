#ifndef SETTINGSOBJECT_H
#define SETTINGSOBJECT_H
//C++ includes.
#include <iostream> //Basic input and output.
#include <string> //Basic string.
#include <map> //STL map.

class SettingsObject
{
 public:
  SettingsObject(std::string SettingsFile = "");
  std::string Query(std::string Key);
  friend std::ostream& operator<<(std::ostream& os, const SettingsObject& Obj);
 private:
  std::map<std::string, std::string> SettingsMap;
};
#endif
