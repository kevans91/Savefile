#ifndef SAVEFILE_H
#define SAVEFILE_H

#include <fstream>
#include <string>
#include <sys/stat.h>

#include "netbuf.h"

class Savefile {
protected:
private:
	std::string	fileName;
	Netbuf		*fileBuf;
public:
	Savefile(const std::string& name) : fileName(name), fileBuf(NULL) {
		Load();
	}

	~Savefile() {
		delete fileBuf;
		fileBuf = NULL;
	}

	bool Load() {
		if(fileName.empty()) return false;
		struct stat st;
		unsigned int fileSize = 0;
		unsigned char *dataBuf = NULL;
		if(stat(fileName.c_str(), &st) != 0) return false;
		fileSize = st.st_size;
		dataBuf = new unsigned char[fileSize + 1];
		dataBuf[fileSize] = 0;
		std::ifstream inFile(fileName.c_str(), std::ios::binary);
		inFile.read(reinterpret_cast<char*>(dataBuf), fileSize);
		inFile.close();
		if(fileBuf) fileBuf->Load(dataBuf, fileSize);
		else fileBuf = new Netbuf(dataBuf, fileSize);

		delete [] dataBuf;
		dataBuf = NULL;
		return true;
	}

	bool Load(const std::string& name) {
		if(name.empty()) return false;
		fileName = name;
		return Load();
	}

	inline bool operator !() const { return (!fileBuf || !(*fileBuf)); }
};

#endif		// SAVEFILE_H
