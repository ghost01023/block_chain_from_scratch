#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdint.h>
#include <pthread.h>

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

// VERIFY PACKET CONTENTS
void print_bits(char c)
{
    printf("\n");
    for (int i = 7; i >= 0; i--)
    {
        printf("%d", (c >> i) & 1);
    }
    printf("\n");
}

// Function to receive a blob
void *receive_packet(void *void_empty)
{
    // Create socket
    int *exit_status = malloc(sizeof(int));
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET)
    {
        printf("Socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        *exit_status = 1;
        return exit_status;
    }

    // Bind the socket to a specific port (3309)
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(3009);
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        *exit_status = 1;
        return exit_status;
    }

    // Listen for incoming connections
    if (listen(serverSocket, 1) == SOCKET_ERROR)
    {
        printf("Listen failed: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        *exit_status = 1;
        return exit_status;
    }
    printf("Listening on port %d...\n", 3009);

    for (int i = 0; i < 15; i++)
    { // Receive the blob (assuming it is small enough to fit in the buffer)
        // Accept an incoming connection
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET)
        {
            printf("Accept failed: %d\n", WSAGetLastError());
            closesocket(serverSocket);
            WSACleanup();
            *exit_status = 1;
            return exit_status;
        }
        printf("Client connected with iteration number: %d\n", i);

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
            int flag_end_index = bytesReceived - 1;
            for (int i = 0; i < bytesReceived; i++)
            {
                // if (i < DATA_INDEX || i == flag_end_index)
                // {
                //     print_bits(buffer[i]);
                //     continue;
                // }
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
    int *exit_status = malloc(sizeof(int));
    PacketConfig *packet = void_packet;
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

    // // Send blob size
    // if (send(sock, (const char *)&blob_size, sizeof(blob_size), 0) == SOCKET_ERROR)
    // {
    //     fprintf(stderr, "Failed to send blob size: %d\n", WSAGetLastError());
    //     closesocket(sock);
    //     WSACleanup();
    //     return -1;
    // }

    // Send blob data
    if (send(sock, (const char *)blob, blob_size, 0) == SOCKET_ERROR)
    {
        fprintf(stderr, "Failed to send blob data: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        *exit_status = 1;
        return exit_status;
    }

    printf("Blob successfully sent to %s:%d\n", ip, port);

    // Cleanup
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
    const int TOTAL_BYTES_TO_ALLOCATE = (sizeof(uint8_t) * 6) + (sizeof(char) * strlen(data));
    printf("\n%d\n", TOTAL_BYTES_TO_ALLOCATE);
    char *a2a_packet = malloc(TOTAL_BYTES_TO_ALLOCATE);
    *(a2a_packet + 0) = FLAG_START;
    *(a2a_packet + 1) = PROTOCOL_VERSION_MAJOR;
    *(a2a_packet + 2) = PROTOCOL_VERSION_MINOR;
    *(a2a_packet + 3) = nature_of_data;
    // printf("%d", strlen(data) + 1);
    *(a2a_packet + 4) = strlen(data);
    strcpy(a2a_packet + 5, data);
    *(a2a_packet + TOTAL_BYTES_TO_ALLOCATE - 1) = FLAG_END;
    for (int i = 0; i < TOTAL_BYTES_TO_ALLOCATE; i++)
    {
        if (i < 5 || i == TOTAL_BYTES_TO_ALLOCATE - 1)
        {
            print_bits(*(a2a_packet + i));
            continue;
        }
        printf("%c", *(a2a_packet + i));
    }
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

int main()
{
    init_winsock();
    char *data = "WANI\\BLOCK_HEIGHT=00001232;\\SEQUENCE_POSITION=28478274;";
    char *packet = create_packet(0x01, data); // SPECIFY THAT THIS IS A BLOCK
    pthread_t thread_send_packet[10], thread_receive_packet, thread_mine_block;
    // printf("\nStarting thread to send packet...\n");
    PacketConfig packet_config = {"192.168.1.5", 3009, packet, strlen(packet)};
    for (int i = 0; i < 10; i++)
    {
        pthread_create(&thread_send_packet[i], NULL, send_packet, &packet_config);
    }
    printf("\nStarting thread to receive packet...\n");
    EmptyStruct empty = {1};
    pthread_create(&thread_receive_packet, NULL, receive_packet, &empty);
    for (int i = 0; i < 10; i++)
    {
        pthread_join(thread_send_packet[i], NULL);
    }
    pthread_join(thread_receive_packet, NULL);
    free(packet);
    // send_packet("192.168.1.5", 3009, packet, strlen(packet));
    return EXIT_SUCCESS;
}