#ifndef ENTRY_H
#define ENTRY_H

#include <map>
#include <sstream>

#include "datatypes.h"
#include "readable.h"

struct ListInfo;
struct FileInfo;
struct EntryData;
struct Entry;

typedef std::map<EntryData *, EntryData *> dmList;

struct ListInfo {
	unsigned int	length;
	unsigned char	type;

	dmList		entries;

	ListInfo() : length(0), type(0) {
	}
};

struct FileInfo {
	unsigned int size;
	unsigned int crc32;
	unsigned char type;
	std::string name;
	unsigned char * data;
	FileInfo() : size(0), crc32(0), type(0), data(NULL) {

	};

	~FileInfo() {
		if(data != NULL) {
			delete [] data;
			data = NULL;
		}
	}
};

struct EntryData : public Readable {
	EntryData(ExtraInfo * const listExInfo)
		: dataType(0), dataSize(0), isListVal(true), floatValue(0),
			fileValue(NULL), listValue(NULL), extraInfo(listExInfo) {
	}

	EntryData()
		: dataType(0), dataSize(0), isListVal(false), floatValue(0),
			fileValue(NULL), listValue(NULL), extraInfo(NULL) {
	}

	~EntryData() {
		if(!isListVal) {
			delete extraInfo;
			extraInfo = NULL;
		}
	}
	std::string ToString() const {
		std::ostringstream out;
		switch(dataType) {
			case DT_List: {
				if(!listValue) {
					out << "ERROR";
				} else {
					out << "list(";
					for(dmList::const_iterator iter = listValue->entries.begin(); iter != listValue->entries.end(); ++iter) {
						if(iter != listValue->entries.begin()) out << ",";
						out << iter->first->ToString();
						if(iter->second != NULL) {
							out << " = " << iter->second->ToString();
						}
					}
					out << ")";
				}
				break;
			}
			case DT_File: {
				if(!fileValue || !fileValue->data) {
					out << "ERROR";
				} else {
					StringEncoder strEncoder;
					char ch = out.fill('0');
					unsigned int width = out.width(8);
					out << "filedata(\"name=" << strEncoder.UrlEncode(fileValue->name) << ";length=" << fileValue->size << ";crc32=0x";
					out << std::hex << fileValue->crc32 << std::dec << ";encoding=base64\",{\"" << std::endl;
					out << strEncoder.Base64Encode(fileValue->data, fileValue->size) << std::endl;
					out << "\"})";
					out.width(width);
					out.fill(ch);
				}
				break;
			}
			case DT_Float: {
				out << floatValue;
				break;
			}
			case DT_Object: {
				out << "object(\"" << strValue << "\")";
				break;
			}
			case DT_Path: {
				out << strValue;
				break;
			}
			case DT_String: {
				out << '"' << strValue << '"';
				break;
			}
			case DT_Null: {
				out << "NULL";
				break;
			}
		}
		return out.str();
	}

	bool Read(Netbuf * const fileBuf) {
		if(isListVal) {
			NumberEncoder<unsigned char> charEncoder;
			dataType = charEncoder(fileBuf->ReadInt<unsigned char>(), *extraInfo, isListVal);
		} else {
			dataSize = fileBuf->ReadInt<unsigned int>();
			dataType = fileBuf->ReadInt<unsigned char>();

			if(!dataSize) return false;
			dataType ^= (0x40 - 6);
			extraInfo = new ExtraInfo(fileBuf->Pos(), fileBuf->Pos(), fileBuf->Pos());
		}

		switch(dataType) {
			// List
			case DT_List: {
				NumberEncoder<unsigned int> intEncoder;
				NumberEncoder<unsigned char> charEncoder;
				if(listValue != NULL) delete listValue;
				listValue = new ListInfo;
				listValue->length = intEncoder(fileBuf->ReadInt<unsigned int>(), *extraInfo, isListVal);
				if(!isListVal) extraInfo->entryOffset = extraInfo->dataOffset;
				listValue->type = charEncoder(fileBuf->ReadInt<unsigned char>(), *extraInfo, true);
					// First entry is non-associative by default.
				bool assocMode = false, lastSeq = false;
				EntryData *key = NULL, *value = NULL;
				for(unsigned int i = 0; i < listValue->length; i++) {
					key = new EntryData(extraInfo);
					key->Read(fileBuf);
					if(listValue->type == LT_Associative && (i + 1) == listValue->length) {
						if(!lastSeq) {
							listValue->length += intEncoder(fileBuf->ReadInt<unsigned int>(), *extraInfo, true) - 1;
							lastSeq = (charEncoder(fileBuf->ReadInt<unsigned char>(), *extraInfo, true) == 0x00);
							assocMode =! assocMode;
						}
					}

					if(assocMode) {
						value = new EntryData(extraInfo);
						value->Read(fileBuf);
						listValue->entries[key] = value;
					} else {
						listValue->entries[key] = NULL;
					}
				}
				break;
			}
			case DT_Overlay: {
			}
			case DT_File: {
				NumberEncoder<unsigned int> intEncoder;
				if(fileValue != NULL) delete fileValue;
				fileValue = new FileInfo;
				fileValue->size = intEncoder(fileBuf->ReadInt<unsigned int>(), *extraInfo, isListVal);
				fileValue->crc32 = intEncoder(fileBuf->ReadInt<unsigned int>(), *extraInfo, isListVal);
				fileValue->type = fileBuf->ReadInt<unsigned char>();
				extraInfo->dataOffset ++;
				StringEncoder strEncoder;
				fileValue->name = strEncoder.Decode(fileBuf, *extraInfo, isListVal);
				fileValue->data = fileBuf->ReadStr(fileValue->size);
				strEncoder.Decode(fileValue->data, fileValue->size, *extraInfo, isListVal);
				break;
			}
			case DT_Float: {
				NumberEncoder<float> fEncoder;
				floatValue = fEncoder(fileBuf->ReadInt<float>(), *extraInfo, isListVal);
				break;
			}
			case DT_Object: {
			}
			case DT_Path: {
			}
			case DT_String: {
				StringEncoder strEncoder;
				strValue = strEncoder.Decode(fileBuf, *extraInfo, isListVal);
				break;
			}
			case DT_Null: {
				break;
			}
			default: {
				printf("Unhandled type %.02x\n", dataType);
				break;
			}
		}
		return true;
	}

	bool Write(Netbuf * const fileBuf) {
		return true;
	}

	unsigned int Size() const { return dataSize; }
	unsigned char Type() const { return dataType; }
private:
	unsigned char dataType;
	unsigned int dataSize;
	const bool isListVal;

	float floatValue;
	std::string strValue;
	FileInfo *fileValue;
	ListInfo *listValue;

	ExtraInfo *extraInfo;
};

struct Entry : public Readable {
	bool Active() const { return active; }
	unsigned int ParentIndex() const { return parent; }
	unsigned int Index() const { return index; }
	std::string Name() const { return std::string(reinterpret_cast<char*>(name), nameSize); }

	EntryData *Data() const { return data; }

	Entry() : size(0), active(0), index(0), parent(0),
			nameSize(0), name(NULL), data(NULL) {
	}

	~Entry() {
		if(name) {
			delete [] name;
			name = NULL;
		}
	}

	bool Read(Netbuf * const dataBuf) {
		size = dataBuf->ReadInt<unsigned int>();
		active = dataBuf->ReadInt<unsigned char>();
		unsigned int dataStart = dataBuf->Pos();
		if(!dataBuf->Available(size)) return false;
		if(!active) {
			dataBuf->Skip(size);
			return true;
		}
		index = dataBuf->ReadInt<unsigned int>();
		parent = dataBuf->ReadInt<unsigned int>();
		nameSize = dataBuf->ReadInt<unsigned char>();
		if(nameSize != 0) {
			unsigned int nameStart = dataBuf->Pos() - dataStart;
			name = dataBuf->ReadStr(nameSize);
			for(unsigned char i = 0; i < nameSize; i++) {
				name[i] ^= (2 + (9 * (nameStart + i)));
			}
		}
		data = new EntryData;
		data->Read(dataBuf);

		if(!data->Size()) {
			delete data;
			data = NULL;
		}
		dataBuf->Seek(dataStart + size);
	}

	bool Write(Netbuf * const dataBuf) {
	}

	unsigned int Size() const { return size; }
private:
	unsigned int size;
	unsigned char active;
	unsigned int index;
	unsigned int parent;
	unsigned char nameSize;
	unsigned char * name;

	EntryData * data;

};

#endif		// ENTRY_H
