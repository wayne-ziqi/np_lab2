//
// Created by ziqi on 2023/3/25.
//

#ifndef WORDGAME_SERVERERROR_H
#define WORDGAME_SERVERERROR_H

#include <exception>
#include <string>
#include <utility>

enum ExcNo {
    PLAY1_DUP_MV,   // player1 has sent a duplicate move request
    PLAY2_DUP_MV,   // player2 has sent a duplicate move request
    COMP_DID_END,
    COMP_FULL,
    COMP_MISS,
    COMP_LCH_ERR,
    COMP_MISMATCH,
    COMP_NOT_READY,
    BIND_ERR,
    LISTEN_ERR,
    CLIENT_SOCKET_ERR,
    RCV_INV,
    SND_ERR,
    USR_MISS,
    USR_BUSY,
    USER_OUT,
    OPPO_BUSY,
    OPPO_IDLE
};

struct SException : std::exception {
    ExcNo excNo;
    std::string message;

    SException(ExcNo err, std::string message) : excNo(err), message(std::move(message)) {}
};



#endif //WORDGAME_SERVERERROR_H
