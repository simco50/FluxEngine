#pragma once
#include "Async\Thread.h"
#include "Diagnostics\Timer.h"

class FileWatcher : public HookableThread
{
public:
	FileWatcher();
	virtual ~FileWatcher();

	bool StartWatching(const std::string& directory, const bool recursiveWatch = true);
	void StopWatching();
	bool GetNextChange(std::string& fileName);

private:
	virtual int ThreadFunction() override;
	void AddChange(const std::string& fileName);

	static const int BUFFERSIZE = 2048;
	bool m_Exiting = true;
	bool m_RecursiveWatch = true;
	Mutex m_Mutex;
	HANDLE m_FileHandle = nullptr;
	std::map<std::string, Timer> m_Changes;
};