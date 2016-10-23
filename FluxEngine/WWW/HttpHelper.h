#pragma once
class HttpHelper
{
public:
	static string HttpGet(const string host, const string path);

private:
	static const int RECV_BUFFER_SIZE = 100;
};

class SocketError : public runtime_error
{
public:
	SocketError(const wstring& context, const int errorCode) :
		runtime_error("")
	{
		m_Message = ErrorMessage(context, errorCode);
	}

	const wstring& GetMessageW() const { return m_Message; }

private:
	std::wstring ErrorMessage(const wstring& context, const int errorCode) const;
	wstring m_Message;
};