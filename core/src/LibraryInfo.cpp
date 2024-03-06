#include "LibraryInfo.h"


#include <iostream>

namespace Log
{
	void LibraryInfo::printInfo()
	{
		printInfo(std::cout);
	}
	void LibraryInfo::printInfo(std::ostream& stream)
	{
		std::stringstream ss;
		ss << "Library Name: " << name << "\n"
			<< "Author: " << author << "\n"
			<< "Email: " << email << "\n"
			<< "Website: " << website << "\n"
			<< "License: " << license << "\n"
			<< "Version: " << versionStr() << "\n"
			<< "Compilation Date: " << compilationDate << "\n"
			<< "Compilation Time: " << compilationTime << "\n";

		stream << ss.str();
	}
}