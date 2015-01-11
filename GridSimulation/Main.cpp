#include "Simulator.h"
#include "Globals.h"
#include <ctime>
void main(int argc, char *argv[])
{

	//isSoftUPS = true;
	string ioDirectory = "./";
	string outputDataFileName = ioDirectory + "A--OutPut.csv";
	FileIO file;
	char *configFilePath=NULL;
	double aashiyanaPenetration=-1;
	for (int i = 1; i < argc; i++)  /* Skip argv[0] (program name). */
	{
		/*
		* Use the 'strcmp' function to compare the argv values
		* to a string of your choice (here, it's the optional
		* argument "-q").  When strcmp returns 0, it means that the
		* two strings are identical.
		*/

		if (strcmp(argv[i], "-o") == 0 && i+1<argc)  /* Process optional arguments. */
		{
			outputDataFileName = argv[i + 1];
		}
		else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc)
		{
			configFilePath = argv[i + 1];
		}
		else if(strcmp(argv[i], "-p") == 0 && i + 1<argc)
		{
			aashiyanaPenetration = atof(argv[i + 1]);
			/* Process non-optional arguments here. */
		}
	}
	string message;
	OUTCON(0, "MAIN", "Loading Configration", 3);
	if (configFilePath == NULL)
	{
		configFilePath = "input.cfg";
	}
	LoadConfigration(configFilePath);
	if (aashiyanaPenetration != -1)
	{
		gAashiyanaPenetrationRatio = aashiyanaPenetration;
	}
	//vector<double> data=file.ReadGridLabdData(1);
	//isSoftUPS = true;
	OUTCON(0, "NumberOfHomeAgents:", to_string(gMaxHomeAgents).c_str(), 3);
	OUTCON(0, "DataPath", FileIO::DiretoryPath.c_str(), 3);
	if (gVariableSupply)
	{
		OUTCON(0, "MaxPowerGenerationCapacity:Variable", to_string(1-gSupplyDemandVariationRatio).c_str(), 3);
	}
	else 
	{
		OUTCON(0, "MaxPowerGenerationCapacity", to_string(Node::MAX_POWER_GENERATION).c_str(), 3);
	}
	OUTCON(0, "LoadSheddingPolicy", (gLoadSheddingType == 0 ? "LoadShedding" : (gLoadSheddingType == 1 ? "DistributedBackOff" : (gLoadSheddingType == 2? "SmartMeterDistributedBackOff": "Central"))), 3);
	OUTCON(0, "PowerReductionType", (powerReductionType == 0 ? "Agressive":"Conservative"), 3);
	OUTCON(0, "UlilityType", (ulilityTypeImplementation == 0 ? "HighDiscomfort" : (ulilityTypeImplementation==1?"Middle": "HighRelief")), 3);
	OUTCON(0, "AashiyanaPenetrationRatio", to_string(gAashiyanaPenetrationRatio).c_str(), 3);
	OUTCON(0, "TotalFeederGroups", to_string(gTotalFeederGroups).c_str(), 3);
	int totalNumberOfDevices=HouseHoldDeviceNames.size();
	gHouseHoldLoadProfiles = new HouseHoldLoadProfile[totalNumberOfDevices];
	for (int dev = 0; dev < totalNumberOfDevices; dev++)
	{
		gHouseHoldLoadProfiles[dev].SetHouseHoldLoadType((HouseHoldDeviceType)dev);
	}
	FILE *stream;
	string rand = to_string(std::clock());
	//string outFileName = ioDirectory + "trace" + rand + ".out";
	string outFileName = ioDirectory + "A - trace---.out";
	message = "Initializing Simulator";
	OUTCON(0, "MAIN", message.c_str(), 3);
	/*for (int i = 1; i <= 2; i++)
	{
		gCdfMap[i] = file.ReadCdfData(i);
	}*/
	if (fopen_s(&stream, outFileName.c_str(), "w+") == 0)
	{
		Simulator simulator(stream, 3);
		//Simulator simulator(fopen(outFileName.c_str(),"w+"),10);
		//simulator.fptr=fopen(outFileName.c_str(),"w+");
		//OUTFILE(simulator.fptr,"-1","Test","Writing test",3);
		int simTimeinHour = 24;
		simulator.MAX_SIMTIME = (long)(simTimeinHour*HR_TO_TIMETICK);//run Simulator for simTimeinHour
		simulator.SimStart();
		_fcloseall();
	}
	else
	{
		message = "File opening error";
		OUTCON(0, "MAIN", message.c_str(), 1);
	}
	/*FILE *streamFeeders;
	string outFeederFileName = ioDirectory + "feederTrace" + rand + ".out";
	if (fopen_s(&streamFeeders, outFeederFileName.c_str(), "w+") == 0)
	{
		for (auto iterator = gFeedersTurnedOff.begin(); iterator < gFeedersTurnedOff.end(); iterator++)
		{
			fprintf(streamFeeders, "%d\n", (*iterator));
		}
		_fcloseall();
	}*/
	FILE *streamOutPut;
	//string outputDataFileName = ioDirectory + "OutPut" + rand + ".out";
	
	if (fopen_s(&streamOutPut, outputDataFileName.c_str(), "w+") == 0)
	{
		string line = "Actual Demand,Power Supplied,Number Of Feeders Turned Off,User Comfort,Homes In Power Level1,Homes In Power Level2,Homes In Power Level3,Homes In Power Level4,Homes In Power Level5";
		fprintf(streamOutPut, "%s\n", line.c_str());
		for (auto iterator = gOutputData.begin(); iterator < gOutputData.end(); iterator++)
		{
			string line = to_string((*iterator).actualPowerDemand) + "," 
				+ to_string((*iterator).powerSupplied) + "," + to_string((*iterator).numberOfFeedersTurnedOff) + "," + to_string((*iterator).userComfort)+"," + to_string((*iterator).homesInPowerLevel0)
				+ "," + to_string((*iterator).homesInPowerLevel1)
				+ "," + to_string((*iterator).homesInPowerLevel2)
				+ "," + to_string((*iterator).homesInPowerLevel3)
				+ "," + to_string((*iterator).homesInPowerLevelFull);
			fprintf(streamOutPut, "%s\n", line.c_str());
		}
		_fcloseall();
	}

}
