// C++ equivalent
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <winsock2.h>
#include <stdio.h>
#include <Ws2tcpip.h>
// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996)
int main()
{
    // Initialize WSADATA in Windows
    WSADATA wsadata;
    struct sockaddr_in server_addr;
    while (true)
    {
        int err = WSAStartup(0x101, (LPWSADATA)&wsadata); // 2.2 version
        if (err != 0)
        {
            printf("WSAStartup failed with error: %d\n", err);
        }
        // Create socket and set port
        int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
        std::cout << "Socket successfully created" << std::endl;
        int port = 9999;
        const char* ipadd = "127.0.0.1";
        
        // Bind socket to port
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(ipadd);
        server_addr.sin_port = htons(port);
        bind(server_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
        std::cout << "Socket is listening..." << std::endl;
        // Start listening to requests
        listen(server_sockfd, 5);

        std::cout << "Server is waiting..." << std::endl;
        struct sockaddr_in client_addr;

        // Establish connection with client
        int client_len = sizeof(client_addr);
        int client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_addr, &client_len);

        // Pending for any client acceptance... then run the following code
        char client_addr_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_addr_str, INET_ADDRSTRLEN);
        std::cout << "Server connected to " << client_addr_str << std::endl;

        // Respond the client with server clock time
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        std::string time_str = std::ctime(&time);
        const char* time_ch = time_str.c_str();
        send(client_sockfd, time_ch, time_str.size(), 0);
        std::cout << "Server send time data : " << time_ch << " with size " << time_str.size() <<
            std::endl;
        std::cout << "Server send time to " << client_addr_str << std::endl;

        // Close the connection with the client process
        char error_code = closesocket(client_sockfd);
        if (error_code == 0)
        {
            std::cout << "Server closed connection with " << client_addr_str << std::endl;
        }
        else
        {
            std::cout << "Server failed to close connection with " << client_addr_str << " : ERRCODE " <<
                WSAGetLastError() << std::endl;
        }
        WSACleanup();
        system("pause");
    }
}
