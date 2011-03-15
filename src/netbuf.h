#ifndef NETBUF_H
#define NETBUF_H

#include <iomanip>
#include <iostream>
#ifdef WIN32
#include <Winsock2.h>
#else
#include <netinet/in.h>
#endif
#include <sstream>

enum BufPos {
	BP_BEG = 0x00,
	BP_END = 0x01
};

enum EndianMode {
	EM_Unspec = -1,
	EM_Little = 0x00,
	EM_Big = 0x01,
};

class Netbuf {
protected:
private:
	unsigned char *	bufData;
	unsigned int	bufSize;
	unsigned int	bufPtr;
	EndianMode	bufEndian;
	EndianMode	systemEndian;


	template<typename T>
	T hton(T val) {
		if(systemEndian == bufEndian) return val;
		if(sizeof(T) == 2) return htons(val);
		else if(sizeof(T) == 4) return htonl(val);
		else return val;
	}

	template<typename T>
	T ntoh(T val) {
		if(systemEndian == bufEndian) return val;
		if(sizeof(T) == 2) return ntohs(val);
		else if(sizeof(T) == 4) return ntohl(val);
		else return(val);
	}

	void Init() {
		unsigned short fVal = 0x01;
		unsigned char *memLayout = reinterpret_cast<unsigned char*>(&fVal);
		if(memLayout[0]) {
			systemEndian = EM_Little;
		} else {
			systemEndian = EM_Big;
		}
		bufEndian = systemEndian;
	}

public:
	bool Load(const unsigned char * const data, const unsigned int size, BufPos ptrPos = BP_BEG) {
		if(!data || !size) return false;
		bufSize = size;
		bufData = new unsigned char[bufSize + 1];
		bufData[bufSize] = 0;
		for(unsigned int i = 0; i < bufSize; i++) bufData[i] = data[i];
		if(bufEndian == EM_Unspec || systemEndian == EM_Unspec) Init();
		if(ptrPos == BP_END) bufPtr = bufSize;
		else bufPtr = 0;
		return true;
	}

	bool Load(const Netbuf * const buf, BufPos ptrPos = BP_BEG) {
		if(!buf || !(*buf)) return false;
		return Load(buf->bufData, buf->bufSize, ptrPos);
	}

	Netbuf(const unsigned char * const data, const unsigned int size, BufPos ptrPos = BP_BEG)
			: bufData(NULL), bufSize(0), bufPtr(0), bufEndian(EM_Unspec), systemEndian(EM_Unspec) {
		Load(data, size, ptrPos);
	}

	Netbuf(const Netbuf& rhs)
			: bufData(NULL), bufSize(0), bufPtr(0), bufEndian(EM_Unspec), systemEndian(EM_Unspec) {
		Load(rhs.bufData, rhs.bufSize);
		bufPtr = rhs.bufPtr;
	}

	Netbuf() : bufData(NULL), bufSize(0), bufPtr(0), bufEndian(EM_Unspec), systemEndian(EM_Unspec) {
		Init();
	}

	~Netbuf() {
		delete [] bufData;
		bufData = NULL;
	}

	void Reset(unsigned int newSize = 0) {
		bufPtr = bufSize = 0;
		delete [] bufData;
		bufData = NULL;
		if(newSize) Resize(newSize);
	}

	void Resize(unsigned int n) {
		if(!n) return;
		unsigned char *tmp = bufData;
		bufData = new unsigned char[n + 1];
		bufData[n] = 0;
		if(tmp != NULL) {
			for(unsigned int i = 0; i < n; i++) {
				if(i < bufSize) bufData[i] = tmp[i];
			}
			delete [] tmp;
			tmp = NULL;
		} else {
			for(unsigned int i = 0; i < n; i++) bufData[i] = 0;
		}
		bufSize = n;
	}

	inline Netbuf& operator=(const Netbuf& rhs) {
		bufSize = rhs.bufSize;
		if(bufData != NULL) {
			delete [] bufData;
			bufData = NULL;
		}
		bufData = new unsigned char[bufSize + 1];
		bufData[bufSize] = 0;
		for(unsigned int i = 0; i < bufSize; i++) {
			bufData[i] = rhs.bufData[i];
		}

		return (*this);
	}

	template<typename T>
	unsigned char WriteInt(T num) {
		if(!Available(sizeof(T))) Resize(bufPtr + sizeof(T));
		*reinterpret_cast<T*>(bufData + bufPtr) = hton<T>(num);
		bufPtr += sizeof(T);
		return sizeof(T);
	}

	template<typename T>
	T ReadInt() {
		if(!Available(sizeof(T))) return 0;
		T ret = ntoh<T>(*reinterpret_cast<T*>(bufData + bufPtr));
		bufPtr += sizeof(T);
		return ret;
	}

	template<typename T>
	T WriteStr(const unsigned char *str, const T length) {
		if(!Available(sizeof(T) + length)) Resize(bufPtr + sizeof(T) + length);
		WriteInt<T>(length);
		for(unsigned int start = bufPtr; (bufPtr - start) < length; bufPtr++) bufData[bufPtr] = str[(bufPtr - start)];
		return (length + sizeof(T));
	}

	template<typename T>
	T WriteStr(const char *str, const T length) {
		if(!Available(sizeof(T) + length)) Resize(bufPtr + sizeof(T) + length);
		WriteInt<T>(length);
		for(unsigned int start = bufPtr; (bufPtr - start) < length; bufPtr++) bufData[bufPtr] = str[(bufPtr - start)];
		return (length + sizeof(T));
	}

	template<typename T>
	unsigned char *ReadStr(T *length) {
		if(!length) return NULL;
		(*length) = ReadInt<T>();
		if(!Available(*length)) {
			bufPtr -= sizeof(T);
			return NULL;
		}
		unsigned char *ret = new unsigned char[(*length) + 1];
		ret[(*length)] = 0;
		for(unsigned int i = 0; i < (*length); i++) ret[i] = bufData[bufPtr++];
		return ret;
	}

	unsigned char *ReadData(unsigned int length) {
		if(!length || !Available(length)) return NULL;
		unsigned char * ret = new unsigned char[length + 1];
		ret[length] = 0;
		for(unsigned int i = 0; i < length; i++) ret[i] = bufData[bufPtr++];
		return ret;
	}

	std::string ReadStr(char finalChr = 0x0) {
		std::string readStr;
		for(; bufData[bufPtr] != finalChr && bufPtr < bufSize; bufPtr++) {
			readStr += static_cast<char>(bufData[bufPtr]);
		}
		bufPtr ++;
		return readStr;
	}

	std::string ReadStr(unsigned int length) {
		std::string readStr;
		readStr.resize(length);
		for(; readStr.size() < length && bufPtr < bufSize; bufPtr++) {
			readStr += static_cast<char>(bufData[bufPtr]);
		}
		bufPtr ++;
		return readStr;
	}


	bool Append(const Netbuf* rhs, unsigned int start = 0, unsigned int nBytes = 0) {
		if(!rhs) return false;
		if(!nBytes || nBytes > rhs->Size()) nBytes = (rhs->Size() - start);
		return Append(rhs->Data() + start, nBytes);
	}

	bool Append(const unsigned char * const data, const unsigned int size) {
		if(!data || !size) return false;
		if(!Available(size)) Resize(bufPtr + size);
		for(unsigned int offset = 0; offset < size && bufPtr < bufSize; offset++) {
			bufData[bufPtr++] = data[offset];
		}
		return true;
	}

	Netbuf * Copy(unsigned int nBytes) {
		if(!bufData || !nBytes) return NULL;
		if(!Available(nBytes)) return new Netbuf(bufData + bufPtr, (bufSize - bufPtr), BP_END);
		return new Netbuf(bufData + bufPtr, nBytes, BP_END);
	}

	Netbuf * Cut(unsigned int nBytes) {
		if(!bufData || !nBytes) return NULL;
		if(!Available(nBytes)) nBytes = (bufSize - bufPtr);
		Netbuf * retBuf = new Netbuf(bufData + bufPtr, nBytes, BP_END);
		unsigned char *tmp = bufData;
		unsigned int newSize = (bufSize - nBytes);
		if(newSize) {
			// We're cut'ing from the position in the string, so...
			bufData = new unsigned char[newSize + 1];
			bufData[newSize] = 0;
			for(unsigned int i = 0; i < newSize && (i + nBytes) < bufSize; i++) {
				// We want to skip the data we just copied.
				if(i >= bufPtr) bufData[i] = tmp[i + nBytes];
				else bufData[i] = tmp[i];
			}
		}
		delete [] tmp;
		tmp = NULL;
		bufSize -= nBytes;
		return retBuf;
	}


	unsigned char * operator ()() const { return bufData; }
	unsigned char * Data() const { return bufData; }
	unsigned int Size() const { return bufSize; }
	unsigned int Pos() const { return bufPtr; }
	unsigned int Seek(unsigned int pos) { return (bufPtr = pos); }
	bool Available(unsigned int bytes) { return ((bufPtr + bytes) <= bufSize); }
	bool More() const { return (bufPtr < bufSize); }
	
	EndianMode SystemEndian() const { return systemEndian; }
	EndianMode Endian() const { return bufEndian; }
	EndianMode Endian(EndianMode newMode) { return (bufEndian = newMode); }
	unsigned int Skip(unsigned int bytes) {
		if((bytes + bufPtr) > bufSize) bufPtr = bufSize;
		bufPtr += bytes;
		return bufPtr;
	}

	bool Contains(const Netbuf * const Buffer) {
		return Contains(Buffer->Data(), Buffer->Size());
	}

	bool Contains(const unsigned char * const buf, const unsigned int size) {
		if(!Available(size) || !bufData || !buf || !size) return false;

		unsigned int seqOff = 0;
		for(unsigned int i = 0; i < bufSize; i++) {
			if(bufData[i] == buf[seqOff]) {
				// Start sequence
				seqOff ++;
				for(; seqOff < size; seqOff++) {
					if(bufData[i + seqOff] != buf[seqOff]) break;
				}
				if(seqOff == size) {
					return true;
				} else {
					seqOff = 0;
				}
			}
		}

		return false;
	}
	std::string Dump(unsigned int nBytes = -1) const {
		if(!bufData) return "(empty)";
		if(nBytes > bufSize) nBytes = bufSize;
		std::ostringstream dumpStr;
		dumpStr.setf(std::ios::hex, std::ios::basefield);
		dumpStr.fill('0');
		for(unsigned int i = 0; i < nBytes; i++) {
			if(i > 0) dumpStr << ' ';
			dumpStr << std::setw(2) << static_cast<unsigned int>(bufData[i]);
		}
		dumpStr.flush();
		return dumpStr.str();
	}

	inline bool operator !() const {
		return ((!bufSize && !bufData) || (bufSize && !bufData) || (!bufSize && bufData));
	}

	inline bool operator ==(const Netbuf& rhs) {
			// If they don't have the same size or either one is invalid and bufSize > 0
		if(bufSize != rhs.bufSize || (bufSize && (!(*this) || !rhs))) return false;

			// If there is no bufSize then invalidity doesn't matter, they're the same.
		if(!bufSize) return true;

			// We've got two valid buffers of the same size, so we compare.
		for(unsigned int i = 0; i < bufSize; i++) {
			if(bufData[i] != rhs.bufData[i]) return false;
		}

		return true;
	}

	inline bool operator !=(const Netbuf& rhs) {
			// If they have the same size or either one is invalid and bufSize > 0
		if(bufSize != rhs.bufSize || (bufSize && (!(*this) || !rhs))) return true;

			// bufSize == rhs.bufSize and they're both valid, are they the same/
		if(!bufSize) return true;

			// We've got two valid buffers of the same size, so we compare.
		for(unsigned int i = 0; i < bufSize; i++) {
			if(bufData[i] != rhs.bufData[i]) return true;
		}

		return false;
	}

};

#endif		// NETBUF_H
