#include "klines.h"

Klines::Klines(const QString &__symbol, const QString &__timeframe, const QString &__limit)
    : symbol{__symbol}, timeframe{__timeframe}, limit{__limit}
{
    connect_manager(this);
    download();

    QSignalSpy spy = QSignalSpy(this, SIGNAL(reply_complete()));
    spy.wait(500000);
}

Klines::Klines(Klines &&moved) noexcept:
    symbol{std::move(moved.symbol)},
    timeframe{std::move(moved.timeframe)},
    limit{std::move(moved.limit)},
    klines{std::move(moved.klines)}
{
}

void Klines::download()
{
    QUrl url = "https://fapi.binance.com/fapi/v1/klines?symbol=" +
               symbol + "&interval=" +
               timeframe + "&limit=" +
               limit;

    get(std::move(url));
}

void Klines::create(QJsonArray &&arr)
{

    if (!klines.empty()){
        klines.clear();
    }

    klines.reserve(arr.size());

    for(auto i = 0, i_prev = 0; i < arr.size(); i++){
        klines.emplace_back(CandleStick(arr[i][0].toInteger(),
                                        arr[i][1].toString().toDouble(),
                                        arr[i][2].toString().toDouble(),
                                        arr[i][3].toString().toDouble(),
                                        arr[i][4].toString().toDouble(),
                                        arr[i][6].toInteger()
                                        )
                            );
    }
}

void Klines::set_thrand()
{
    if(klines.at(klines.size()-24).hight * 1.01 < klines.at(klines.size()-1).hight)
        thrand = Thrand::up;
    else if(klines.at(klines.size()-24).low > klines.at(klines.size()-1).low * 1.01)
        thrand = Thrand::down;
    else
        thrand = Thrand::range;
}

void Klines::replyFinished(QNetworkReply *reply)
{
     QJsonDocument doc(QJsonDocument::fromJson(reply->readAll()));
     reply->deleteLater();

     create(doc.array());
     if(klines.size() != 0)
        set_thrand();

     emit reply_complete();
}
