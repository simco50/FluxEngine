#pragma once

namespace Compression
{
	bool DecompressZlib(void *pInData, size_t inDataSize, std::vector<char> &outData, bool zlibHeader = true);
	bool CompressZlib(void *pInData, size_t inDataSize, std::vector<char> &outData);

	bool DecompressLZ4(const void *pInData, size_t compressedSize, size_t uncompressedSize, std::vector<char> &outData);
	bool CompressLZ4(const void *pInData, size_t inDataSize, const bool hc, std::vector<char> &outData);
};