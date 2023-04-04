//
// Created by ziqi on 2023/3/27.
//

#include "MainScene.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QObject>
#include <QGraphicsBlurEffect>

GClient::MainWidget::MainWidget() {

    // window basic settings
    this->setWindowTitle("Word Game Client");
    this->resize(640, 640);
//    QGraphicsBlurEffect *blur = new QGraphicsBlurEffect(this);
//    blur->setBlurRadius(10);
//    this->setGraphicsEffect(blur);

    // Create left layout
    auto *leftLayout = new QVBoxLayout;
    // Create top left layout
    auto *topLeftLayout = new QVBoxLayout;
    auto *topLeftTitle = new QLabel("Basic Settings");
    topLeftTitle->setAlignment(Qt::AlignHCenter);
    statLabel = new QLabel("offline");
    ipEdit = new QLineEdit;
    userNameEdit = new QLineEdit;
    portSpinBox = new QSpinBox;
    auto *portRowLayout = new QHBoxLayout();
    auto *portLabel = new QLabel("Port(default: 20154):");
    portRowLayout->addWidget(portLabel);
    portRowLayout->addWidget(portSpinBox);
    logInButton = new QPushButton("Log in");
    logOutButton = new QPushButton("Log out");
    topLeftLayout->addWidget(topLeftTitle);
    topLeftLayout->addWidget(statLabel);
    topLeftLayout->addWidget(userNameEdit);
    topLeftLayout->addWidget(ipEdit);
    topLeftLayout->addLayout(portRowLayout);
    topLeftLayout->addWidget(logInButton);
    topLeftLayout->addWidget(logOutButton);

    // top left layout configure
    topLeftTitle->setStyleSheet("font-weight: bold");
    userNameEdit->setPlaceholderText("Your name");
    ipEdit->setText("127.0.0.1");
    ipEdit->setPlaceholderText("Server ip(format: x.x.x.x)");
    portSpinBox->setRange(0, 65535);
    portSpinBox->setValue(20154);

    // Create mid left layout
    auto *midLeftLayout = new QVBoxLayout;
    auto *midLeftTitle = new QLabel("Chat Space");
    midLeftTitle->setAlignment(Qt::AlignHCenter);
    rcverEdit = new QLineEdit;
    chatEdit = new QTextEdit;
    msgButton = new QPushButton("Send");
    midLeftLayout->addWidget(midLeftTitle);
    midLeftLayout->addWidget(rcverEdit);
    midLeftLayout->addWidget(chatEdit);
    midLeftLayout->addWidget(msgButton);

    midLeftTitle->setStyleSheet("font-weight: bold");

    // mid left layout config
    rcverEdit->setPlaceholderText("Choose a user to send a message");
    chatEdit->setPlaceholderText("Say something ...");

    // Create bottom left layout
    auto *bottomLeftLayout = new QVBoxLayout;
    auto *bottomLeftTitle = new QLabel("Playground");
    auto *hbox1 = new QHBoxLayout;
    auto *hbox2 = new QHBoxLayout;
    auto *oppoTitle = new QLabel("Check an user's state or raise a challenge");
    auto *movTitle = new QLabel("Make a move");
    oppoTitle->setAlignment(Qt::AlignHCenter);
    movTitle->setAlignment(Qt::AlignHCenter);
    bottomLeftTitle->setAlignment(Qt::AlignHCenter);
    oppoEdit = new QLineEdit;
    chkOppoButton = new QPushButton("Check state");
    inviteButton = new QPushButton("Invite");
    button1 = new QPushButton("Rock");
    button2 = new QPushButton("Paper");
    button3 = new QPushButton("Scissors");
    quitButton = new QPushButton("Quit");

    bottomLeftTitle->setStyleSheet("font-weight: bold");
    bottomLeftLayout->addWidget(bottomLeftTitle);
    bottomLeftLayout->addWidget(oppoTitle);
    bottomLeftLayout->addWidget(oppoEdit);
    hbox1->addWidget(chkOppoButton), hbox1->addWidget(inviteButton);
    bottomLeftLayout->addLayout(hbox1);
    bottomLeftLayout->addWidget(inviteButton);
    bottomLeftLayout->addWidget(movTitle);
    hbox2->addWidget(button1), hbox2->addWidget(button2), hbox2->addWidget(button3);
    bottomLeftLayout->addLayout(hbox2);
    bottomLeftLayout->addWidget(quitButton);

    oppoEdit->setPlaceholderText("Choose a user to challenge");

    // Add top and bottom layouts to left layout
    leftLayout->addLayout(topLeftLayout);
    leftLayout->addLayout(midLeftLayout);
    leftLayout->addLayout(bottomLeftLayout);

    // Create right layout
    auto *rightLayout = new QVBoxLayout;

    // Create top right layout
    auto *topRightLayout = new QVBoxLayout;
    auto *topRightTitle = new QLabel("Local information");
    topRightTitle->setAlignment(Qt::AlignHCenter);
    localInfoText = new QTextEdit;
    localInfoText->setReadOnly(true);
    topRightLayout->addWidget(topRightTitle);
    topRightLayout->addWidget(localInfoText);
    topRightTitle->setStyleSheet("font-weight: bold");

    // Create bottom right layout
    auto *bottomRightLayout = new QVBoxLayout;
    auto *bottomRightTitle = new QLabel("Global information");
    bottomRightTitle->setAlignment(Qt::AlignHCenter);
    globalInfoText = new QTextEdit;
    globalInfoText->setReadOnly(true);
    bottomRightLayout->addWidget(bottomRightTitle);
    bottomRightLayout->addWidget(globalInfoText);

    bottomRightTitle->setStyleSheet("font-weight: bold");

    // Add top and bottom layouts to right layout
    rightLayout->addLayout(topRightLayout);
    rightLayout->addLayout(bottomRightLayout);

    // Create main layout and add left and right layouts
    auto *mainLayout = new QHBoxLayout;
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);

    // Set main layout
    this->setLayout(mainLayout);

    // Connect buttons to slot
    QObject::connect(logInButton, &QPushButton::clicked, [=]() {
        emit didSetID(userNameEdit->text(), ipEdit->text(), portSpinBox->value());
    });
    QObject::connect(logOutButton, &QPushButton::clicked, [=]() {
        emit didLogOut();
    });
    QObject::connect(msgButton, &QPushButton::clicked, [=]() {
        emit didSendMsg(rcverEdit->text(), chatEdit->toPlainText());
    });
    QObject::connect(chkOppoButton, &QPushButton::clicked, [=]() {
        emit didChkStat(oppoEdit->text());
    });
    QObject::connect(inviteButton, &QPushButton::clicked, [=]() {
        emit didInvite(oppoEdit->text());
    });
    QObject::connect(button1, &QPushButton::clicked, [=]() {
        qDebug() << "Rock clicked";
        emit didMove(1);
    });
    QObject::connect(button2, &QPushButton::clicked, [=]() {
        qDebug() << "Paper clicked.";
        emit didMove(2);
    });
    QObject::connect(button3, &QPushButton::clicked, [=]() {
        qDebug() << "Scissors clicked.";
        emit didMove(3);
    });
    QObject::connect(quitButton, &QPushButton::clicked, [=]() {
        emit didQuit();
    });
}
