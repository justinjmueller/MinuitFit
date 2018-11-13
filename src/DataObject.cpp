//C++ includes.
#include <vector> //STL vector.
#include <fstream> //Basic file input and output.
#include <string> //Basic string.
#include <iostream> //Basic input and output.

//Custom includes.
#include "DataObject.h" //Header file for this implementation.

DataObject::DataObject(std::string FileName)
{
  std::ifstream Input(FileName);
  std::size_t prev, pos;
  std::string line;
  std::vector<double> DataList;
  if(Input.is_open()) //Verify that the file is open.
  {
    while(std::getline(Input, line)) //Loop over each line in the data file.
    {
      prev = 0;
      if(line[0] != '#') //Ignore lines starting with a '#' (comment).
      {
	while( (pos = line.find_first_of(",", prev)) != std::string::npos) //This loop grabs strings delimited by a ','.
	  {
	    if(pos > prev) DataList.push_back( stof(line.substr(prev, pos-prev)) );
	    prev = pos + 1;
	  }
	if(prev < line.length()) DataList.push_back( stof(line.substr(prev, std::string::npos)) );
      }
      else prev += line.size();
    }
    for(unsigned int i(0); i < DataList.size(); i+=7) //Parse and store the data.
    {
      if(i + 6 < DataList.size())
      {
	DataX.push_back(DataList.at(i+0));
	DataXErrLow.push_back(DataList.at(i+1));
	DataXErrHigh.push_back(DataList.at(i+2));
	DataY.push_back(DataList.at(i+3));
	DataZ.push_back(DataList.at(i+4));
	DataZErrLow.push_back(DataList.at(i+5));
	DataZErrHigh.push_back(DataList.at(i+6));
      }
      else std::cerr << "Data file configured incorrectly. Check for a missing entry in a column." << std::endl;
    }
  }
  Input.close(); //Close the input file.
}

std::vector<double> DataObject::GetDataX()
{
  return DataX;
}
std::vector<double> DataObject::GetDataXErrLow()
{
  return DataXErrLow;
}
std::vector<double> DataObject::GetDataXErrHigh()
{
  return DataXErrHigh;
}
std::vector<double> DataObject::GetDataY()
{
  return DataY;
}
std::vector<double> DataObject::GetDataZ()
{
  return DataZ;
}
std::vector<double> DataObject::GetDataZErrLow()
{
  return DataZErrLow;
}
std::vector<double> DataObject::GetDataZErrHigh()
{
  return DataZErrHigh;
}

std::ostream &operator<< (std::ostream &out, const DataObject &Obj)
{
  for(unsigned int i(0); i < Obj.DataX.size(); ++i)
  {
    out << Obj.DataX.at(i) << ','
	<< Obj.DataXErrLow.at(i) << ','
	<< Obj.DataXErrHigh.at(i) << ','
	<< Obj.DataY.at(i) << ','
	<< Obj.DataZ.at(i) << ','
	<< Obj.DataZErrLow.at(i) << ','
	<< Obj.DataZErrHigh.at(i) << std::endl;
  }
  return out;
}
