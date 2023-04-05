//
// Created by ziqi on 2023/4/5.
//

#ifndef WORDGAME_TOMSGBOX_HPP
#define WORDGAME_TOMSGBOX_HPP

#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>
#include <QDialogButtonBox>

class ToMsgBox : public QMessageBox {
Q_OBJECT
public:
    ToMsgBox(QWidget *parent = nullptr) : QMessageBox(parent) {
        timer_ = new QTimer(this);
        timer_->setSingleShot(true);
        timer_->setInterval(5000);
        setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        setDefaultButton(QMessageBox::Yes);
        // 将 QTimer 对象与 QDialog 对象的 reject() 槽函数连接起来
        QObject::connect(timer_, &QTimer::timeout, this, &QMessageBox::reject);
    }

    ~ToMsgBox() override {
        // 停止定时器，避免内存泄漏
        timer_->stop();
    }

    void showMsg(QString title, QString msg) {
        // 显示对话框，开始计时
        this->setWindowTitle(title);
        this->setText(msg);
        show();
        timer_->start();
    }

private:
    QTimer *timer_;

};


#endif //WORDGAME_TOMSGBOX_HPP
