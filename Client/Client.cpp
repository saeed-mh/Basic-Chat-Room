#include <iostream>
#include <string>
#include <thread>

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")

SOCKET _socket;

void recv_t()
{
    char buffer[1024];

    while (true)
    {
        if (auto result = ::recv(_socket, buffer, 1024, 0); result > 0)
        {
            std::string data(buffer, result);

            std::cout << data << std::endl;
        }
    }
}

int main(int argc, char* argv[])
{
    std::cout << "Client!" << std::endl;

    WSADATA data;
    if (auto result = ::WSAStartup(MAKEWORD(2, 2), &data); result != NO_ERROR)
    {
        std::cout << "Error!" << std::endl;
    }

    if (_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); _socket == INVALID_SOCKET)
    {
        std::cout << "Error!" << std::endl;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6969);

    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.S_un.S_addr);

    if (auto result = ::connect(_socket, (SOCKADDR*)&addr, sizeof(addr)); result == SOCKET_ERROR)
    {
        std::cout << WSAGetLastError() << std::endl;
    }

    std::string message;

    std::cout << "Your Name: ";
    std::getline(std::cin, message);

    if (auto result = ::send(_socket, message.data(), message.size(), 0); result == SOCKET_ERROR)
    {
        std::cout << "Error!" << std::endl;
    }

    std::thread recv_thread(recv_t);

    while (true)
    {
        std::getline(std::cin, message);

        if (auto result = ::send(_socket, message.data(), message.size(), 0); result == SOCKET_ERROR)
        {
            std::cout << "Error!" << std::endl;
        }
    }

    if (recv_thread.joinable())
    {
        recv_thread.join();
    }

    if (auto result = ::closesocket(_socket); result == SOCKET_ERROR)
    {
        std::cout << WSAGetLastError() << std::endl;
    }

    if (auto result = ::WSACleanup(); result != NO_ERROR)
    {
        std::cout << "Error!" << std::endl;
    }

    return 0;
}