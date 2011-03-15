#include <iostream>
#include <fstream>

#include "args.h"
#include "datatypes.h"
#include "savefile.h"

int main(int argc, char **argv) {
	Args arglist(argc, argv);
	if(arglist[0].empty()) {
		std::cout << "ERROR: Filename required." << std::endl;
	} else {
		std::string fileName = arglist[0];
		Savefile sfile(fileName);
		if(!sfile) {
			std::cout << "ERROR: Failed to load savefile." << std::endl;
		} else {
			std::cout << "Reading savefile..." << std::endl;
			sfile.Read();
			std::string cdDir = arglist.Value("-cd");
			if(cdDir.empty()) cdDir = "/";
			std::cout << sfile.ExportText(cdDir) << std::endl;
		}
	}
	return 0;
};
