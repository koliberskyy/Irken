#ifndef INSTRUMENTSINFO_H
#define INSTRUMENTSINFO_H

#include <cmath>
#include <unordered_map>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <mutex>
#include <thread>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkReply>
#include <QJsonArray>
#include <vector>
#include <iostream>
#include <iomanip>

namespace symbol {
const std::vector<QByteArray> utf8 = {
        "BTCUSDT",
        "BNBUSDT",
        "XRPUSDT",
        "EOSUSDT",
        "ETHUSDT",
        "LTCUSDT",
        "LINKUSDT",
        "TRXUSDT",
        "XLMUSDT",
        "ADAUSDT",
        "DASHUSDT",
        "ZECUSDT",
        "XTZUSDT",
        "ATOMUSDT",
        "ETCUSDT",
        "BCHUSDT",
        "SXPUSDT",
        "IOTAUSDT",
        "MATICUSDT",
        "FILUSDT",
        "NEARUSDT",
        "EGLDUSDT",
        "THETAUSDT",
        "DOTUSDT",
        "SOLUSDT",
        "NEOUSDT",
        "INJUSDT",
        "ENJUSDT",
        "EGLDUSDT",
        "SUIUSDT",
        "FTMUSDT",
        "1INCHUSDT",
        "SEIUSDT",
        "AVAXUSDT",
        "KSMUSDT",
        "SUSHIUSDT",
        "KAVAUSDT",
        "ZILUSDT",
        "ARBUSDT",
        "SANDUSDT",
        "UMAUSDT",
        "TRBUSDT",
        "ENSUSDT",
        "RUNEUSDT",
        "CELOUSDT",
        "GALAUSDT",
        "XAIUSDT",
        "DYDXUSDT",
        "IOTXUSDT",
        "COMPUSDT",
        "OCEANUSDT",
        "TIAUSDT",
        "LSKUSDT",
        "API3USDT",
        "RNDRUSDT"

};
const std::vector<QByteArray> intervals = {
    "240"
};




inline int id(const QByteArray &symbol){
    for(int i = 0; i < utf8.size(); i++){
        if(utf8[i] == symbol)
            return i;
    }
    return -1;// обращение к элементу массива с таким индесом вызовет краш - это и есть цель
}
}

namespace instruments{

void replyError(const QUrl &url, const QByteArray &reply);


enum class Filter_type{price, lotSize, leverage, lotSize_without_leverage_multipy};

class InstrumentsInfo
{
    struct Filter{
        QByteArray max{""};
        QByteArray min{""};
        double step{0.0};
        int dap{0}; //digits after point
        Filter(){dap = digits_after_point(QByteArray::fromStdString(std::to_string(step)));}
        QByteArray double_to_utf8(double val);
        int digits_after_point(const QByteArray &digit);

        bool isEmpty(){    if(max == "" || min == "" || step == 0.0)return true; else return false;}

        Filter &operator =(Filter &&move){ min = std::move(move.min); max = std::move(move.max); step = std::move(move.step); dap = std::move(move.dap); return *this;}
    };
    struct Price:public Filter{
        Price(){}
        Price(QJsonObject &&obj){max = obj["maxPrice"].toString().toUtf8();
                                 min = obj["minPrice"].toString().toUtf8();
                                 step = obj["tickSize"].toString().toDouble();
                                dap = digits_after_point(obj["tickSize"].toString().toUtf8());}
    };
    struct LotSize:public Filter{
        LotSize(){}
        LotSize(QJsonObject &&obj){max = obj["maxOrderQty"].toString().toUtf8();
                                   min = obj["minOrderQty"].toString().toUtf8();
                                   step = obj["qtyStep"].toString().toDouble();
                                  dap = digits_after_point(obj["qtyStep"].toString().toUtf8());}
    };
    struct Leverage:public Filter{
        Leverage(){}
        Leverage(QJsonObject &&obj){max = obj["maxLeverage"].toString().toUtf8();
                                   min = obj["minLeverage"].toString().toUtf8();
                                   step = obj["leverageStep"].toString().toDouble();
                                  dap = digits_after_point(obj["leverageStep"].toString().toUtf8());}
    };
    struct Filters{
        std::unordered_map<QByteArray, Price> price;
        std::unordered_map<QByteArray, LotSize> lotSize;
        std::unordered_map<QByteArray, Leverage> leverage;
    };

    Filters filters;
public:
    static double trading_leverage(const QByteArray &symbol);
    double maxLeverage(const QByteArray &symbol);
    explicit InstrumentsInfo();
    void update_filters();

    QByteArray double_to_utf8(const QByteArray &symbol, Filter_type f_type, double val);
};

static std::unique_ptr<InstrumentsInfo> ii = nullptr;

inline QByteArray double_to_utf8(const QByteArray &symbol, Filter_type f_type, double val){
    if (ii == nullptr){
        ii = std::make_unique<InstrumentsInfo>();
    }

    return ii->double_to_utf8(symbol, f_type, val);
}

inline double maxLeverage(const QByteArray &symbol){
    if (ii == nullptr){
        ii = std::make_unique<InstrumentsInfo>();
    }

    return ii->maxLeverage(symbol);
}

inline double tradingLeverage(const QByteArray &symbol){
    if (ii == nullptr){
        ii = std::make_unique<InstrumentsInfo>();
    }
    return InstrumentsInfo::trading_leverage(symbol);
}

}//namespace instrumetntinfo


#endif // INSTRUMENTSINFO_H
