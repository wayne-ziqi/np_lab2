//
// Created by ziqi on 2023/3/25.
//


#include "Server.h"
#include <iostream>
#include "ServerException.h"

GServer::Server::Server() {
    for (int i = 1; i <= MAX_COMP; ++i) {
        queAviaID.push(i);
    }
}

int GServer::Server::getAvaiCompID() {
    assert(competitions.size() + queAviaID.size() == MAX_COMP);
    if (!queAviaID.empty()) {
        int id = queAviaID.front();
        queAviaID.pop();
        return id;
    }
    throw SException(COMP_FULL, "Server is too busy to launch a new challenge");
}


void GServer::Server::clientHandler(int client_socket) {
    // TODO: receive packet from the client socket,
    //  and process using data got from server.
    /**
     * server's doing:
     * 1. log in/out the current client, to/from the players set
     * 2. handle challenge request
     */
    std::cout << "[Server] Create a new thread for socket: " << client_socket << std::endl;
    while (true) {
        std::string user_name;
        try {
            RcvPacket rcv_pkt{};
            memset(&rcv_pkt, 0x00, sizeof(struct RcvPacket));
            ssize_t rcv_size = recv(client_socket, &rcv_pkt, sizeof(struct RcvPacket), 0);
            if (rcv_size <= 0)   // maybe timeout
                throw SException(RCV_INV, "Receive packet error, connection is closed");

            user_name = rcv_pkt.getUserName();
            switch (rcv_pkt.op) {
                case 0x01: {
                    // client requests to become a member
                    logInPlayer(user_name, client_socket);
                    break;
                }
                case 0x02: {
                    // client requests to log out
                    logOutPlayer(user_name, client_socket);
                    throw SException(USER_OUT, "User log out finished");
                }
                case 0x03: {
                    std::string oppo_name = rcv_pkt.getOppoName();
                    Player *user = checkUserPresence(user_name, user_name, client_socket);
                    Player *oppo = checkUserPresence(oppo_name, user_name, client_socket);
                    int compID = user->get_compID();
                    if (compID != oppo->get_compID())
                        throw SException(COMP_MISMATCH, "Users are not in the same game");
                    Competition *comp = getCompetition(compID);
                    if (!comp->isReady())
                        throw SException(COMP_NOT_READY, "Competition's not online");
                    auto user_mov = (GMove) rcv_pkt.user_move;
                    play_mtx.lock();
                    comp->advanceCompState(user_name, user_mov);
                    play_mtx.unlock();
                    // 1 waiting 2 or 2 waiting 1
                    if (!comp->isCool() && !comp->isFinished())
                        break;

                    SndPacket user_snd_pkt, oppo_snd_pkt;
                    if (comp->isFinished()) {
                        user_snd_pkt.setBasicData(0x03, comp->getResult(user_name), user_name, oppo_name);
                        oppo_snd_pkt.setBasicData(0x03, comp->getResult(oppo_name), oppo_name, user_name);
                        user_snd_pkt.oppo_move = comp->getLatestMove(oppo_name);
                        oppo_snd_pkt.oppo_move = comp->getLatestMove(user_name);
                        user_snd_pkt.user_stamina = oppo_snd_pkt.oppo_stamina = comp->getStamina(user_name);
                        user_snd_pkt.oppo_stamina = oppo_snd_pkt.user_stamina = comp->getStamina(oppo_name);
                        play_mtx.lock();
                        user->quit_comp();
                        oppo->quit_comp();
                        play_mtx.unlock();
                        delCompetition(compID);
                    } else if (comp->isCool()) {
                        // one round finished, send back the
                        user_snd_pkt.setBasicData(0x03, 0x04, user_name, oppo_name);
                        oppo_snd_pkt.setBasicData(0x03, 0x04, oppo_name, user_name);
                        user_snd_pkt.oppo_move = comp->getLatestMove(oppo_name);
                        oppo_snd_pkt.oppo_move = comp->getLatestMove(user_name);
                        user_snd_pkt.user_stamina = oppo_snd_pkt.oppo_stamina = comp->getStamina(user_name);
                        user_snd_pkt.oppo_stamina = oppo_snd_pkt.user_stamina = comp->getStamina(oppo_name);
                    }
                    netController.send_data(client_socket, user_snd_pkt);
                    netController.send_data(oppo->get_socket(), oppo_snd_pkt);
                    break;
                }
                case 0x04: {
                    if (rcv_pkt.flag == 0x01)
                        checkUserState(rcv_pkt.getOppoName(), user_name, client_socket);
                    else if (rcv_pkt.flag == 0x02) {
                        for (auto &player: players) {
                            checkUserState(player.first, user_name, client_socket);
                        }
                        // send end of sending packet
                        SndPacket snd_pkt;
                        snd_pkt.setBasicData(0x04, 0x04, user_name, {});
                        netController.send_data(client_socket, snd_pkt);
                    }
                    break;
                }
                case 0x05: {
                    handleChallenge(user_name, client_socket, rcv_pkt);
                    break;
                }
            }// switch(op)
        } catch (SException &e) {
            if (e.excNo == USER_OUT) {
                std::cout << "[Server " << client_socket << "] User <" << user_name << "> exception: " << e.message
                          << std::endl;
                break;
            }
            // should clear all relative competitions and players, remember to inform the other player in the competition
            // FIXME: need to be examined more carefully
            if (e.excNo == RCV_INV || e.excNo == SND_ERR) {
                for (auto it = players.cbegin(), next_it = it;
                     it != players.cend(); it = next_it) {
                    ++next_it;
                    if (it->second.get_socket() == client_socket) {
                        int compID = it->second.get_compID();
                        if (compID > 0) {
                            auto compit = getCompetition(compID);
                            if (compit != nullptr) {
                                std::string player1_name = it->second.get_name();
                                Player *player2 = compit->getTheOther(it->second.get_name());
                                if (player2 != nullptr) {
                                    play_mtx.lock();
                                    player2->quit_comp();
                                    play_mtx.unlock();
                                    std::string player2_name = player2->get_name();
                                    SndPacket snd_pkt;
                                    snd_pkt.setBasicData(0x04, 0x03, player2_name, player1_name);
                                    try {   // try to inform player2
                                        netController.send_data(player2->get_socket(), snd_pkt);
                                    } catch (SException &e) {
                                        std::cout << "[Server]: Failed to send oppo offline info to " << player2_name
                                                  << std::endl;
                                    }
                                }
                                delCompetition(compID);
                            }
                        }
                        SndPacket snd{};
                        snd.setBasicData(0x06, 0x02, it->first, it->first);
                        sendBroadCast(snd);
                        usr_mtx.lock();
                        players.erase(it);
                        usr_mtx.unlock();
                    }
                }
                std::cout << "[Server " << client_socket << "] User <" << user_name << "> exception: " << e.message
                          << std::endl;
                break;  // break loop for receive error
            } else if (e.excNo == COMP_DID_END) {
                std::cout << "[Server " << client_socket << "] User <" << user_name << "> exception: " << e.message
                          << std::endl;
                continue;
            }
        }
    }
    // should not close socket until logging out /
    close(client_socket);
}

void GServer::Server::run() {
    std::cout << "[Server] Run\n";
    struct sockaddr_in cliaddr{};
    try {
        netController.do_bind();
        netController.do_listen();
    } catch (SException &e) {
        std::cout << "[Server] " << e.message << std::endl;
        return;
    }
    while (true) {
        try {
            socklen_t clilen = sizeof(cliaddr);
            int client_socket = netController.do_accept(&cliaddr, &clilen);
            std::cout << "[Server] Client connects from socket: " << client_socket << std::endl;
            std::thread client_thread(&Server::clientHandler, this, client_socket);
            client_thread.detach();
        } catch (SException &e) {
            if (e.excNo == CLIENT_SOCKET_ERR)continue;
            std::cout << "[Server] " << e.message << std::endl;
        }
    }
}


void GServer::Server::logInPlayer(const std::string &name, int socket_fd) {
    int flag;
    usr_mtx.lock();
    std::cout << "[Server " << socket_fd << "] Try to log in user: " << name << std::endl;
    if (players.find(name) != players.end()) {
        std::cout << "[Server] Socket " << socket_fd << " error: Duplicate signing user name: " << name << std::endl;
        flag = 0x02;
    } else {
        players.insert({name, Player(name, socket_fd)});
        flag = 0x01;
    }
    usr_mtx.unlock();
    SndPacket snd_pkt;
    snd_pkt.setBasicData(0x01, flag, name, {});
    netController.send_data(socket_fd, snd_pkt);
    // broadcast
    if (flag == 0x01) {
        snd_pkt.setBasicData(0x06, 0x01, name, name);
        sendBroadCast(snd_pkt);
    } else
        throw SException(USER_OUT, "user log in failed");
}

void GServer::Server::logOutPlayer(const std::string &name, int socket_fd) {
    int flag;
    usr_mtx.lock();
    std::cout << "[Server " << socket_fd << "] Try to log out user: " << name << std::endl;
    if (players.find(name) == players.end()) {
        std::cout << "[Server " << socket_fd << "] error: No user named: " << name << std::endl;
        flag = 0x02;
    } else {
        players.erase(name);
        flag = 0x01;
    }
    usr_mtx.unlock();
    SndPacket snd_pkt;
    snd_pkt.setBasicData(0x02, flag, name, {});
    netController.send_data(socket_fd, snd_pkt);

    if (flag == 0x01) {
        snd_pkt.setBasicData(0x06, 0x02, name, name);
        sendBroadCast(snd_pkt);
    }
}

void GServer::Server::handleChallenge(const std::string &user_name, int socket_fd, RcvPacket &rcv_pkt) {
    int flag = rcv_pkt.flag;
    std::string oppo_name = rcv_pkt.getOppoName();
    if (flag == 0x01) {
        // this is a challenge request sent by user to opponent, will help the player check
        // the opponent's state first
        Player *player1 = checkUserPresence(user_name, user_name, socket_fd);
        Player *player2 = checkUserState(oppo_name, user_name, socket_fd);
        if (player2->get_compID() > 0)
            throw SException(OPPO_BUSY, "user has requested a busy opponent");
        Competition *new_comp = addNewCompetition();
        play_mtx.lock();
        new_comp->advanceLchState(*player1, 2);
        player1->join_comp(new_comp->getID());
        play_mtx.unlock();
        // help player1 send a challenge request to the opponent
        SndPacket snd_pkt;
        snd_pkt.setBasicData(0x05, 0x01, rcv_pkt.getOppoName(), user_name);
        netController.send_data(player2->get_socket(), snd_pkt);
    } else if (flag == 0x02 || flag == 0x03) {
        Player *user = checkUserPresence(user_name, user_name, socket_fd);
        if (user->get_compID() > 0)
            throw SException(USR_BUSY, "User is in a competition, unable to agree/reject to a new one");
        Player *oppo = checkUserPresence(oppo_name, user_name, socket_fd);
        int compID = oppo->get_compID();
        if (compID == 0)
            throw SException(OPPO_IDLE, "The opponent hasn't raised any challenge or user missed the request");
        Competition *comp = getCompetition(compID);
        if (comp == nullptr) {
            if (flag == 0x03) return;
            throw SException(COMP_MISS, "Competition is not raised");
        }
        if (comp->getLchState() != 1)
            throw SException(COMP_LCH_ERR, "Competition launch state miss match");

        play_mtx.lock();
        comp->advanceLchState(*user, flag);
        if (flag == 0x02)
            user->join_comp(compID);
        play_mtx.unlock();

        if (flag == 0x03) {
            assert(comp->isDeclined());
            play_mtx.lock();
            oppo->quit_comp();
            play_mtx.unlock();
            delCompetition(compID);
        }
        // send challenge-accepted reply to player1
        SndPacket snd_pkt;
        snd_pkt.setBasicData(0x05, flag, rcv_pkt.getOppoName(), user_name);
        netController.send_data(oppo->get_socket(), snd_pkt);
        // now it's online to start the competition
    } else
        assert(0);
}

void GServer::Server::sendBroadCast(SndPacket &snd_pkt) {
    for (auto &player: players) {
        try {
            if (snd_pkt.getUserName() == player.first)continue;
            snd_pkt.setUserName(player.first);
            netController.send_data(player.second.get_socket(), snd_pkt);
        } catch (SException &e) {
            continue;
        }
    }
}

GServer::Player *
GServer::Server::checkUserPresence(const std::string &name, const std::string &asker, int askerSocket) {
    usr_mtx.lock();
    auto it = players.find(name);
    if (it == players.end()) {
        usr_mtx.unlock();
        SndPacket snd_pkt;
        snd_pkt.setBasicData(0x04, 0x03, asker, name);
        netController.send_data(askerSocket, snd_pkt);
        // socket is unavailable
        throw SException(USR_MISS, "User is offline");
    }
    usr_mtx.unlock();
    return &it->second;
}

GServer::Player *
GServer::Server::checkUserState(const std::string &name, const std::string &asker, int askerSocket) {
    Player *player = checkUserPresence(name, asker, askerSocket);
    int state = player->get_compID();
    int flag = state > 0 ? 0x02 : 0x01;
    SndPacket snd_pkt;
    snd_pkt.setBasicData(0x04, flag, asker, name);
    netController.send_data(askerSocket, snd_pkt);
    return player;
}

GServer::Competition *GServer::Server::addNewCompetition() {
    comp_mtx.lock();
    int compID = getAvaiCompID();
    competitions.insert({compID, Competition(compID)});
    auto its = competitions.find(compID);
    comp_mtx.unlock();
    return &its->second;
}

GServer::Competition *GServer::Server::getCompetition(int id) {
    Competition *comp = nullptr;
    comp_mtx.lock();
    auto its = competitions.find(id);
    if (its != competitions.end())
        comp = &its->second;
    comp_mtx.unlock();
    return comp;
}

void GServer::Server::delCompetition(int id) {
    comp_mtx.lock();
//    if (competitions.find(id) != competitions.end()) {
    competitions.erase(id);
    queAviaID.push(id);
//    }
    comp_mtx.unlock();
}








