//
// Created by ziqi on 2023/3/8.
//

#include "TCPController.h"
#include "ClientException.h"
#include <iostream>

TCPController::TCPController(const std::string &ip_server, unsigned port_server) : ip_server(ip_server),
                                                                                   port_server(port_server),
                                                                                   connected(false){
    socket_fd = 0;
    bzero(&servAddr, sizeof servAddr);
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port_server);
    inet_pton(AF_INET, ip_server.c_str(), &servAddr.sin_addr);
}

void TCPController::reset(const std::string &ip_str, int port) {
    ip_server = ip_str;
    port_server = port;
    socket_fd = 0;
    bzero(&servAddr, sizeof servAddr);
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port_server);
    if (inet_pton(AF_INET, ip_str.c_str(), &servAddr.sin_addr) <= 0)
        throw CException(IPFMT_ERR, "Ip format error");
    connected = false;
}

TCPController::TCPController(const TCPController &controller) {
    ip_server = controller.ip_server;
    port_server = controller.port_server;
    servAddr = controller.servAddr;
    socket_fd = controller.socket_fd;
    connected = controller.connected;
}

void TCPController::do_connect() {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
        throw CException(SOCKET_ERR, "socket error");
    int ret = connect(socket_fd, (struct sockaddr *) &servAddr, sizeof servAddr);
    if (ret < 0)
        throw CException(CONNECT_FAIL, "connection failed");
    connected = true;
}







