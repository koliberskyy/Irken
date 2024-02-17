#include "instrumentsinfo.h"
namespace instruments{

double InstrumentsInfo::trading_leverage(const QByteArray &symbol)
{
    return 10.0;
}

double InstrumentsInfo::maxLeverage(const QByteArray &symbol)
{
    return filters.leverage[symbol].max.toDouble();
}

InstrumentsInfo::InstrumentsInfo()
{
    update_filters();
}

//GET /v5/market/instruments-info?category=linear&symbol=BTCUSDT HTTP/1.1
void InstrumentsInfo::update_filters()
{
    std::mutex mutex;
    auto lambada = [filt = &filters, mtx = &mutex](const QByteArray &symbol){
        if(filt->price[symbol].isEmpty() || filt->leverage[symbol].isEmpty() || filt->lotSize[symbol].isEmpty() )
        {
            QEventLoop eventLoop;
            QNetworkAccessManager mgr;
            QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
            QByteArray query("symbol=" + symbol + "&category=linear");
            QUrl url("https://api.bybit.com//v5/market/instruments-info?" + std::move(query));

            QNetworkRequest req(url);

            QNetworkReply *reply = mgr.get(req);
            eventLoop.exec();

            if (reply->error() == QNetworkReply::NoError){
                auto obj = QJsonDocument::fromJson(reply->readAll()).object();
                auto retCode = obj["retCode"].toString().toUInt();
                if(retCode == 0){
                    auto list = obj["result"].toObject()["list"].toArray()[0].toObject();


                    filt->price[symbol] = list["priceFilter"].toObject();
                    filt->lotSize[symbol] = list["lotSizeFilter"].toObject();
                    filt->leverage[symbol] = list["leverageFilter"].toObject();

                }
                else{
                    qDebug() << "Failure:" << retCode << "\n";
                }

            }
            else {
                //failure
                qDebug() << "Failure" << QJsonDocument::fromJson(reply->readAll()).toJson();
            }
            reply->deleteLater();


        }
    };

    std::vector<std::thread> thr_vec;
    int threadLimit{0};
    for(const auto &it : symbol::utf8){
        thr_vec.emplace_back(std::thread(lambada, it));
        threadLimit++;
        if(threadLimit == 10){
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        }
    }
    for(auto &it : thr_vec){
        if(it.joinable()){
            it.join();
        }
    }
}

QByteArray InstrumentsInfo::double_to_utf8(const QByteArray &symbol, Filter_type f_type, double val)
{
    switch (f_type) {
    case Filter_type::price:
        return filters.price[symbol].double_to_utf8(val);
        break;
    case Filter_type::leverage:
        return filters.leverage[symbol].double_to_utf8(val);
        break;
    case Filter_type::lotSize:
        return filters.lotSize[symbol].double_to_utf8(val * trading_leverage(symbol));
        break;
    case Filter_type::lotSize_without_leverage_multipy:
        return filters.lotSize[symbol].double_to_utf8(val);
        break;
    default:
        return "";
    }
}

//32.598 && step = 0.05 -> 32.55
QByteArray InstrumentsInfo::Filter::double_to_utf8(double val)
{
//    if(max == "" || min == "" || step == 0.0 || dap ==0)
//        return "";

    if(val >= max.toDouble()){
        return max;
    }
    if(val <= min.toDouble()){
        return min;
    }

    //0.598
    auto mantis = val - floor(val);
    //32.0
    auto digit = floor(val);
    //59
    mantis = floor(mantis * pow(10, dap));
    //55         59             5
    mantis -= int(mantis) % int(step * pow(10, dap));
    //0.55
    mantis /= pow(10, dap);
    //32.55

    //zamena zapyatoy na tochku
    auto res = QByteArray::fromStdString(std::to_string(digit + mantis));
    auto iter = res.begin();
    while(iter != res.end()){
        if(*iter == ','){
            *iter = '.';
            break;
        }
        iter++;
    }

    return std::move(res);
}


//2324.151617 -> '.' at i(5) && size = 11 && 11 - 5 == 6
int InstrumentsInfo::Filter::digits_after_point(const QByteArray &digit)
{
    int i = 0;
    while(digit.at(i) != '.' && digit.at(i) != ','){
                i++;
        //sigmentation fault evasion
        if(i == digit.size())
            break;
    }
    return digit.size() - i;
}

void replyError(const QUrl &url, const QByteArray &reply)
{
    time_t timer{std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())};
    std::tm bt = *std::localtime(&timer);
    std::cout << std::put_time(&bt, "%d.%m.%y, %H:%M") << "_______________\n" << std::endl;
    std::cout << "Failure: \n" << "Request: " << url.url().toStdString()
              << "\nQuery: " << url.query().toStdString()
              << "\nReply: \n" << QJsonDocument::fromJson(reply).toJson().toStdString()
              << "\nuser Info: " << url.userInfo().toStdString() << std::endl;
}

QByteArray timeToByteArray(QDateTime count){
    return QByteArray(std::to_string(count.toMSecsSinceEpoch()).c_str());}


}//namespace bybit, instrumentsinfo
