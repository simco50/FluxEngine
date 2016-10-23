#include "stdafx.h"
#include "BaseLoader.h"

BaseLoader::BaseLoader()
{
}


BaseLoader::~BaseLoader()
{
	for(auto &p : m_VertexData)
	{
		delete[] p.second.pData;
		p.second.pData = nullptr;
	}
}

#define WRITE(stream, vector, id, datatype)\
if(vector.size() > 0) \
{\
	Write<Block_ID>(file, id);\
	unsigned int length = vector.size() * sizeof(datatype);\
	Write<unsigned int>(file, length);\
	for (const datatype& data : vector)\
		Write<datatype>(file, data);\
}

void BaseLoader::Write(const string& filePath)
{
	cout << "Saving to file " << filePath << endl;

	stringstream outputPath(filePath);
	if (filePath.find(".flux") == string::npos)
		outputPath << ".flux" << endl;

	ofstream file(outputPath.str(), ios::out | ios::binary);
	if (file.fail())
		throw(string("Output stream creation failed!"));

	//Write the vector data to a file
	Write<unsigned char>(file, SE_VERSION);

	for(auto data : m_VertexData)
	{
		string name = data.first;
		Write<unsigned int>(file, name.length());
		file << name;
		Write<unsigned int>(file, data.second.Count);
		Write<unsigned int>(file, data.second.Stride);
		Write(file, data.second.pData, data.second.Count * data.second.Stride);
	}

	string name = "END";
	Write<unsigned int>(file, name.length());
	file << name;

	file.close();

	cout << "File saved." << endl;
}