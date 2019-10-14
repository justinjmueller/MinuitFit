#ifndef DATAOBJECT_H
#define DATAOBJECT_H

//ROOT includes.
#include "TMatrixT.h"
#include "TF2.h"

//C++ includes.
#include <vector> //STL vector.
#include <string> //Basic string.
#include <memory> //For using shared_ptr.
#include <fstream> //For using file input/output.

//Custom includes.
#include "FunctionObject.h" //Modularizes the input of model functions from a .txt file.

class DataObject
{
 public:
  DataObject(std::vector<std::string> Sets, std::vector<std::string> Recipes, double DefaultYieldUncertainty, double DefaultEnergyUncertainty, double DefaultFieldUncertainty, double LowField);
  TMatrixT<double> GetCovariance();
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
  TMatrixT<double> BuildCovariance(std::vector< std::vector<double> > Data, TMatrixT<double> V_P);
  double Derivative(double* x, double* p, int axis);
  int ReadData(std::ifstream &Input, std::vector<double> &List);
  TMatrixT<double> Covariance;
  std::vector<double> DataX;
  std::vector<double> DataXErrLow;
  std::vector<double> DataXErrHigh;
  std::vector<double> DataY;
  std::vector<double> DataYErrLow;
  std::vector<double> DataYErrHigh;
  std::vector<double> DataZ;
  std::vector<double> DataZErrLow;
  std::vector<double> DataZErrHigh;
  std::shared_ptr<FunctionObject> FuncObject;
  std::shared_ptr<TF2> RecipeModel;
};


#endif
