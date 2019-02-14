# MinuitFit
MinuitFit is intended to make the testing and fitting of different yield models as quick and efficient as possible. To that end, the program makes use of configuration files which contain all relevant model definitions and settings. Adding or changing a model can be accomplished by simply editing the definitions file, with no recompilation necessary. Once the appropriate models are defined in the definitions file, the program can then perform the fit to that model on data loaded from a file (location specified in the settings file). The class implementing this functionality, called BasicModel, is also capable of drawing the resulting fit in a graph and saving the plot to a directory. This allows a user to very quickly change or add models, then check their ability to describe the actual data.

## Prerequisites
MinuitFit makes use of several ROOT libraries. The only version tested is my current version (v6-10-08). Additionally, some C++11 coding standards are used, so a compiler with support for this is required.

## Building MinuitFit
The program depends on several ROOT libraries, so before proceeding it is necessary to set up the ROOT environmental variables properly. In the directory where ROOT is installed, execute

```
source ./bin/thisroot.sh
```

Now in the top level directory of MinuitFit, execute

```
cmake .
```

to generate the make file, then

```
make
```

to build the program.

## Usage
The program can be run by issuing the command

```
./MinuitFit ModelType ModelID
```

ModelType, as the name would imply, specifies which type of yield you want to model. For nuclear recoil charge yield, this would be NRQY. These ModelTypes are defined in the function definitions file. For each model type, there may exist multiple functions you wish to try. ModelID specifies which of these functions you would like to use. As an example, to fit model 0 of NR charge yield, issue the command

```
./MinuitFit NRQY 0
```

### Adding or Modifying Models

The included definitions file is ModelDefinitions.txt. The program will attempt to search any non-empty line that does not start with a '#'. This allows for commenting and organization of the definitions by model type. When initializing a BasicModel object, the ModelType and ModelID command line arguments will be used to search this file. Each model has five required fields that must be initialized. The first is the functional form, which is specified exactly as would be done in ROOT's TF2 class constructor, as it is used directly to initialize a TF2 object. The second field is the initial parameters. These are very important, as bad guesses may result in a non-convergent fit. These are listed in the same order as defined in the function definition. The third and fourth fields are the lower and upper limits on the parameters, listed in the same order. To keep them unrestricted (as is most desirable), zeroes can be entered for both the lower and upper limits. The last field specifies the step size for each parameter.

An example of the structure in the definitions file is listed below:

```
NRQYF0:"1 / (TMath::Power(x+[0], 0.5) * [1] * TMath::Power(y, [2]))"
NRQYP0:"10,0.1,-0.1"
NRQYLL0:"0,0,0"
NRQYLH0:"0,0,0"
NRQYS0:"0.1,0.001,0.001"
```

There are a few important details to note here. Each line has "NRQY" at the beginning. This specifies the ModelType of the model. When the constructor of BasicModel receives the ModelType command line argument, it uses that to determine which lines in the definitions file to look for. If "NRQY" was your ModelType, then the program will only look at lines with "NRQY" placed at the beginning. These are not restricted to only four characters. Also notice that the last character before the colon, a "0" in this example, specifies the ModelID. In between the ModelType and ModelID are characters which denote which field is being defined: "F" for function, "P" for initial parameters, "LL" and "LH" for lower and upper limits respectively, and "S" for step sizes. After each search key is a colon, followed by the relevant value wrapped in double quotes. When BasicModel is initialized, it uses the ModelType and ModelID to key in on each of these six fields. All five fields must be specified in order for the model to be considered adequate by the program.

To add a new model, it is enough to duplicate these five lines, and change the relevant pieces of information (ModelID, defined values). To add an entirely new ModelType, it is necessary to replace "NRQY" by whatever you wish to denote your new ModelType by and set the data location in the Settings.txt file, as well as several fields related to axis labels and ranges for the ModelType. The program will be able to find the new ModelType if it is specified at the command line. Changing models can be done simply by editing the already existing values. No recompilation is necessary after adding or changing model definitions.

### Changing MinuitFit Settings

The included Settings.txt file defines a large number of settings that MinuitFit uses while running. Non-empty lines without a '#' beginning the line are searched by the program to find the relevant settings. Each setting has a comment describing what it is for, so configuration should be streamlined. The settings file should be located in the directory where the program is being run, but the data files and function definitions file can have their location (relative to the current directory) are specified inside the settings file. This allows a user to keep separate function definitions files. When defining a new ModelType (as discussed above), it is necessary to also add a setting specifying the data file name and location for the new ModelType. For NR charge yield, this setting is named "NRQYData". The program expects this structure, so if a ModelType called XYZ is added, then the setting XYZData must also be added to the settings file and filled with the proper file location. Following this same format, there are several fields that specify axis titles and ranges that also need to be constructed.

By default, new plots will be created in the current directory. This can be changed by setting "PlotScheme" to the desired location in the settings file. Additionally, there is functionality within the program to output the graphs in a ROOT file, though this is disabled by default.

### Data File Structure

The data files are regular .csv files. The program will ignore any line beginning with a '#', allowing for comments. This is useful to separate and label sections for each data source. There are seven fields that are expected to be filled: energy, energy uncertainty (lower error bar), energy uncertainty (uppper error bar), field, yield, yield uncertainty (lower error bar), and yield uncertainty (upper error bar). Each data point occupies a row, with the seven fields composing the columns. The data is separated only be a ',' and no spaces are used in non-comment lines. The constructor of BasicModel will load the appropriate data file (specified by ModelType with a location given in the settings file) and initialize the data appropriately.

A live copy of the digitized data can be found [here](https://docs.google.com/spreadsheets/d/18edTy3dwWbM6z4-YwYmQLV3v6QErBznoDbScF0Qsizs/edit#gid=2042525553).

