#include "methods.h"

const QString Methods::getBalance(const QString &api, const QString &secret)
{
    QString method("https://api.bybit.com/v5/account/wallet-balance?");
    QByteArray query("accountType=UNIFIED&coin=USDT");

    auto obj = Requests::get(method, query, api, secret);
    if(!obj.isEmpty()){
        auto retCode = obj["retCode"].toInt();
        if(retCode == 0){
            auto result = obj["result"].toObject();
            auto b = result["list"].toArray()[0].toObject()["coin"].toArray()[0].toObject()["walletBalance"].toString();
            return b;
        }
    }

    return "";
}

const bybitInfo Methods::getPositionList(const QString &api, const QString &secret)
{
    QString method("https://api.bybit.com/v5/position/list?");
    QByteArray query("category=linear&settleCoin=USDT");

    auto obj = Requests::get(method, query, api, secret, api);
    bybitInfo info{obj};

    if(!obj.isEmpty()){
        if(info.retCode() == 0){
            return info;
        }
    }

    return bybitInfo();
}

const bybitInfo Methods::setTradingStop(const QString &symbol, const QString &api, const QString &secret, const QString &sl, const QString &tp)
{
    QString method("https://api.bybit.com/v5/position/trading-stop");
    QJsonObject obj;
    obj.insert("category", "linear");
    obj.insert("symbol", symbol);
    obj.insert("takeProfit", tp);
    obj.insert("stopLoss", sl);

    auto data = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    auto reply = Requests::post(method, data, api, secret, api);
    bybitInfo info{reply};

    if(!reply.isEmpty()){
        if(info.retCode() == 0){
            return info;
        }
    }

    return bybitInfo();
}

const bybitInfo Methods::setTradingStop(const QString &symbol, const QString &api, const QString &secret, double sl, double tp)
{
    return setTradingStop(symbol, api, secret,
                          instruments::double_to_utf8(symbol.toUtf8(), instruments::Filter_type::price, sl),
                          instruments::double_to_utf8(symbol.toUtf8(), instruments::Filter_type::price, tp));
}

const bybitInfo Methods::placeOrder(const QJsonObject &order, const QString &api, const QString &secret)
{
    QString method("https://api.bybit.com/v5/order/create");
    auto data = QJsonDocument(order).toJson(QJsonDocument::Compact);

    auto reply = Requests::post(method, data, api, secret, api);

    bybitInfo info{reply};

    if(!reply.isEmpty()){
        if(info.retCode() == 0){
            return info;
        }
    }

    return bybitInfo();

}

const bybitInfo Methods::setLeverage(const QString &symbol, double leverage, const QString &api, const QString &secret)
{
    QString method("https://api.bybit.com/v5/position/set-leverage");

    auto levStr = QString(instruments::double_to_utf8(symbol.toUtf8(), instruments::Filter_type::leverage, leverage));

    QJsonObject obj;
    obj.insert("symbol", symbol);
    obj.insert("category", "linear");
    obj.insert("buyLeverage", levStr);
    obj.insert("sellLeverage", levStr);

    auto data = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    auto reply = Requests::post(method, data, api, secret, api, settings::recv_window.toInt(), [](const QUrl &url, const QByteArray &data, const QByteArray &reply) ->QJsonObject
    {
       auto obj = QJsonDocument::fromJson(reply).object();
       auto retCode = obj["retCode"].toInt();
       if((retCode != 0 && retCode != 110043) || reply.isEmpty()){
           instruments::replyError(url, data, reply);
           return QJsonObject();
       }
       return std::move(obj);
    });

    bybitInfo info{reply};

    if(!reply.isEmpty()){
        if(info.retCode() == 0 || info.retCode() == 110043){
            return info;
        }
    }

    return bybitInfo();
}

const bybitInfo Methods::getOpenOrders(const QString &api, const QString &secret)
{
    QString method("https://api.bybit.com/v5/order/realtime?");
    QByteArray query("category=linear&settleCoin=USDT");

    auto obj = Requests::get(method, query, api, secret, api);
    bybitInfo info{obj};

    if(!obj.isEmpty()){
        if(info.retCode() == 0){
            return info;
        }
    }

    return bybitInfo();
}

const double Methods::qty_to_post(double acc_balance, double price, double percent_from_balance)
{
    if(percent_from_balance <= 0)
        return settings::one_order_qty_pc_from_balance * acc_balance / (100 * price);

    return percent_from_balance * acc_balance / (100 * price);
}
