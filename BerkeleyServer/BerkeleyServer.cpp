// C++ equivalent
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <winsock2.h>
#include <stdio.h>
#include <Ws2tcpip.h>
#include <vector>
// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996)

std::vector<std::string> split(std::string s, std::string delimiter)
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
    {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

// Code for master
int main()
{
    // Init WSADATA
    WSADATA wsadata;

    // Prepare to store client data
    std::vector<float> clients_local_clocks;
    std::vector<int> client_sockets;
    std::vector<std::string> client_ips;
    std::vector<int> client_ports;

    // Setup Server socket
    struct sockaddr_in server_addr;
    int port = 9999;
    const char* ipadd = "127.0.0.1";
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ipadd);
    server_addr.sin_port = htons(port);

    // Startup WSA 
    const int err = WSAStartup(0x101, (LPWSADATA)&wsadata); // 2.2 version
    if (err != 0)
    {
        printf("WSAStartup failed with error: %d\n", err);
    }

    // Create socket and set port
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd == 0)
    {
        std::cout << "Socket creation failed - ERR " << WSAGetLastError() << std::endl;
        return -1;
    }
    std::cout << "Socket at master node successfully created" << std::endl;

    // Bind socket to port
    if (bind(server_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        std::cout << "Socket bind failed - ERR " << WSAGetLastError() << std::endl;
        return -1;
    }
    std::cout << "Clock server is listening..." << std::endl;

    // Start listening to requests
    if (listen(server_sockfd, 5) < 0)
    {
        std::cout << "Socket listen failed - ERR " << WSAGetLastError() << std::endl;
        return -1;
    }

    int clients_counter = 0;
    char recv_buf[65536];
    bool flag = true;

    // Start receiving client
    while (flag)
    {
        // Block on accept() until positive fd or error
        struct sockaddr_in client_addr;
        int client_len = sizeof(client_addr);
        int client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_addr, &client_len);
        if (client_sockfd < 0)
        {
            std::cout << "Socket accept failed - ERR " << WSAGetLastError() << std::endl;
            return -1;
        }


        // Convert info to save
        char client_addr_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_addr_str, INET_ADDRSTRLEN);
        std::cout << "Server: New connection from " << client_addr_str << ":" << ntohs(client_addr.sin_port) <<
            std::endl;
        clients_counter++;
        std::cout << clients_counter << " Client " << " connected" << std::endl;

        // Store client info into vectors
        client_sockets.push_back(client_sockfd);
        client_ips.push_back(client_addr_str);
        client_ports.push_back(ntohs(client_addr.sin_port));

        std::cout << "Current clients amount : " << int(client_sockets.size()) << std::endl;
        std::cout << "Do you want to add more client? 1 for yes, 0 for no" << std::endl;
        int choice;
        std::cin >> choice;
        if (choice == 0)
        {
            flag = false;
        }
    }

    // End receiving client

    std::cout << "Server:" << int(client_sockets.size()) << "  clients connected" << std::endl;
    std::cout << "Server: Waiting for clients to send data..." << std::endl;

    // Start receiving data from clients
    for (int i = 0; i < client_sockets.size(); i ++)
    {
        // Message from server
        const char* message = "Hello from server, please send local clock value";
        send(client_sockets[i], message, strlen(message), 0);
        std::cout << "Server: Message sent to client " << i << std::endl;

        // Receive data from client
        while (recv(client_sockets[i], recv_buf, sizeof(recv_buf), 0) > 0)
        {
            std::cout << "Server: Received data from client " << i << std::endl;
            std::cout << "Server: Data received: " << recv_buf << std::endl;

            std::string recv_msg = std::string(recv_buf);

            if (recv_msg.find("Hello from client, my local clock value is") != std::string::npos)
            {
                std::string substr_after_last_space;
                std::vector<std::string> split_str = split(recv_msg, " ");
                substr_after_last_space = split_str[split_str.size() - 1];

                std::cout << "Server: Client " << i << " local clock value is (string)" << substr_after_last_space <<
                    std::endl;
                float substr_after_last_space_f = std::stof(substr_after_last_space);
                std::cout << "Server: Client " << i << " local clock value is (float)" << substr_after_last_space_f <<
                    std::endl;
                clients_local_clocks.push_back(substr_after_last_space_f);
            }

            memset(recv_buf, '\0', strlen(recv_buf));
            break;
        }
    }

    // End receiving data from clients

    // Calculate average clock values
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    float all_clock_sum = time;
    for (int i = 0; i < clients_local_clocks.size(); i++)
    {
        all_clock_sum += clients_local_clocks[i];
    }
    float avg_clock = all_clock_sum / (clients_local_clocks.size() + 1);

    // Tell clients to update their clocks
    for (int i = 0; i < client_sockets.size(); i++)
    {
        float offset = clients_local_clocks[i] - avg_clock;
        std::string operation;
        if (offset >= 0)
        {
            operation = "add";
        }
        else
        {
            operation = "subtract";
        }
        std::string msg_str = "Hello from server, please " + operation + " " + std::to_string(abs(offset)) +
            " seconds to your local clock";
        char msg[65536];
        strcpy(msg, msg_str.c_str());

        // Send message to client
        send(client_sockets[i], msg, strlen(msg), 0);
        std::cout << "Server: Message sent to client " << i << std::endl;
    }

    // Stop server
    closesocket(server_sockfd);
    WSACleanup();
}
