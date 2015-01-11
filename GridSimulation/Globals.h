#include <map>
#include "HouseHoldLoadProfile.h"
cVTime globalClock;///Our global clock!
vector<Node*> gNodeVector;///vector that will keep track of all the nodes in the Simulator
vector<Node*> gAllNodes;
//vector<nodeInfo> gNodeInfoVector;
Timers gTimerManager;
//vector<double> gXiData;
//std::map<int, vector<double>> gCdfMap;
vector<uint_32> gFeedersTurnedOff;
vector<Output> gOutputData;
//std::map<int, vector<double>> gValData;
double gTotalUpsLosses = 0;
double gTotalSoftUpsConsumption = 0;
double gSaving = 0;
int gHomeAgents = 0;
int gMaxHomeAgents = 0;
//int Node::powerReduction=0;
//Path to you REDD data parsing files.
string FileIO::DiretoryPath = "E:/SysNet-SVN/HomeOS-Group/thesis/Zohaib/Scripts/ReadAllFiles of Red Parsed data/ksdensity";
uint_32 Node::MAX_POWER_GENERATION = 10000;
bool isSoftUPS = false;
LoadSheddingType gLoadSheddingType = lt_loadShedding;
PowerLevel powerLevel = PowerLevel0;
PowerReductionType powerReductionType = prt_conservative;
UlilityType ulilityTypeImplementation = ut_HighRelief;
float gAashiyanaPenetrationRatio = 0.6;
short gTotalFeederGroups = 10;
bool gVariableSupply = false;
float gSupplyDemandVariationRatio = 0;

HouseHoldLoadProfile *gHouseHoldLoadProfiles = NULL;

bool LoadConfigFile(char *file)/**< a pointer to the first character in the file name string */
{
	char *p = NULL;
	int fsize = 0;
	FILE *fp;
	fp = fopen(file, "rt");
	if (fp == NULL)
	{
		std::cerr << "Error: Configration file(" << file << ") not found" << endl;
	std:exit(1);
	}
	string message = "Reading config file: " + string(file);
	OUTCON(0, "MAIN", message.c_str(), 1);
	char line[10240];
	bool isComment = false;
	bool isData = false;
	bool isConfigProperlyRead = false;
	int fileCounter = 0;
	while (fgets(line, sizeof(line), fp) != NULL)
	{
		char *c = strstr(line, "\n");  /*Check if line contains \n*/
		if (c != NULL) /* If \n found remove it */
			strcpy(c, "");
		if (strcmp(line, "/*") == 0)
			isComment = true;
		else if (isComment)
		{
			if (strcmp(line, "*/") == 0)
			{
				isComment = false;
			}
		}
		else if (line[0] == '{')
		{
			isData = true;
		}
		else if (line[0] == '}')
		{
			isData = false;
			isConfigProperlyRead = true;
			break;
		}
		else if (isData)
		{
			c = strstr(line, ",");
			if (c != NULL) /* truncate at comment */
				strcpy(c, "");
			switch (fileCounter)
			{
			case 0:
				gMaxHomeAgents = atoi(line);
				break;
			case 1:
				FileIO::DiretoryPath = string(line);
				break;
			case 2:
				Node::MAX_POWER_GENERATION = atof(line);
				break;
			case 3:
				gLoadSheddingType = (LoadSheddingType)(atoi(line) < 4 ? atoi(line) : 1);
				break;
			case 4:
				powerReductionType = (PowerReductionType)(atoi(line) < 2 ? atoi(line) : 0);
				break;
			case 5:
				ulilityTypeImplementation = (UlilityType)(atoi(line) < 3 ? atoi(line) : 1);
				break;
			case 6:
				gAashiyanaPenetrationRatio = (atof(line) <= 1 ? atof(line) : 0.50);
				break;
			case 7:
				gTotalFeederGroups = atoi(line) >= 1 ? atoi(line):10;
				break;
			case 8:
			{
					  float variation = atof(line);
					  if (variation <= 0 || variation > 1)
					  {
						  gVariableSupply = false;
						  OUTCON(0, "Error: ", "Invalid configration of SupplyDemandGap it should be between 0 to 1", 0);
					  }
					  else
					  {
						  gVariableSupply = true;
						  gSupplyDemandVariationRatio = (1 - variation);
					  }
			}
				break;
			default:
				break;
			}
			fileCounter++;
		}
	}
	return isConfigProperlyRead;

}
void LoadConfigration(char *filePath)
{
	LoadConfigFile(filePath);
}
