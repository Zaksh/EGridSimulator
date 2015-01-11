#include "FileIO.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
FileIO::FileIO(void)
{
}


FileIO::~FileIO(void)
{
}
std::vector<double> FileIO::ReadFile2(std::string fileName)
{
	//courtesy: http://stackoverflow.com/questions/6755111/read-input-files-fastest-way-possible
	//http://stackoverflow.com/questions/8365013/reading-line-from-text-file-and-putting-the-strings-into-a-vector
	std::vector<double> vec;
	std::ifstream is(fileName);
	copy(std::istream_iterator<double>(is),
		std::istream_iterator<double>(),
		back_inserter(vec));
	is.close();
	return vec;
}
std::vector<double> FileIO::ReadFile(std::string fileName)
{
	std::vector<double> values;
	FILE *fp;
	fp = fopen(fileName.c_str(), "rt");
	if (fp == NULL)
	{
		std::cout << "Error: UNABLE TO READ FILE " << fileName << std::endl;
		//OUTCON(0, "Error: ", message.c_str(), 0);
		exit(1);
		//return values;
	}
	else{
		char line[10240];
		while (fgets(line, sizeof(line), fp) != NULL)
		{
			char *c = strstr(line, "\n");  /*Check if line contains \n*/
			if (c != NULL) /* If \n found remove it */
				strcpy(c, "");
			double lineData = atof(line);
			values.push_back(lineData);
		}
		fclose(fp);
	}
	return values;

}
std::vector<double> FileIO::ReadCdfData(int hour)
{
	std::string fileName = DiretoryPath + "/fr" + std::to_string(hour) + ".txt";
	return ReadFile(fileName);
}
std::vector<double> FileIO::ReadXiData()
{
	std::string fileName = DiretoryPath + "/xi.txt";
	return ReadFile(fileName);
}
std::vector<double> FileIO::ReadValData(int hour)
{
	std::string fileName = DiretoryPath + "/xi" + std::to_string(hour) + ".txt";
	return ReadFile(fileName);
}
std::vector<double> FileIO::ReadGridLabdData(int home)
{
	return ReadGridLabdFile(DiretoryPath + "\\" + std::to_string(home) + ".csv");
}
std::vector<double> FileIO::ReadGridLabdFile(std::string fileName)
{
	std::vector<double> values;
	char *p = NULL;
	char buffer[20480];
	int fsize = 0;
	FILE *fp;
	fp = fopen(fileName.c_str(), "rt");
	if (fp == NULL)
		return values;
	char line[10240];
	bool isComment = false;
	bool isDataFound = false;
	bool isConfigProperlyRead = false;
	int fileCounter = 0;
	while (fgets(line, sizeof(line), fp) != NULL)
	{
		int len;
		char subst[65536];
		char *c = strstr(line, "\n");  /*Check if line contains \n*/
		if (c != NULL) /* If \n found remove it */
			strcpy(c, "");
		if (strstr(line, "timestamp") != NULL)
		{
			isDataFound = true;
		}
		else if (strstr(line, "end of tape") != NULL)
		{
			isConfigProperlyRead = true;
		}
		else if (isConfigProperlyRead)
		{
			break;
		}
		else if (isDataFound)
		{
			char *copy = line;
			while (*copy != NULL && *copy != '\0' && *copy != ',')
			{
				copy++;
			}
			copy++;
			values.push_back(atof(copy));
		}
	}
	fclose(fp);
	return values;
}
