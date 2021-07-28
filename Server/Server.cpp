#include <iostream>
#include <string>
#include <queue>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include <WinSock2.h>

#pragma comment (lib, "Ws2_32.lib")

std::mutex _client_sockets_mutex, _messages_mutex;
std::condition_variable _condition_variable;

SOCKET _socket;
sockaddr_in _addr, _client_addr;
int _client_addr_size = sizeof(_client_addr);

std::map<SOCKET, std::string> _client_sockets;
std::queue<std::pair<SOCKET, std::string>> _messages;

void accept_t()
{
    SOCKET client_socket;

    while (true)
    {
        if (client_socket = ::accept(_socket, (SOCKADDR*)&_addr, &_client_addr_size); client_socket != INVALID_SOCKET)
        {
            std::unique_lock<std::mutex> gurad(_client_sockets_mutex);

            _client_sockets.insert({ client_socket, "" });
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void recv_t()
{
    char buffer[1024];

    while (true)
    {
        std::unique_lock<std::mutex> gurad(_client_sockets_mutex);

        for (auto it = _client_sockets.begin(); it != _client_sockets.end(); it++)
        {
            if (auto result = ::recv(it->first, buffer, 1024, 0); result > 0)
            {
                std::unique_lock<std::mutex> gurad(_messages_mutex);

                std::string data(buffer, result);

                if (it->second == "")
                {
                    it->second = data;

                    _messages.push({ 0, "Welcome " + data });
                }
                else
                {
                    _messages.push({ it->first, it->second + ": " + data });
                }

                _condition_variable.notify_one();
            }
            else if (result == 0)
            {
                _client_sockets.erase(it);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void send_t()
{
    while (true)
    {
        std::unique_lock<std::mutex> gurad(_messages_mutex);
        _condition_variable.wait(gurad, [] { return !_messages.empty(); });

        while (!_messages.empty())
        {
            auto message = _messages.front();
            _messages.pop();

            std::unique_lock<std::mutex> gurad(_client_sockets_mutex);

            for (auto it = _client_sockets.begin(); it != _client_sockets.end(); it++)
            {
                if (it->first != message.first)
                {
                    if (auto result = ::send(it->first, message.second.data(), message.second.size(), 0); result == SOCKET_ERROR)
                    {
                        std::cout << "Error!" << std::endl;
                    }
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

int main(int argc, char* argv[])
{
    std::cout << "Server!" << std::endl;

    WSADATA data;
    if (auto result = ::WSAStartup(MAKEWORD(2, 2), &data); result != NO_ERROR)
    {
        std::cout << "Error!" << std::endl;
    }

    if (_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); _socket == INVALID_SOCKET)
    {
        std::cout << "Error!" << std::endl;
    }

    u_long cmd = 1;
    if (auto result = ::ioctlsocket(_socket, FIONBIO, &cmd); result == SOCKET_ERROR)
    {
        std::cout << "Error!" << std::endl;
    }

    _addr.sin_family = AF_INET;
    _addr.sin_addr.S_un.S_addr = INADDR_ANY;
    _addr.sin_port = htons(6969);

    if (auto result = ::bind(_socket, (SOCKADDR*)&_addr, sizeof(_addr)); result == SOCKET_ERROR)
    {
        std::cout << "Error!" << std::endl;
    }

    if (auto result = ::listen(_socket, SOMAXCONN); result == SOCKET_ERROR)
    {
        std::cout << "Error!" << std::endl;
    }

    std::thread accept_thread(accept_t);
    std::thread recv_thread(recv_t);
    std::thread send_thread(send_t);

    if (accept_thread.joinable())
    {
        accept_thread.join();
    }

    if (recv_thread.joinable())
    {
        recv_thread.join();
    }

    if (send_thread.joinable())
    {
        send_thread.join();
    }

    if (auto result = ::closesocket(_socket); result == SOCKET_ERROR)
    {
        std::cout << "Error!" << std::endl;
    }

    if (auto result = ::WSACleanup(); result != NO_ERROR)
    {
        std::cout << "Error!" << std::endl;
    }

    return 0;
}