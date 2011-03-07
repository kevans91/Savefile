#ifndef SAVEFILE_H
#define SAVEFILE_H

#include <fstream>
#include <string>
#include <sys/stat.h>
#include <vector>

#include "entry.h"
#include "netbuf.h"

struct EntryComp {
        bool operator()(Entry *lhs, Entry *rhs) {
                return (lhs->Index() < rhs->Index());
        }
};

struct Node {
	unsigned int		id;
	Node			*parent;
	std::vector<Node *>	children;

	std::string		name;
	EntryData		*data;

	Node(unsigned int nodeId, const std::string& nodeName, EntryData * nodeData) : id(nodeId), parent(NULL), name(nodeName), data(nodeData) {
	}

	~Node() {
		delete data;
		data = NULL;

		for(std::vector<Node *>::iterator iter = children.begin(); iter != children.end(); ) {
			Node * node = (*iter);
			iter = children.erase(iter);
			delete node;
			node = NULL;
		}
	}

	bool AddChild(Node *node) {
		if(node != NULL) {
			children.push_back(node);
			node->parent = this;
			return true;
		}

		return false;
	}

	Node * Child(std::string name) {
		size_t spos = name.find('/');

		while(spos != std::string::npos) {
			if(!spos) name.erase(spos, 1);
			else name.erase(spos);
			spos = name.find('/');
		}

		for(std::vector<Node *>::iterator iter = children.begin(); iter != children.end(); iter++) {
			if((*iter)->name == name) {
				return (*iter);
			}
		}
		return NULL;
	}

	std::string Dump(signed char level = 0, bool dumpSelf = true) {
		std::ostringstream retString;
		std::string tabString = "";
		for(unsigned char i = 0; i < level; i++) {
			tabString += '\t';
		}

		if(dumpSelf) {
			if(!name.empty()) {
				retString << tabString << name;
				if(data != NULL) {
					retString << " = " << data->ToString();
				}
			}
		} else {
			level --;
		}
		retString << '\n';
		if(children.size()) {
			for(std::vector<Node *>::iterator iter = children.begin(); iter != children.end(); ++iter) {
				retString << (*iter)->Dump(level + 1);
			}
		} else if(!dumpSelf && data != NULL) {
			retString << tabString << ". = " << data->ToString();
		}

		return retString.str();
	}

	Node *Find(unsigned int index) {
		if(index == id) return this;
		if(children.empty()) return NULL;
		Node * returnNode = NULL;
		for(std::vector<Node *>::iterator iter = children.begin(); iter != children.end(); ++iter) {
			returnNode = (*iter)->Find(index);
			if(returnNode != NULL) break;
		}

		return returnNode;
	}
};

class Savefile {
protected:
private:
	std::string	fileName;
	Netbuf		*fileBuf;
	Node		*tree;
public:
	Savefile(const std::string& name) : fileName(name), fileBuf(NULL), tree(NULL) {
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

	Node * cd(std::string path) {
		Node * curNode = tree, *newNode = NULL;
		if(path[0] == '/') path.erase(0,1);
		while(!path.empty()) {
			size_t slashPos = path.find('/');
			std::string dir = path.substr(0, slashPos);
			newNode = curNode->Child(dir);
			if(!newNode) break;
			curNode = newNode;

			if(slashPos != std::string::npos) path.erase(0, slashPos + 1);
			else break;
		}

		return curNode;
	}

	std::string ExportText(const std::string& path) {
		Node * curNode = cd(path);
		return curNode->Dump(0, false);
	}

	bool Read();

	inline Node * operator [](const std::string& path) { return cd(path); }
	inline bool operator !() const { return (!fileBuf || !(*fileBuf)); }
};

#endif		// SAVEFILE_H
