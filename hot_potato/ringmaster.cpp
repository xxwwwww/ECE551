#include <iostream>
#include "ringmaster.h"
#include "potato.h"

int main(int argc, char *argv[]) {
    //check the argument number
    if (argc != 4) {
        std::cerr << "Error: wrong argument" << std::endl;
        return 1;
    }

    //check players and hops and print a msg to start the game
    int num_players = atoi(argv[2]);
    int num_hops = atoi(argv[3]);
    if (num_players <= 1) {
        std::cerr << "Error: wrong player number" << std::endl;
        return 1;
    }
    if (num_hops < 0 || num_hops > 512) {
        std::cerr << "Error: wrong num of hops" << std::endl;
        return 1;
    }
    print_initial_info(num_players, num_hops);
    const char *port = argv[1];
    std::vector<int> player_fds;
    std::vector<int> player_ports;
    std::vector<std::string> player_addrs;
    int socket_fd = connect_with_players(port, num_players, num_hops, player_fds, player_ports, player_addrs);
    generate_circle(num_players, player_fds, player_ports, player_addrs);
    Potato potato;
    potato.hops = num_hops;
    send_potato(potato, num_players, player_fds);
    close(socket_fd);
    return 0;
}
