#include "system_setup.h"
#include "sqlite_database.h"
#include "a2a_protocol.h"
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>

bool is_folder_present(const char *folder_name)
{
    struct stat st;
    // Check if the folder exists and is a directory
    if (stat(folder_name, &st) == 0 && S_ISDIR(st.st_mode))
    {
        return true;
    }
    return false;
}

int system_setup_sqlite_and_path()
{
    /*
    when the program is run for the first time, we'll check
    whether or not sqlite has been setup already.
    if not, we run the following extractors and add
    sqlite to the PATH environment variables
    */
    const char *arch_32_dll = "sqlite-dll-win-x86-3480000.zip";
    const char *arch_64_dll = "sqlite-dll-win-x64-3480000.zip";
    const char *binaries = "sqlite-tools-win-x64-3480000.zip";
    if (get_windows_architecture() == 32)
    {
        extract_zip_file(arch_32_dll);
    }
    else
    {
        extract_zip_file(arch_64_dll);
    }
    extract_zip_file(binaries);
    add_to_path("C:\\SQLite");
    /*
    FINISH SQLITE + PATH SETUP
    */

    return EXIT_SUCCESS;
}

int main()
{
    if (!is_folder_present("SQLite"))
    {
        system_setup_sqlite_and_path();
    }
    /*CHECKS FOR VALID QUERY CONSTRUCTION*/
    // fetch_block("hello_block");
    // insert_block("hello", "timestamp", "miner_id", "merkle_root is this", "this is the nonce value", "this is the hash of the block", "this is the block of the previous block, linked-list, baby!");
    // fetch_transactions("trans");
    // insert_transaction("tr_id", "block_id", "sender is this", "this is recv", "amount", "now the raw amount", "timi-timey", "hash of transaction", "signed with love", "public key");
    // delete_transaction("delete_down_this");
    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////
    /*VALIDATE A2A PROTOCOL COMMS*/
    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////
    init_winsock();
    char *data = "\\BLOCK_HEIGHT=00001232;\\SEQUENCE_POSITION=28478274;";
    char *packet = create_packet(0x01, data); // SPECIFY THAT THIS IS A BLOCK
    pthread_t thread_send_packet, thread_receive_packet, thread_mine_block;
    // printf("\nStarting thread to send packet...\n");
    // PacketConfig packet_config = {"192.168.1.5", 3009, packet, strlen(packet)};
    // pthread_create(&thread_send_packet, NULL, send_packet, &packet_config);
    printf("\n\nStarting thread to receive packet...\n");
    EmptyStruct empty = {1};
    pthread_create(&thread_receive_packet, NULL, receive_packet, &empty);
    // pthread_join(thread_send_packet, NULL);
    pthread_join(thread_receive_packet, NULL);
    free(packet);
    // send_packet("192.168.1.5", 3009, packet, strlen(packet));

    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////
    return EXIT_SUCCESS;
}