#pragma once

namespace Compression
{
	bool Decompress(void *pInData, size_t inDataSize, std::vector<char> &outData);
	bool Compress(void *pInData, size_t inDataSize, std::vector<char> &outData);
};