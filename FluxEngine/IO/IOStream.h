#pragma once
#include "InputStream.h"
#include "OutputStream.h"

//Base for classes that have both input and output
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