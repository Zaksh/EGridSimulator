#pragma once
#include <string>
#include <vector>
class FileIO
{
	std::vector<double> ReadGridLabdFile(std::string fileName);
public:
	static std::string DiretoryPath; 
	FileIO(void);
	~FileIO(void);
	std::vector<double> ReadCdfData(int hour);
	std::vector<double> ReadXiData();
	std::vector<double> ReadValData(int hour);
	std::vector<double> ReadGridLabdData(int home);
	std::vector<double> ReadFile(std::string fileName);
	std::vector<double> ReadFile2(std::string fileName);
};

