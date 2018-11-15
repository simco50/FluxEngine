#pragma once

namespace Compression
{
	bool Decompress(void *pInData, size_t inDataSize, std::vector<char> &outData, bool zlibHeader = true);
	bool Compress(void *pInData, size_t inDataSize, std::vector<char> &outData);
};