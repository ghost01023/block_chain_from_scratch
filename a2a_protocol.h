// #ifndef A2A_PROTOCOL_H
// #define A2A_PROTOCOL_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ws2tcpip.h>
#include <stdint.h>
#include <pthread.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib") // THE WINSOCK LIBARARY

#define MAX_BLOB_SIZE 1024 // TEMP
#define START_FLAG_INDEX 0
#define PROTOCOL_VERSION_MAJOR_INDEX 1
#define PROTOCOL_VERSION_MINOR_INDEX 2
#define NATURE_OF_DATA_INDEX 3
#define SIZE_OF_DATA_INDEX 4
#define DATA_INDEX 5

// START FLAG 10000000
// MAJOR VERSION 00000001
// MINOR VERSION 00000001
//
// DATA (4mb MAX)
// END FLAG 00000001

typedef struct
{
    const char *ip;
    const int port;
    void *blob;
    const size_t blob_size;
} PacketConfig;

typedef struct
{
    const int empty;
} EmptyStruct;

// INITIATE SOCKET FOR DATA TRANSFER
void init_winsock()
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        printf("WSAStartup failed: %d\n", result);
        exit(1);
    }
}

// VERIFY PACKET CONTENT
void print_bits(char c)
{
    printf("\n");
    for (int i = 7; i >= 0; i--)
    {
        printf("%d", (c >> i) & 1);
    }
    printf("\n");
}

void *receive_packet(void *void_empty)
{
    int *exit_status = (int *)malloc(sizeof(int));
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET)
    {
        printf("Socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        *exit_status = 1;
        return exit_status;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(3009);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        *exit_status = 1;
        return exit_status;
    }

    if (listen(serverSocket, 1) == SOCKET_ERROR)
    {
        printf("Listen failed: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        *exit_status = 1;
        return exit_status;
    }
    printf("\nListening on port %d...\n", 3009);
    // Accept all incoming connections
    unsigned long time_start = time(NULL);
    while (1)
    {
        printf("\n%lu", time(NULL) - time_start);
        if (time(NULL) - time_start >= 60) // RUN FOR 1 MINUTE
        {
            break;
        }
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET)
        {
            printf("Accept failed: %d\n", WSAGetLastError());
            closesocket(serverSocket);
            WSACleanup();
            *exit_status = 1;
            return exit_status;
        }
        printf("Client connected\n");

        // Receive the blob (assuming it is small enough to fit in the buffer)
        char buffer[4096];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived == SOCKET_ERROR)
        {
            printf("Receive failed: %d\n", WSAGetLastError());
        }
        else
        {
            printf("Received %d bytes of data\n", bytesReceived);
            printf("Data received:\n");
            /*
            PROCESS THE DATA BASED ON OF IT IS A BLOCK OR TRANSACTION OR SOMETHING ELSE
            */
            int flag_end_index = bytesReceived - 1;
            for (int i = 0; i < bytesReceived; i++)
            {
                if (i < DATA_INDEX || i == flag_end_index)
                {
                    print_bits(buffer[i]);
                    continue;
                }
                printf("%c", buffer[i]);
                // for (int i = 0; i < bytesReceived; i++) {
                //     printf("%02X ", (unsigned char)buffer[i]);
                // }
            }
            printf("\n");
        }

        // Clean up
        closesocket(clientSocket);
    }
    closesocket(serverSocket);
    WSACleanup();
    *exit_status = 0;
    return exit_status;
}

// Function to send a blob to a target device
void *send_packet(void *void_packet)
{
    printf("\nAttempting to send packet...\n");
    int *exit_status = (int *)malloc(sizeof(int));
    PacketConfig *packet = (PacketConfig *)void_packet;
    // DESTRUCTURE PacketConfig STRUCT
    const char *ip = packet->ip;
    const int port = packet->port;
    const void *blob = packet->blob;
    const size_t blob_size = packet->blob_size;

    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in server_addr;
    int result;

    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        fprintf(stderr, "WSAStartup failed: %d\n", result);
        *exit_status = 1;
        return exit_status;
    }

    // Validate blob size
    if (blob_size > MAX_BLOB_SIZE)
    {
        fprintf(stderr, "Error: Blob size exceeds maximum of %d bytes.\n", MAX_BLOB_SIZE);
        WSACleanup();
        *exit_status = 1;
        return exit_status;
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        fprintf(stderr, "Socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        *exit_status = 1;
        return exit_status;
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        fprintf(stderr, "Invalid IP address.\n");
        closesocket(sock);
        WSACleanup();
        *exit_status = 1;
        return exit_status;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        fprintf(stderr, "Connection to %s:%d failed: %d\n", ip, port, WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        *exit_status = 1;
        return exit_status;
    }

    if (send(sock, (const char *)blob, blob_size, 0) == SOCKET_ERROR)
    {
        fprintf(stderr, "Failed to send blob data: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        *exit_status = 1;
        return exit_status;
    }

    printf("Blob successfully sent to %s:%d\n", ip, port);
    closesocket(sock);
    WSACleanup();
    *exit_status = 0;
    return exit_status;
}

// Function to create packet for transmission via the A2A protocol
void *create_packet(uint8_t nature_of_data, void *data)
{
    /*
    STRUCTURE OF A PACKET
    - FLAG_START
    - MAJOR VERSION
    - MINOR VERSION
    - NATURE OF DATA (block [1], transaction [2], mined_block [3], request[4])
    request in turn has data of 3 types: it's requesting block || transaction || wants to send mined block
    - SIZE OF DATA (in bytes)
    - DATA
    - FLAG END
    */
    const uint8_t FLAG_START = 0x80; // START AS 10000000
    const uint8_t PROTOCOL_VERSION_MAJOR = 0x01;
    const uint8_t PROTOCOL_VERSION_MINOR = 0x01; // VERSION 1.0
    const uint8_t FLAG_END = 0x01;               // END AS 00000001
    const int TOTAL_BYTES_TO_ALLOCATE = (sizeof(uint8_t) * 6) + (sizeof(char) * strlen((char *)data));
    printf("\n%d\n", TOTAL_BYTES_TO_ALLOCATE);
    char *a2a_packet = (char *)malloc(TOTAL_BYTES_TO_ALLOCATE);
    *(a2a_packet + 0) = FLAG_START;
    *(a2a_packet + 1) = PROTOCOL_VERSION_MAJOR;
    *(a2a_packet + 2) = PROTOCOL_VERSION_MINOR;
    *(a2a_packet + 3) = nature_of_data;
    // printf("%d", strlen(data) + 1);
    *(a2a_packet + 4) = strlen((char *)data);
    strcpy(a2a_packet + 5, (char *)data);
    *(a2a_packet + TOTAL_BYTES_TO_ALLOCATE - 1) = FLAG_END;
    /////////////////////////////////////////////////////
    /*
    CHECK IF PACKET BYTES ARE ARRANGED IN A VALID SEQUENCE
    */
    //////////////////////////////////////////////////////
    // for (int i = 0; i < TOTAL_BYTES_TO_ALLOCATE; i++)
    // {
    //     if (i < 5 || i == TOTAL_BYTES_TO_ALLOCATE - 1)
    //     {
    //         print_bits(*(a2a_packet + i));
    //         continue;
    //     }
    //     printf("%c", *(a2a_packet + i));
    // }
    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////
    return a2a_packet;
}

int send_block()
{
    printf("Sent block!");
    return EXIT_SUCCESS;
}

int broadcast_transaction()
{
    printf("Transaction broadcasted!");
    return EXIT_SUCCESS;
}

// #endif