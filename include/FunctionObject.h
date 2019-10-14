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
  std::vector<double> GetParameters();
  std::vector<double> GetLimitsLow();
  std::vector<double> GetLimitsHigh();
  std::vector<double> GetStepSizes();
  std::vector<std::string> GetSets();
  std::vector<std::string> GetRecipes();
 private:
  std::string Function;
  std::vector<double> Parameters;
  std::vector<double> LimitsLow;
  std::vector<double> LimitsHigh;
  std::vector<double> StepSizes;
  std::vector<std::string> Sets;
  std::vector<std::string> Recipes;
  std::map<std::string,std::string> DefinitionsMap;
};
#endif
