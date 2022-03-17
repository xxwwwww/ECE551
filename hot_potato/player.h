//
// Created by SDY on 2/13/2022.
//

#ifndef HOT_POTATO_PLAYER_H
#define HOT_POTATO_PLAYER_H

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <cassert>
#include "potato.h"


int connect_with_master(const char *hostname, const char *port) {
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        std::cerr << "Error: cannot get address info for host" << std::endl;
        return -1;
    }

    socket_fd = socket(host_info_list->ai_family,
                       host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    if (socket_fd == -1) {
        std::cerr << "Error: cannot create socket" << std::endl;
        return -1;
    }

    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        std::cerr << "Error: cannot connect to socket" << std::endl;
        return -1;
    }

    freeaddrinfo(host_info_list);
    return socket_fd;
}


int init_server(const char *port) {
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    const char *hostname = NULL;

    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags = AI_PASSIVE;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        std::cerr << "Error: cannot get address info for host" << std::endl;
        return -1;
    }
    socket_fd = socket(host_info_list->ai_family,
                       host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    if (socket_fd == -1) {
        std::cerr << "Error: cannot create socket" << std::endl;
        return -1;
    }

    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        std::cerr << "Error: cannot bind socket" << std::endl;
        return -1;
    }

    status = listen(socket_fd, 100);
    if (status == -1) {
        std::cerr << "Error: cannot listen on socket" << std::endl;
        return -1;
    }
    freeaddrinfo(host_info_list);
    return socket_fd;
}

int init_client(const char *port, const char *hostname) {

    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        std::cerr << "Error: cannot get address info for host" << std::endl;
        return -1;
    }
    socket_fd = socket(host_info_list->ai_family,
                       host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    if (socket_fd == -1) {
        std::cerr << "Error: cannot create socket" << std::endl;
        return -1;
    }
    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        std::cerr << "Error: cannot connect to socket" << std::endl;
        return -1;
    }
    freeaddrinfo(host_info_list);
    return socket_fd;
}


int connect_with_client(int connection_fd, std::string *ip_addr){
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    int client_connection_fd;
    client_connection_fd = accept(connection_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1) {
        std::cerr << "Error: cannot accept connection on socket" << std::endl;
        return -1;
    } //if
    *ip_addr = inet_ntoa(((struct sockaddr_in *)&socket_addr)->sin_addr);
    return client_connection_fd;
}





#endif //HOT_POTATO_PLAYER_H
