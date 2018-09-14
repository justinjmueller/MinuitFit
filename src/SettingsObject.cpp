//C++ includes.
#include <iostream> //For basic input and output.
#include <fstream> //For basic file input/output.
#include <map> //STL map.
#include <string> //Basic string.
#include <stdexcept> //Exception handling.

//Custom includes.
#include "SettingsObject.h" //Header file for this implementation file.

SettingsObject::SettingsObject(std::string SettingsFile)
{
  std::ifstream Input(SettingsFile);
  std::string Line, TmpKey, TmpValue;
  unsigned int First(0), Last(0);
    
  if(Input.is_open()) //Verify that the file is open.
  {
    while(std::getline(Input,Line)) //Loop over each line in the file.
    {
      if(Line.length() != 0 && Line[0] != '#') //Check to see that it's not a comment line or blank.
      {
	Last = Line.find(":"); //Look for ':' in the line.
	if(Last != std::string::npos) //Verify that the line actually has a key/value pair.
	{
	  TmpKey = Line.substr(0,Last); //Extract the key.
	  First = Line.find("\""); //Find the first '"'.
	  Last = Line.find("\"", First+1); //Find the second '"'.
	  TmpValue = Line.substr(First+1,Last-First-1); //Extract the value from between the '"' pair.
	  SettingsMap.insert(std::make_pair(TmpKey,TmpValue)); //Place the pair in the settings map.
	}
      }
    }
    Input.close();
  }
  else std::cerr << "Settings file not found." << std::endl;
}

std::string SettingsObject::Query(std::string Key)
{
  std::string QueryResult;
  if(SettingsMap.count(Key) != 0) QueryResult = SettingsMap.at(Key);
  else
  {
    std::cerr << "Invalid Query: " << Key << std::endl;
    throw std::exception();
  }
  return QueryResult; //Return the value associated with Key.
}

std::ostream& operator<<(std::ostream& os, const SettingsObject& Obj)
{
  std::map<std::string, std::string>::const_iterator MapIt = Obj.SettingsMap.begin(); //Map iterator.
  while(MapIt != Obj.SettingsMap.end())
  {
    os << MapIt->first << ":" << MapIt->second << std::endl; //Write key/value pair to output.
    ++MapIt; //Increment the iterator to move to the next pair.
  }
  return os; //Return the output stream object.
}
