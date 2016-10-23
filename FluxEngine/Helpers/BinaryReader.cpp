#include "stdafx.h"
#include "BinaryReader.h"


BinaryReader::BinaryReader()
{
}

BinaryReader::~BinaryReader()
{
	Close();
	m_pStream.reset();
}

void BinaryReader::Open(const wstring& filePath)
{
	m_pStream = make_unique<ifstream>();
	m_pStream->open(filePath, ios::binary);
	if(m_pStream->fail())
	{
		wstringstream stream;
		stream << L"BinaryReader::Open() -> File " << filePath << L" not found!" << endl;
		DebugLog::Log(stream.str(), LogType::ERROR);
		m_pStream.reset();
		return;
	}
}

void BinaryReader::Close()
{
	IsOpen();
	m_pStream->close();
}

unsigned int BinaryReader::GetPosition() const
{
	IsOpen();
	return static_cast<unsigned int>(m_pStream->tellg());
}

void BinaryReader::SetPosition(const unsigned position)
{
	IsOpen();
	m_pStream->seekg(position);
}

void BinaryReader::MovePosition(const unsigned places)
{
	IsOpen();
	m_pStream->seekg(GetPosition() + places);
}

bool BinaryReader::IsOpen() const
{
	if(!(m_pStream != nullptr && m_pStream->is_open()))
	{
		DebugLog::Log(L"BinaryReader::IsOpen() --> Failed!");
		return false;
	}
	return true;
}

string BinaryReader::ReadString()
{
	unsigned int stringLength = (unsigned int)Read<unsigned int>();
	stringstream ss;
	for (size_t i = 0; i < stringLength; ++i)
	{
		ss << Read<char>();
	}
	return ss.str();
}
