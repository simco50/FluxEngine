#pragma once

inline std::string D3DBlobToString(ID3DBlob* pBlob)
{
	return std::string((char*)pBlob->GetBufferPointer(), (char*)pBlob->GetBufferPointer() + pBlob->GetBufferSize());
}

void D3DBlobToVector(ID3DBlob* pBlob, vector<char>& buffer)
{
	buffer.resize(pBlob->GetBufferSize());
	memcpy(buffer.data(), pBlob->GetBufferPointer(), buffer.size());
}