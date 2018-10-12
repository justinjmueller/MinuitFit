#ifndef FUNCTIONOBJECT_H
#define FUNCTIONOBJECT_H
//C++ includes.
#include <string>
#include <vector>
#include <map>

//ROOT includes.

//Custom includes.

class FunctionObject
{
 public:
  FunctionObject(std::string Definitions, std::string SearchString, unsigned int ModelID, bool& Success);
  std::string GetFunction();
  std::string GetDerivative();
  std::vector<double> GetParameters();
  std::vector<double> GetLimitsLow();
  std::vector<double> GetLimitsHigh();
  std::vector<double> GetStepSizes();
 private:
  std::string Function;
  std::string Derivative;
  std::vector<double> Parameters;
  std::vector<double> LimitsLow;
  std::vector<double> LimitsHigh;
  std::vector<double> StepSizes;
  std::map<std::string,std::string> DefinitionsMap;
};
#endif
