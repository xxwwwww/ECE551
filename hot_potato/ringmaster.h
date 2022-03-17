
#ifndef HOT_POTATO_RINGMASTER_H
#define HOT_POTATO_RINGMASTER_H
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

/* ********* ringmaster part ******************* */
void print_initial_info(int num_players,int num_hops){
    std::cout<<"Potato Ringmaster" << std::endl;
    std::cout<<"Players = "<< num_players << std::endl;
    std::cout<<"Hops = "<< num_hops << std::endl;
}

int connect_with_players(const char *server_port,int num_players,int num_hops,std::vector<int> &player_fds,std::vector<int> &player_ports,std::vector<std::string> &player_addrs){
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    const char *hostname = NULL;

    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags    = AI_PASSIVE;

    status = getaddrinfo(hostname, server_port, &host_info, &host_info_list);
    if (status != 0) {
        std::cerr << "Error: cannot get address info for host" << std::endl;
        return -1;
    }
    socket_fd = socket(host_info_list->ai_family,
                       host_info_list->ai_socktype,
                       host_info_list->ai_protocol);
    if (socket_fd == -1) {
        std::cerr << "Error: cannot create socket" << std::endl;
        return -1 ;
    }

    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        std::cerr << "Error: cannot bind socket" << std::endl;
       return -1;
    }
    freeaddrinfo(host_info_list);
    status = listen(socket_fd, 100);
    if (status == -1) {
        std::cerr << "Error: cannot listen on socket" << std::endl;
       return -1;
    }
    //accept connections from players

    for(int player_id = 0; player_id < num_players; player_id++){
        std::string ip_addr;
        int port;
        int connect_player_fd;
        struct sockaddr_storage socket_addr;
        socklen_t socket_addr_len = sizeof(socket_addr);
        connect_player_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
        if (connect_player_fd == -1) {
            std::cerr << "Error: cannot accept connection on socket" << std::endl;
            return -1;
        }
        ip_addr = inet_ntoa(((struct sockaddr_in *)&socket_addr)->sin_addr);
        send(connect_player_fd,&player_id,sizeof(player_id),0);
        send(connect_player_fd, &num_players, sizeof(num_players),0);
        // recv listening port of player i
        recv(connect_player_fd,&port,sizeof(port),MSG_WAITALL);


        player_fds.push_back(connect_player_fd);
        player_addrs.push_back(ip_addr);
        player_ports.push_back(port);

        std::cout<<"Player "<<player_id<<" is ready to play"<<std::endl;
    }
    return socket_fd;
}
void generate_circle(int num_players,std::vector<int> &player_fds,std::vector<int> &player_ports, std::vector<std::string> &player_addrs) {
    for (int player_id = 0; player_id < num_players; player_id++) {
        if (player_id == num_players - 1) {
            char next_port[100];
            sprintf(next_port, "%d", player_ports[0]);
            char next_addr[1000];
            memset(next_addr, 0, sizeof(next_addr));
            strcpy(next_addr, player_addrs[0].c_str());
            send(player_fds[player_id], &next_port, sizeof(next_port), 0);
            send(player_fds[player_id], next_addr, sizeof(next_addr), 0);
        } else {
            int next_player_port = player_ports[player_id + 1];
            char next_port[100];
            sprintf(next_port, "%d", next_player_port);
            char next_addr[1000];
            memset(next_addr, 0, sizeof(next_addr));
            strcpy(next_addr, player_addrs[player_id + 1].c_str());
            send(player_fds[player_id], &next_port, sizeof(next_port), 0);
            send(player_fds[player_id], &next_addr, sizeof(next_addr), 0);
        }
    }
}


void print_path(Potato &potato) {
    printf("Trace of potato:\n");
    for (int i = 0; i < (potato.round-1); i++) {
        std::cout << potato.path[i] << "," ;
    }
    std::cout<< potato.path[potato.round -1 ] <<std::endl;

}


void wait_the_end(std::vector<int>player_fds,Potato &potato){
//    create fd_set know when any of the sockets in the set is ready to recv() data
    fd_set readfds;
//    clear the set
    FD_ZERO(&readfds);
//    add descripters in the readfds : fd_set()
    for(int i=0;i<player_fds.size();i++){
        FD_SET(player_fds[i],&readfds);
    }
    int nfds = player_fds[0]+1;
    select(nfds+1,&readfds,NULL,NULL,NULL);
    for (int i = 0; i<player_fds.size();i++) {
        if (FD_ISSET(player_fds[i], &readfds)) {
            recv(player_fds[i], &potato, sizeof(potato), MSG_WAITALL);
            break;
        }
    }
}

void send_potato(Potato potato, int num_players,std::vector<int> player_fds){
    if(potato.hops == 0){
        for (int i = 0; i < player_fds.size(); i++) {
            close(player_fds[i]);
        }
    }
    else {
        assert(num_players>=1);
        srand((unsigned int) time(NULL) + num_players);
        int random_start_player = rand() % num_players;
        std::cout << "Ready to start the game, sending potato to player " << random_start_player << std::endl;
        send(player_fds[random_start_player], &potato, sizeof(potato), 0);
        wait_the_end(player_fds,potato);
        for (int fd : player_fds) {
            send(fd, &potato, sizeof(potato), 0);
        }
        print_path(potato);
    }
    for (int i = 0; i < player_fds.size(); i++) {
        close(player_fds[i]);
    }
}









#endif //HOT_POTATO_RINGMASTER_H
