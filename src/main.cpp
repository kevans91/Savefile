#include <iostream>

#include "args.h"
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
		}
	}
	return 0;
};
