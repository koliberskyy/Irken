#include "smartmoney.h"

SmartMoney::SmartMoney(QObject *parent)
    : QObject{parent},
      manager{new QNetworkAccessManager()}
{
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
}

void SmartMoney::updateSmartMoney()
{
    for(auto symbol : symbol::utf8){
            QByteArray query("category=linear&symbol=" + symbol + "&interval=" + "W" + "&limit=320");
            QUrl url("https://api.bybit.com/v5/market/mark-price-kline?" + std::move(query));
            url.setUserInfo("W");
            QNetworkRequest req(url);

            manager->get(req);
    }
}

void SmartMoney::replyFinished(QNetworkReply *reply)
{
    auto url = reply->request().url();
    auto data = reply->readAll();
    auto obj = QJsonDocument::fromJson(data).object();
    auto retCode = obj["retCode"].toString().toInt();

    auto interval = url.userInfo();

    if(retCode != 0 || data.isEmpty() || url.userInfo() == ""){
        instruments::replyError(url, data);
    }
    else if(interval == "W"){
        auto result = obj["result"].toObject();
        updateLiquids(result["list"].toArray(), result["symbol"].toString());
    }
    else{

    }

    reply->deleteLater();
}

void SmartMoney::updateAreas(TradingWindow window, double currentPrice)
{
    auto symbol = window.symbol;

    auto toByteArray = [](qint64 count){return QByteArray(std::to_string(count).c_str());};

    auto lowStart = window.lowWindowBegin.toMSecsSinceEpoch();
    auto lowEnd = window.lowWindowEnd.toMSecsSinceEpoch();

    auto highStart = window.highWindowBegin.toMSecsSinceEpoch();
    auto highEnd = window.highWindowEnd.toMSecsSinceEpoch();

    auto forHighTakeProfit = instruments::double_to_utf8(symbol.toUtf8(), instruments::Filter_type::price, window.lowWindowPriceHigh);
    auto forLowTakeProfit = instruments::double_to_utf8(symbol.toUtf8(), instruments::Filter_type::price, window.highWindowPriceLow);


    if(highStart > 0 && highEnd > 0 && window.isHighZoneAvaible) //закоментировано условие вхождения текущей цены в зону
        for(auto interval: symbol::intervals){
            QEventLoop loop;
            QTimer timer;
            QNetworkAccessManager mgr;
            QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
            connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
            QByteArray query("category=linear&symbol=" + symbol.toUtf8() + "&interval=" + interval + "&limit=1000" + "&start=" + toByteArray(highStart) + "&end=" + toByteArray(highEnd));
            QUrl url("https://api.bybit.com/v5/market/mark-price-kline?" + std::move(query));
            url.setUserInfo("h" + interval);
            QNetworkRequest req(url);
            mgr.get(req);

            timer.start(10000);
            QNetworkReply *reply = mgr.get(req);
            loop.exec();

            auto byteArray = reply->readAll();

            if(!byteArray.isEmpty()){
                auto obj = QJsonDocument::fromJson(byteArray);
                auto result = obj["result"].toObject();
                auto orders = updateOrders(result["list"].toArray(), result["symbol"].toString(), forHighTakeProfit, "Sell", currentPrice);
                if(!orders.empty()){
                    //выдаем дальше
                    orderMap[symbol] = std::move(orders);
                    break;
                }
            }
            else{
                break;
            }
            reply->deleteLater();
        }

        if(lowStart > 0 && lowEnd > 0 && !window.isHighZoneAvaible) //закоментировано условие вхождения текущей цены в зону
            for(auto interval: symbol::intervals){
                QEventLoop loop;
                QTimer timer;
                QNetworkAccessManager mgr;
                QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
                connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
                QByteArray query("category=linear&symbol=" + symbol.toUtf8() + "&interval=" + interval + "&limit=1000" + "&start=" + toByteArray(lowStart) + "&end=" + toByteArray(lowEnd));
                QUrl url("https://api.bybit.com/v5/market/mark-price-kline?" + std::move(query));
                url.setUserInfo("h" + interval);
                QNetworkRequest req(url);
                mgr.get(req);

                timer.start(10000);
                QNetworkReply *reply = mgr.get(req);
                loop.exec();

                auto byteArray = reply->readAll();

                if(!byteArray.isEmpty()){
                    auto obj = QJsonDocument::fromJson(byteArray);
                    auto result = obj["result"].toObject();

                    auto orders = updateOrders(result["list"].toArray(), result["symbol"].toString(), forLowTakeProfit, "Buy", currentPrice);
                    if(!orders.isEmpty()){
                        //выдаем дальше
                        orderMap[symbol] = std::move(orders);
                        break;
                    }
                }
                else{
                    break;
                }


            reply->deleteLater();
        }

}


void SmartMoney::updateLiquids(const QJsonArray &klines, const QString &symbol)
{

    auto it = klines.size() - 1;

    std::set<liquid> hights;
    std::set<liquid> lows;

    hights.clear();
    lows.clear();


    hights.insert(liquid{klines.at(it).toArray().at(2).toString().toDouble(), QDateTime::fromMSecsSinceEpoch(klines.at(it).toArray().at(0).toString().toLongLong()), false});
    lows.emplace(liquid{klines.at(it).toArray().at(3).toString().toDouble(), QDateTime::fromMSecsSinceEpoch(klines.at(it).toArray().at(0).toString().toLongLong()), false});

    it--;

    for(; it != 0; it--){

        //liquids
        auto high = liquid{klines[it].toArray()[2].toString().toDouble(),
                QDateTime::fromMSecsSinceEpoch(klines.at(it).toArray().at(0).toString().toLongLong()),
                false};
        auto low = liquid{klines[it].toArray()[3].toString().toDouble(),
                QDateTime::fromMSecsSinceEpoch(klines.at(it).toArray().at(0).toString().toLongLong()),
                false};


        auto higher = hights.upper_bound(high);

        //high
        if(higher == hights.end()){
            hights.clear();
            hights.insert(high);

            low.setRed();
            lows.insert(low);

        }
        else if(higher == hights.begin()){
            //do nothing
        }
        else{
            hights.erase(hights.begin(), higher);
            hights.insert(high);

            auto new_hights = hights;
            hights.clear();

            for( auto &it : new_hights){
                auto liquid = it;
                liquid.setBlue();
                hights.insert(liquid);
            }

            low.setRed();
            lows.insert(low);
        }

        auto lower = lows.upper_bound(low);

        //low
        if(lower == lows.begin()){
            lows.clear();
            lows.insert(low);

            high.setRed();
            hights.insert(high);
        }
        else if(lower == lows.end()){
            //do nothing
        }
        else{
            lows.erase(lower, lows.end());
            lows.insert(low);

            auto new_lows = lows;
            lows.clear();

            for( auto &it : new_lows){
                auto liquid = it;
                liquid.setBlue();
                lows.insert(liquid);
            }

            high.setRed();
            hights.insert(high);
        }
    }



    auto currentPrice = klines.begin()->toArray()[4].toString().toDouble();

    TradingWindow window;
    window.symbol = symbol;

    auto highestPrice = klines.begin()->toArray()[2].toString().toDouble();
    auto lowestPrice = klines.begin()->toArray()[3].toString().toDouble();

    //setWindows
    auto low_iter = lows.rbegin();
    if(low_iter->isRed){
        if(hights.begin()->price > highestPrice)
            low_iter++;
    }
    if(low_iter != lows.rend()){
        window.lowWindowEnd = low_iter->time;
        window.lowWindowPriceHigh = low_iter->price;

        low_iter++;
    }
    if(low_iter != lows.rend()){
        window.lowWindowBegin = low_iter->time;
        window.lowWindowPriceLow = low_iter->price;

    }

    auto high_iter = hights.begin();

    if(high_iter->isRed){
        if(lows.rbegin()->price < lowestPrice)
            high_iter++;
    }
    if(high_iter != hights.end()){
        window.highWindowEnd = high_iter->time;
        window.highWindowPriceLow = high_iter->price;

        high_iter++;
    }
    if(high_iter != hights.end()){
        window.highWindowBegin = high_iter->time;
        window.highWindowPriceHigh = high_iter->price;

    }


    if((currentPrice > window.highWindowPriceLow && currentPrice < window.highWindowPriceHigh)){
        window.isHighZoneAvaible = true;
        updateAreas(window, highestPrice);
    }

    if((currentPrice > window.lowWindowPriceLow && currentPrice < window.lowWindowPriceHigh)){
        window.isHighZoneAvaible = false;
        updateAreas(window, lowestPrice);
    }



}

QList<QJsonObject> SmartMoney::updateOrders(const QJsonArray &klines, const QString &symbol, const QString &takeProfit, const QString &side, const double currentPrice)
{

    QList<QJsonObject> reply;

    auto time =     [&klines](int i){return klines.at(i).toArray()[0].toString().toLongLong(); };
    auto open =     [&klines](int i){return klines.at(i).toArray()[1].toString().toDouble(); };
    auto high =     [&klines](int i){return klines.at(i).toArray()[2].toString().toDouble(); };
    auto low =      [&klines](int i){return klines.at(i).toArray()[3].toString().toDouble(); };
    auto close =    [&klines](int i){return klines.at(i).toArray()[4].toString().toDouble(); };

    auto range =        [](double hight, double low){if(hight > low )return hight - low; else return -1.0;};
    auto isInclude =    [](double price, double hight, double low){return (price < hight && price > low);};
    auto isGreen =      [](double open, double close){return close > open;};
    auto findHigh =     [&high, &low](int begin, int end){
        double highest{-1.0};
        if(end > begin){
            while(begin != end){
                if(high(begin) > highest){
                    highest = high(begin);
                }
                begin++;
            }
        }
        return highest;
    };

    auto findLow =     [&high, &low](int begin, int end){
        double lowest{10000000000000.0};
        if(end > begin){
            while(begin != end){
                if(low(begin) < lowest){
                    lowest = low(begin);
                }
                begin++;
            }
        }
        return lowest;
    };

    auto ORDERBLOOOOOOKSUUUUKAAA = [&time, &high, &low, &symbol, &side, &takeProfit, &reply, &currentPrice, &findLow, &findHigh](int index){
        QJsonObject obj;
        auto dateTime = QDateTime::fromMSecsSinceEpoch(time(index));
        auto h = high(index);
        auto l = low(index);
        auto breakPoint = true;
        auto price = (h+l)/2;// TBX

        auto actuality = price;
        if(side == "Buy"){
            if(currentPrice < price)
                actuality =  -1.0;
            else{
                //(bolshee - menshee / menshee) * 100
                actuality = 100*(currentPrice - price)/price;
            }
        }
        else{
            if(currentPrice > price)
                actuality = -1.0;
            else{
                //(bolshee - menshee / menshee) * 100
                actuality = 100*(price - currentPrice)/currentPrice;
            }
        }

        //index - 2 потому что надо пропустить сам об и его имбаланс
        auto lowest = findLow(0, index-2);
        auto highest = findHigh(0, index-2);

        if((side == "Buy" && price < currentPrice && price < lowest)
                || (side == "Sell" && price > currentPrice && price > highest)){
            if(actuality < settings::actualityFilter){
                auto stopLoss = [&side, &h, &l](){
                    if(side == "Buy"){
                        return l * 0.995;
                    }
                    else{
                        return h * 1.005;
                    }
                }();

                obj.insert("symbol", symbol);
                obj.insert("side", side);
                obj.insert("orderType", "Limit");
                obj.insert("price", QString::fromUtf8(instruments::double_to_utf8(symbol.toUtf8(), instruments::Filter_type::price, price)));
                obj.insert("takeProfit", takeProfit);
                obj.insert("stopLoss", QString::fromUtf8(instruments::double_to_utf8(symbol.toUtf8(), instruments::Filter_type::price, stopLoss)));

//                //#ifdef DEBUG
//                obj.insert("", dateTime.toString());
//                //#endif

                reply.append(obj);
            }
        }

    };

    auto ordersMustBeLong = (side == "Buy");

    // from 1 to size - 1 потому что ималанс ищем изучая теущую свечу относительно следуюзщей и предидущей
    for(auto i = 1; i < klines.size()-1; i++){
        //OB
        //imbalance

        //check imbalance

        //looooooooooooooooooooooooooooooooooooooooooooooooooooooooooong
        if(ordersMustBeLong
                && range(high(i), low(i)) > range(high(i-1), low(i-1)) + range(high(i+1), low(i+1))
                && low(i-1) > high(i+1))
        {
            auto dateImbalance = QDateTime::fromMSecsSinceEpoch(time(i));

            //find absorbed (hight || low)
            if(isGreen(open(i), close(i))){
                const auto idxOb = i+1;               //индекс свечи ордерблока
                auto lowerPriceIdxShift = 0;    //смещение индеса свечи соседствующей с ордерлоком (в случае когда её лой ниже лоя ордерблока)
                double boss{-1.0};              //ищем мы именно его

                auto lowerPrice = low(idxOb);
                while(low(idxOb+lowerPriceIdxShift) <= lowerPrice){
                    lowerPrice = low(idxOb+lowerPriceIdxShift);
                    lowerPriceIdxShift++;
                    if(lowerPriceIdxShift + idxOb == klines.size()){
                        break;
                    }
                }

                if(lowerPriceIdxShift + idxOb != klines.size()){
                    //потому что цикл перед проверкой условия итерирует индекс
                    lowerPriceIdxShift--;
                    auto firstLow = low(idxOb + lowerPriceIdxShift);
                    auto firstLowIdx = idxOb + lowerPriceIdxShift;

                    auto index = firstLowIdx;               //индекс рассматриваемой в цикле свечи
                    while(low(index) >= firstLow){
                        index++;
                        if(index == klines.size()){
                            break;
                        }
                    }
                    if(index != klines.size()){
                        auto secondLow = low(index);
                        auto secondLowIdx = index;

                        boss = findHigh(firstLowIdx, secondLowIdx);

                        auto date_firstLow = QDateTime::fromMSecsSinceEpoch(time(firstLowIdx));
                        auto date_secondLow = QDateTime::fromMSecsSinceEpoch(time(secondLowIdx));
                        auto date_ob = QDateTime::fromMSecsSinceEpoch(time(idxOb));
                        auto breakpoint = true;
                    }

                }



                //если босс собран телом ибаланса и хай ордерблока меньше босса
                if(isInclude(boss, close(i), open(i)) && boss > high(idxOb)){
                    ORDERBLOOOOOOKSUUUUKAAA(idxOb);
                }
                else{
                    auto breakPoint = true;
                }

            }

        }
        //end of //looooooooooooooooooooooooooooooooooooooooooooooooooooooooooong

        //shoooooooorrrrrrrrrrtttttttttt
        if(!ordersMustBeLong
                && range(high(i), low(i)) > range(high(i-1), low(i-1)) + range(high(i+1), low(i+1))
                && low(i+1) > high(i-1))
        {
            auto dateImbalance = QDateTime::fromMSecsSinceEpoch(time(i));

            //find absorbed (hight || low)
            if(!isGreen(open(i), close(i))){
                const auto idxOb = i+1;               //индекс свечи ордерблока
                auto higherPriceIdxShift = 0;    //смещение индеса свечи соседствующей с ордерлоком (в случае когда её лой ниже лоя ордерблока)
                double boss{-1.0};              //ищем мы именно его

                auto higherPrice = high(idxOb);
                while(high(idxOb+higherPriceIdxShift) >= higherPrice){ //<---- надо убрать лишнюю итерацию
                    higherPrice = high(idxOb+higherPriceIdxShift);
                    higherPriceIdxShift++;
                    if(higherPriceIdxShift + idxOb == klines.size()){
                        break;
                    }
                }

                if(higherPriceIdxShift + idxOb != klines.size()){
                    //потому что цикл перед проверкой условия итерирует индекс
                    higherPriceIdxShift--;
                    auto firstHigh = high(idxOb + higherPriceIdxShift);
                    auto firstHighIdx = idxOb + higherPriceIdxShift;

                    auto index = firstHighIdx;               //индекс рассматриваемой в цикле свечи
                    while(high(index) <= firstHigh){
                        index++;
                        if(index == klines.size()){
                            break;
                        }
                    }
                    if(index != klines.size()){
                        auto secondHigh = high(index);
                        auto secondHighIdx = index;

                        boss = findLow(firstHighIdx, secondHighIdx);

                        auto date_firstHigh= QDateTime::fromMSecsSinceEpoch(time(firstHighIdx));
                        auto date_secondHigh = QDateTime::fromMSecsSinceEpoch(time(secondHighIdx));
                        auto date_ob = QDateTime::fromMSecsSinceEpoch(time(idxOb));
                        auto breakpoint = true;
                    }

                }

                //если босс собран телом ибаланса и хай ордерблока меньше босса
                if(isInclude(boss, open(i), close(i)) && boss < low(idxOb)){
                    ORDERBLOOOOOOKSUUUUKAAA(idxOb);
                }
                else{
                    auto breakPoint = true;
                }

            }

        }
        //end of //shoooooooorrrrrrrrrrtttttttttt
    }

    return reply;
}
