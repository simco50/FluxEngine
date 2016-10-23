#pragma once
#include <unordered_map>
#include "../Debugging/PerfTimer.h"

class Loader
{
public:
	Loader(void){}
	virtual ~Loader(void){}

	virtual const type_info& GetType() const = 0;
	virtual void Unload() = 0;
	virtual void SetDevice(ID3D11Device* pDevice) { m_pDevice = pDevice; }

protected:
	ID3D11Device* m_pDevice = nullptr;

private:
	Loader( const Loader &obj);
	Loader& operator=( const Loader& obj );
};

template <class T>
class ResourceLoader : public Loader
{
public:
	ResourceLoader()	{ ++m_loaderReferences; }

	~ResourceLoader() {}

	ResourceLoader(const ResourceLoader &obj) = delete;
	ResourceLoader& operator=(const ResourceLoader& obj) = delete;

	virtual const type_info& GetType() const { return typeid(T); }

	T* GetContent(const wstring& assetFile)
	{
		for(pair<wstring, T*> kvp:m_contentReferences)
		{
			if(kvp.first.compare(assetFile)==0)
				return kvp.second;
		}

		//Does File Exists?
		struct _stat buff;
		int result = _wstat(assetFile.c_str(), &buff);
		if(result != 0)
		{
			wstringstream ss;
			ss<<"ResourceManager> File not found!\nPath: ";
			ss<<assetFile;
			DebugLog::Log(ss.str(), LogType::ERROR);
		}

		wstringstream stream;
		stream << assetFile << " loaded.";
		PerfTimer timer(stream.str());
		T* content = LoadContent(assetFile);
		if(content!=nullptr)m_contentReferences.insert(pair<wstring,T*>(assetFile, content));
		timer.Stop();
		return content;
	}

	T* GetContent_Unmanaged(const wstring& assetFile)
	{
		wstringstream stream;
		stream << assetFile << " loaded";
		PerfTimer timer(stream.str());
		T* content = LoadContent(assetFile);
		DebugLog::LogFormat(LogType::WARNING, L"ResourceManager > Asset '%s' loaded unmanaged.", assetFile.c_str());
		timer.Stop();
		return content;
	}

	virtual void Unload()
	{
		--m_loaderReferences;

		if(m_loaderReferences<=0)
		{
			for(pair<wstring,T*> kvp:m_contentReferences)
			{
				Destroy(kvp.second);
			}

			m_contentReferences.clear();
		}
	}

protected:
	virtual T* LoadContent(const wstring& assetFile) = 0;
	virtual void Destroy(T* objToDestroy) = 0;

private:
	static unordered_map<wstring, T*> m_contentReferences;
	static int m_loaderReferences;
};

template<class T>
unordered_map<wstring, T*> ResourceLoader<T>::m_contentReferences = unordered_map<wstring, T*>();

template<class T>
int ResourceLoader<T>::m_loaderReferences = 0;

