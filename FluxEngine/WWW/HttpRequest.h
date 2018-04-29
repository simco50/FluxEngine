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

private:
	bool m_IsInitialized = false;
	HINTERNET m_InternetHandle = nullptr;

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

	void ProcessResponse();

	const std::string& GetContent() const { return m_Response; }
	HttpCode GetResponseCode() const { return m_ResponseCode; }
	const std::map<std::string, std::string>& GetResponseHeaders() const { return m_ResponseHeaders; }
	bool GetSuccess() const { return m_Success; }

private:
	bool QueryResponseCode();
	bool QueryResponseHeaders();

	HttpRequest* m_pOwner;
	std::string m_Response;
	HttpCode m_ResponseCode = HttpCode::NONE;
	std::map<std::string, std::string> m_ResponseHeaders;
	bool m_Success = false;
};

using OnHttpProcessComplete = SinglecastDelegate<void, const HttpRequest&, const HttpResponse&, bool>;

class HttpRequest
{
public:
	HttpRequest();

	bool StartRequest();
	void FinishRequest();

	void SetUrl(const std::string& url);
	void SetVerb(const std::string& verb);
	void SetContent(const std::string& content);
	void SetHeader(const std::string& name, const std::string& data);

	std::string GenerateHeaderBuffer(uint32 contentLength);

	HINTERNET GetRequestHandle() const { return m_RequestHandle; }
	HttpResponse* GetResponse() { return &m_Response; }

	OnHttpProcessComplete& OnComplete() { return m_OnRequestComplete; }

private:
	OnHttpProcessComplete m_OnRequestComplete;

	std::string m_Verb;
	std::vector<char> m_RequestPayload;
	std::map<std::string, std::string> m_RequestHeaders;

	HINTERNET m_ConnectionHandle = nullptr;
	HINTERNET m_RequestHandle = nullptr;

	RequestUrl m_Url;
	HttpResponse m_Response;
};