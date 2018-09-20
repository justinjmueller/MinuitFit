#ifndef MODELS_H
#define MODELS_H

//C++ includes.
#include <memory>
#include <vector>
#include <cmath>

//ROOT includes.
#include "TMinuit.h"
#include "TMath.h"
#include "TF2.h"
#include "TF1.h"

//Custom Includes
#include "SettingsObject.h"
#include "FunctionObject.h"

namespace NESTModel
{
  class BasicModel
  {
  public:
    BasicModel(std::string modeltype, unsigned int id = 0);
    double operator()(double* x, double* p);
    double Derivative(double* x, double* p);
    bool Minimize();
    void PrintResults();
    void DrawGraphs();
    void SetDefaultField(double Field);
    std::vector<double> GetParameters();
    std::vector<double> GetParameterErrors();
    std::vector<double> GetDataX();
    std::vector<double> GetDataXErrLow();
    std::vector<double> GetDataXErrHigh();
    std::vector<double> GetDataY();
    std::vector<double> GetDataZ();
    std::vector<double> GetDataZErrLow();
    std::vector<double> GetDataZErrHigh();
    
  private:
    unsigned int ID;
    unsigned int NData;
    unsigned int NPar;
    double DefaultField;
    bool Success;
    bool Is2DFit;
    double Chisquare;
    double EDM;
    std::shared_ptr<TMinuit> MinuitMinimizer;
    std::vector<double> InitialVect;
    std::vector<double> StepVect;
    std::vector<double> LimitsLow;
    std::vector<double> LimitsHigh;
    std::vector<double> Parameters;
    std::vector<double> ParameterErrors;
    std::shared_ptr<SettingsObject> Settings;
    std::shared_ptr<FunctionObject> FuncObject;
    std::shared_ptr<TF2> ModelFunction2D;
    std::shared_ptr<TF2> ModelDerivative2D;
    std::shared_ptr<TF1> ModelFunction1D;
    std::shared_ptr<TF1> ModelDerivative1D;
    std::string ModelType;
    std::vector<double> DataX;
    std::vector<double> DataXErrLow;
    std::vector<double> DataXErrHigh;
    std::vector<double> DataY;
    std::vector<double> DataZ;
    std::vector<double> DataZErrLow;
    std::vector<double> DataZErrHigh;
  };

  /*The following is left as an example for inheritance. You want to specify the following, as well
    as redefine the desired member functions.
  class NRChargeYield : public BasicModel
  {
  public:
    using BasicModel::BasicModel; //Tell the compiler that we WANT to inherit BaseModel's constructors.
    };*/

  BasicModel* GlobalModel;

  void Chi2(int& npar, double *x, double &result, double *par, int flag)
  {
    double WRSS(0);
    double Difference(0);
    double xData[2];
    for(unsigned int datum(0); datum < GlobalModel->GetDataX().size(); ++datum)
    {
      xData[0] = GlobalModel->GetDataX().at(datum);
      xData[1] = GlobalModel->GetDataY().at(datum);
      Difference = GlobalModel->GetDataZ().at(datum) - (*GlobalModel)(xData, par);
      if(Difference < 0) WRSS += (TMath::Power(Difference, 2.0)) / ( TMath::Power(GlobalModel->GetDataZErrHigh().at(datum),2.0) + TMath::Power( (0.5*(GlobalModel->GetDataXErrLow().at(datum) + GlobalModel->GetDataXErrHigh().at(datum))*(GlobalModel->Derivative(xData, par)) ), 2.0) );
      else WRSS += (TMath::Power(Difference, 2.0)) / ( TMath::Power(GlobalModel->GetDataZErrLow().at(datum),2.0) + TMath::Power( (0.5*(GlobalModel->GetDataXErrLow().at(datum) + GlobalModel->GetDataXErrHigh().at(datum))*(GlobalModel->Derivative(xData, par)) ), 2.0) );
    }
    result = WRSS;
  }
}
#endif
