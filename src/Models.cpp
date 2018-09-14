//C++ includes
#include <iostream> //Basic input and output.
#include <memory> //For using shared_ptr.
#include <vector> //STL vector.
#include <set> //STL set.
#include <map> //STL map.
#include <algorithm> //For searching containers and std::copy.
#include <sstream> //Useful for number -> string conversion.
#include <string> //Basic string.
#include <iomanip> //Set precision for output stream.

//ROOT includes
#include "TMath.h" //Basic math functions.
#include "TGraphAsymmErrors.h" //ROOT 1D graph with asymmetric error bars.
#include "TAxis.h" //For using the TAxis object.
#include "TCanvas.h" //For using the TCanvas object.
#include "TF2.h" //ROOT 1D function.
#include "TMultiGraph.h" //Useful for managing multiple graphs on one plot.
#include "TPaveText.h" //For adding text boxes to a TCanvas.
#include "TFile.h" //ROOT file input/output.
#include "TColor.h" //ROOT graph coloring.
#include "TH2F.h" //Dummy to create a TPaletteAxis object.
#include "TPaletteAxis.h" //Gradient axis bar.
#include "TGaxis.h" //Axis contained in TPaletteAxis.

//Custom includes
#include "Models.h" //Header file for this implementation.
#include "DataObject.h" //Modularizes the input of data sets from .txt files.
#include "FunctionObject.h" //Modularizes the input of model functions from a .txt file.
#include "SettingsObject.h" //Modularizes the input of settings from a .txt file.

NESTModel::BasicModel::BasicModel(std::string modeltype, unsigned int id)
{
  ID = id; //Set the model ID number.
  Settings.reset(new SettingsObject("Settings.txt")); //Set and load the settings file. This location is relative to where the program is being run.
  NData = 0; //NData should be zero until data is loaded.
  Success = false; //By default, no success.
  DefaultField = -1; //-1 tells the operator() function that both the energy and field were provided.
                     //Otherwise, operator() will use the value in DefaultField for the field value.
  ModelType = modeltype; //Set the model type, which specifies where to look in the function definitions
  FuncObject.reset(new FunctionObject(Settings->Query("FunctionDefinitions"), ModelType, ID, Success)); //Load the function object from the functions definitions file. Success is captured in "Success".
  if(Success)
  {
    InitialVect = FuncObject->GetParameters(); //Load the initial parameters.
    LimitsLow = FuncObject->GetLimitsLow(); //Load lower limits on parameters.
    LimitsHigh = FuncObject->GetLimitsHigh(); //Load upper limits on parameters.
    StepVect = FuncObject->GetStepSizes(); //Load step sizes.
    NPar = InitialVect.size(); //Set the number of parameters.
    MinuitMinimizer.reset(new TMinuit(NPar)); //Create the TMinuit object.
    ModelFunction.reset(new TF2("ModelFunction", FuncObject->GetFunction().c_str(), 0, 1000, 0, 5000)); //Create the 2D function that will do the heavy lifting for the function evaluating.
    ModelDerivative.reset(new TF2("ModelDerivative", FuncObject->GetDerivative().c_str(), 0, 1000, 0, 5000)); //Create the 2D derivative function that will do the heavy lifting for the derivative evaluating.
    DataObject DataObj(Settings->Query(std::string(ModelType+"Data"))); //Load data from the data file.
    DataX = DataObj.GetDataX(); //Set energy data.
    DataY = DataObj.GetDataY(); //Set field data.
    DataZ = DataObj.GetDataZ(); //Set yield data.
    DataXErrLow = DataObj.GetDataXErrLow(); //Set lower energy error bar.
    DataXErrHigh = DataObj.GetDataXErrHigh(); //Set upper energy error bar.
    DataZErrLow = DataObj.GetDataZErrLow(); //Set lower yield error bar.
    DataZErrHigh = DataObj.GetDataZErrHigh(); //Set upper yield error bar.
    NData = DataX.size(); //Set NData properly.
  }
  else std::cerr << "NESTModel::BasicModel::BasicModel(): A proper model was not found in definitions file." << std::endl;
}

double NESTModel::BasicModel::operator()(double* x, double* p)
{
  double CalculatedValue(0); //Default calculated value.
  if(DefaultField == -1) CalculatedValue = ModelFunction->EvalPar(x,p); //Use given values in "x".
  else //Use energy from "x" and field in "DefaultField".
  {
    double x2[2];
    x2[0] = x[0];
    x2[1] = DefaultField;
    CalculatedValue = ModelFunction->EvalPar(x2,p);
  }
  return CalculatedValue;
}

double NESTModel::BasicModel::Derivative(double* x, double* p)
{
  return ModelDerivative->EvalPar(x,p); //Evaluate derivative and return.
}

bool NESTModel::BasicModel::Minimize()
{
  if(Success)
  {
    MinuitMinimizer->SetPrintLevel(stoi(Settings->Query("Verbosity"))); //Set how loud the minimizer will be. 0 is normal, -1 low, and 1 high.
    MinuitMinimizer->SetFCN(NESTModel::Chi2); //Set the function to be minimized.
    double arglist[5]; //Create an array for possible arguments.
    int ierflg(0); //Error flag. Modified by minuit commands.
    arglist[0] = std::stod(Settings->Query("UP")); //Load the value of UP.
    MinuitMinimizer->mnexcm("SET ERR", arglist, 1, ierflg); //Set UP.
    for(unsigned int i(0); i < NPar; ++i) MinuitMinimizer->mnparm(i, std::string("a"+std::to_string(i)).c_str(), InitialVect.at(i), StepVect.at(i), LimitsLow.at(i), LimitsHigh.at(i), ierflg); //Set initial parameters, step sizes, and limits in the minimizer.
    arglist[0] = std::stoi(Settings->Query("MaxCalls")); //Maximum number of calls.
    arglist[1] = std::stoi(Settings->Query("Tolerance")); //Tolerance. Stops when EDM < 0.01*[Tolerance]*UP.
    MinuitMinimizer->mnexcm(Settings->Query("Algorithm").c_str(), arglist, 2, ierflg); //Execute minimization.
    if(ierflg == 0)
    {
      double Parameter, ParameterError;
      for(unsigned int i(0); i < NPar; ++i) //If successful, retrieve fit parameters and their error.
      {
	MinuitMinimizer->GetParameter(i, Parameter, ParameterError);
	Parameters.push_back(Parameter);
	ParameterErrors.push_back(ParameterError);
      }
    }
    return (ierflg == 0) ? true : false; //Return success based on ierflg.
  }
  else
  {
    std::cerr << "NESTModel::BasicModel::Minimize(): Can't initialize minimizer. Invalid definition loaded." << std::endl;
    return false;
  }
}

void NESTModel::BasicModel::PrintResults()
{
  if(Success)
  {
    double MinChi2, EDM, ErrDef;
    int NParI, NParX, IStat;
    MinuitMinimizer->mnstat(MinChi2, EDM, ErrDef, NParI, NParX, IStat);
    std::cout << "******************************************************" << std::endl;
    std::cout << "ModelID: " << ID << std::endl;
    std::cout << "Minimum Chi^2: " << MinChi2 << std::endl;
    std::cout << "Reduced Chi^2: " << MinChi2/(NData-NParX) << std::endl;
    for(unsigned int i(0); i < Parameters.size(); ++i)
    {
      std::cout << "Parameter " << i << ": "
		<< Parameters.at(i) << " +/- " << ParameterErrors.at(i)
		<< std::endl;
    }
    std::cout << "******************************************************" << std::endl;
  }
  else std::cerr << "No results to print." << std::endl;
}

void NESTModel::BasicModel::DrawGraphs()
{
  //Graph properties.
  std::string Title(Settings->Query("GraphTitles"));
  std::string XTitle(Settings->Query("XTitles"));
  std::string ZTitle(Settings->Query("ZTitles"));
  std::string ROOTName(Settings->Query("ROOTName"));
  std::string PlotScheme(Settings->Query("PlotScheme"));
  std::string PlotExtension(Settings->Query("PlotExtension"));
  double XLow(std::stod(Settings->Query("XLow")));
  double XHigh(std::stod(Settings->Query("XHigh")));
  double ZLow(std::stod(Settings->Query("ZLow")));
  double ZHigh(std::stod(Settings->Query("ZHigh")));
  bool LogX(Settings->Query("LogX") == "true" ? true : false);
  bool LogY(Settings->Query("LogY") == "true" ? true : false);
  bool OutputToFile(Settings->Query("OutputToFile") == "true" ? true : false);
  unsigned int PaletteEnum(stoi(Settings->Query("Palette")));
  unsigned int MarkerStyle(stoi(Settings->Query("MarkerStyle")));
  unsigned int MarkerSize(stoi(Settings->Query("MarkerSize")));
  unsigned int LineStyle(stoi(Settings->Query("LineStyle")));
  unsigned int LineSize(stoi(Settings->Query("LineSize")));
  
  TColor::SetPalette(PaletteEnum,0);
  std::vector<int> Colors;
  for(unsigned int i(0); i < (unsigned int)TColor::GetNumberOfColors(); ++i) Colors.push_back(TColor::GetColorPalette(i));
  
  //Separate data by field.
  std::map<double, std::vector< std::vector<double> > > Map;
  std::vector< std::vector<double> > TempOuterVector;
  std::vector<double> TempInnerVector = {-1};
  double YLow(0), YHigh(0);
  for(unsigned int i(0); i < DataY.size(); ++i)
  {
    if(DataY.at(i) > YHigh) YHigh = DataY.at(i);
    //if(DataY.at(i) < YLow) YLow = DataY.at(i);
    if(Map.count(DataY.at(i)) == 0) //Not a previously entered field value.
    {
      //Create the first entry for this field value with it's corresponding points.
      TempInnerVector.at(0) = DataX.at(i);
      TempOuterVector.push_back(TempInnerVector);
      TempInnerVector.at(0) = DataXErrLow.at(i);
      TempOuterVector.push_back(TempInnerVector);
      TempInnerVector.at(0) = DataXErrHigh.at(i);
      TempOuterVector.push_back(TempInnerVector);
      TempInnerVector.at(0) = DataZ.at(i);
      TempOuterVector.push_back(TempInnerVector);
      TempInnerVector.at(0) = DataZErrLow.at(i);
      TempOuterVector.push_back(TempInnerVector);
      TempInnerVector.at(0) = DataZErrHigh.at(i);
      TempOuterVector.push_back(TempInnerVector);
      Map.emplace(DataY.at(i), TempOuterVector);
      TempOuterVector.clear();
    }
    else //The field has been previously entered.
    {
      //Add a new set of points to this field value.
      Map.find(DataY.at(i))->second.at(0).push_back(DataX.at(i));
      Map.find(DataY.at(i))->second.at(1).push_back(DataXErrLow.at(i));
      Map.find(DataY.at(i))->second.at(2).push_back(DataXErrHigh.at(i));
      Map.find(DataY.at(i))->second.at(3).push_back(DataZ.at(i));
      Map.find(DataY.at(i))->second.at(4).push_back(DataZErrLow.at(i));
      Map.find(DataY.at(i))->second.at(5).push_back(DataZErrHigh.at(i));
    }
  }
  const unsigned int MapSize(Map.size()); //Need to create this many separate graphs.
  TGraphAsymmErrors* GraphArray[MapSize]; //Create the array that will hold these graphs.
  TF1* FunctionArray[MapSize]; //Create the array that will hold the functions to draw.
  const unsigned int MaxPoints(NData); //At most, we have this many points for a given field.
  double DataXArr[MaxPoints], DataXErrLowArr[MaxPoints], DataXErrHighArr[MaxPoints], DataZArr[MaxPoints], DataZErrLowArr[MaxPoints], DataZErrHighArr[MaxPoints]; //Arrays for handing off data to graph constructor.
  unsigned int FieldIndex(0); //Will need this later.
  double ParameterArr[NPar]; //Need to have parameters stored in an array so we can set the functions parameters.
  unsigned int ColorList[MapSize]; //Stores the color of each function.
  TMultiGraph* MultiGraph = new TMultiGraph(); //Create the multigraph object.
  MultiGraph->SetTitle(std::string(Title+";"+XTitle+";"+ZTitle).c_str());
  for(std::map<double, std::vector< std::vector<double> > >::iterator MapIterator = Map.begin(); MapIterator != Map.end(); ++MapIterator, ++FieldIndex)
  {
    //Copy the data into arrays for use in the TGraphAsymmErrors construction.
    std::copy(MapIterator->second.at(0).begin(), MapIterator->second.at(0).end(), DataXArr);
    std::copy(MapIterator->second.at(1).begin(), MapIterator->second.at(1).end(), DataXErrLowArr);
    std::copy(MapIterator->second.at(2).begin(), MapIterator->second.at(2).end(), DataXErrHighArr);
    std::copy(MapIterator->second.at(3).begin(), MapIterator->second.at(3).end(), DataZArr);
    std::copy(MapIterator->second.at(4).begin(), MapIterator->second.at(4).end(), DataZErrLowArr);
    std::copy(MapIterator->second.at(5).begin(), MapIterator->second.at(5).end(), DataZErrHighArr);
    GraphArray[FieldIndex] = new TGraphAsymmErrors(MapIterator->second.at(0).size(), DataXArr, DataZArr, DataXErrLowArr, DataXErrHighArr, DataZErrLowArr, DataZErrHighArr); //Create the graph object for this field.
    DefaultField = MapIterator->first; //Set the field so that operator() understands what field to use.
    FunctionArray[FieldIndex] = new TF1("f", *this, XLow, XHigh, NPar); //Create the function object.
    std::copy(Parameters.begin(), Parameters.end(), ParameterArr); //Copy the best fit parameters.
    FunctionArray[FieldIndex]->SetParameters(ParameterArr); //Set the parameters in the function.
    DefaultField = -1; //Reset after creating the functions.
    ColorList[FieldIndex] = Colors.at(int((MapIterator->first - YLow)/((YHigh - YLow)/(Colors.size()-1)))); //Calculate the color associated with this field value by breaking the field range into bins.
    //Set graph and function draw options.
    GraphArray[FieldIndex]->SetMarkerColor(ColorList[FieldIndex]);
    GraphArray[FieldIndex]->SetLineColor(ColorList[FieldIndex]);
    FunctionArray[FieldIndex]->SetLineColor(ColorList[FieldIndex]);
    GraphArray[FieldIndex]->SetMarkerSize(MarkerSize);
    GraphArray[FieldIndex]->SetMarkerStyle(MarkerStyle);
    GraphArray[FieldIndex]->SetLineWidth(LineSize);
    GraphArray[FieldIndex]->SetLineStyle(LineStyle);
    MultiGraph->Add(GraphArray[FieldIndex]);
  }
  auto Canvas = new TCanvas("YieldCanvas", "YieldCanvas", 1920, 1080);
  if(LogX) Canvas->SetLogx();
  if(LogY) Canvas->SetLogy();
  Canvas->SetRightMargin(0.15);

  //Add the color bar on the right side of the graph. This requires using a dummy TH2 object to create
  //the TPaletteAxis object, then "stealing" and repurposing it for our uses.
  TH2F *HistDummy = new TH2F("HistDummy", "HistDummy", 100, 0, 10, 100, 0, 10);
  HistDummy->Fill(5,5,YHigh);
  HistDummy->SetContour(Colors.size());
  HistDummy->GetZaxis()->SetTitle("Field [V/cm]");
  HistDummy->Draw("COLZ");
  gPad->Update();
  TPaletteAxis* PaletteAxis = (TPaletteAxis*)HistDummy->GetListOfFunctions()->FindObject("palette");
  PaletteAxis->GetAxis()->SetTitle("Field [V/cm]");
  PaletteAxis->GetAxis()->CenterTitle();
  
  TFile *OutputFile;
  if(OutputToFile) OutputFile = new TFile(ROOTName.c_str(), "RECREATE"); //If we want to output to the file, then do so.
  //Draw the TMultiGraph and set appropriate titles and features.
  MultiGraph->Draw("AP");
  MultiGraph->GetXaxis()->SetLimits(XLow,XHigh);
  MultiGraph->GetXaxis()->CenterTitle();
  MultiGraph->GetYaxis()->SetRangeUser(ZLow,ZHigh);
  MultiGraph->GetYaxis()->CenterTitle();
  PaletteAxis->Draw(); //Draw the TPaletteAxis.
  for(unsigned int i(0); i < MapSize; ++i) FunctionArray[i]->Draw("SAME"); //Draw each of the functions.
  if(OutputToFile && OutputFile->IsOpen())
  {
    Canvas->Write();
    MultiGraph->Write();
  }
  Canvas->SaveAs(static_cast<std::stringstream&>(std::stringstream("").flush() << PlotScheme << ModelType << "_Model" << ID << PlotExtension.c_str()).str().c_str());
  delete Canvas;
  if(OutputToFile) delete OutputFile;
  delete MultiGraph;
  delete HistDummy;
  for(unsigned int i(0); i < MapSize; ++i) delete FunctionArray[i];
}

void NESTModel::BasicModel::SetDefaultField(double Field)
{
  DefaultField = Field;
}

std::vector<double> NESTModel::BasicModel::GetParameters()
{
  return Parameters;
}

std::vector<double> NESTModel::BasicModel::GetParameterErrors()
{
  return ParameterErrors;
}

std::vector<double> NESTModel::BasicModel::GetDataX()
{
  return DataX;
}

std::vector<double> NESTModel::BasicModel::GetDataXErrLow()
{
  return DataXErrLow;
}

std::vector<double> NESTModel::BasicModel::GetDataXErrHigh()
{
  return DataXErrHigh;
}

std::vector<double> NESTModel::BasicModel::GetDataY()
{
  return DataY;
}

std::vector<double> NESTModel::BasicModel::GetDataZ()
{
  return DataZ;
}

std::vector<double> NESTModel::BasicModel::GetDataZErrLow()
{
  return DataZErrLow;
}

std::vector<double> NESTModel::BasicModel::GetDataZErrHigh()
{
  return DataZErrHigh;
}
