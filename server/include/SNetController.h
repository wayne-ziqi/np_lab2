//
// Created by ziqi on 2023/3/25.
//

#ifndef WORDGAME_SNETCONTROLLER_H
#define WORDGAME_SNETCONTROLLER_H

/// server net controller base class
#include "lib/common.h"
#include <string>

namespace GServer {

# pragma pack(1)

    struct RcvPacket {
        // 0x01: register,
        // 0x02: log out game
        // 0x03: make move,
        // 0x04: check opponent's state,
        // 0x05: request a challenge to opponent
        uint8_t op;
        // op=0x04(check opponent state): 0x01: certain player, 0x02: all players
        // op=0x05(request a challenge to opponent) 0x01: send a request to opponent, 0x02: opponent's challenge accepted, 0x03: opponent's challenge declined
        uint8_t flag;
        char user_name[30];
        char oppo_name[30];
        uint8_t user_move;   //0x01: rock, 0x02: paper, 0x03: scissors

        [[nodiscard]] std::string getUserName() const {
            char tmp_name[31];
            memset(tmp_name, 0, sizeof tmp_name);
            memcpy(tmp_name, user_name, 30);
            return {tmp_name};
        }

        [[nodiscard]] std::string getOppoName() const {
            char tmp_name[31];
            memset(tmp_name, 0, sizeof tmp_name);
            memcpy(tmp_name, oppo_name, 30);
            return {tmp_name};
        }
    };

    struct SndPacket {
        // 0x01: response to the registration
        // 0x02: response to quit game
        // 0x03: response to the move
        // 0x04: response to the opponent's state
        // 0x05: response to the challenge request
        // 0x06: information and warnings
        uint8_t op;
        // op = 0x01 (rsp to register): 0x01: registration succeed, 0x02: registration failed, change user's name
        // op = 0x02 (rsp to quit game): 0x01: log out succeed, 0x02: out fail
        // op = 0x03 (rsp to move):  0x01: user win, 0x02: user lose, 0x03: even, 0x04: not finished ,move stored in oppo_move
        // op = 0x04 (rsp to opponent's state): 0x01: available, 0x02: occupied, 0x03: offline, 0x04: end of sending for all players
        // op = 0x05 (rsp to challenge request): 0x01: request sent from the opponent, 0x02: request accepted, 0x03: request declined
        // op = 0x06 (send broadcast) 0x01: the opponent is online, 0x02: the opponent is offline
        uint8_t flag;
        char user_name[30];
        char oppo_name[30];
        uint8_t oppo_move;
        uint8_t user_stamina;
        uint8_t oppo_stamina;

        SndPacket() {
            memset(this, 0x0, sizeof(struct SndPacket));
        }

        void setUserName(const std::string &name) {
            memset(user_name, 0x00, sizeof(user_name));
            memcpy(user_name, name.c_str(), name.length());
        }

        [[nodiscard]] std::string getUserName() const {
            char tmp_name[31];
            memset(tmp_name, 0, sizeof tmp_name);
            memcpy(tmp_name, user_name, 30);
            return {tmp_name};
        }

        [[nodiscard]] std::string getOppoName() const {
            char tmp_name[31];
            memset(tmp_name, 0, sizeof tmp_name);
            memcpy(tmp_name, oppo_name, 30);
            return {tmp_name};
        }

        void
        setBasicData(int snd_op, int snd_flag, const std::string &snd_user_name, const std::string &snd_oppo_name) {
            this->op = snd_op, this->flag = snd_flag;
            memcpy(user_name, snd_user_name.c_str(), snd_user_name.length());
            memcpy(oppo_name, snd_oppo_name.c_str(), snd_oppo_name.length());
        }
    };

#pragma pack()

    class SNetController {
    private:
        struct sockaddr_in server_address{};
        int socket_fd;
        std::string ip_addr;
        int port;
    public:
        SNetController();

        /**
         * bind socket for server
         */
        void do_bind();

        /**
         * server listen on socket
         */
        void do_listen();

        int do_accept(struct sockaddr_in *client_address, socklen_t *clilen);

        void send_data(int client_socket, SndPacket pkt);

    };
}


#endif //WORDGAME_SNETCONTROLLER_H
