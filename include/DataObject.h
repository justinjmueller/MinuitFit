#ifndef DATAOBJECT_H
#define DATAOBJECT_H

//C++ includes.
#include <vector> //STL vector.
#include <string> //Basic string.

class DataObject
{
 public:
  DataObject(std::string FileName, double DefaultYieldUncertainty, double DefaultEnergyUncertainty, double DefaultFieldUncertainty, double LowField);
  std::vector<double> GetDataX();
  std::vector<double> GetDataXErrLow();
  std::vector<double> GetDataXErrHigh();
  std::vector<double> GetDataY();
  std::vector<double> GetDataYErrLow();
  std::vector<double> GetDataYErrHigh();
  std::vector<double> GetDataZ();
  std::vector<double> GetDataZErrLow();
  std::vector<double> GetDataZErrHigh();
  friend std::ostream &operator<< (std::ostream &out, const DataObject &Obj);
 private:
  DataObject();
  std::vector<double> DataX;
  std::vector<double> DataXErrLow;
  std::vector<double> DataXErrHigh;
  std::vector<double> DataY;
  std::vector<double> DataYErrLow;
  std::vector<double> DataYErrHigh;
  std::vector<double> DataZ;
  std::vector<double> DataZErrLow;
  std::vector<double> DataZErrHigh;
};


#endif
