#ifndef DATATYPES_H
#define DATATYPES_H

#include <map>
#include "netbuf.h"

enum DataType {
        DT_NoType = -1,
        DT_Null = 0x00,
        DT_String = 0x01,
        DT_File = 0x02,
        DT_Path = 0x03,
        DT_Float = 0x04,
	DT_Overlay = 0x09,
        DT_Object = 0x0B,
        DT_List = 0x0D,
};

enum ListType {
        LT_Normal = 0x00,
        LT_Associative = 0x01,
};


struct ExtraInfo {
	unsigned int entryOffset;
	unsigned int firstOffset;
	unsigned int dataOffset;

	ExtraInfo()
		: entryOffset(0), firstOffset(0), dataOffset(0) {
	}

	ExtraInfo(unsigned int entry, unsigned int first, unsigned int dOff)
		: entryOffset(entry), firstOffset(first), dataOffset(dOff) {
	}

	inline ExtraInfo& operator =(const ExtraInfo &rhs) {
		entryOffset = rhs.entryOffset;
		firstOffset = rhs.firstOffset;
		dataOffset = rhs.dataOffset;

		return (*this);
	}
};

template<typename T>
struct NumberEncoder {
	inline T operator ()(T val, ExtraInfo &extra, bool listVal = false) {
		unsigned int first = (extra.firstOffset - extra.entryOffset) + 1;
		unsigned char num_bytes = sizeof(T);
		T new_val = val;
		unsigned char *memLayout = reinterpret_cast<unsigned char*>(&new_val);
		unsigned char offset = 0;
		for(; offset < num_bytes; offset++) {
			if(!listVal) memLayout[offset] ^= (0x42 + first + (9 * ((extra.dataOffset + offset) - extra.entryOffset)));
			else memLayout[offset] ^= (0x67 + (9 * ((extra.dataOffset + offset) - extra.entryOffset)));
		}
		extra.dataOffset += offset;
		return new_val;
	}
};
struct StringEncoder {
	bool Decode(unsigned char * const str, unsigned short &length, ExtraInfo &extra, bool listVal = false) {
		if(!str || !length) return false;
		unsigned int first = (extra.firstOffset - extra.entryOffset) + 1;
		unsigned int offset = 0;
		NumberEncoder<unsigned short> encoder;
		length = encoder(length, extra, listVal);

		for(unsigned short i = 0; i < length; i++) {
			if(!listVal) str[i] ^= (0x42 + first + (9 * ((extra.dataOffset + offset++) - extra.entryOffset)));
			else str[i] ^= (0x67 + (9 * ((extra.dataOffset + offset++) - extra.entryOffset)));
		}
		extra.dataOffset += offset;
		return true;

	}

	bool Decode(unsigned char * const buf, unsigned int size, ExtraInfo &extra, bool listVal = false) {
		if(!buf || !size) return false;
		unsigned int first = (extra.firstOffset - extra.entryOffset) + 1;

		for(unsigned short i = 0; i < size; i++) {
			if(!listVal) buf[i] ^= (0x42 + first + (9 * ((extra.dataOffset + i) - extra.entryOffset)));
			else buf[i] ^= (0x67 + (9 * ((extra.dataOffset + i) - extra.entryOffset)));
		}
		extra.dataOffset += size;
		return true;

	}

	std::string Decode(Netbuf * const dataBuf, ExtraInfo &extra, bool listVal = false) {
		if(!dataBuf) return "";
		unsigned int first = (extra.firstOffset - extra.entryOffset) + 1;
		NumberEncoder<unsigned short> encoder;

		unsigned short length = dataBuf->ReadInt<unsigned short>();
		length = encoder(length, extra, listVal);
		unsigned char * str = dataBuf->ReadData(length);
		if(!str) return "";
		for(unsigned short i = 0; i < length; i++) {
			if(!listVal) str[i] ^= (0x42 + first + (9 * ((extra.dataOffset + i) - extra.entryOffset)));
			else str[i] ^= (0x67 + (9 * ((extra.dataOffset + i) - extra.entryOffset)));
		}
		std::string resultStr = reinterpret_cast<char*>(str);
		delete [] str;
		str = NULL;

		extra.dataOffset += length;
		return resultStr;

	}

	bool Encode(unsigned char * const str, unsigned short &length, ExtraInfo &extra, bool listVal = false) {
		if(!str || !length) return false;
		unsigned int first = (extra.firstOffset - extra.entryOffset) + 1;
		unsigned int offset = sizeof(unsigned short);
		for(unsigned short i = 0; i < length; i++) {
			if(!listVal) str[i] ^= (0x42 + first + (9 * ((extra.dataOffset + offset++) - extra.entryOffset)));
			else str[i] ^= (0x67 + (9 * ((extra.dataOffset + offset++) - extra.entryOffset)));
		}

		NumberEncoder<unsigned short> encoder;
		length = encoder(length, extra, listVal);
		extra.dataOffset += offset;
		return true;
	}

	std::string UrlEncode(const std::string& url) {
		std::ostringstream encodedStr("");
		for(unsigned int i = 0; i < url.size(); ++i) {
			if((url[i] >= 'a' && url[i] <= 'z') || (url[i] >= 'A' && url[i] <= 'Z') || (url[i] >= '0' && url[i] <= '9')) {
				encodedStr << url[i];
			} else {
				encodedStr.fill('0');
				encodedStr << '%' << std::setw(2) << std::hex << static_cast<unsigned short>(url[i]) << std::dec;
			}
		}
		return encodedStr.str();
	}

	std::string UrlDecode(const std::string& url) {
		std::ostringstream decodedStr("");
		std::istringstream numStr;
		unsigned short decVal = 0;
		numStr.setf(std::ios::hex, std::ios::basefield);
		for(unsigned int i = 0; i < url.size(); ++i ) {
			if(url[i] == '%') {
				++i;
				numStr.str(url.substr(i, 2));
				numStr >> decVal;
				numStr.clear();
				decodedStr << static_cast<unsigned char>(decVal);
				++i;
			} else {
				decodedStr << url[i];
			}
		}
		return decodedStr.str();
	}

	std::string Base64Encode(const unsigned char *data, unsigned int size) {
		if(!data || !size) return "";
		std::ostringstream encodedStr("");
		unsigned int buffer = 0, bytesWritten = 0, mask = (0xFF >> 0x02);
		for(unsigned int i = 0; i < size; i += 3) {
			buffer = data[i];
			buffer <<= 0x10;
			if((i + 2) < size) {
				buffer |= (data[i + 1] << 0x08);
				buffer |= (data[i + 2]);
			} else if((i + 1) < size) {
				buffer |= (data[i + 1] << 0x08);
			}
			encodedStr << base64table[(buffer & (mask << 0x12)) >> 0x12];		// First 6-bits will always be there.
			encodedStr << base64table[(buffer & (mask << 0x0C)) >> 0x0C];		// As will at least the two following bits
			if(!(buffer & (mask << 0x06)) && (i + 1) >= size) {
				encodedStr << '=';
			} else {
				encodedStr << base64table[(buffer & (mask << 0x06)) >> 0x06];
			}

			if(!(buffer & mask) && (i + 2) >= size) {
				encodedStr << '=';
			} else {
				encodedStr << base64table[buffer & mask];
			}
			bytesWritten += 4;
			if(!(bytesWritten%76)) {
				encodedStr << '\n';
			}
		}
		return encodedStr.str();
	}

	unsigned char * Base64Decode(const std::string &base64str, unsigned int fullLength) {
		if(!fullLength) return 0;
		std::map<unsigned char, unsigned char> base64rtable = Base64ReverseTable();
		unsigned int buffer = 0, offset = 0;
		unsigned char * data = new unsigned char[fullLength + 1];
		unsigned int mask = 0xFF;
		bool thirdVal = true;
		data[fullLength] = 0;
		for(unsigned int i = 0; i < base64str.size(); i += 4) {
			if(base64str[i] == '\n') {
				if(++i >= base64str.size()) break;
			}
			buffer = base64rtable[base64str[i]];
			buffer <<= 0x12;	// 6-bit value, align back into 24-bit spot
			buffer |= base64rtable[base64str[i + 1]] << 0x0C;

			data[offset++] = ((buffer & (mask << 0x010)) >> 0x10);	// First character
			if(base64str[i + 2] != '=') { // 3 6-bit values = 18-bit
				buffer |= base64rtable[base64str[i + 2]] << 0x06;
			} else {
				thirdVal = false;
			}
			data[offset++] = ((buffer & (mask << 0x08)) >> 0x08);
			if(thirdVal && base64str[i + 3] != '=') {
				buffer |= base64rtable[base64str[i + 3]];
			}

			if(thirdVal) data[offset++] = (buffer & mask);
				// Now we extract our three characters
		}
		return data;
	}
private:
	static const char base64table[];

	static std::map<unsigned char, unsigned char> Base64ReverseTable() {
		unsigned int length = 64;		// base64...
		std::map<unsigned char, unsigned char> base64rtable;
		for(unsigned int i = 0; i < length; i++) {
			base64rtable[base64table[i]] = i;
		}
		return base64rtable;
	}
};


#endif		// DATATYPES_H
