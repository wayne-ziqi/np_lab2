//
// Created by ziqi on 2023/3/25.
//

/// server designed for the word game

#ifndef WORDGAME_SERVER_H
#define WORDGAME_SERVER_H

#include "SNetController.h"
#include "Competition.h"
#include <utility>
#include <list>
#include <map>
#include <queue>
#include <thread>

namespace GServer {
#define MAX_CLIENT 10
#define MAX_COMP (MAX_CLIENT / 2)


    class Server {
    private:
        std::map<std::string, Player> players;
        std::map<int, Competition> competitions;
        std::queue<int> queAviaID;
        // any access to players and competitions should use mutex
        std::mutex usr_mtx;     // used to add/look up some player
        std::mutex comp_mtx;    // used to add/look up some competition
        std::mutex play_mtx;    // used to update player's state

        int getAvaiCompID();

        void clientHandler(int client_socket);

        void logInPlayer(const std::string &name, int socket_fd);

        void logOutPlayer(const std::string &name, int socket_fd);

        // return the player's competition id, if available, return 0
        Player *checkUserState(const std::string &name, const std::string &asker, int askerSocket);

        // check if the user is offline, if so send the offline msg to askerSocket
        Player *checkUserPresence(const std::string &name, const std::string &asker, int askerSocket);

        Competition *addNewCompetition();

        Competition *getCompetition(int id);

        void delCompetition(int id);

        void handleChallenge(const std::string &user_name, int socket_fd, RcvPacket &rcv_packet);

        // broadcast will not check if the packet is sent successfully
        void sendBroadCast(SndPacket &snd_pkt);


    protected:
        // controller to receive data from clients
        SNetController netController;

    public:

        Server();

        void run();

    };
}


#endif //WORDGAME_SERVER_H
