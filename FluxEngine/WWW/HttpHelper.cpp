#include "stdafx.h"
#include "HttpHelper.h"

#include <winsock2.h>
#include <ws2tcpip.h>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

string HttpHelper::HttpGet(string host, string path)
{
	WSADATA wsaData;
	int result;

	// Initialize Winsock
	result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
		throw SocketError(L"WSAStartUp", result);

	// Resolve the server address and port
	addrinfo * pAddrInfo;
	result = getaddrinfo(host.c_str(), "80", nullptr, &pAddrInfo);
	if (result != 0)
		throw SocketError(L"addrinfo", result);

	//Create the socket
	SOCKET sock = socket(pAddrInfo->ai_family, pAddrInfo->ai_socktype, pAddrInfo->ai_protocol);

	if (sock == INVALID_SOCKET)
		throw SocketError(L"Socket", WSAGetLastError());

	// Connect to server.
	result = connect(sock, pAddrInfo->ai_addr, pAddrInfo->ai_addrlen);
	if (result != 0)
		throw SocketError(L"Connect", WSAGetLastError());

	const string request = "GET " + path + " HTTP/1.1\nHost: " + host + "\n\n";

	// Send an initial buffer
	result = send(sock, request.c_str(), request.size(), 0);
	if (result == SOCKET_ERROR)
		throw SocketError(L"Send", WSAGetLastError());

	// shutdown the connection since no more data will be sent
	result = shutdown(sock, SD_SEND);
	if (result == SOCKET_ERROR)
		throw SocketError(L"Close send connection", WSAGetLastError());

	// Receive until the peer closes the connection
	string response;

	char buffer[RECV_BUFFER_SIZE];
	int bytesRecv = 0;

	for (;;)
	{
		result = recv(sock, buffer, sizeof(buffer), 0);
		if (result == SOCKET_ERROR)
			throw SocketError(L"Recv", WSAGetLastError());
		if (result == 0)
			break;
		response += string(buffer, result);
		wstringstream stream;
		stream << L"HttpGet() > Bytes received: " << bytesRecv;
		DebugLog::Log(stream.str(), LogType::INFO);
		bytesRecv += result;
	}
	wstringstream stream;
	stream << L"HttpGet() > Bytes received: " << bytesRecv;
	DebugLog::Log(stream.str(), LogType::INFO);

	// cleanup
	result = closesocket(sock);
	if (result == SOCKET_ERROR)
		throw SocketError(L"Closesocket", WSAGetLastError());

	result = WSACleanup();
	if (result == SOCKET_ERROR)
		throw SocketError(L"WSACleanup", WSAGetLastError());

	freeaddrinfo(pAddrInfo);

	DebugLog::Log(L"HttpGet() > Cleanup Successful ", LogType::INFO);

	return response;
}

wstring SocketError::ErrorMessage(const wstring & context, const int errorCode) const
{
	wchar_t buf[1024];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, errorCode, 0, buf, sizeof(buf), nullptr);
	wchar_t * newLine = wcsrchr(buf, '\r');
	if (newLine) *newLine = '\0';
	wstringstream stream;
	stream << L"Socket error in " << context << L" (" << errorCode << L"): " << buf;
	return stream.str();
}