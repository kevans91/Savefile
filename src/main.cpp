#include <iostream>
#include <fstream>

#include "args.h"
#include "datatypes.h"
#include "savefile.h"

int main(int argc, char **argv) {
	Args arglist(argc, argv);
	if(!arglist) {
		std::cout << "ERROR: Filename required." << std::endl;
	} else {
		std::string fileName = arglist.Input();
		Savefile sfile(fileName);
		if(!sfile) {
			std::cout << "ERROR: Failed to load savefile." << std::endl;
		} else {
			std::cout << "Reading savefile..." << std::endl;
			sfile.Read();
			if(arglist.Exists("-cd")) {
				std::string cdDir = arglist.Value("-cd");
				std::cout << sfile.ExportText(cdDir) << std::endl;
			} else {
				std::cout << sfile.ExportText("/") << std::endl;
			}
		}
	}
	return 0;
};
