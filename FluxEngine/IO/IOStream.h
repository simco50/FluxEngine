#pragma once
#include "InputStream.h"
#include "OutputStream.h"

//Base for classes that have both input and output
class IOStream : public InputStream, public OutputStream
{
public:
	IOStream() = default;
	virtual ~IOStream() = default;

	explicit IOStream(const size_t size)
		: InputStream(size)
	{}
};