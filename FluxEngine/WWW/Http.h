#pragma once
class Http
{
public:
	static string HttpGet(const string host, const string path);

private:
	static const int RECV_BUFFER_SIZE = 100;
};

class SocketError : public runtime_error
{
public:
	SocketError(const string& context, const int errorCode) :
		runtime_error("")
	{
		m_Message = ErrorMessage(context, errorCode);
	}

	const string& GetMessageW() const { return m_Message; }

private:
	std::string ErrorMessage(const string& context, const int errorCode) const;
	string m_Message;
};