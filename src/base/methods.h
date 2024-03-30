#ifndef METHODS_H
#define METHODS_H


#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QMessageAuthenticationCode>
#include "requests.h"
#include "settings.h"

struct bybitInfo
{
    QJsonObject obj;
    auto retCode() const          {if(!obj.isEmpty())return obj["retCode"].toInt();
                                   else              return 222222;}
    auto result() const           {return obj["result"].toObject();}
    auto list()const              {return result()["list"].toArray();}

    void clear(){obj = QJsonObject();}
};


namespace Methods
{
const QString getBalance(const QString &api, const QString &secret);

const bybitInfo getPositionList(const QString &api, const QString &secret);

const bybitInfo setTradingStop(const QString &symbol, const QString &api, const QString &secret, const QString &sl, const QString &tp);
const bybitInfo setTradingStop(const QString &symbol, const QString &api, const QString &secret, double sl, double tp);

const bybitInfo placeOrder(const QJsonObject &order, const QString &api, const QString &secret);

const bybitInfo setLeverage(const QString &symbol, double leverage, const QString &api, const QString &secret);

const bybitInfo getOpenOrders(const QString &api, const QString &secret);

const double qty_to_post(double acc_balance, double price, double percent_from_balance = -1.0);

}

#endif // METHODS_H
