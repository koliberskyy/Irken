#include "telegramalert.h"

TelegramAlert::TelegramAlert(QWidget *parent)
    : QWidget{parent},
      enabledCheck{new QCheckBox("Enabled")},
    authToken{"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyX2lkIjoyMjk5MzAwOCwid2ViX3ZpZXdfaW5pdF9kYXRhIjp7InF1ZXJ5X2lkIjoiQUFHbTFMWWtBQUFBQUtiVXRpUkRiR1VqIiwidXNlciI6eyJpZCI6NjE1OTYxNzY2LCJpc19ib3QiOm51bGwsImZpcnN0X25hbWUiOiJIb29sZXkiLCJsYXN0X25hbWUiOiIiLCJ1c2VybmFtZSI6Im1mbmJieSIsImxhbmd1YWdlX2NvZGUiOiJydSIsImFsbG93c193cml0ZV90b19wbSI6dHJ1ZSwicGhvdG9fdXJsIjoiaHR0cHM6Ly90Lm1lL2kvdXNlcnBpYy8zMjAvNG1RcVlIa1Uyd3dsLWZqWEU0VjRLX09paG4wUzZBaHJJT3lmTlR4VE1iRS5zdmciLCJhZGRlZF90b19hdHRhY2htZW50X21lbnUiOnRydWV9LCJyZWNlaXZlciI6eyJpZCI6MTk4NTczNzUwNiwiaXNfYm90Ijp0cnVlLCJmaXJzdF9uYW1lIjoiV2FsbGV0IiwibGFzdF9uYW1lIjoiIiwidXNlcm5hbWUiOiJ3YWxsZXQiLCJsYW5ndWFnZV9jb2RlIjpudWxsLCJhbGxvd3Nfd3JpdGVfdG9fcG0iOm51bGwsInBob3RvX3VybCI6Imh0dHBzOi8vdC5tZS9pL3VzZXJwaWMvMzIwL2Z5U0JHS2R3VkhiSWJnaEdtakFVY01VNUtkSmhFNHViX0hyNTF1T0U1cEUuc3ZnIiwiYWRkZWRfdG9fYXR0YWNobWVudF9tZW51IjpudWxsfSwic3RhcnRfcGFyYW0iOm51bGwsImF1dGhfZGF0ZSI6MTcyMzEyNjkwNSwiaGFzaCI6IjBkNTlmOTg2NmY5MzMyOGFiMzc3ZTZiMTYyY2EwODhkZmYzMmM2OWU4NDFmMzYyNTFmNTIyYWUzMjUyNWRkMzIiLCJjb3VudHJ5X2NvZGVfYnlfaXAiOiJSVSJ9LCJ3YWxsZXRfZGF0YSI6bnVsbCwibWFpbl9kYXRhIjoiZ0FBQUFBQm10TlI2TWJOdUwwV184YXcybkxKR3RLWUNXRnNpVDdvN1JzYU15a0ZjWm94S2h1TVl3TGUxUWMyeEFMTk9aMFFuc2hTU1RBU1dGaEF1TXpLSlBvUDEwNzJlNVZLMmR5dENPWFkySjNkVVZMOTFGSjJ6ZWllQ3M0cXJZNXMtLWlPM1g4b1NfVUJ4QUdtMWFNWmZya0NHZjNpM2lMWm9TenJ5TVFUM1NiUmdrYVJmRmtONzhKWGdBWm9xRU9henZyMHlPd1lQb0xEYzZDNVNKM1BhUFFfWXFmMGpFcWtYNVNoRzBVcEExQW1vZ216SDNHNGlOc0dSMWpULVNBUkFqWkFHbjg0Q0tTam9RMjh4MXJzdk5jQVM5ZG1JbE9HSkxPQTQtYTV4ZHAxemNWZjJ2Um9TMGU4OEFJZTQzajNxY3k2R1F6MEciLCJleHAiOjE3MjMxMjg3MDYsInZlcnNpb24iOjJ9.ndg35gw-Fy88n7Txh-eWCcSyO_K5BDDFCYw1cup5et8"}
{
    // ***MAPPING***
    auto layMain = new QVBoxLayout();
    layMain->addWidget(new QLabel("Оповещения ТГ"));
    layMain->addWidget(enabledCheck);

    getAvaibleOrders("RUB", "USDT", "SALE");

    this->setLayout(layMain);
}

QJsonArray TelegramAlert::getAvaibleOrders(const QString &baseCoin, const QString &quoteCoin, const QString &offerType, const QString &limit, double minVolume, double maxVolume) const
{
    const QString host{"walletbot.me"};
    const QString path{"/p2p/public-api/v2/offer/depth-of-market"};

    QJsonObject dataObj;
    dataObj.insert("baseCurrencyCode", quoteCoin);
    dataObj.insert("quoteCurrencyCode", baseCoin);
    dataObj.insert("offerType", offerType);
    dataObj.insert("limit", limit);

    QUrl url;

    url.setScheme("https");
    url.setHost(host);
    url.setPath(path);

    QNetworkRequest request;
    request.setRawHeader("content-type", "application/json");
    request.setRawHeader("authorization", "Bearer " + authToken.toUtf8());
    request.setUrl(url);

    auto reply = Requests::post(request, QJsonDocument(dataObj).toJson());
    return reply["data"].toArray();
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
