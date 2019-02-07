#pragma once

inline std::string D3DBlobToString(ID3DBlob* pBlob)
{
	std::string data;
	data.resize(pBlob->GetBufferSize());
	memcpy(&data[0], pBlob->GetBufferPointer(), pBlob->GetBufferSize());
	return data;
}

inline void D3DBlobToVector(ID3DBlob* pBlob, std::vector<char>& buffer)
{
	buffer.resize(pBlob->GetBufferSize());
	memcpy(buffer.data(), pBlob->GetBufferPointer(), buffer.size());
}