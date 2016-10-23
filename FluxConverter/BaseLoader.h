#pragma once
const unsigned char SE_VERSION = 4;

class BaseLoader
{
public:
	BaseLoader();
	virtual ~BaseLoader();

	virtual void Load(const string& filePath) = 0;
	void Write(const string& filePath);

	template <class T>
	void Write(ofstream& file, T data)
	{
		file.write(reinterpret_cast<char*>(&data), sizeof(T));
	}
	void Write(ofstream& file, void* pData, int size)
	{
		file.write(reinterpret_cast<char*>(pData), size);
	}

protected:
	struct VertexData
	{
		int Count;
		int Stride;
		void* pData;
	};
	map<string, VertexData> m_VertexData;
};

