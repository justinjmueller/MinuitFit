//C++ includes.
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
#include <map>

//ROOT includes.

//Custom includes.
#include "FunctionObject.h"

FunctionObject::FunctionObject(std::string Definitions, std::string SearchString, unsigned int ModelID, bool& Success)
{
  std::ifstream Input(Definitions);
  std::string Line, FunctionString, ParameterString, LimitLowString, LimitHighString, StepSizesString, Tmp, ModelIDString, SetsString, RecipesString;
  std::size_t First, Last;
  bool Found(false);

  if(Input.is_open())
  {
    while(std::getline(Input, Line))
    {
      if(Line[0] != '#')
      {
	if(Line.find(SearchString+std::string("F")+std::to_string(ModelID)) !=std::string::npos)
	{
	  First = Line.find("\"");
	  Last = Line.find("\"",First+1);
	  FunctionString = Line.substr(First+1,Last-First-1);
	  DefinitionsMap.emplace(SearchString+std::string("F")+std::to_string(ModelID), FunctionString);
	}
	else if(Line.find(SearchString+std::string("P")+std::to_string(ModelID)) !=std::string::npos)
	{
	  First = Line.find("\"");
	  Last = Line.find("\"",First+1);
	  ParameterString = Line.substr(First+1,Last-First-1);
	  DefinitionsMap.emplace(SearchString+std::string("P")+std::to_string(ModelID), ParameterString);
	}
	else if(Line.find(SearchString+std::string("LL")+std::to_string(ModelID)) !=std::string::npos)
	{
	  First = Line.find("\"");
	  Last = Line.find("\"",First+1);
	  LimitLowString = Line.substr(First+1,Last-First-1);
	  DefinitionsMap.emplace(SearchString+std::string("LL")+std::to_string(ModelID), LimitLowString);
	}
	else if(Line.find(SearchString+std::string("LH")+std::to_string(ModelID)) !=std::string::npos)
	{
	  First = Line.find("\"");
	  Last = Line.find("\"",First+1);
	  LimitHighString = Line.substr(First+1,Last-First-1);
	  DefinitionsMap.emplace(SearchString+std::string("LH")+std::to_string(ModelID), LimitHighString);
	}
	else if(Line.find(SearchString+std::string("S")+std::to_string(ModelID)) !=std::string::npos)
	{
	  First = Line.find("\"");
	  Last = Line.find("\"",First+1);
	  StepSizesString = Line.substr(First+1,Last-First-1);
	  DefinitionsMap.emplace(SearchString+std::string("S")+std::to_string(ModelID), StepSizesString);
	}
	else if(Line.find(SearchString+std::string("Sets")) !=std::string::npos)
	{
	  First = Line.find("\"");
	  Last = Line.find("\"", First+1);
	  SetsString = Line.substr(First+1,Last-First-1);
	  DefinitionsMap.emplace(SearchString+std::string("Sets"), SetsString);
	}
	else if(Line.find(SearchString+std::string("Recipes")) !=std::string::npos)
	{
	  First = Line.find("\"");
	  Last = Line.find("\"", First+1);
	  RecipesString = Line.substr(First+1,Last-First-1);
	  DefinitionsMap.emplace(SearchString+std::string("Recipes"), RecipesString);
	}
	Found = (FunctionString != "" && ParameterString != "" && LimitLowString != "", LimitHighString != "" && StepSizesString != "");	
      }
    }
  }

  //for(unsigned int i(0); i < Line.size(); ++i) if(std::isdigit(Line[i])) ModelIDString += Line[i];
  //if(ModelIDString != "")
  
  if(Found)
  {
    Function = FunctionString;
    for(unsigned int i(0); i < ParameterString.length(); ++i)
    {
      if(ParameterString[i] != ',') Tmp += ParameterString[i];
      else
      {
	Parameters.push_back(std::stod(Tmp));
	Tmp = "";
      }
    }
    if(Tmp != "") Parameters.push_back(std::stod(Tmp));
    Tmp = "";
    for(unsigned int i(0); i < LimitLowString.length(); ++i)
    {
      if(LimitLowString[i] != ',') Tmp += LimitLowString[i];
      else
      {
	LimitsLow.push_back(std::stod(Tmp));
	Tmp = "";
      }
    }
    if(Tmp != "") LimitsLow.push_back(std::stod(Tmp));
    Tmp = "";
    for(unsigned int i(0); i < LimitHighString.length(); ++i)
    {
      if(LimitHighString[i] != ',') Tmp += LimitHighString[i];
      else
      {
	LimitsHigh.push_back(std::stod(Tmp));
	Tmp = "";
      }
    }
    if(Tmp != "") LimitsHigh.push_back(std::stod(Tmp));
    Tmp = "";
    for(unsigned int i(0); i < StepSizesString.length(); ++i)
    {
      if(StepSizesString[i] != ',') Tmp += StepSizesString[i];
      else
      {
	StepSizes.push_back(std::stod(Tmp));
	Tmp = "";
      }
    }
    if(Tmp != "") StepSizes.push_back(std::stod(Tmp));
    Tmp = "";
    for(unsigned int i(0); i < SetsString.length(); ++i)
    {
      if(SetsString[i] != ',') Tmp += SetsString[i];
      else
      {
	Sets.push_back(Tmp);
	Tmp = "";
      }
    }
    if(Tmp != "") Sets.push_back(Tmp);
    Tmp = "";
    for(unsigned int i(0); i < RecipesString.length(); ++i)
    {
      if(RecipesString[i] != ',') Tmp += RecipesString[i];
      else
      {
	Recipes.push_back(Tmp);
	Tmp = "";
      }
    }
    if(Tmp != "") Recipes.push_back(Tmp);
    Success = true;
  }
  else Success = false;
}
std::string FunctionObject::GetFunction()
{
  return Function;
}

std::vector<double> FunctionObject::GetParameters()
{
  return Parameters;
}

std::vector<double> FunctionObject::GetLimitsLow()
{
  return LimitsLow;
}

std::vector<double> FunctionObject::GetLimitsHigh()
{
  return LimitsHigh;
}

std::vector<double> FunctionObject::GetStepSizes()
{
  return StepSizes;
}

std::vector<std::string> FunctionObject::GetSets()
{
  return Sets;
}

std::vector<std::string> FunctionObject::GetRecipes()
{
  return Recipes;
}
