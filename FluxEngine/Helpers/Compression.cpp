#include "FluxEngine.h"
#include "Compression.h"

#include <Zlib.h>

namespace Compression
{
	bool Decompress(void *pInData, size_t inDataSize, std::vector<char> &outData)
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

		inflateInit(&strm);

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

	bool Compress(void *pInData, size_t inDataSize, std::vector<char> &outData)
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
}