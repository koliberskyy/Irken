#include "telegramalert.h"

TelegramAlert::TelegramAlert(QWidget *parent)
    : QWidget{parent},
      enabledCheck{new QCheckBox("Enabled")}
{
    // ***MAPPING***
    auto layMain = new QVBoxLayout();
    layMain->addWidget(new QLabel("Оповещения ТГ"));
    layMain->addWidget(enabledCheck);

    this->setLayout(layMain);
}

void TelegramAlert::updateKlines(QString symbol, QString interval, QJsonArray klines)
{
    if(enabledCheck->isChecked() && interval != "D" && interval != "W"){
        auto       time    = [](const QJsonArray &kline)   {return kline[0].toString().toLongLong(); };
        auto       open    = [](const QJsonArray &kline)   {return kline[1].toString().toDouble(); };
        auto       high    = [](const QJsonArray &kline)   {return kline[2].toString().toDouble(); };
        auto       low     = [](const QJsonArray &kline)   {return kline[3].toString().toDouble(); };
        auto       close   = [](const QJsonArray &kline)   {return kline[4].toString().toDouble(); };

        auto cheakableKline = klines[1].toArray();
        double difference = 0;
        QString upDown;
        if(open(cheakableKline) < close(cheakableKline)){
            upDown = "up-";
            difference = 100 * (close(cheakableKline) - open(cheakableKline)) / open(cheakableKline);
        }
        else{
            upDown = "down-";
            difference = 100 * (open(cheakableKline) - close(cheakableKline)) / close(cheakableKline);

        }

        QString host{"https://api.telegram.org/bot6387989209:AAEwzRfOfEUGC0FsW5Mo1e9fvd0tV_xjkbI"};
        QString chat_id="-1001964821237";
        QString text{symbol + "-"};
        if(difference > alertDifference && lastSendedTimestamp != time(cheakableKline)){
            lastSendedTimestamp = time(cheakableKline);
            text.append(upDown);
            text.append(QString::fromStdString(std::to_string(difference)));
            Requests::get(host, QString("/sendMessage?chat_id=" + chat_id + "&text=" + text + "-tf-" + interval + "m").toUtf8());
        }

    }
}
