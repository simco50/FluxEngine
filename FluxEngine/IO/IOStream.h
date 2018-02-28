#pragma once
#include "InputStream.h"
#include "OutputStream.h"

class IOStream : public InputStream, public OutputStream
{
public:
	IOStream()
	{}

	IOStream(const size_t size) :
		InputStream(size)
	{}

	virtual ~IOStream()
	{}
};