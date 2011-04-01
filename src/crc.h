#ifndef CRC_H
#define CRC_H

#include <algorithm>
#include <boost/crc.hpp>
#include <fstream>
#include <string>

class CRC {
protected:
private:
	unsigned char *data;
	unsigned int size;
public:
	CRC(unsigned char *buf, unsigned int len) : data(NULL), size(len) {
		data = new unsigned char[size + 1];
		data[size] = 0;
		for(unsigned int i = 0; i < size; i++) {
			data[i] = buf[i];
		}
	}

	CRC(const std::string& fileName) : data(NULL), size(0) {
		std::ifstream inStream(fileName.c_str(), std::ios::binary | std::ios::ate);
		size = inStream.tellg();
		inStream.seekg(0, std::ios::beg);
		data = new unsigned char[size + 1];
		data[size] = 0;
		inStream.read(reinterpret_cast<char*>(data), size);
		inStream.close();
	}

	~CRC() {
		delete [] data;
		data = NULL;
	}

	inline unsigned int operator ()() {
		boost::crc_optimal<32, 0xAF, 0xFFFFFFFF> crc_byond;
		crc_byond = std::for_each(data, data + size, crc_byond);
		return crc_byond();
	}
};

#endif
