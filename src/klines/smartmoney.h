#ifndef SMARTMONEY_H
#define SMARTMONEY_H

#include <QObject>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include <QList>
#include <QCandlestickSet>


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

class requests{
public:
    static QJsonObject get(QUrl url,
                          const QByteArray &query,
                          const QString &userInfo = "",
                          int recWindow_msec = 10000,
                          std::function<QJsonObject (const QUrl &, const QByteArray &)> customReplyErrorParser = [](const QUrl &url, const QByteArray &data) ->QJsonObject
    {
        auto obj = QJsonDocument::fromJson(data).object();
        auto retCode = obj["retCode"].toInt();
        if(retCode != 0 || data.isEmpty()){
            instruments::replyError(url, data);
            return QJsonObject();
        }
        return std::move(obj);
    });
};
class Klines{
public:
    /*!
    Возвращает массив свечей ВСЕХ!!!!!!!
    */
    static QJsonArray downloadKlines(const QString &symbol, const QString &interval, const QString &limit, const QString &begin = "", const QString &end = "");

    /*!
    принимает ОДНУ свечу!!!!!!!
    */
    static QCandlestickSet *toQCandlestickSetPtr(const QJsonArray &kline);

    static long long    time    (const QJsonArray &kline)   {return kline[0].toString().toLongLong(); }
    static double       open    (const QJsonArray &kline)   {return kline[1].toString().toDouble(); }
    static double       high    (const QJsonArray &kline)   {return kline[2].toString().toDouble(); }
    static double       low     (const QJsonArray &kline)   {return kline[3].toString().toDouble(); }
    static double       close   (const QJsonArray &kline)   {return kline[4].toString().toDouble(); }

};

class SmartMoney : public QObject
{
    Q_OBJECT
public:
    explicit SmartMoney(QObject *parent = nullptr);
    auto getOrders()const {return &orderMap;}
    bool isUpdateFinished{true};
    bool firstRun(){
        if(firstRunTrigger){
            firstRunTrigger = false;
            return true;
        }
        return false;
    }


public slots:
    void updateSmartMoney(int klinesPackageSize = 6);
    void replyFinished(QNetworkReply *reply);
private slots:
    void updateAreas(TradingWindow window, double filterPrice, double currentPrice);
signals:
    void updated(QJsonArray);
    void liquidsUpdated(TradingWindow);
    void updateProgressChanged(int);

private:
    bool firstRunTrigger{true};

    std::unordered_map<QString, QList<QJsonObject>> orderMap;
    QNetworkAccessManager *manager;
    void updateLiquids(const QJsonArray &klines, const QString &symbol);
    QList<QJsonObject> updateOrders(const QJsonArray &klines, const QString &symbol, const QString &takeProfit, const QString &side, const double curentPrice, const double filterPrice);
    std::unordered_map<QString, QJsonArray> downloadKlinesPackage(const QString &interval, const QString &limit, std::vector<QByteArray>::const_iterator begin = symbol::utf8.begin(), std::vector<QByteArray>::const_iterator end = symbol::utf8.end(), const QString &timeBegin = "", const QString &timeEnd = "");


};

#endif // SMARTMONEY_H
