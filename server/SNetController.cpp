//
// Created by ziqi on 2023/3/25.
//
#include "SNetController.h"
#include "ServerException.h"
#include <iostream>

namespace GServer {

    SNetController::SNetController() {
        port = 20154;
        ip_addr = "127.0.0.1";
        memset(&server_address, 0, sizeof(server_address));
        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port);
        server_address.sin_addr.s_addr = htonl(INADDR_ANY);
        std::cout << "[Server] settings complete\n";
    }

    void SNetController::do_bind() {
        if (bind(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) == -1)
            throw SException(BIND_ERR, "Bind socket error");
    }

    void SNetController::do_listen() {
        if (listen(socket_fd, 100) == -1)
            throw SException(LISTEN_ERR, "Listen error");
        std::cout << "[Server] Listen on " << ip_addr << ":" << port << std::endl;
    }

    int SNetController::do_accept(struct sockaddr_in *client_address, socklen_t *clilen) {
        int cl_socket = accept(socket_fd, (struct sockaddr *) client_address, clilen);
        if (cl_socket < 0)
            throw SException(CLIENT_SOCKET_ERR, "Client socket error");
        return cl_socket;
    }

    void SNetController::send_data(int client_socket, SndPacket pkt) {
        if (send(client_socket, &pkt, sizeof(struct SndPacket), 0) < 0)
            throw SException(SND_ERR, "send packet error");
    }


}





