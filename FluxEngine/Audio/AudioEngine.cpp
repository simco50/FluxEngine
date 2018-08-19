#include "FluxEngine.h"
#include "AudioEngine.h"

namespace FmodCallbacks
{
	FMOD_RESULT F_CALLBACK FileOpenCallback(const char* name, unsigned int* fileSize, void** handle, void* userData)
	{
		UNREFERENCED_PARAMETER(fileSize);
		UNREFERENCED_PARAMETER(userData);
		std::unique_ptr<File> pFile = FileSystem::GetFile(name);
		if (pFile == nullptr)
			return FMOD_ERR_FILE_NOTFOUND;
		if (!pFile->OpenWrite())
			return FMOD_ERR_FILE_BAD;
		*fileSize = (unsigned int)pFile->GetSize();
		File* pHandle = pFile.release();
		*handle = (void*)pHandle;
		return FMOD_OK;
	}

	FMOD_RESULT F_CALLBACK FileCloseCallback(void* handle, void* userData)
	{
		UNREFERENCED_PARAMETER(userData);
		File* pFile = (File*)handle;
		if (pFile)
		{
			pFile->Close();
			delete pFile;
		}
		return FMOD_OK;
	}

	FMOD_RESULT F_CALLBACK FileReadCallback(void* handle, void* buffer, unsigned int sizeBytes, unsigned int *bytesRead, void* userData)
	{
		UNREFERENCED_PARAMETER(userData);
		UNREFERENCED_PARAMETER(bytesRead);
		File* pFile = (File*)handle;
		if (pFile == nullptr)
			return FMOD_ERR_FILE_BAD;
		*bytesRead = (unsigned int)pFile->Read(buffer, sizeBytes);
		if (*bytesRead < sizeBytes)
			return FMOD_ERR_FILE_EOF;
		return FMOD_OK;
	}

	FMOD_RESULT F_CALLBACK FileSeekCallback(void* handle, unsigned int pos, void* userData)
	{
		UNREFERENCED_PARAMETER(userData);
		if (handle == nullptr)
			return FMOD_ERR_FILE_BAD;
		File* pFile = (File*)handle;
		if (!pFile->SetPointer(pos))
			return FMOD_ERR_FILE_COULDNOTSEEK;
		return FMOD_OK;
	}
}

AudioEngine::AudioEngine(Context* pContext) :
	Subsystem(pContext)
{
	AUTOPROFILE(AudioEngine_Initialize);

	FMOD_RESULT result;

	result = FMOD::System_Create(&m_pSystem);
	FLUX_LOG_FMOD(result);

	result = m_pSystem->init(512, FMOD_INIT_NORMAL, nullptr);
	FLUX_LOG_FMOD(result);
	result = m_pSystem->setUserData(this);
	FLUX_LOG_FMOD(result);
	result = m_pSystem->setFileSystem(FmodCallbacks::FileOpenCallback, FmodCallbacks::FileCloseCallback, FmodCallbacks::FileReadCallback, FmodCallbacks::FileSeekCallback, nullptr, nullptr, -1);
	FLUX_LOG_FMOD(result);
}

AudioEngine::~AudioEngine()
{
	m_pSystem->release();
}

void AudioEngine::Update()
{
	AUTOPROFILE(AudioEngine_Update);
	m_pSystem->update();
}
