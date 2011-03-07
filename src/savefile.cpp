#include <algorithm>
#include <limits>
#include "savefile.h"

bool Savefile::Read() {
	Entry * entry = NULL;
	std::vector<Entry *> entries;
	EntryComp entryComp;
	entry = new Entry;
	entry->Read(fileBuf);
	while(fileBuf->More()) {
		entry = new Entry;
		entry->Read(fileBuf);
		if(entry->Active()) entries.push_back(entry);
		else delete entry;
	}

	std::sort(entries.begin(), entries.end(), entryComp);

	for(std::vector<Entry *>::iterator iter = entries.begin(); iter != entries.end(); ) {
		Entry * entry = (*iter);
		if(!tree && entry->ParentIndex() == std::numeric_limits<unsigned int>::max()) {
			tree = new Node(entry->Index(), "", NULL);
			entries.erase(iter);
			delete entry;
			entry = NULL;
		} else {
			if(!tree) {
				++iter;
				continue;
			}
			Node *parent = tree->Find(entry->ParentIndex());
			if(!parent) {
				if(entries.size() == 1) {
					delete tree;
					tree = NULL;
					iter = entries.erase(iter);
					break;
				}
				++iter;
				continue;
			}
			Node *addNode = new Node(entry->Index(), entry->Name(), entry->Data());
			parent->AddChild(addNode);
			iter = entries.erase(iter);
			delete entry;
			entry = NULL;
		}
	}
	if(!tree) return false;

	return true;
}
