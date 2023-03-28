//
// Created by ziqi on 2023/3/25.
//

#ifndef WORDGAME_PLAYER_H
#define WORDGAME_PLAYER_H

#include <string>
#include <utility>

namespace GServer {
    class Player {
    private:
        std::string name;
        int inCompID;   // mark the game the player participating in， -1 means available
        int client_socket;
    public:
        Player() : inCompID(0), client_socket(0) {}

        Player(std::string name, int socket_fd) : name(std::move(name)), inCompID(0), client_socket(socket_fd) {}

        void join_comp(int game_id) { inCompID = game_id; }

        void quit_comp() { inCompID = 0; }

        [[nodiscard]] std::string get_name() const { return name; }

        [[nodiscard]] int get_compID() const { return inCompID; }

        [[nodiscard]] int get_socket() const { return client_socket; }
    };
}

#endif //WORDGAME_PLAYER_H
