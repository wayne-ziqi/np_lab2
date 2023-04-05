//
// Created by ziqi on 2023/3/27.
//

#ifndef WORDGAME_CLIENT_HPP
#define WORDGAME_CLIENT_HPP

#include <string>
#include <QString>
#include <QObject>
#include <QDebug>
#include <QTimer>
#include "GTCPController.h"

namespace GClient {
    enum GMove {
        Empty = 0, Rock, Paper, Scissor
    };

    enum UserState {
        OFFLINE, WAITING, VACANCY, BUSY
    };

    class Client : public QObject {
    Q_OBJECT
    private:
        UserState user_state;
        std::string user_name;
        std::string vs_oppo;
        int lastMov = 0;
        GTCPController gtcpController;
        QTimer *timer;

        void checkNameFormat(const std::string &name);

        void reset();

        void setVacancy();

    public:

        Client(QObject *parent = nullptr) : QObject(parent), user_state(OFFLINE) {
            timer = new QTimer(this);
            connect(timer, &QTimer::timeout, this, &Client::fetchPkt);
            timer->start(1);
        }

        QString genNotice(const std::string &notice);

        QString genResult(int userMov, int userStam, int oppoMov, int oppoStam);

    public slots:

        void fetchPkt();

        void setID(QString name, QString ip_str, int port);

        void logOut();

        void sendMsg(QString rcver, QString msg);

        void makeMove(int mov);

        void quitGame();

        void checkOppoState(QString oppo_name);

        void inviteOppo(QString oppo_name);

        void acceptChallenge(QString oppo_name);

        void rejectChallenge(QString oppo_name);

    signals:

        void updateUserState(int state);

        void infoToLocal(QString info);

        void infoToGlobal(QString info);

        void infoToBoard(QString info);

        void incomeChallenge(QString oppo);
    };
}

#endif //WORDGAME_CLIENT_HPP
