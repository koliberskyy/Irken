#ifndef TELEGRAMBOT_H
#define TELEGRAMBOT_H

#include <requests.h>
#include <QObject>
#include <QSignalSpy>
#include <QString>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <iostream>
#include <QJsonDocument>
#include <QByteArray>
#include <fstream>
#include <QFile>
namespace tg{

    class TelegramBot : public Requests
    {
        Q_OBJECT
    public:
        explicit TelegramBot();
        void send_message(const QString &massege);

    public slots:
        virtual void replyFinished(QNetworkReply* reply);
    signals:
        void reply_complete();
    private:
        QString token = "6094722667:AAEVC02dxf9HGWq9xivjJRPl8ZwIBklpvEQ";
        //QString chat_id = "-1001798375923";
        QString chat_id = "615961766";
    };

    inline QString convertTime(qint64 time){
        time_t timer{time};
        timer = timer/1000;
        std::tm bt = *std::localtime(&timer);

        std::ostringstream oss;

        oss << std::put_time(&bt, "%d.%m.%y, %H:%M"); // HH:MM:SS
        oss << '.' << std::setfill('0') << std::setw(3);


        return QString::fromStdString(oss.str());
    }

}

#endif // TELEGRAMBOT_H
