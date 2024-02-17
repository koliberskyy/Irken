#ifndef SMARTMONEY_H
#define SMARTMONEY_H

#include <QObject>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include <QList>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <set>
#include <unordered_map>
#include <string>
#include "graphicsview.h"

#include "instrumentsinfo.h"
#include "settings.h"

struct liquid{
    double price;
    QDateTime time;
    bool isRed;
    bool operator <(const liquid &other) const{
        return  price < other.price;
    }
    bool operator >(const liquid &other) const{
        return !(*this < other);
    }

    void setRed(){
        isRed = true;
    }
    void setBlue(){
        isRed = false;
    }
};

struct TradingWindow{
    QString symbol;

    QDateTime highWindowBegin;
    QDateTime highWindowEnd;
    QDateTime lowWindowBegin;
    QDateTime lowWindowEnd;

    double highWindowPriceLow;
    double highWindowPriceHigh;

    double lowWindowPriceLow;
    double lowWindowPriceHigh;

    bool isHighZoneAvaible;


};


class SmartMoney : public QObject
{
    Q_OBJECT
public:
    explicit SmartMoney(QObject *parent = nullptr);
    auto getOrders()const {return &orderMap;}

public slots:
    void updateSmartMoney();
    void replyFinished(QNetworkReply *reply);
private slots:
    void updateAreas(TradingWindow window, double currentPrice);
signals:
    void updated(QJsonArray);
    void liquidsUpdated(TradingWindow);

private:
    std::unordered_map<QString, QList<QJsonObject>> orderMap;
    QNetworkAccessManager *manager;
    void updateLiquids(const QJsonArray &klines, const QString &symbol);
    QList<QJsonObject> updateOrders(const QJsonArray &klines, const QString &symbol, const QString &takeProfit, const QString &side, const double curentPrice);


};

#endif // SMARTMONEY_H
