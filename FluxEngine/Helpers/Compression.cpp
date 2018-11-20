#include "FluxEngine.h"
#include "Compression.h"

#include <Zlib.h>
#include "External/LZ4/lz4.h"

namespace Compression
{
	bool DecompressZlib(void *pInData, size_t inDataSize, std::vector<char> &outData, bool zlibHeader)
	{
		const size_t BUFSIZE = 128 * 1024;

		z_stream strm;
		strm.zalloc = nullptr;
		strm.zfree = nullptr;
		strm.next_in = reinterpret_cast<unsigned char*>(pInData);
		strm.avail_in = (uInt)inDataSize;
		strm.next_out = reinterpret_cast<unsigned char*>(outData.data());
		strm.avail_out = BUFSIZE;

		unsigned int currSize = 0;

		if (zlibHeader)
		{
			inflateInit(&strm);
		}
		else
		{
			inflateInit2(&strm, -MAX_WBITS);
		}

		while (strm.avail_in != 0)
		{
			const int res = inflate(&strm, Z_NO_FLUSH);
			if (res != Z_OK && res != Z_STREAM_END)
			{
				return false;
			}
			if (strm.avail_out == 0)
			{
				currSize += BUFSIZE;
				strm.next_out = reinterpret_cast<unsigned char*>(outData.data() + currSize);
				strm.avail_out = BUFSIZE;
			}
		}

		int deflate_res = Z_OK;
		while (deflate_res == Z_OK)
		{
			if (strm.avail_out == 0)
			{
				currSize += BUFSIZE;
				strm.next_out = reinterpret_cast<unsigned char*>(outData.data() + currSize);
				strm.avail_out = BUFSIZE;
			}
			deflate_res = inflate(&strm, Z_FINISH);
		}

		if (deflate_res != Z_STREAM_END)
		{
			return false;
		}

		inflateEnd(&strm);

		return true;
	}

	bool CompressZlib(void *pInData, size_t inDataSize, std::vector<char> &outData)
	{
		const size_t BUFSIZE = 128 * 1024;
		unsigned char temp_buffer[BUFSIZE];

		z_stream strm;
		strm.zalloc = nullptr;
		strm.zfree = nullptr;
		strm.next_in = reinterpret_cast<unsigned char*>(pInData);
		strm.avail_in = (uInt)inDataSize;
		strm.next_out = temp_buffer;
		strm.avail_out = BUFSIZE;

		deflateInit(&strm, Z_BEST_SPEED);

		while (strm.avail_in != 0)
		{
			int res = deflate(&strm, Z_NO_FLUSH);
			if (res != Z_OK)
			{
				return false;
			}

			if (strm.avail_out == 0)
			{
				outData.insert(outData.end(), temp_buffer, temp_buffer + BUFSIZE);
				strm.next_out = temp_buffer;
				strm.avail_out = BUFSIZE;
			}
		}

		int deflate_res = Z_OK;
		while (deflate_res == Z_OK)
		{
			if (strm.avail_out == 0)
			{
				outData.insert(outData.end(), temp_buffer, temp_buffer + BUFSIZE);
				strm.next_out = temp_buffer;
				strm.avail_out = BUFSIZE;
			}
			deflate_res = deflate(&strm, Z_FINISH);
		}

		if (deflate_res != Z_STREAM_END)
		{
			return false;
		}

		outData.insert(outData.end(), temp_buffer, temp_buffer + BUFSIZE - strm.avail_out);
		deflateEnd(&strm);

		return true;
	}

	bool DecompressLZ4(const void *pInData, size_t compressedSize, size_t uncompressedSize, std::vector<char> &outData)
	{
		outData.resize(uncompressedSize);
		const int decompressedSize = LZ4_decompress_safe((const char*)pInData, outData.data(), (int)compressedSize, (int)uncompressedSize);
		return decompressedSize > 0;
	}

	bool CompressLZ4(const void *pInData, size_t inDataSize, std::vector<char> &outData)
	{
		const int maxDstSize = LZ4_compressBound((int)inDataSize);
		outData.resize((size_t)maxDstSize);

		const int compressDataSize = LZ4_compress_default((const char*)pInData, outData.data(), (int)inDataSize, (int)maxDstSize);
		if (compressDataSize < 0)
		{
			return false;
		}

		outData.resize((size_t)compressDataSize);
		return true;
	}
}