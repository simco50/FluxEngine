#include "stdafx.h"
#include "SpriteFontLoader.h"
#include "../Helpers/BinaryReader.h"


SpriteFontLoader::SpriteFontLoader()
{}


SpriteFontLoader::~SpriteFontLoader()
{}

#pragma warning( disable : 4702 )
SpriteFont* SpriteFontLoader::LoadContent(const wstring& assetFile)
{
	auto pBinReader = new BinaryReader(); //Prevent memory leaks
	pBinReader->Open(assetFile);

	if (!pBinReader->IsOpen())
	{
		delete pBinReader;
		DebugLog::LogFormat(LogType::ERROR, L"SpriteFontLoader::LoadContent > Failed to read the assetFile!\nPath: \'%s\'", assetFile.c_str());
		
		return nullptr;
	}

	//See BMFont Documentation for Binary Layout
	char b = pBinReader->Read<char>();
	char m = pBinReader->Read<char>();
	char f = pBinReader->Read<char>();
	if(!(b == 66 && m == 77 && f == 70))
	{	
		DebugLog::Log(L"SpriteFontLoader::LoadContent > Not a valid .fnt font", LogType::ERROR);	
		return nullptr;
	}
	//Parse the Identification bytes (B,M,F)
	//If Identification bytes doesn't match B|M|F,
	//Log Error (SpriteFontLoader::LoadContent > Not a valid .fnt font) &
	//return nullptr

	//Parse the version (version 3 required)
	//If version is < 3,
	char version = pBinReader->Read<char>();
	if(version != 3)
	{
		//Log Error (SpriteFontLoader::LoadContent > Only .fnt version 3 is supported)
		DebugLog::Log(L"SpriteFontLoader::LoadContent > Not a valid .fnt font", LogType::ERROR);
		//return nullptr
		return nullptr;
	}
	
	//Valid .fnt file
	auto pSpriteFont = new SpriteFont();
	//SpriteFontLoader is a friend class of SpriteFont
	//That means you have access to its privates (pSpriteFont->m_FontName = ... is valid)

	//**********
	// BLOCK 0 *
	//**********
	//Retrieve the blockId and blockSize
	char id = pBinReader->Read<char>();
	int charInfoSize = pBinReader->Read<int>();
	//Retrieve the FontSize (will be -25, BMF bug) [SpriteFont::m_FontSize]
	pSpriteFont->m_FontSize = pBinReader->Read<short>();
	//Move the binreader to the start of the FontName [BinaryReader::MoveBufferPosition(...) or you can set its position using BinaryReader::SetBufferPosition(...))
	pBinReader->MovePosition(12);
	//Retrieve the FontName [SpriteFont::m_FontName]
	pSpriteFont->m_FontName = pBinReader->ReadNullString();

	//**********
	// BLOCK 1 *
	//**********
	//Retrieve the blockId and blockSize
	id = pBinReader->Read<char>();
	int blockSize = pBinReader->Read<int>();
	//Retrieve Texture Width & Height [SpriteFont::m_TextureWidth/m_TextureHeight]
	pBinReader->MovePosition(4);
	pSpriteFont->m_TextureWidth = pBinReader->Read<unsigned short>();
	pSpriteFont->m_TextureHeight = pBinReader->Read<unsigned short>();
	//Retrieve PageCount
	short pages = pBinReader->Read<unsigned char>();
	//> if pagecount > 1
	if(pages > 1)
		DebugLog::Log(L"SpriteFontLoader::LoadContent > SpriteFont (.fnt): Only one texture per font allowed", LogType::ERROR);
	//> Log Error (SpriteFontLoader::LoadContent > SpriteFont (.fnt): Only one texture per font allowed)
	//Advance to Block2 (Move Reader)
	pBinReader->MovePosition(6);

	//**********
	// BLOCK 2 *
	//**********
	//Retrieve the blockId and blockSize
	id = pBinReader->Read<char>();
	blockSize = pBinReader->Read<int>();
	//Retrieve the PageName (store Local)
	wstring pageName = pBinReader->ReadNullString();
	//	> If PageName is empty
	if(pageName.empty())
		DebugLog::Log(L"SpriteFontLoader::LoadContent > SpriteFont (.fnt): Invalid Font Sprite [Empty]", LogType::ERROR);
	//	> Log Error (SpriteFontLoader::LoadContent > SpriteFont (.fnt): Invalid Font Sprite [Empty])
	//>Retrieve texture filepath from the assetFile path
	int endIdx = assetFile.rfind('/');
	wstring path = assetFile.substr(0, endIdx + 1);
	//> (ex. c:/Example/somefont.fnt => c:/Example/) [Have a look at: wstring::rfind()]
	//>Use path and PageName to load the texture using the ContentManager [SpriteFont::m_pTexture]
	//> (ex. c:/Example/ + 'PageName' => c:/Example/somefont_0.png)
	pSpriteFont->m_pTexture = ResourceManager::Load<Texture>(path + pageName);
	
	//**********
	// BLOCK 3 *
	//**********
	//Retrieve the blockId and blockSize
	id = pBinReader->Read<char>();
	blockSize = pBinReader->Read<int>();
	//Retrieve Character Count (see documentation)
	int charCount = blockSize / charInfoSize;
	//Retrieve Every Character, For every Character:
	for(int i = 0; i < charCount; ++i)
	{
		//> Check if CharacterId is valid (SpriteFont::IsCharValid), Log Warning and advance to next character if not valid
		wchar_t charId = static_cast<wchar_t>(pBinReader->Read<unsigned int>());
		if(!pSpriteFont->IsCharValid(charId))
		{
			//Logger::LogFormat(LogLevel::Warning, L"Character '%c' not valid", charId);
			pBinReader->MovePosition(16);
			continue;
		}
		//> Retrieve the corresponding FontMetric (SpriteFont::GetMetric) [REFERENCE!!!]
		FontMetric &metric = pSpriteFont->GetMetric(charId);
		//> Set IsValid to true [FontMetric::IsValid]
		metric.IsValid = true;
		//> Set Character (CharacterId) [FontMetric::Character]
		metric.Character = charId;
		//> Retrieve Xposition (store Local)
		short xPos = pBinReader->Read<unsigned short>();
		//> Retrieve Yposition (store Local)
		short yPos = pBinReader->Read<unsigned short>();
		//> Retrieve & Set Width [FontMetric::Width]
		metric.Width = pBinReader->Read<unsigned short>();
		//> Retrieve & Set Height [FontMetric::Height]
		metric.Height = pBinReader->Read<unsigned short>();
		//> Retrieve & Set OffsetX [FontMetric::OffsetX]
		metric.OffsetX = pBinReader->Read<short>();
		//> Retrieve & Set OffsetY [FontMetric::OffsetY]
		metric.OffsetY = pBinReader->Read<short>();
		//> Retrieve & Set AdvanceX [FontMetric::AdvanceX]
		metric.AdvanceX = pBinReader->Read<short>();
		//> Retrieve & Set Page [FontMetric::Page]
		metric.Page = pBinReader->Read<unsigned char>();
		//> Retrieve Channel (BITFIELD!!!) 
		//	> See documentation for BitField meaning [FontMetrix::Channel]
		unsigned char chann = pBinReader->Read<unsigned char>();
		switch(chann)
		{
		case 1:
			pSpriteFont->GetMetric(charId).Channel = 2;
			break;
		case 2:
			pSpriteFont->GetMetric(charId).Channel = 1;
			break;
		case 4:
			pSpriteFont->GetMetric(charId).Channel = 0;
			break;
		case 8:
			pSpriteFont->GetMetric(charId).Channel = 3;
			break;
		default:
			pSpriteFont->GetMetric(charId).Channel = 4;
			break;
		}
		//> Calculate Texture Coordinates using Xposition, Yposition, TextureWidth & TextureHeight [FontMetric::TexCoord]
		metric.TexCoord.x = (float)xPos / (float)pSpriteFont->m_TextureWidth;
		metric.TexCoord.y = (float)yPos / (float)pSpriteFont->m_TextureHeight;
	}

	delete pBinReader;
	return pSpriteFont;
	
	#pragma warning(default:4702)  
}

void SpriteFontLoader::Destroy(SpriteFont* objToDestroy)
{
	SafeDelete(objToDestroy);
}