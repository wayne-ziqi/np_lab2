//
// Created by ziqi on 2023/3/27.
//

#ifndef WORDGAME_CLIENTEXCEPTION_H
#define WORDGAME_CLIENTEXCEPTION_H

#include <exception>
#include <string>

enum ExcNo {
    SOCKET_ERR,
    RCV_INV,
    CONNECT_CLOSE,
    CONNECT_FAIL,
    SEND_FAIL,
    NAME_ERR,
    IPFMT_ERR,
    USR_STAT_ERR,
};

struct CException : std::exception {
    ExcNo excNo;
    std::string message;

    CException(ExcNo err, std::string message) : excNo(err), message(std::move(message)) {}
};

#endif //WORDGAME_CLIENTEXCEPTION_H
