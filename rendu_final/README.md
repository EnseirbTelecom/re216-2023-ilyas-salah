# Chat Application Project

## Introduction

This project is a chat application implemented in C, facilitating communication between a server and multiple clients. The application allows users to connect, choose a pseudo, view online users, retrieve user information, send private messages, and communicate in channels. The project is structured into several components for clarity and modularity.

## Dependencies
To run this project, you will need to have the following dependencies installed:

- `CMake`

## How to Build and Run
To build and run the project, follow these steps:

1. Open a terminal window and navigate to the project directory.

2. run the CMake: `cmake CMakeLists.txt`.

3. Run make: `make`

6. Run the server program: `./server <Numero_port>`

7. Run the client program: `./client <@IP_server> <Numero_Port> `

## Project Structure

The project is organized into several header files and source files:

### Server-Side Components

- **common.h**: A common header file includes msg_struct.h and being imported in every C and H file.
- **msg_struct.h**: Defines structures for messaging between clients and the server.
- **clients_base.h**: Manages the functionality of clients, handling connections between them.
- **channels_base.h**: Manages the functionality of channels, allowing clients to join and communicate within them.
- **handle_msg.h**: Contains functions implementing a simple communication protocol, sending the message structure and payload.
- **communicate.h**: Gathers functionalities from channels_base.h, clients_base.h, and handle_msg.h. It handles client requests on the server side.

### Client-Side Components

- **file_base.h**: Manages the implementation of the peer-to-peer (P2P) file transfer protocol.

## Project Milestones

### Jalon 1: Basic Setup

- Establishes a basic communication setup between clients and the server.

### Jalon 2: User Information Management

- Clients can choose a pseudo.
- View pseudos of other online users.
- Retrieve information about users.
- Send private messages to a specific user or broadcast messages to all users.

### Jalon 3: Channel System

- Added a channel system for clients to join and communicate.
- Uses a linked list to manage channels, each containing basic information and a list of clients.

### Jalon 4: P2P File Transfer

- Implemented a P2P file transfer system between clients.
- Supports sending various file types, including large files.

### Bonus: User Interface and Experience and Memory Optimization

- Enhanced the user interface with colors.
- Implemented a loading bar for file transfers.
- Improved user experience with server-side handling of incorrect commands and suggestions.
- Optimized memory usage in channel implementation.
- Limited the number of clients in a channel for optimal memory utilization.
- Connection between two clients in every local network (even if they have ip adresses distinct for example: two laptops connected with RG45 cable).

### Note:

- The application was tested for two laptops connected with RG45 cable.

## License

This project is licensed under the [MIT License](LICENSE).
