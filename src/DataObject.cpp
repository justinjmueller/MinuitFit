//C++ includes.
#include <vector> //STL vector.
#include <fstream> //Basic file input and output.
#include <string> //Basic string.
#include <iostream> //Basic input and output.
#include <cmath> //Basic math functions.
#include <memory> //For using shared_ptr.

//Custom includes.
#include "DataObject.h" //Header file for this implementation.
#include "FunctionObject.h" //Modularizes the input of model functions from a .txt file.

DataObject::DataObject(std::vector<std::string> Sets, std::vector<std::string> Recipes, double DefaultYieldUncertainty, double DefaultEnergyUncertainty, double DefaultFieldUncertainty, double LowField)
{
  std::ifstream Input;
  std::string FileName;
  std::vector<double> DataList;

  std::string substr;
  int base_set, npar;
  bool success(false), recipe(false);
  TMatrixT<double> ModelCovariance(1,1);
  std::vector<double> ModelPieces, TMPDataX, TMPDataXErrLow, TMPDataXErrHigh, TMPDataY, TMPDataYErrLow, TMPDataYErrHigh, TMPDataZ, TMPDataZErrLow, TMPDataZErrHigh;
  std::vector< std::vector<double> > DataVector;
  std::vector<TMatrixT<double> > SetCovariances;
  
  for(unsigned int set(0); set < Sets.size(); ++set)
  {
    FileName = Sets.at(set) + ".csv";
    Input.open(FileName);  
    if(Input.is_open()) //Verify that the file is open.
    {
      ReadData(Input, DataList);
      Input.close(); //Close the input file.

      //Process DataList using Recipes list.
      if(Recipes.at(set) == ".")
      {
	//No processing required.
	recipe = false;
      }
      else if(Recipes.at(set).find(".") == std::string::npos)
      {
	//Process recipe.
	substr = Recipes.at(set);
	FuncObject.reset(new FunctionObject("ModelDefinitions.txt", substr.substr(0,substr.length()-1), stoi(substr.substr(substr.length()-1, 1)), success));
	if(success)
	{
	  RecipeModel.reset(new TF2("RecipeModel", (FuncObject->GetFunction()+ "+0*y").c_str(), 0, 1000, 0, 5000));
	  recipe=true;
	  Input.open(substr + "Log.txt");
	  npar = ReadData(Input, ModelPieces);
	  Input.close();
	  ModelCovariance.ResizeTo(npar,npar);
	  for(unsigned int par(0); par < npar; ++par) RecipeModel->SetParameter(par, ModelPieces.at(par));
	  for(unsigned int i(0); i < npar; ++i) for(unsigned int j(0); j < npar; ++j) ModelCovariance(i,j) = ModelPieces.at(npar*(1+i)+j);
	}
	else std::cerr << "Improper model configuration when processing dataset recipes." << std::endl;
      }

      for(unsigned int i(0); i < DataList.size(); i+=9) //Parse and store the data.
      {
	if(i + 8 < DataList.size())
	{
	  TMPDataX.push_back(DataList.at(i+0));
	  TMPDataXErrLow.push_back(DataList.at(i+1) == 0 ? DataList.at(i+0)*DefaultEnergyUncertainty : DataList.at(i+1));
	  TMPDataXErrHigh.push_back(DataList.at(i+2) == 0 ? DataList.at(i+0)*DefaultEnergyUncertainty : DataList.at(i+2));
	  if(DataList.at(i+3) == 0) DataList.at(i+3) = LowField; //Null field doesn't work with every model, so use the default "low" field value instead for null field points.
	  TMPDataY.push_back(DataList.at(i+3));
	  TMPDataYErrLow.push_back(DataList.at(i+4) == 0 ? DataList.at(i+3)*DefaultFieldUncertainty : DataList.at(i+4));
	  TMPDataYErrHigh.push_back(DataList.at(i+5) == 0 ? DataList.at(i+3)*DefaultFieldUncertainty : DataList.at(i+5));
	  if(recipe) TMPDataZ.push_back(RecipeModel->Eval(TMPDataX.back(), TMPDataY.back()) - DataList.at(i+6));
	  else TMPDataZ.push_back(DataList.at(i+6));
	  TMPDataZErrLow.push_back(DataList.at(i+7) == 0 ? DataList.at(i+6)*DefaultYieldUncertainty : DataList.at(i+7));
	  TMPDataZErrHigh.push_back(DataList.at(i+8) == 0 ? DataList.at(i+6)*DefaultYieldUncertainty : DataList.at(i+8));
	}
	else std::cerr << "Data file configured incorrectly. Check for a missing entry in a column." << std::endl;
      }
      DataVector.push_back(TMPDataX);
      DataVector.push_back(TMPDataXErrLow);
      DataVector.push_back(TMPDataXErrHigh);
      DataVector.push_back(TMPDataY);
      DataVector.push_back(TMPDataYErrLow);
      DataVector.push_back(TMPDataYErrHigh);
      DataVector.push_back(TMPDataZ);
      DataVector.push_back(TMPDataZErrLow);
      DataVector.push_back(TMPDataZErrHigh);
      if(recipe)
      {
	SetCovariances.push_back(BuildCovariance(DataVector, ModelCovariance));
	for(unsigned int l(0); l < TMPDataX.size(); ++l)
	{
	  DataVector.at(7).at(l) = sqrt(SetCovariances.back()(l,l));
	  DataVector.at(8).at(l) = DataVector.at(7).at(l);
	}
      }
      else
      {
	ModelCovariance.Clear();
	ModelCovariance.ResizeTo(TMPDataX.size(), TMPDataX.size());
	for(unsigned int l(0); l < TMPDataX.size(); ++l) ModelCovariance(l,l) = pow((TMPDataZErrLow.at(l) + TMPDataZErrHigh.at(l))/2.0, 2.0);
	SetCovariances.push_back(ModelCovariance);
      }
    }
    for(unsigned int k(0); k < TMPDataX.size(); ++k)
    {
      DataX.push_back(DataVector.at(0).at(k));
      DataXErrLow.push_back(DataVector.at(1).at(k));
      DataXErrHigh.push_back(DataVector.at(2).at(k));
      DataY.push_back(DataVector.at(3).at(k));
      DataYErrLow.push_back(DataVector.at(4).at(k));
      DataYErrHigh.push_back(DataVector.at(5).at(k));
      DataZ.push_back(DataVector.at(6).at(k));
      DataZErrLow.push_back(DataVector.at(7).at(k));
      DataZErrHigh.push_back(DataVector.at(8).at(k));
    }
    Covariance.ResizeTo(DataX.size(),DataX.size());
    Covariance.SetSub(DataX.size() - TMPDataX.size(), DataX.size() - TMPDataX.size(), SetCovariances.back());
    TMPDataX.clear();
    TMPDataXErrLow.clear();
    TMPDataXErrHigh.clear();
    TMPDataY.clear();
    TMPDataYErrLow.clear();
    TMPDataYErrHigh.clear();
    TMPDataZ.clear();
    TMPDataZErrLow.clear();
    TMPDataZErrHigh.clear();
    DataList.clear();
    DataVector.clear();
  }
}

TMatrixT<double> DataObject::GetCovariance()
{
  return Covariance;
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
std::vector<double> DataObject::GetDataYErrLow()
{
  return DataYErrLow;
}
std::vector<double> DataObject::GetDataYErrHigh()
{
  return DataYErrHigh;
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
	<< Obj.DataYErrLow.at(i) << ','
	<< Obj.DataYErrHigh.at(i) << ','
	<< Obj.DataZ.at(i) << ','
	<< Obj.DataZErrLow.at(i) << ','
	<< Obj.DataZErrHigh.at(i) << std::endl;
  }
  return out;
}

TMatrixT<double> DataObject::BuildCovariance(std::vector< std::vector<double> > Data, TMatrixT<double> V_P)
{
  int N(Data.at(0).size()), P(RecipeModel->GetNpar());
  TMatrixT<double> V_N(N,N);
  TMatrixT<double> V_E(N,N);
  TMatrixT<double> V_F(N,N);
  TMatrixT<double> V_x(3*N+P, 3*N+P);
  TMatrixT<double> G_N(N,N);
  TMatrixT<double> G_E(N,N);
  TMatrixT<double> G_F(N,N);
  TMatrixT<double> G_P(P,P);
  TMatrixT<double> G(N,3*N+P);
  TMatrixT<double> V_f(N,N);
  double x[2];
  double *p;

  for(unsigned int i(0); i < N; ++i)
  {
    V_N(i,i) = pow((Data.at(7).at(i) + Data.at(8).at(i))/2.0, 2.0);
    V_E(i,i) = pow((Data.at(1).at(i) + Data.at(2).at(i))/2.0, 2.0);
    V_F(i,i) = pow((Data.at(4).at(i) + Data.at(5).at(i))/2.0, 2.0);
    G_N(i,i) = -1.0;    
  }
  //std::cerr << "V_N" << std::endl;
  //V_N.Print();
  //std::cerr << "V_E" << std::endl;
  //V_E.Print();
  //std::cerr << "V_F" << std::endl;
  //V_F.Print();
  //std::cerr << "V_P" << std::endl;
  //V_P.Print();
  //std::cerr << "G_N" << std::endl;
  //G_N.Print();
  V_x.SetSub(0,0, V_N);
  V_x.SetSub(N,N, V_E);
  V_x.SetSub(2*N,2*N, V_F);
  V_x.SetSub(3*N,3*N, V_P);
  //std::cerr << "V_x" << std::endl;
  //V_x.Print();
  for(unsigned int i(0); i < N; ++i)
  {
    for(unsigned int j(0); j < N; ++j)
    {
      //dt_i/dE_j
      p = RecipeModel->GetParameters();
      x[0] = Data.at(0).at(i);
      x[1] = Data.at(3).at(i);
      G_E(i,j) = i == j ? Derivative(x, p, 0) : 0;
      G_F(i,j) = i == j ? Derivative(x, p, 1) : 0;
    }
    if(i < P) for(unsigned int j(0); j < P; ++j) G_P(i,j) = Derivative(x, p, j+2);
  }
  //std::cerr << "G_E" << std::endl;
  //G_E.Print();
  //std::cerr << "G_F" << std::endl;
  //G_F.Print();
  //std::cerr << "G_P" << std::endl;
  //G_P.Print();
  G.SetSub(0,0, G_N);
  G.SetSub(0,N, G_E);
  G.SetSub(0,2*N, G_F);
  G.SetSub(0,3*N, G_P);
  //std::cerr << "G" << std::endl;
  //G.Print();

  TMatrixT<double> VG(3*N+P, N);
  VG.MultT(V_x, G);
  //std::cerr << "VGT" << std::endl;
  //VG.Print();
  V_f.Mult(G, VG);
  //std::cerr << "V_f" << std::endl;
  //V_f.Print();
  return V_f;
}

double DataObject::Derivative(double* x, double* p, int axis)
{
  bool Is2D(true);
  double FPoints1[5];
  double X0[2];
  double xTemp[2];
  double P0[RecipeModel->GetNpar()];
  double pTemp[RecipeModel->GetNpar()];
  double h(0.001);
  double CalculatedValue(0);

  if(axis < 2)
  {
    Is2D = FuncObject->GetFunction().find("y") != std::string::npos;
    if(Is2D || axis == 0)
    {
      h *= x[axis];
      for(unsigned int k(0); k < 2; ++k)
      {
	X0[k] = x[k];
	xTemp[k] = x[k];
      }
      for(int i(-2); i < 3; ++i)
      {
	xTemp[axis] = X0[axis]+i*h;
	FPoints1[i+2] = RecipeModel->EvalPar(xTemp, p);
      }
      CalculatedValue = (FPoints1[0] - 8*FPoints1[1] + 8*FPoints1[3] - FPoints1[4])/(12*h);
    }
  }
  else
  {
    h *= p[axis-2];
    for(unsigned int k(0); k < RecipeModel->GetNpar(); ++k)
    {
      P0[k] = p[k];
      pTemp[k] = p[k];
    }
    for(int i(-2); i < 3; ++i)
    {
      pTemp[axis-2] = P0[axis-2]+i*h;
      FPoints1[i+2] = RecipeModel->EvalPar(x, pTemp);
    }
    CalculatedValue = (FPoints1[0] - 8*FPoints1[1] + 8*FPoints1[3] - FPoints1[4])/(12*h);
  }
  return CalculatedValue;
}

int DataObject::ReadData(std::ifstream &Input, std::vector<double> &List)
{
  List.clear();
  int prev, pos, columns(0);
  bool first_iteration(true);
  std::string line;
  if(Input.is_open())
  {
    while(std::getline(Input, line)) //Loop over each line in the data file.
    {
      prev = 0;
      if(line[0] != '#') //Ignore lines starting with a '#' (comment).
      {
	while( (pos = line.find_first_of(",", prev)) != std::string::npos) //This loop grabs strings delimited by a ','.
	{
	  if(pos > prev)
	  {
	    List.push_back( stof(line.substr(prev, pos-prev)) );
	    if(first_iteration) ++columns;
	  }
	  prev = pos + 1;
	}
	if(prev < line.length())
	{
	  List.push_back( stof(line.substr(prev, std::string::npos)) );
	  if(first_iteration) ++columns;
	}
	first_iteration = false;
      }
      else prev += line.size();
    }
  }
  else std::cerr << "File not found when reading in data." << std::endl;
  return columns;
}
