//C++ includes.
#include <string> //Basic string.
#include <iostream> //Basic input and output.

//Custom includes.
#include "Models.h" //Header file for the model objects.

int main(int argc, char** argv)
{
  if(argc == 3)
  {
    std::string ModelType;
    unsigned int ModelArg(0);
    ModelType = argv[1];
    ModelArg = std::stoi(argv[2]);
    NESTModel::BasicModel Model(ModelType,ModelArg);
    NESTModel::GlobalModel = &Model;
    Model.Minimize();
    Model.PrintResults();
    Model.SaveParameters();
    Model.DrawGraphs();
  }
  else std::cerr << "Invalid arguments. First argument must be the model type (as listed in function definitions file; e.g. \"NRQY\"). Second argument must be a valid model ID. Example: \'./MinuitFit NRQY 0\'" << std::endl;
  return 0;
}
