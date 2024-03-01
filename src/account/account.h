#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QObject>
#include <QEventLoop>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTreeWidgetItem>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageAuthenticationCode>
#include <thread>
#include <QTimer>
#include <QThread>
#include <iostream>
#include <iomanip>


#include "settings.h"
#include "instrumentsinfo.h"


class Position{
public:
    virtual void foo() = 0;
    static bool setTradingStop(const QJsonObject &pos, const QString &api, const QString &secret, const QString &sl = "", const QString &tp = "");
    static void reducePosition(const QJsonObject &pos, double reducePercent, const QString &api, const QString &secret);

    /*!
    возвращает ПНЛ в процентах (НЕ В СОТЫХ ДОЛЯХ, А В ПРОЦЕНТАХ БЛЯДЬ)
    */
    static double unrealizedPnlPercent(const QJsonObject &pos);//
    /*!
    возвращает ПНЛ в процентах (НЕ В СОТЫХ ДОЛЯХ, А В ПРОЦЕНТАХ БЛЯДЬ)
    */
    static QString unrealizedPnlPercentQString(const QJsonObject &pos);
    /*!
    принимает ПНЛ в процентах (НЕ В СОТЫХ ДОЛЯХ, А В ПРОЦЕНТАХ БЛЯДЬ)
    */
    static double markPriceFromPnl(const QJsonObject &pos, double pnl);


};


class Order{
public:
    virtual void foo() = 0;
    static bool place(const QJsonObject &order, const QString &api, const QString &secret);
    static bool place(const QJsonObject &order, const QString &api, const QString &secret, double qty);
    static void batch_place(const QList<QJsonObject> orders, const QString &api, const QString &secret, const double balance);
    static void batch_cancel(const QList<QJsonObject> orders, const QString &api, const QString &secret);
    static double qty_to_post(double acc_balance, double price);
    static double qty_to_post(double acc_balance, double price, double percent_from_balance);


};

class Account : public QObject
{
    Q_OBJECT
    QJsonObject data;
    QJsonArray positions;
    struct reducePair{
        int reducePnl;          //процент при котром закрывается чсть позиции и перемещается стоп
        int stopPnl;            //процент на который перетавляется стоп лосс
        double reducePercent;   //процент на который закрывается позиция
    };
    std::vector<reducePair> reduceTable;

public:
    explicit Account(QJsonObject &&acc_json, QObject *parent = nullptr);
    //account
    QTreeWidgetItem toTreeItem() const;
    QTreeWidgetItem *toTreeItemPtr() const;
    //positions
    void moveStopAndReduse(QJsonObject &&pos);
    //каждая из лямбд возвращает индекс элемента в таблице сокращений или -1 в случае несоблюдения условий
    std::vector<std::function<int(std::vector<reducePair> table, const QJsonObject &pos)>> vec_positionControlLambdas;
    //orders
    void refreshOrderList(const QList<QJsonObject> &ordersToPost);
    double getBalance()const {return data["balance"].toString().toDouble();}
    QByteArray api() const;


public slots:
    void updateBalance();
    void updatePositions();
    void updateOrders();
    void placeOrder(QJsonObject order);
    void replyFinished(QNetworkReply *reply);
    void cancelOrder(QJsonObject order);


signals:
    void balanceUpdated(QString);
    void positionsUpdated(QJsonArray);
    void ordersUpdated(QJsonArray);

private:
    QByteArray secret() const;
    double balance()const;
    QNetworkAccessManager *netManager;

    QList<QJsonObject> orders;

    void parceReply(const QJsonObject &obj, const QUrl &url);
    void posDownloaded(const QJsonObject &obj);
    void ordDownloaded(const QJsonObject &obj);
    void setLeverage(double leverage = 20.0);

public:
    static std::vector<std::pair<QByteArray, QByteArray> > make_headers(const QByteArray &data, const QByteArray &api_key, const QByteArray &secret_key);
    static QByteArray make_signature(const QByteArray &data, const QByteArray &ts, const QByteArray &api_key, const QByteArray &secret_key);
    static QByteArray timestamp();

};

#endif // ACCOUNT_H
