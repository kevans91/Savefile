#ifndef ARGS_H
#define ARGS_H

#include <map>
#include <string>
#include <vector>

class Args {
protected:
private:
	std::vector<std::string>		appValidSwitches;
	std::vector<std::string>		appSwitches;
	std::vector<std::string>		appInputs;
	std::map<std::string, std::string>	appVars;
	std::string				appPath;

	bool IsVar(const std::string& varKey) {
		return (appVars.find(varKey) != appVars.end());
	}

	bool IsSwitch(const std::string& switchStr) {
		for(std::vector<std::string>::const_iterator iter = appValidSwitches.begin(); iter != appValidSwitches.end(); ++iter) {
			if((*iter) == switchStr) return true;
		}

		return false;
	}
public:
#ifdef TESTVER
	Args(unsigned int argc, const char **argv) {
		Parse(argc, const_cast<char**>(argv));
	}
#endif

	Args(unsigned int argc, char **argv) {
		Parse(argc, argv);
	}

	Args() {
	}

#ifdef TESTVER
	void Parse(unsigned int argc, const char **argv) {
		Parse(argc, const_cast<char**>(argv));
	}
#endif
	void Reset() {
		appPath.clear();
		appSwitches.clear();
		appVars.clear();
		appInputs.clear();
	}

	void Parse(unsigned int argc, char **argv) {
		Reset();
		if(argc) {
			appPath = argv[0];
			for(unsigned int i = 1; i < argc; i++) {
				if(argv[i][0] == '-') {
					if(IsSwitch(argv[i])) {
						appSwitches.push_back(argv[i]);
					} else if((i + 1) < argc && argv[i + 1][0] != '-') {
						appVars[argv[i]] = argv[i + 1];
						++ i;
					}
				} else {
					appInputs.push_back(argv[i]);
				}
			}
		}
	}

	bool AddSwitch(const std::string& switchStr) {
		if(switchStr[0] != '-') return false;
		appValidSwitches.push_back(switchStr);
		return true;
	}

	bool IsSet(const std::string& switchStr) {
		for(std::vector<std::string>::iterator iter = appSwitches.begin(); iter != appSwitches.end(); ++iter) {
			if((*iter) == switchStr) return true;
		}

		return false;
	}

	std::string Value(const std::string& var) {
		std::map<std::string, std::string>::iterator iter = appVars.find(var);
		if(iter == appVars.end()) return "";
		else return iter->second;
	}

	unsigned int Count() const { return appInputs.size(); }
	inline std::string operator[](unsigned int index) {
		if((index + 1) > appInputs.size()) return "";
		return appInputs[index];
	}

	inline bool operator !() const { return (!appInputs.size() && !appVars.size() && !appSwitches.size()); }
};

#endif	// ARGS_H

