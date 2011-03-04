#ifndef ARGS_H
#define ARGS_H

#include <map>
#include <string>

class Args {
protected:
private:
	std::map<std::string, std::string>	argList;
	std::string				argInput;
	std::string				appPath;
public:
	Args(int argc, char **argv, bool inputReq = true) {
		if(argc) {
			appPath = argv[0]; 
			for(unsigned int i = 1; i < argc; i++) {
				if(argv[i][0] != '-') {
					argInput = argv[i];
					continue;
				} else if(argc > (i + 1) && (argv[i + 1][0] != '-' && (!inputReq || (inputReq && (i + 2 < argc))))) {
					// -k value
					argList[argv[i]] = argv[i + 1];
					i ++;
				} else {		// It's a switch.
					argList[argv[i]] = "";
				}
			}
		}
	}

	unsigned int Count() const { return argList.size(); }
	std::string Input() const { return argInput; }

	bool Exists(const std::string& key) const { return (argList.find(key) != argList.end()); }

	std::string Value(const std::string& key) const {
		std::map<std::string, std::string>::const_iterator iter = argList.find(key);
		if(iter != argList.end()) return iter->second;
		else return "";
	}

	inline bool operator !() const { return (!argList.size() && argInput.empty()); }
};

#endif	// ARGS_H
