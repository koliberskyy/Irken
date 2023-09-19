#include "telegrambot.h"

namespace tg{
    TelegramBot::TelegramBot()
    {
        connect_manager(this);
    }

    void TelegramBot::send_message(const QString &massege){
        if(!massege.isEmpty()){
            QUrl url = "https://api.telegram.org/bot" +
                    token + "/sendMessage?chat_id=" + chat_id + "&text=" + massege;
            get(url);

            QSignalSpy spy = QSignalSpy(this, SIGNAL(reply_complete()));
            spy.wait(500000);
        }
    }

    void TelegramBot::send_document(const QString &path){
        QUrl url = "https://api.telegram.org/bot" +
                   token + "/sendDocument?chat_id=" + chat_id + "&document=" + path;

        QFile *compressedFile = new QFile(path);
        compressedFile->open(QIODevice::ReadOnly);
        auto arr = compressedFile->readAll();

        post(url, &arr);
        QSignalSpy spy = QSignalSpy(this, SIGNAL(reply_complete()));
        spy.wait(500000);
    }

    void TelegramBot::replyFinished(QNetworkReply* reply){
        reply->deleteLater();
        emit reply_complete();
    }
}
