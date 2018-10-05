#pragma once

#include <iostream>
#include <string>
#include <math.h>

struct OSCOrientation
{
	bool exists = false;
	std::string type = ""; //Wrong type
	double h = NAN;
	double p = NAN;
	double r = NAN;

	void printOSCOrientation()
	{
		std::cout << "\t" << " - Orientation" << std::endl;
		std::cout << "\t" << "h = " << h << std::endl;
		std::cout << "\t" << "p = " << p << std::endl;
		std::cout << "\t" << "r = " << r << std::endl;
		std::cout << std::endl;
	};
};