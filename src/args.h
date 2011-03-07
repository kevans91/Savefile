#ifndef ARGS_H
#define ARGS_H

#include <string>
#include <vector>

class Args {
protected:
private:
	std::vector<std::string>		argList;
	std::string				appPath;
public:
	Args(int argc, char **argv) {
		if(argc) {
			appPath = argv[0]; 
			for(unsigned int i = 1; i < argc; i++) {
				argList.push_back(argv[i]);
			}
		}
	}

	unsigned int Count() const { return argList.size(); }
	std::string Input() const { 
		for(unsigned int i = 0; i < argList.size(); ++i) {
			if(argList[i][0] == '-') {
				++i;
				if((i + 1) >= argList.size()) {
					 // There's nothing left.
					return argList[i];
				}
			} else {
				return argList[i];
			}
		}

		return "";
	}

	bool Exists(const std::string& key) const {
		for(unsigned int i = 0; i < argList.size(); ++i) {
			if(argList[i] == key) return true;
		}
		return false;
	}

	std::string Value(const std::string& key) const {
		for(unsigned int i = 0; i < argList.size(); ++i ) {
			if(argList[i] == key) {
				if((i + 1) > argList.size()) return "";
				return argList[i + 1];
			}
		}
		return "";
	}

	inline bool operator !() const { return (!argList.size()); }
};

#endif	// ARGS_H
