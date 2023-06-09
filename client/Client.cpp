//
// Created by ziqi on 2023/3/27.
//

#include <QTime>
#include <QThread>
#include "Client.hpp"
#include "ClientException.h"

#define MAX_STAMINA 3

static std::string genMovEmoji(int mov);

void GClient::Client::reset() {
    user_state = OFFLINE;
    user_name.clear();
    gtcpController.reset(DEFAULT_IPSTR, DEFAULT_PORT);
}

void GClient::Client::setID(QString name, QString ip_str, int port) {
    qDebug() << "set info name: " << name << "ip_str: " << ip_str << "port: " << port;
    try {
        if (user_state != OFFLINE)
            throw CException(USR_STAT_ERR, " You are online or waiting for server's response, can't log in again");
        std::string std_name = name.toStdString();
        checkNameFormat(std_name);
        user_name = std_name;
        gtcpController.reset(ip_str.toStdString(), port);
        gtcpController.do_connect();
        gtcpController.setSndPkt(0x01, 0x00, name.toStdString(), {}, 0);
        gtcpController.send_data();
        user_state = WAITING;

    } catch (CException &e) {
        if (e.excNo != USR_STAT_ERR) {
            reset();
        }
        emit infoToLocal(genNotice(e.message));
    }
}

void GClient::Client::fetchPkt() {
    emit updateUserState((int) user_state);
    if (user_state == OFFLINE)   // only receive packet when online or is configured
        return;

    try {
        gtcpController.recv_data();
        RcvPacket rcv_pkt = gtcpController.getRcvPkt();
        if (rcv_pkt.getUserName() != user_name) {
            qDebug() << "Received a wrong-user packet named" << QString(rcv_pkt.getUserName().c_str());
            assert(0);
        }
        switch (rcv_pkt.op) {
            case 0x01: {
                int flag = rcv_pkt.flag;
                if (flag == 0x01) {
                    // user's log-in is confirmed
                    std::string info("You<");
                    info += user_name;
                    info += "> logged in successfully";
                    emit infoToLocal(genNotice(info));
                    setVacancy();
                } else if (flag == 0x02) {
                    std::string info("this user<");
                    info += user_name;
                    info += "> is online, please change another name";
                    emit infoToLocal(genNotice(info));
                    user_state = OFFLINE;
                } else {
                    // give up packet and raise debug
                    qDebug() << "Error flag in rcv: op: 0x01, flag: " << rcv_pkt.flag;
                    user_state = OFFLINE;
                }
                break;
            }
            case 0x02: {
                int flag = rcv_pkt.flag;
                if (flag == 0x01) {
                    std::string info("You<");
                    info += user_name;
                    info += "> logged out successfully";
                    emit infoToLocal(genNotice(info));
                    user_state = OFFLINE;
                } else if (flag == 0x02) {
                    std::string info("You<");
                    info += user_name;
                    info += "> are not recorded in the server, please log in again";
                    emit infoToGlobal(genNotice(info));
                    user_state = OFFLINE;
                } else {
                    qDebug() << "Error flag in rcv: op: 0x02, flag: " << rcv_pkt.flag;
                    user_state = OFFLINE;
                }
                break;
            }
            case 0x03: {
                if (user_state == OFFLINE || user_state == VACANCY) {
                    qDebug() << "Error state in game";
                    assert(0);
                }
                int flag = rcv_pkt.flag;
                if (flag == 0x01) {
                    // user win
                    assert(rcv_pkt.getOppoName() == vs_oppo);
                    QString res = genResult(lastMov, rcv_pkt.user_stamina, rcv_pkt.oppo_move, rcv_pkt.oppo_stamina);
                    res += "\nCongratulations, you win!\n<=========game ends.=========>";
                    emit infoToBoard(res);
                    setVacancy();
                } else if (flag == 0x02) {
                    // user lose
                    assert(rcv_pkt.getOppoName() == vs_oppo);
                    QString res = genResult(lastMov, rcv_pkt.user_stamina, rcv_pkt.oppo_move, rcv_pkt.oppo_stamina);
                    res += "\nWhat a pity, you lose.\n<=========game ends.=========>";
                    emit infoToBoard(res);
                    setVacancy();
                } else if (flag == 0x03) {
                    assert(rcv_pkt.getOppoName() == vs_oppo);
                    QString res = genResult(lastMov, rcv_pkt.user_stamina, rcv_pkt.oppo_move, rcv_pkt.oppo_stamina);
                    res += "\nGame is a draw.\n<=========game ends.=========>";
                    emit infoToBoard(res);
                    setVacancy();
                } else if (flag == 0x04) {
                    assert(rcv_pkt.getOppoName() == vs_oppo);
                    QString res = genResult(lastMov, rcv_pkt.user_stamina, rcv_pkt.oppo_move, rcv_pkt.oppo_stamina);
                    emit infoToBoard(res);
                    user_state = BUSY;
                } else if (flag == 0x05) {
                    std::string info("Your opponent <");
                    info += vs_oppo;
                    info += "> quit the game\n<=========game ends.=========>";
                    emit infoToBoard(info.c_str());
                    setVacancy();
                } else if (flag == 0x06) {
                    std::string info("Your opponent <");
                    info += vs_oppo;
                    info += "> has made a move";
                    emit infoToBoard(info.c_str());
                }
                break;
            }
            case 0x04: {
                int flag = rcv_pkt.flag;
                if (flag <= 0 || flag >= 4) {
                    qDebug() << "Error flag in rcv: op: 0x04, flag: " << flag;
                    setVacancy();
                    break;
                }
                std::string info("Player <");
                info += rcv_pkt.getOppoName();
                info += "> is ";
                if (flag == 0x01) {
                    info += "Vacancy";
                } else if (flag == 0x02) {
                    info += "In Game";
                    setVacancy();
                } else if (flag == 0x03) {
                    // reset for challenge request waiting
                    if ((user_state == WAITING && !vs_oppo.empty() || user_state == BUSY )&& vs_oppo == rcv_pkt.getOppoName()) setVacancy();
                    info += "Offline";
                }
                emit infoToGlobal(genNotice(info));
                break;
            }
            case 0x05: {
                int flag = rcv_pkt.flag;
                if (flag == 0x01) {
                    if (user_state == VACANCY) {
                        // the user should either accept or reject
                        user_state = WAITING;
                        emit incomeChallenge(QString(rcv_pkt.getOppoName().c_str()));
                    } else {
                        // should send decline message
                        gtcpController.setSndPkt(0x05, 0x03, user_name, rcv_pkt.getOppoName(), 0);
                        gtcpController.send_data();
                        qDebug() << "user is not vacancy to handle request from " << rcv_pkt.getOppoName().c_str();
                    }
                } else if (flag == 0x02) {
                    user_state = BUSY;
                    vs_oppo = rcv_pkt.getOppoName();
                    std::string info("Player <");
                    info += rcv_pkt.getOppoName();
                    info += "> accepted your challenge";
                    info += "\n<=========game begins!=========>";
                    emit infoToBoard(genNotice(info));
                } else if (flag == 0x03) {
                    setVacancy();
                    std::string info("Player <");
                    info += rcv_pkt.getOppoName();
                    info += "> rejected your challenge";
                    emit infoToLocal(genNotice(info));
                }
                break;
            }
            case 0x06: {    // broadcast information
                int flag = rcv_pkt.flag;
                if (rcv_pkt.getOppoName() == user_name)break;
                if (flag == 0x01) {
                    std::string info("Player <");
                    info += rcv_pkt.getOppoName();
                    info += "> logged in";
                    emit infoToGlobal(genNotice(info));
                } else if (flag == 0x02) {
                    std::string oppo_name = rcv_pkt.getOppoName();
                    std::string info("Player <");
                    info += oppo_name;
                    info += "> logged out";
                    emit infoToGlobal(genNotice(info));
                    if (user_state == WAITING && !vs_oppo.empty() && vs_oppo == oppo_name) {
                        qDebug() << "clear " << oppo_name.c_str();
                        setVacancy();
                    }
                }
                break;
            }
            case 0x07: {
                std::string msgstr("<");
                msgstr += rcv_pkt.getOppoName();
                msgstr += "> messaged you: ";
                msgstr += rcv_pkt.getMessage();
                emit infoToGlobal(msgstr.c_str());
                break;
            }
        }// switch(op)
    } catch (CException &e) {
        if (e.excNo == RCV_INV)  // received packet is invalid
            return;
        reset();
        emit infoToLocal(genNotice(e.message));
    }
}

QString GClient::Client::genNotice(const std::string &notice) {
    QString ret("[Client]<");
    QTime time = QTime::currentTime();
    QString formattedTime = time.toString("hh:mm:ss");
    ret += formattedTime, ret += ">: ";
    ret.append(notice.c_str());
    qDebug() << "genNotice: " << ret;
    return ret;
}


void GClient::Client::checkNameFormat(const std::string &name) {
    if (name.length() > 30)
        throw CException(NAME_ERR, "Name must be in 30 characters");
    if (name.empty())
        throw CException(NAME_ERR, "Name can't be empty");
}

void GClient::Client::logOut() {
    if (user_state == OFFLINE) {
        emit infoToLocal(genNotice("You are not online"));
    }else if(user_state == BUSY){
        emit infoToLocal(genNotice("You are in game, please quit game and try again"));
    } else {
        try {
            gtcpController.setSndPkt(0x02, 0x00, user_name, {}, 0);
            gtcpController.send_data();
            user_state = WAITING;
        } catch (CException &e) {
            reset();
            emit infoToLocal(genNotice(e.message));
        }
    }
}

void GClient::Client::makeMove(int mov) {
    if (user_state != BUSY) {
        emit infoToLocal(genNotice("You can't make a move now"));
    } else {
        try {
            gtcpController.setSndPkt(0x03, 0x00, user_name, vs_oppo, mov);
            gtcpController.send_data();
            lastMov = mov;
            user_state = WAITING;
            std::string info("Your move: ");
            info += genMovEmoji(lastMov);
            emit infoToBoard(genNotice(info));
        } catch (CException &e) {
            reset();
            emit infoToLocal(genNotice(e.message));
        }
    }
}

void GClient::Client::quitGame() {
    if (user_state != BUSY) {
        emit infoToLocal(genNotice("You can't quit now"));
    } else {
        try {
            gtcpController.setSndPkt(0x03, 0x00, user_name, vs_oppo, 0);
            gtcpController.send_data();
            setVacancy();
            std::string info("You quit the game.\n<=========game ends.=========>");
            emit infoToBoard(info.c_str());
        } catch (CException &e) {
            reset();
            emit infoToLocal(genNotice(e.message));
        }
    }
}

void GClient::Client::checkOppoState(QString oppo_name) {
    if (user_state != VACANCY) {
        emit infoToLocal(genNotice("You are not vacancy"));
    } else {
        try {
            checkNameFormat(oppo_name.toStdString());
            gtcpController.setSndPkt(0x04, 0x01, user_name, oppo_name.toStdString(), 0);
            gtcpController.send_data();
        } catch (CException &e) {
            if (e.excNo != NAME_ERR)
                reset();
            emit infoToLocal(genNotice(e.message));
        }
    }

}

void GClient::Client::inviteOppo(QString oppo_name) {

    if (user_state != VACANCY) {
        emit infoToLocal(genNotice("You are not vacancy"));
    } else if (user_name == oppo_name.toStdString()) {
        emit infoToLocal(genNotice("You can't challenge yourself"));
    } else {
        try {
            if (!vs_oppo.empty()) {
                qDebug() << vs_oppo.c_str();
                assert(0);
            }
            checkNameFormat(oppo_name.toStdString());
            gtcpController.setSndPkt(0x05, 0x01, user_name, oppo_name.toStdString(), 0);
            gtcpController.send_data();
            std::string info("Challenge to player <");
            info += oppo_name.toStdString();
            info += "> is sent, please wait for response";
            emit infoToLocal(genNotice(info));
            vs_oppo = oppo_name.toStdString();
            user_state = WAITING;
        } catch (CException &e) {
            if (e.excNo != NAME_ERR)
                reset();
            emit infoToLocal(genNotice(e.message));
        }
    }
}

void GClient::Client::acceptChallenge(QString oppo_name) {
    assert(user_state == WAITING);
    try {
        gtcpController.setSndPkt(0x05, 0x02, user_name, oppo_name.toStdString(), 0);
        gtcpController.send_data();
        std::string info("You accepted challenge from player <");
        info += oppo_name.toStdString();
        info += ">\n<=========game begins!=========>";
        emit infoToBoard(genNotice(info));
        user_state = BUSY;
        vs_oppo = oppo_name.toStdString();
    } catch (CException &e) {
        reset();
        emit infoToLocal(genNotice(e.message));
    }
}

void GClient::Client::rejectChallenge(QString oppo_name) {
    assert(user_state == WAITING);
    user_state = VACANCY;
    try {
        gtcpController.setSndPkt(0x05, 0x03, user_name, oppo_name.toStdString(), 0);
        gtcpController.send_data();
    } catch (CException &e) {
        reset();
        emit infoToLocal(genNotice(e.message));
    }
}

static std::string genMovEmoji(int mov) {
    if (mov == 0x01)    //rock
        return {"✊"};
    else if (mov == 0x02)   // paper
        return {"✋"};
    else if (mov == 0x03)   // scissors
        return {"✂️️"};   // ✌ is too small
    else {
        assert(0);
        exit(0);
    }
}

QString GClient::Client::genResult(int userMov, int userStam, int oppoMov, int oppoStam) {
    std::string str("======================\n");
    str += "Your move: ";
    str += genMovEmoji(userMov);
    str += "\nYour Stamina: ";
    for (int i = 0; i < userStam; ++i) {
        str += "♥︎";
    }
    for (int i = 0; i < MAX_STAMINA - userStam; ++i) {
        str += "♡︎";
    }
    str += "\n<";
    str += vs_oppo;
    str += "> move: ";
    str += genMovEmoji(oppoMov);
    str += "\n<";
    str += vs_oppo;
    str += "> Stamina: ";
    for (int i = 0; i < oppoStam; ++i) {
        str += "♥︎";
    }
    for (int i = 0; i < MAX_STAMINA - oppoStam; ++i) {
        str += "♡︎";
    }
    str += "\n======================";

    return {str.c_str()};
}


void GClient::Client::sendMsg(QString rcver, QString msg) {
    if (user_state == OFFLINE) {
        emit infoToLocal(genNotice("You are not online"));
    } else if (rcver.toStdString() == user_name) {
        emit infoToLocal(genNotice("You can't send message to yourself"));
    } else {
        try {
            checkNameFormat(rcver.toStdString());
            if (msg.isEmpty()) {
                emit infoToLocal(genNotice("You can not send an empty message"));
            } else if (msg.size() > 256) {
                emit infoToLocal(genNotice("Please send a message in 256 characters"));
            } else {
                std::string rcverstr = rcver.toStdString();
                std::string msgstr = msg.toStdString();
                std::string glo("You messaged <");
                glo += rcverstr;
                glo += ">: ";
                glo += msgstr;
                emit infoToGlobal(glo.c_str());
                gtcpController.setSndPkt(0x07, 0x00, user_name, rcverstr, msgstr);
                gtcpController.send_data();
            }
        } catch (CException &e) {
            if (e.excNo != NAME_ERR)
                reset();
            emit infoToLocal(genNotice(e.message));
        }
    }
}

void GClient::Client::setVacancy() {
    user_state = VACANCY;
    vs_oppo.clear();
}












