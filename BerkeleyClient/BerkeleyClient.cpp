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

int main()
{
    srand((unsigned int)time(NULL)); // avoid always same output of rand()
    float client_local_clock = rand() % 10; // range from 0 to 9

    WSADATA wsadata;
    struct sockaddr_in server_addr;
    int port = 9999;
    const char* ipadd = "127.0.0.1";
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ipadd);
    server_addr.sin_port = htons(port);

    int err = WSAStartup(0x202, (LPWSADATA)&wsadata); // 2.2 version
    if (err != 0)
    {
        printf("WSAStartup failed with error: %d\n", err);
    }

    // Create socket, set port and ip address
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0)
    {
        printf("Socket creation failed\n");
        return -1;
    }
    std::cout << "Socket successfully created" << std::endl << std::endl;

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
    {
        printf("\nClient: Invalid address/ Address not supported \n");
        return -1;
    }

    // Connecting server, return 0 with success, return -1 with error
    if (connect(server_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("\nClient: Connection Failed \n");
        return -1;
    }

    char client_read_buffer[1024] = {0};
    int valread = recv(server_sockfd, client_read_buffer, 1024, 0);
    std::cout << "Client received: " << client_read_buffer << std::endl;
    std::string msg_str;

    // First round communication (Send local clock value)
    if (strcmp(client_read_buffer, "Hello from server, please send local clock value") == 0)
    {
        // Prepare to send local clock value
        msg_str = "Hello from client, my local clock value is" + std::to_string(client_local_clock);
        const int new_msg_size = msg_str.length() + 1;
        char* msg = new char[new_msg_size];
        strcpy(msg, msg_str.c_str());

        // Send local clock value
        send(server_sockfd, msg, strlen(msg), 0);
        std::cout << "Client sent: " << msg << std::endl;
    }

    // Second round communication (Receive server clock value)
    valread = recv(server_sockfd, client_read_buffer, 1024, 0);
    std::cout << "Client received: " << client_read_buffer << std::endl;
    msg_str = std::string(client_read_buffer);

    if (msg_str.find("Hello from server, please") != std::string::npos)
    {
        std::string substr_after_lastbutone_space;
        std::string substr_after_last_space;
        std::vector<std::string> split_str = split(msg_str, " ");
        substr_after_lastbutone_space = split_str[split_str.size() - 2];
        substr_after_last_space = split_str[split_str.size() - 1];

        std::cout << "Client: received local clock adjustment offset (string) is " << substr_after_lastbutone_space <<
            " " << substr_after_last_space << std::endl;
        float substr_after_last_space_f = stof(substr_after_last_space);
        std::cout << "Client: received local clock adjustment offset (float) is " << substr_after_lastbutone_space <<
            " " << substr_after_last_space_f << std::endl;

        const int new_size = substr_after_lastbutone_space.length() + 1;
        char* oper_char_array = new char[new_size];
        strcpy(oper_char_array, substr_after_lastbutone_space.c_str());
        if (strcmp(oper_char_array, "add") == 0)
        {
            client_local_clock += substr_after_last_space_f;
        }
        else if (strcmp(oper_char_array, "minus") == 0)
        {
            client_local_clock -= substr_after_last_space_f;
        }

        printf("Client local clock is %f \n\n", client_local_clock);
    }
}
