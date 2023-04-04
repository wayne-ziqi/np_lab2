//
// Created by ziqi on 2023/3/27.
//
#include <iostream>
#include "GTCPController.h"
#include "ClientException.h"

namespace GClient {

    void GTCPController::send_data() {
//        if (queSnd.empty())return;
//        SndPacket sndPacket = queSnd.front();
//        queSnd.pop();
        ssize_t n_written = send(socket_fd, &snd_pkt, sizeof(struct SndPacket), 0);
        if (n_written <= 0) {
            std::string errInfo("send request failed for ");
            errInfo.append((char *) snd_pkt.user_name);
            throw CException(SEND_FAIL, errInfo);
        }
        std::cout << "data sent with op: " << (int) snd_pkt.op << std::endl;
    }

    void GTCPController::recv_data() {
        memset(&rcv_pkt, 0x00, sizeof(struct RcvPacket));
        ssize_t read_n = recv(socket_fd, &rcv_pkt, sizeof(struct RcvPacket), MSG_DONTWAIT); // 0: wait
        if (read_n == 0) {
            connected = false;
            throw CException(CONNECT_CLOSE, "connection has been closed");
        } else if (read_n < 0)
            throw CException(RCV_INV, "receive error");

    }

    void GTCPController::setSndPkt(int op, int flag, const std::string &user_name, const std::string &oppo_name,
                                   int user_move = 0) {
        memset(&snd_pkt, 0x00, sizeof(struct SndPacket));
        snd_pkt.op = op;
        snd_pkt.flag = flag;
        snd_pkt.user_move = user_move;
        memcpy(&snd_pkt.user_name, user_name.c_str(), user_name.length());
        memcpy(&snd_pkt.oppo_name, oppo_name.c_str(), oppo_name.length());
//        queSnd.push(snd_pkt);
    }

    RcvPacket GTCPController::getRcvPkt() {
        return rcv_pkt;
    }

    void GTCPController::setSndPkt(int op, int flag, const std::string &user_name, const std::string &oppo_name,
                                   const std::string &msg) {
        memset(&snd_pkt, 0x00, sizeof(struct SndPacket));
        snd_pkt.op = op;
        snd_pkt.flag = flag;
        memcpy(&snd_pkt.user_name, user_name.c_str(), user_name.length());
        memcpy(&snd_pkt.oppo_name, oppo_name.c_str(), oppo_name.length());
        memcpy(&snd_pkt.msg, msg.c_str(), msg.length());
    }
} // GClient