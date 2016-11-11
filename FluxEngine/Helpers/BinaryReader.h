#pragma once
class BinaryReader
{
public:
	BinaryReader();
	~BinaryReader();

	void Open(const wstring& filePath);
	void Close();

	unsigned int GetPosition() const;
	void SetPosition(const unsigned int position);
	void MovePosition(const unsigned int places);
	bool IsOpen() const;

	template<typename T>
	T Read()
	{
		T value;
		m_pStream->read((char*)(&value), sizeof(T));
		return value;
	}

	void Read(void* pTarget, size_t size)
	{
		m_pStream->read((char*)pTarget, size);
	}

	string ReadString();
	wstring ReadLongString();
	wstring ReadNullString();

private:
	unique_ptr<ifstream> m_pStream;
};