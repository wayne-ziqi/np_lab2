//
// Created by ziqi on 2023/3/27.
//

#ifndef WORDGAME_GTCPCONTROLLER_H
#define WORDGAME_GTCPCONTROLLER_H

#include "TCPController.h"
#include <queue>

namespace GClient {
#define DEFAULT_IPSTR "127.0.0.1"
#define DEFAULT_PORT 20154
#pragma pack(1)
    struct SndPacket {
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
    };

    // has total length of 127
    struct RcvPacket {
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
        uint8_t flag;
        char user_name[30];
        char oppo_name[30];
        uint8_t oppo_move;
        uint8_t user_stamina;
        uint8_t oppo_stamina;

        [[nodiscard]] std::string getUserName() const {
            char tmp[31];
            memset(tmp, 0, sizeof(tmp));
            memcpy(tmp, user_name, 30);
            return {tmp};
        }
        [[nodiscard]] std::string getOppoName() const {
            char tmp[31];
            memset(tmp, 0, sizeof(tmp));
            memcpy(tmp, oppo_name, 30);
            return {tmp};
        }
    };

#pragma pack()

    class GTCPController : public TCPController {
    private:
        std::queue<SndPacket> queSnd;
        SndPacket snd_pkt{};
        RcvPacket rcv_pkt{};
    public:
        GTCPController() : TCPController(std::string(DEFAULT_IPSTR), DEFAULT_PORT) {}

        GTCPController(std::string &ip_str, int port) : TCPController(ip_str, port) {}

        void setSndPkt(int op, int flag, const std::string &user_name, const std::string &oppo_name, int user_move);

        void send_data() override;

        void recv_data() override;

        RcvPacket getRcvPkt();
    };

} // GClient

#endif //WORDGAME_GTCPCONTROLLER_H
