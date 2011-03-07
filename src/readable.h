#ifndef READABLE_H
#define READABLE_H

#include "netbuf.h"

struct Readable {
	virtual bool Read(Netbuf * const data) = 0;
	virtual bool Write(Netbuf * const data) = 0;

	virtual unsigned int Size() const = 0;
};

#endif		// READABLE_H
