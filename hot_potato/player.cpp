
#include "player.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Error: wrong argument" << std::endl;
        return -1;
    }
    const char *hostname = argv[1];
    const char *port = argv[2];
    int socket_fd = connect_with_master(hostname, port);
    int num_players, player_id;
    recv(socket_fd, &player_id, sizeof(player_id), MSG_WAITALL);
    recv(socket_fd, &num_players, sizeof(num_players), MSG_WAITALL);
    std::cout << "Connected as player " << player_id << " out of " << num_players << " total players" << std::endl;
//  server
    int player_as_server_fd = init_server("");
    struct sockaddr_in addr;
    socklen_t size_addr = sizeof(addr);
    if (getsockname(player_as_server_fd, (struct sockaddr *) &addr, &size_addr) == -1) {
        std::cerr << "Error: getsockname error" << std::endl;
    }
    int assign_port = ntohs(addr.sin_port);
    send(socket_fd, &assign_port, sizeof(assign_port), 0);

    char receive_from_master_port[100];
    char receive_from_master_addr[1000];
    recv(socket_fd, &receive_from_master_port, sizeof(receive_from_master_port), MSG_WAITALL);
    //std::cout << "next port is: " << receive_from_master_port << std::endl;
    recv(socket_fd, &receive_from_master_addr, sizeof(receive_from_master_addr), MSG_WAITALL);
    //std::cout << "next addr is: " << receive_from_master_addr << std::endl;


    //initial client(要连接的是下一个player的port and addr) client_fd 是自己与next player的通信标识
    int client_fd = init_client(receive_from_master_port, receive_from_master_addr);
    int client_id = (player_id + 1) % num_players;

    std::string client_addr;
    int connect_fd = connect_with_client(player_as_server_fd, &client_addr);
    int server_id = (player_id - 1 + num_players) % num_players;


//pass potato
    Potato potato;

    std::vector<int> all_fds;
    all_fds.push_back(socket_fd);
    all_fds.push_back(client_fd);
    all_fds.push_back(connect_fd);

    do {
        fd_set readfds;
        int nfds;

        FD_ZERO(&readfds);
        nfds = all_fds[0];
        for (int i = 0; i < 3; i++) {
            FD_SET(all_fds[i], &readfds);
            if (all_fds[i] > nfds) {
                nfds = all_fds[i];
            }
        }
        select(nfds + 1, &readfds, NULL, NULL, NULL);
        for (int i = 0; i < 3; i++) {
            if (FD_ISSET(all_fds[i], &readfds)) {
                recv(all_fds[i], &potato, sizeof(potato), MSG_WAITALL);
                break;
            }
        }
        if (potato.hops != 0) {
            potato.hops--;
            potato.path[potato.round] = player_id;
            potato.round++;
            if (potato.hops == 0) {
                std::cout << "I'm it" << std::endl;
                send(socket_fd, &potato, sizeof(potato), 0);
            } else {
                srand((unsigned int) time(NULL) + potato.round);
                int random = rand() % 2;
                std::cout << "Sending potato to " << ((random == 0) ? server_id : client_id) << std::endl;
                send(((random == 0) ? connect_fd : client_fd), &potato, sizeof(potato), 0);
            }
        } else {
            break;
        }
    } while (true);

    for (int i = 0; i < all_fds.size(); i++) {
        close(all_fds[i]);
        return 0;
    }
}
