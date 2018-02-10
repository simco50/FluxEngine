#pragma once
class Http
{
public:
	static std::string HttpGet(const std::string& host, const std::string& path);

private:
	static const int RECV_BUFFER_SIZE = 100;
};

class SocketError : public std::runtime_error
{
public:
	SocketError(const std::string& context, const int errorCode) :
		runtime_error("")
	{
		m_Message = ErrorMessage(context, errorCode);
	}

	const std::string& GetMessageW() const { return m_Message; }

private:
	std::string ErrorMessage(const std::string& context, const int errorCode) const;
	std::string m_Message;
};