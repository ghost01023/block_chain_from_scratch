cmake_minimum_required(VERSION 3.10)

# Project name and settings
project(BlockChain C)

# Specify the executable target
add_executable(send_blob send_blob.c)

# Link the ws2_32 library for Winsock support
target_link_libraries(send_blob ws2_32)