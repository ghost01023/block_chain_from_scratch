#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdint.h>

#pragma comment(lib, "ws2_32.lib") // THE WINSOCK LIBARARY

#define MAX_BLOB_SIZE 1024 // TEMP

// START FLAG 10000000
// MAJOR VERSION 00000001
// MINOR VERSION 00000001
//
// DATA (4mb MAX)
// END FLAG 00000001

// Function to send a blob to a target device
int send_blob(const char *ip, int port, const void *blob, size_t blob_size)
{
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in server_addr;
    int result;

    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        fprintf(stderr, "WSAStartup failed: %d\n", result);
        return -1;
    }

    // Validate blob size
    if (blob_size > MAX_BLOB_SIZE)
    {
        fprintf(stderr, "Error: Blob size exceeds maximum of %d bytes.\n", MAX_BLOB_SIZE);
        WSACleanup();
        return -1;
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET)
    {
        fprintf(stderr, "Socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    // Configure server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
    {
        fprintf(stderr, "Invalid IP address.\n");
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        fprintf(stderr, "Connection to %s:%d failed: %d\n", ip, port, WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    // Send blob size
    if (send(sock, (const char *)&blob_size, sizeof(blob_size), 0) == SOCKET_ERROR)
    {
        fprintf(stderr, "Failed to send blob size: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    // Send blob data
    if (send(sock, (const char *)blob, blob_size, 0) == SOCKET_ERROR)
    {
        fprintf(stderr, "Failed to send blob data: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    printf("Blob successfully sent to %s:%d\n", ip, port);

    // Cleanup
    closesocket(sock);
    WSACleanup();
    return 0;
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

// Function to create packet for transmission via the A2A protocol
void create_packet(uint8_t nature_of_data, void *data)
{
    /*
    STRUCTURE OF A PACKET
    - FLAG_START
    - MAJOR VERSION
    - MINOR VERSION
    - NATURE OF DATA (block [0], transaction [1], mined_block [2], ack[3])
    - DATA
    - FLAG END
    */
    const uint8_t FLAG_START = 0x80; // START AS 10000000
    const uint8_t PROTOCOL_VERSION_MAJOR = 0x01;
    const uint8_t PROTOCOL_VERSION_MINOR = 0x00; // VERSION 1.0
    const uint8_t FLAG_END = 0x01;               // END AS 00000001
    const int TOTAL_BYTES_TO_ALLOCATE = (sizeof(uint8_t) * 5) + (sizeof(char) * strlen(data)) + 1;
    char *a2a_packet = malloc(TOTAL_BYTES_TO_ALLOCATE);
    *(a2a_packet + 0) = FLAG_START;
    *(a2a_packet + 1) = PROTOCOL_VERSION_MAJOR;
    *(a2a_packet + 2) = PROTOCOL_VERSION_MINOR;
    *(a2a_packet + 3) = nature_of_data;
    strcpy(a2a_packet + 4, data);
    *(a2a_packet + TOTAL_BYTES_TO_ALLOCATE - 2) = FLAG_END;
    *(a2a_packet + TOTAL_BYTES_TO_ALLOCATE - 1) = '\0';
    for (int i = 0; i < TOTAL_BYTES_TO_ALLOCATE; i++)
    {
        if (i < 4 || i == TOTAL_BYTES_TO_ALLOCATE - 2)
        {
            print_bits(*(a2a_packet + i));
            continue;
        }
        printf("%c", *(a2a_packet + i));
    }
    // printf("%s", a2a_packet);
    free(a2a_packet);
}

int main()
{
    char *data = "Hello, World! HOW DO YOU DO?";
    create_packet(0x00, data);
    // send_blob("192.168.1.5", 3009, data, sizeof(char) * strlen(data));
    return EXIT_SUCCESS;
}