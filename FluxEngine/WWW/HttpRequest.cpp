#include "FluxEngine.h"
#include "HttpRequest.h"
#include "Core\Config.h"

namespace WinInet
{
	std::string TranslateError(int errorCode)
	{
		std::string errorString;
		char buffer[1024];
		const uint32 length = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
			GetModuleHandle(TEXT("wininet.dll")),
			errorCode,
			0,
			buffer,
			ARRAYSIZE(buffer),
			nullptr);
		if (length > 0)
		{
			errorString += buffer;
			while (errorString.back() == '\n' || errorString.back() == '\r')
			{
				errorString.pop_back();
			}
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
		ScopeLock lock(HttpConnection::Get()->RequestLock());

		const int errorCode = Misc::GetLastErrorCode();
		if (errorCode != 0 &&
			errorCode != ERROR_HTTP_HEADER_NOT_FOUND &&
			errorCode != ERROR_IO_PENDING)
		{
			if (errorCode == ERROR_INVALID_HANDLE)
			{
				return;
			}
		}

		HttpRequest* pRequest = reinterpret_cast<HttpRequest*>(context);
		HttpResponse* pResponse = pRequest->GetResponse();

		bool debugLog = pRequest->IsDebug();
		switch (internetStatus)
		{
		case INTERNET_STATUS_PREFETCH:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] STATUS_PREFETCH: %p", context);
			break;
		case INTERNET_STATUS_USER_INPUT_REQUIRED:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] USER_INPUT_REQUIRED: %p", context);
			break;
		case INTERNET_STATUS_DETECTING_PROXY:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] DETECTING_PROXY: %p", context);
			break;
		case INTERNET_STATUS_CLOSING_CONNECTION:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] CLOSING_CONNECTION: %p", context);
			break;
		case INTERNET_STATUS_CONNECTED_TO_SERVER:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] CONNECTED_TO_SERVER: %p", context);
			break;
		case INTERNET_STATUS_CONNECTING_TO_SERVER:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] CONNECTING_TO_SERVER: %p", context);
			break;
		case INTERNET_STATUS_CONNECTION_CLOSED:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] CONNECTION_CLOSED: %p", context);
			break;
		case INTERNET_STATUS_HANDLE_CLOSING:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] HANDLE_CLOSING: %p", context);
			break;
		case INTERNET_STATUS_HANDLE_CREATED:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] HANDLE_CREATED: %p", context);
			break;
		case INTERNET_STATUS_INTERMEDIATE_RESPONSE:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] INTERMEDIATE_RESPONSE: %p", context);
			break;
		case INTERNET_STATUS_NAME_RESOLVED:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] NAME_RESOLVED: %p", context);
			break;
		case INTERNET_STATUS_RECEIVING_RESPONSE:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] RECEIVING_RESPONSE: %p", context);
			break;
		case INTERNET_STATUS_RESPONSE_RECEIVED:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] RESPONSE_RECEIVED (%d bytes): %p", *(uint32*)statusInformation, context);
			break;
		case INTERNET_STATUS_REDIRECT:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] STATUS_REDIRECT: %p", context);
			break;
		case INTERNET_STATUS_REQUEST_COMPLETE:
			if (statusInformation != nullptr)
			{
				const INTERNET_ASYNC_RESULT* asyncResult = (const INTERNET_ASYNC_RESULT*)statusInformation;
				if (asyncResult->dwResult == 0)
				{
					int lastError = Misc::GetLastErrorCode();
					FLUX_LOG(Warning, "[WinInet::InternetCallback] Request failed: %s", TranslateError(lastError).c_str());
					pResponse = nullptr;
				}
			}

			if (pResponse)
			{
				bool finished = false;
				if (pResponse->ProcessResponse(finished) && finished)
				{
					pRequest->FinishRequest();
				}
			}
			break;
		case INTERNET_STATUS_REQUEST_SENT:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] REQUEST_SENT: %p", context);
			break;
		case INTERNET_STATUS_RESOLVING_NAME:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] RESOLVING_NAME: %p", context);
			break;
		case INTERNET_STATUS_SENDING_REQUEST:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] SENDING_REQUEST: %p", context);
			break;
		case INTERNET_STATUS_STATE_CHANGE:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] STATE_CHANGE: %p", context);
			break;
		case INTERNET_STATUS_COOKIE_SENT:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] COOKIE_SENT: %p", context);
			break;
		case INTERNET_STATUS_COOKIE_RECEIVED:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] COOKIE_RECEIVED: %p", context);
			break;
		case INTERNET_STATUS_PRIVACY_IMPACTED:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] PRIVACY_IMPACTED: %p", context);
			break;
		case INTERNET_STATUS_P3P_HEADER:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] P3P_HEADER: %p", context);
			break;
		case INTERNET_STATUS_P3P_POLICYREF:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] P3P_POLICYREF: %p", context);
			break;
		case INTERNET_STATUS_COOKIE_HISTORY:
		{
			const InternetCookieHistory* CookieHistory = (const InternetCookieHistory*)statusInformation;
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] COOKIE_HISTORY: %p. Accepted: %u. Leashed: %u. Downgraded: %u. Rejected: %u.",
				context
				, CookieHistory->fAccepted
				, CookieHistory->fLeashed
				, CookieHistory->fDowngraded
				, CookieHistory->fRejected);
		}
		break;
		default:
			FLUX_LOG_INFO(debugLog, VeryVerbose, "[WinInet::InternetCallback()] STATUS_REDIRECT: %p", context);
			break;
		}
	}
}

HttpRequest::HttpRequest(bool debug) :
	m_Debug(debug), m_Response(this)
{

}

bool HttpRequest::StartRequest()
{
	m_Complete = false;
	m_RequestStartedTime = DateTime::Now();
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

	std::string fullPath = m_Url.GetPath() + std::string(m_Url.GetExtraInfo());
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
	m_RequestCompletedTime = DateTime::Now() - m_RequestStartedTime;
	m_OnRequestComplete.ExecuteIfBound(*this, m_Response.GetSuccess());
	m_Complete = true;
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
	for (const auto& header : m_RequestHeaders)
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

const char* RequestUrl::GetPath()
{
	if (!m_Cracked)
	{
		CrackUrl();
	}
	return m_Components.lpszUrlPath;
}

const char* RequestUrl::GetExtraInfo()
{
	if (!m_Cracked)
	{
		CrackUrl();
	}
	return m_Components.lpszExtraInfo ? m_Components.lpszExtraInfo : "";
}

bool HttpResponse::ProcessResponse(bool& finished)
{
	std::string headerLength;
	int contentLength = 0;
	if (QueryHeaderString(HTTP_QUERY_CONTENT_LENGTH, "", headerLength))
	{
		contentLength = stoi(headerLength);
	}

	if (!QueryResponse(contentLength, finished))
	{
		return false;
	}
	if (finished == false)
	{
		return true;
	}

	if (!QueryResponseCode())
	{
		return false;
	}
	if (!QueryResponseHeaders())
	{
		return false;
	}
	if (QueryHeaderString(HTTP_QUERY_CONTENT_LENGTH, "", headerLength))
	{
		//contentLength = stoi(headerLength);
	}

	m_Success = true;
	return true;
}

bool HttpResponse::QueryResponse(uint32 contentLength, bool& finished)
{
	const size_t bufferSize = 204800;
	size_t newSize = m_Response.size() + contentLength == 0 ? bufferSize : contentLength;
	m_Response.resize(newSize);

	DWORD toRead = bufferSize;
	DWORD read = 0;
	do
	{
		BOOL success = InternetReadFile(m_pOwner->GetRequestHandle(), m_Response.data() + m_BytesRead, toRead, &read);
		int errorCode = Misc::GetLastErrorCode();
		FLUX_LOG_INFO(m_pOwner->IsDebug(), VeryVerbose, "[HttpResponse::QueryResponse] InternetReadFile: %d. (%u bytes received)", errorCode, read);
		if(!success)
		{
			if (errorCode == ERROR_IO_PENDING)
			{
				FLUX_LOG_INFO(m_pOwner->IsDebug(), VeryVerbose, "[HttpResponse::QueryResponse] Async read operation is in progress]");
			}
			else if (errorCode != ERROR_SUCCESS)
			{
				FLUX_LOG_INFO(m_pOwner->IsDebug(), VeryVerbose, "[HttpResponse::QueryResponse] InternetReadFile failed. (%u bytes received). Error: %s", read, WinInet::TranslateError(errorCode).c_str());
				return false;
			}
			finished = false;
			return true;
		}
		m_BytesRead += read;
		toRead -= read;
		if (contentLength == 0 && m_BytesRead >= (DWORD)m_Response.size())
		{
			m_Response.resize(m_Response.size() + bufferSize);
		}
	} while (read > 0);

	finished = true;
	return true;
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
	if (!HttpQueryInfo(m_pOwner->GetRequestHandle(), HTTP_QUERY_RAW_HEADERS_CRLF, nullptr, &headerSize, nullptr))
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
				while (value.back() == '\n' || value.back() == '\r')
				{
					value.pop_back();
				}
				m_ResponseHeaders[name] = value;
			}
		}
	}
	return true;
}

bool HttpResponse::QueryHeaderString(uint32 httpQueryInfoLevel, const std::string& /*headerName*/, std::string& outHeaderString)
{
	DWORD headerSize = 0;
	char headerBuffer[128];
	memset(headerBuffer, 0, sizeof(headerBuffer));
	if (!HttpQueryInfo(m_pOwner->GetRequestHandle(), (DWORD)httpQueryInfoLevel, headerBuffer, &headerSize, nullptr))
	{
		uint32 errorCode = Misc::GetLastErrorCode();
		if (errorCode == ERROR_HTTP_HEADER_NOT_FOUND)
		{
			outHeaderString = "";
			return false;
		}
		else if (errorCode == ERROR_INSUFFICIENT_BUFFER)
		{
			std::vector<char> header(headerSize);
			if (!HttpQueryInfo(m_pOwner->GetRequestHandle(), (DWORD)httpQueryInfoLevel, header.data(), &headerSize, nullptr))
			{
				FLUX_LOG(Warning, "[HttpResponse::QueryHeaderString] Failed to query header info");
				outHeaderString = "";
				return false;
			}
			outHeaderString = header.data();
			return true;
		}
		return false;
	}
	outHeaderString = headerBuffer;
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
		nullptr,
		nullptr,
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
