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

void BinaryReader::Open(const string& filePath)
{
	m_pStream = make_unique<ifstream>();
	m_pStream->open(filePath, ios::binary);
	if(m_pStream->fail())
	{
		stringstream stream;
		stream << "BinaryReader::Open() -> File " << filePath << " not found!" << endl;
		Console::Log(stream.str(), LogType::ERROR);
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
		return false;
	}
	return true;
}

string BinaryReader::ReadString()
{
	int stringLength = (char)Read<char>();
	stringstream ss;
	for (int i = 0; i < stringLength; ++i)
	{
		ss << Read<char>();
	}
	return ss.str();
}

string BinaryReader::ReadLongString()
{
	if (m_pStream == nullptr)
		Console::Log("BinaryReader doesn't exist!\nUnable to read binary data...", LogType::ERROR);

	auto stringLength = Read<UINT>();

	stringstream ss;
	for (UINT i = 0; i<stringLength; ++i)
	{
		ss << Read<wchar_t>();
	}

	return ss.str();
}

string BinaryReader::ReadNullString()
{
	if (m_pStream == nullptr)
	{
		Console::Log("BinaryReader doesn't exist!\nUnable to read binary data...", LogType::ERROR);
		return "";
	}

	string buff;
	getline(*m_pStream, buff, '\0');

	return buff;
}
