//
// Created by ziqi on 2023/3/8.
//

#ifndef WEATHERCLIENT_TCPCONTROLLER_H
#define WEATHERCLIENT_TCPCONTROLLER_H

#include <string>
#include "lib/common.h"

class TCPController {
protected:
    std::string ip_server;
    unsigned port_server;
    struct sockaddr_in servAddr{};
    int socket_fd;
    bool connected;
public:

    TCPController(const std::string& ip_server, unsigned port_server);

    TCPController(const TCPController &controller);

    void reset(const std::string &ip_str, int port);

    void do_connect();

    virtual void send_data()=0;

    virtual void recv_data()=0;

};


#endif //WEATHERCLIENT_TCPCONTROLLER_H
