#include "FluxEngine.h"
#include "HttpRequest.h"
#include "Core\Config.h"

#pragma comment(lib, "wininet.lib")

namespace WinInet
{
	std::string TranslateError(int errorCode)
	{
		std::string errorString;
		char buffer[1024];
		uint32 length = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
			GetModuleHandle(TEXT("wininet.dll")),
			errorCode,
			0,
			buffer,
			ARRAYSIZE(buffer),
			NULL);
		if (length > 0)
		{
			errorString += buffer;
		}

		if (errorCode == ERROR_INTERNET_EXTENDED_ERROR)
		{
			DWORD inetError;
			DWORD extLength = 0;
			InternetGetLastResponseInfo(&inetError, nullptr, &extLength);
			++extLength;
			std::vector<char> extraBuffer(extLength);
			InternetGetLastResponseInfo(&inetError, extraBuffer.data(), &extLength);
			errorString += extraBuffer.data();
		}
		return errorString;
	}

	void CALLBACK InternetCallback(HINTERNET /*internet*/, DWORD_PTR context, DWORD internetStatus, LPVOID statusInformation, DWORD /*statusInformationLength*/)
	{
		HttpRequest* pRequest = (HttpRequest*)context;
		HttpResponse* pResponse = pRequest->GetResponse();
		switch (internetStatus)
		{
		case INTERNET_STATUS_REQUEST_COMPLETE:
			if (statusInformation != NULL)
			{
				const INTERNET_ASYNC_RESULT* asyncResult = (const INTERNET_ASYNC_RESULT*)statusInformation;
				if (asyncResult->dwResult == 0)
				{
					int lastError = Misc::GetLastErrorCode();
					FLUX_LOG(Warning, "[WinInet::InternetCallback] Request failed: %s", TranslateError(lastError).c_str());
				}
			}

			if (pRequest)
			{
				pResponse->ProcessResponse();
				pRequest->FinishRequest();
			}
			break;
		default:
			break;
		}
	}
}

HttpRequest::HttpRequest() :
	m_Response(this)
{

}

bool HttpRequest::StartRequest()
{
	if (!HttpConnection::Get()->IsConnected())
	{
		if (!HttpConnection::Get()->InitializeConnection())
		{
			return false;
		}
	}

	m_ConnectionHandle = InternetConnect(HttpConnection::Get()->GetHandle(), m_Url.GetHost().c_str(), m_Url.GetPort(), nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, (DWORD_PTR)this);
	if (m_ConnectionHandle == nullptr)
	{
		FLUX_LOG(Warning, "[HttpRequest::StartRequest()] Failed to open connection to '%s'", m_Url.GetHost().c_str());
		return false;
	}

	uint32 connectionTimeout = Config::GetInt("ConnectionTimeOut", "HTTP", 360);
	InternetSetOption(m_ConnectionHandle, INTERNET_OPTION_CONNECT_TIMEOUT, &connectionTimeout, sizeof(uint32));
	uint32 httpSendTimeout = Config::GetInt("HttpSendTimeout", "HTTP", 360);
	InternetSetOption(m_ConnectionHandle, INTERNET_OPTION_RECEIVE_TIMEOUT, &httpSendTimeout, sizeof(uint32));
	uint32 httpReceiveTimeout = Config::GetInt("HttpReceiveTimeout", "HTTP", 360);
	InternetSetOption(m_ConnectionHandle, INTERNET_OPTION_SEND_TIMEOUT, &httpReceiveTimeout, sizeof(uint32));
	
	uint32 requestFlags = m_Url.GetComponents().nScheme == INTERNET_SCHEME_HTTPS ? INTERNET_FLAG_SECURE : 0;
	requestFlags |= INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_PRAGMA_NOCACHE;
	requestFlags |= INTERNET_FLAG_KEEP_CONNECTION;

	std::string fullPath = m_Url.GetPath() + m_Url.GetExtraInfo();
	m_RequestHandle = HttpOpenRequest(m_ConnectionHandle, m_Verb.size() > 0 ? m_Verb.c_str() : nullptr, fullPath.c_str(), nullptr, nullptr, nullptr, requestFlags, (DWORD_PTR)this);
	if (m_RequestHandle == nullptr)
	{
		if (m_ConnectionHandle == nullptr)
		{
			int lastError = Misc::GetLastErrorCode();
			FLUX_LOG(Warning, "[HttpRequest::StartRequest()] Failed to open http request", WinInet::TranslateError(lastError).c_str());
			return false;
		}
	}
	
	std::string headers = GenerateHeaderBuffer((uint32)m_RequestPayload.size());
	BOOL requestSent = HttpSendRequest(m_RequestHandle, headers.c_str(), (uint32)headers.size(), m_RequestPayload.size() > 0 ? m_RequestPayload.data() : nullptr, (DWORD)m_RequestPayload.size());
	int lastError = Misc::GetLastErrorCode();
	if (!requestSent && lastError != ERROR_IO_PENDING)
	{
		FLUX_LOG(Warning, "[HttpRequest::StartRequest()] Failed to send http request", WinInet::TranslateError(lastError).c_str());
		return false;
	}
	return true;
}

void HttpRequest::FinishRequest()
{
	if (m_RequestHandle)
	{
		InternetCloseHandle(m_RequestHandle);
		m_RequestHandle = nullptr;
	}
	if (m_ConnectionHandle)
	{
		InternetCloseHandle(m_ConnectionHandle);
		m_ConnectionHandle = nullptr;
	}
	m_OnRequestComplete.ExecuteIfBound(*this, m_Response, m_Response.GetSuccess());
}

void HttpRequest::SetUrl(const std::string& url)
{
	m_Url = RequestUrl(url);
}

void HttpRequest::SetVerb(const std::string& verb)
{
	m_Verb = verb;
}

void HttpRequest::SetContent(const std::string& content)
{
	m_RequestPayload.resize(content.size());
	memcpy(m_RequestPayload.data(), content.data(), m_RequestPayload.size());
}

void HttpRequest::SetHeader(const std::string& name, const std::string& data)
{
	m_RequestHeaders[name] = data;
}

std::string HttpRequest::GenerateHeaderBuffer(uint32 contentLength)
{
	std::stringstream result;
	for (const std::pair<std::string, std::string>& header : m_RequestHeaders)
	{
		result << header.first << ": " << header.second << "\r\n";
	}
	if (contentLength > 0)
	{
		result << "Content-Length: " << contentLength << "\r\n";
	}
	return result.str();
}

bool RequestUrl::CrackUrl()
{
	m_Components = {};
	m_Components.dwStructSize = sizeof(m_Components);
	m_Components.dwHostNameLength = 1;
	m_Components.dwUrlPathLength = 1;
	m_Components.dwExtraInfoLength = 1;
	if (!InternetCrackUrl(m_Url.c_str(), (DWORD)m_Url.length(), 0, &m_Components))
	{
		FLUX_LOG(Warning, "[RequestUrl::CrackUrl()] Failed");
		return false;
	}
	m_Cracked = true;
	return true;
}

void RequestUrl::SetUrl(const std::string& url)
{
	m_Cracked = false;
	m_Url = url;
}

const std::string& RequestUrl::GetUrl()
{
	return m_Url;
}

const URL_COMPONENTS& RequestUrl::GetComponents()
{
	if (!m_Cracked)
	{
		CrackUrl();
	}
	return m_Components;
}

std::string RequestUrl::GetHost()
{
	if (!m_Cracked)
	{
		CrackUrl();
	}
	return std::string(m_Components.lpszHostName, m_Components.lpszHostName + m_Components.dwHostNameLength);
}

INTERNET_PORT RequestUrl::GetPort()
{
	if (!m_Cracked)
	{
		CrackUrl();
	}
	return m_Components.nPort;
}

const std::string RequestUrl::GetPath()
{
	if (!m_Cracked)
	{
		CrackUrl();
	}
	return std::string(m_Components.lpszUrlPath);
}

const std::string RequestUrl::GetExtraInfo()
{
	if (!m_Cracked)
	{
		CrackUrl();
	}
	return m_Components.lpszExtraInfo ? std::string(m_Components.lpszExtraInfo) : std::string();
}

void HttpResponse::ProcessResponse()
{
	m_Success = false;

	std::vector<char> responseBuffer;
	DWORD read = 0;
	DWORD totalRead = 0;
	do 
	{
		responseBuffer.resize(totalRead + 100);
		InternetReadFile(m_pOwner->GetRequestHandle(), responseBuffer.data() + totalRead, 100, &read);
		totalRead += read;
	} while (read > 0);
	m_Response = responseBuffer.data();

	if (!QueryResponseCode())
	{
		return;
	}
	if (!QueryResponseHeaders())
	{
		return;
	}

	m_Success = true;
}

bool HttpResponse::QueryResponseCode()
{
	DWORD codeSize = sizeof(HttpCode);
	if (!HttpQueryInfo(m_pOwner->GetRequestHandle(), HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &m_ResponseCode, &codeSize, nullptr))
	{
		int lastError = Misc::GetLastErrorCode();
		FLUX_LOG(Warning, "[HttpResponse::QueryResponseCode()] Failed to query response code: %s", WinInet::TranslateError(lastError).c_str());
		return false;
	}
	return true;
}

bool HttpResponse::QueryResponseHeaders()
{
	DWORD headerSize = 0;
	if (!HttpQueryInfo(m_pOwner->GetRequestHandle(), HTTP_QUERY_RAW_HEADERS_CRLF, 0, &headerSize, 0))
	{
		int lastError = Misc::GetLastErrorCode();
		if (lastError != ERROR_INSUFFICIENT_BUFFER)
		{
			FLUX_LOG(Warning, "[HttpResponse::QueryResponseHeaders()] Failed to query response headers: %s", WinInet::TranslateError(lastError).c_str());
			return false;
		}

		std::vector<char> headerBuffer(headerSize);
		if (!HttpQueryInfo(m_pOwner->GetRequestHandle(), HTTP_QUERY_RAW_HEADERS_CRLF, headerBuffer.data(), &headerSize, nullptr))
		{
			lastError = Misc::GetLastErrorCode();
			FLUX_LOG(Warning, "[HttpResponse::QueryResponseHeaders()] Failed to query response headers: %s", WinInet::TranslateError(lastError).c_str());
			return false;
		}
		std::stringstream stream(headerBuffer.data());
		std::string line;
		while (std::getline(stream, line))
		{
			size_t colonPos = line.find(':');
			if (colonPos != std::string::npos)
			{
				std::string name = line.substr(0, colonPos);
				std::string value = line.substr(colonPos + 2);
				m_ResponseHeaders[name] = value;
			}
		}
	}
	return true;
}

HttpConnection* HttpConnection::Get()
{
	static HttpConnection instance;
	return &instance;
}

bool HttpConnection::InitializeConnection()
{
	ShutdownConnection();
	AUTOPROFILE(HttpConnection_InitializeConnection);

	if (InternetAttemptConnect(0) != ERROR_SUCCESS)
	{
		FLUX_LOG(Warning, "[HttpConnection::InitializeConnection()] Failed: %s", WinInet::TranslateError(Misc::GetLastErrorCode()).c_str());
		return false;
	}

	m_InternetHandle = InternetOpen("test",
		INTERNET_OPEN_TYPE_PRECONFIG,
		NULL,
		NULL,
		INTERNET_FLAG_ASYNC);

	if (m_InternetHandle == nullptr)
	{
		FLUX_LOG(Warning, "[HttpConnection::InitializeConnection()] Failed: %s", WinInet::TranslateError(Misc::GetLastErrorCode()).c_str());
		return false;
	}
	InternetSetStatusCallback(m_InternetHandle, &WinInet::InternetCallback);
	m_IsInitialized = true;

	return true;
}

bool HttpConnection::ShutdownConnection()
{
	m_IsInitialized = false;
	if (m_InternetHandle != nullptr)
	{
		InternetSetStatusCallback(m_InternetHandle, nullptr);
		if (!InternetCloseHandle(m_InternetHandle))
		{
			FLUX_LOG(Warning, "[HttpConnection::InitializeConnection()] Failed: %s", WinInet::TranslateError(Misc::GetLastErrorCode()).c_str());
			return false;
		}
		m_InternetHandle = nullptr;
	}
	return true;
}

bool HttpConnection::IsConnected()
{
	return m_InternetHandle != nullptr;
}
