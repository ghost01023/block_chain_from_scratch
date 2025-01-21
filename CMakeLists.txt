cmake_minimum_required(VERSION 3.10)

# Project name and settings
project(BlockChain C)

# Specify the executable target
add_executable(latest_build main.c)

# Link the ws2_32 library for Winsock support
target_link_libraries(latest_build ws2_32 ssl crypto z minizip sqlite3)