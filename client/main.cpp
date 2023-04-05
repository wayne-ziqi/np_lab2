#include <QApplication>
#include <QThread>
#include "MainScene.hpp"
#include "Client.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    GClient::MainWidget window;
    GClient::Client client;
    QThread thread;
    client.moveToThread(&thread);
    // connect signal-slot
    QObject::connect(&client, &GClient::Client::infoToLocal, &window, &GClient::MainWidget::processLocalInfo);
    QObject::connect(&client, &GClient::Client::infoToGlobal, &window, &GClient::MainWidget::processGlobalInfo);
    QObject::connect(&client, &GClient::Client::infoToBoard, &window, &GClient::MainWidget::processBoardInfo);
    QObject::connect(&client, &GClient::Client::updateUserState, &window, &GClient::MainWidget::processState);
    QObject::connect(&client, &GClient::Client::incomeChallenge, &window, &GClient::MainWidget::processChallenge);
    QObject::connect(&window, &GClient::MainWidget::didSetID, &client, &GClient::Client::setID);
    QObject::connect(&window, &GClient::MainWidget::didLogOut, &client, &GClient::Client::logOut);
    QObject::connect(&window, &GClient::MainWidget::didSendMsg, &client, &GClient::Client::sendMsg);
    QObject::connect(&window, &GClient::MainWidget::didChkStat, &client, &GClient::Client::checkOppoState);
    QObject::connect(&window, &GClient::MainWidget::didInvite, &client, &GClient::Client::inviteOppo);
    QObject::connect(&window, &GClient::MainWidget::didMove, &client, &GClient::Client::makeMove);
    QObject::connect(&window, &GClient::MainWidget::didQuit, &client, &GClient::Client::quitGame);
    QObject::connect(&window, &GClient::MainWidget::didAcceptChallenge, &client, &GClient::Client::acceptChallenge);
    QObject::connect(&window, &GClient::MainWidget::didRejectChallenge, &client, &GClient::Client::rejectChallenge);

    // show window and starts the client
    thread.start();
    window.show();
//     application starts
    int ret = app.exec();
//     stop the thread
    thread.quit();
    thread.wait();
    return ret;
}