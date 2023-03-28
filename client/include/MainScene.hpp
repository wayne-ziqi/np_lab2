//
// Created by ziqi on 2023/3/27.
//

#ifndef WORDGAME_MAINSCENE_HPP
#define WORDGAME_MAINSCENE_HPP

#include <QWidget>
#include <QString>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QDebug>
#include <QLabel>
#include <QMessageBox>

namespace GClient {
    class MainWidget : public QWidget {
    Q_OBJECT
    private:
        // left top
        QLabel *statLabel;
        QLineEdit *ipEdit;
        QLineEdit *userNameEdit;
        QSpinBox *portSpinBox;
        QPushButton *logInButton;
        QPushButton *logOutButton;
        // left bottom
        QLineEdit *oppoEdit;
        QPushButton *inviteButton;
        QPushButton *chkOppoButton;
        QPushButton *button1;
        QPushButton *button2;
        QPushButton *button3;
        // right top
        QTextEdit *localInfoText;
        // right bottom
        QTextEdit *globalInfoText;

    public:
        MainWidget();

    public slots:

        void processLocalInfo(QString info) {
            localInfoText->append(info);
        };

        void processGlobalInfo(QString info) {
            globalInfoText->append(info);
        }

        void processState(int state) {
            // can only change user when offline
            if (state == 0)userNameEdit->setReadOnly(false);
            else userNameEdit->setReadOnly(true);
            if (state == 1 || state == 3) oppoEdit->setReadOnly(true);
            else oppoEdit->setReadOnly(false);
            QString str("State: ");
            if (state == 0) str += "Offline";
            else if (state == 1) str += "Waiting";
            else if (state == 2) str += "Vacancy";
            else if (state == 3) str += "In game";
            statLabel->setText(str);
        }

        void processChallenge(QString oppo) {
            QString info("Player <");
            info += oppo;
            info += "> has sent a challenge to you, do you accept?";
            QMessageBox::StandardButton reply = QMessageBox::question(this, "Your are challenged", info,
                                                                      QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                emit didAcceptChallenge(oppo);
            } else {
                emit didRejectChallenge(oppo);
            }
        }

    signals:

        void didSetID(QString user_name, QString ip_str, int port);

        void didLogOut();

        void didMove(int mov);

        void didChkStat(QString oppo_name);

        void didInvite(QString oppo_name);

        void didAcceptChallenge(QString oppo_name);

        void didRejectChallenge(QString oppo_name);

    };


}

#endif //WORDGAME_MAINSCENE_HPP
