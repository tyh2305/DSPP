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
    // Initialise WSADATA in Windows
    WSADATA wsadata;
    struct sockaddr_in server_addr;
    int err = WSAStartup(0x202, (LPWSADATA)&wsadata); // 2.2 version
    if (err != 0)
    {
        printf("WSAStartup failed with error: %d\n", err);
    }

    // Create socket, set port and ip address
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    std::cout << "Socket successfully created" << std::endl << std::endl;
    int port = 9999;
    const char* ipadd = "127.0.0.1";
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ipadd);
    server_addr.sin_port = htons(port);
    // Establish connection with server
    int server_len = sizeof(server_addr);
    connect(server_sockfd, (struct sockaddr*)&server_addr, server_len);

    std::chrono::time_point<std::chrono::system_clock> request_time_point =

        std::chrono::system_clock::now();
    std::time_t request_time = std::chrono::system_clock::to_time_t(request_time_point);
    auto request_time_epoch = request_time_point.time_since_epoch();

    // Receive data from the server
    char ch[24];
    char error_code = recv(server_sockfd, ch, 24, 0);
    if (error_code < 0)
    {
        // Error occur
        // Return 10057, WSAENOTCONN, socket is not connected, due to server not started
        switch (WSAGetLastError())
        {
            case WSAENOTCONN:
                printf("Socket is not connected\n");
                break;
            default:
                printf("Unknown Error %d\n", WSAGetLastError());
        }
        return -1;
    }

    // Extract and print exactly 24 characters
    int size_arr = sizeof(ch) / sizeof(char);
    std::string str = "";
    for (int x = 0; x < size_arr; x++)
    {
        str = str + ch[x];
    }
    std::cout << "Data received from server: " << str << std::endl << std::endl;

    std::chrono::time_point<std::chrono::system_clock> response_time_point =

        std::chrono::system_clock::now();
    std::time_t response_time = std::chrono::system_clock::to_time_t(response_time_point);
    auto response_time_epoch = response_time_point.time_since_epoch();

    // To be fixed: cannot convert received &ch to time_t correctly
    const char* time_ch = str.c_str();
    std::time_t server_time = std::time_t(time_ch);
    // std::string server_time_str = std::ctime(&server_time);
    //std::cout << "Time returned by server: " << server_time_str.c_str() << std::endl;
    std::cout << "Time returned by server: " << str << std::endl << std::endl;

    std::chrono::time_point<std::chrono::system_clock> actual_time_point =

        std::chrono::system_clock::now();
    std::time_t actual_time = std::chrono::system_clock::to_time_t(actual_time_point);
    std::string actual_time_str = std::ctime(&actual_time);
    auto actual_time_epoch = actual_time_point.time_since_epoch();
    auto latency_time = response_time_epoch - request_time_epoch;

    auto latency_time_milis =

        std::chrono::duration_cast<std::chrono::microseconds>(latency_time).count();
    std::cout << "Process Delay latency: " << latency_time_milis << " microseconds\n" << std::endl;
    std::cout << "Actual clock time at client side: " << actual_time_str.c_str() << std::endl;

    //synchronize process client clock time
    std::chrono::time_point<std::chrono::system_clock> client_time_point = actual_time_point;

    // To be fixed: time_t object does not have decimal point for second, cannot do calculation below a 	second.
    std::time_t client_time = actual_time; // + (latency_time / 2);
    std::string client_time_str = std::ctime(&client_time);
    auto client_time_epoch = client_time_point.time_since_epoch();
    std::cout << "Synchronized process client time: " << client_time_str.c_str() << std::endl;

    //calculate synchronization error
    auto error_time = actual_time_epoch - client_time_epoch;
    auto error_time_milis = std::chrono::duration_cast<std::chrono::microseconds>(error_time).count();
    std::cout << "Synchronization error : " << error_time_milis << " microseconds" << std::endl;
    closesocket(server_sockfd);
    WSACleanup();
    system("pause");
    return 0;
}
