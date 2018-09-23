#pragma once
#include <wininet.h>
#include "HttpCodes.h"

class HttpConnection
{
public:
	static HttpConnection* Get();

	bool InitializeConnection();
	bool ShutdownConnection();

	bool IsConnected();
	HINTERNET GetHandle() { return m_InternetHandle; }
	Mutex& RequestLock() { return m_RequestMutex; }

private:
	bool m_IsInitialized = false;
	HINTERNET m_InternetHandle = nullptr;
	Mutex m_RequestMutex;

	HttpConnection() {}
	~HttpConnection() {}
};

class RequestUrl
{
public:
	RequestUrl()
	{}

	RequestUrl(const std::string& url) :
		m_Url(url)
	{}

	bool CrackUrl();
	void SetUrl(const std::string& url);
	const std::string& GetUrl();
	const URL_COMPONENTS& GetComponents();

	std::string GetHost();
	INTERNET_PORT GetPort();
	const std::string GetPath();
	const std::string GetExtraInfo();

private:
	bool m_Cracked = false;
	std::string m_Url;
	URL_COMPONENTS m_Components;
};

class HttpRequest;

class HttpResponse
{
public:
	HttpResponse(HttpRequest* pOwner) :
		m_pOwner(pOwner)
	{}

	bool ProcessResponse(bool& finished);

	const std::vector<char>& GetContent() const { return m_Response; }
	HttpCode GetResponseCode() const { return m_ResponseCode; }
	const std::map<std::string, std::string>& GetResponseHeaders() const { return m_ResponseHeaders; }
	bool GetSuccess() const { return m_Success; }

private:
	bool QueryResponse(uint32 contentLength, bool& finished);
	bool QueryResponseCode();
	bool QueryResponseHeaders();
	bool QueryHeaderString(uint32 httpQueryInfoLevel, const std::string& headerName, std::string& outHeaderString);

	HttpRequest* m_pOwner;
	std::vector<char> m_Response;
	HttpCode m_ResponseCode = HttpCode::NONE;
	std::map<std::string, std::string> m_ResponseHeaders;
	uint32 m_BytesRead = 0;
	bool m_Success = false;
};

DECLARE_DELEGATE(OnHttpProcessComplete, const HttpRequest&, bool);

class HttpRequest
{
public:
	HttpRequest(bool debug = false);

	bool StartRequest();
	void FinishRequest();

	void SetUrl(const std::string& url);
	void SetVerb(const std::string& verb);
	void SetContent(const std::string& content);
	void SetHeader(const std::string& name, const std::string& data);

	std::string GenerateHeaderBuffer(uint32 contentLength);

	HINTERNET GetRequestHandle() const { return m_RequestHandle; }
	HttpResponse* GetResponse() { return &m_Response; }
	const HttpResponse* GetResponse() const { return &m_Response; }

	const TimeSpan& GetCompletionTime() const { return m_RequestCompletedTime; }
	OnHttpProcessComplete& OnComplete() { return m_OnRequestComplete; }

	bool IsDebug() const { return m_Debug; }
	bool IsComplete() const { return m_Complete; }

private:
	OnHttpProcessComplete m_OnRequestComplete;

	bool m_Complete = false;
	bool m_Debug;
	DateTime m_RequestStartedTime = DateTime(0);
	TimeSpan m_RequestCompletedTime = TimeSpan(0);

	std::string m_Verb;
	std::vector<char> m_RequestPayload;
	std::map<std::string, std::string> m_RequestHeaders;

	HINTERNET m_ConnectionHandle = nullptr;
	HINTERNET m_RequestHandle = nullptr;

	RequestUrl m_Url;
	HttpResponse m_Response;
};