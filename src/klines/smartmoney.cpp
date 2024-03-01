#include "smartmoney.h"

SmartMoney::SmartMoney(QObject *parent)
    : QObject{parent},
      manager{new QNetworkAccessManager()}
{
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
}

void SmartMoney::updateSmartMoney(int klinesPackageSize)
{
    orderMap.clear();
    //копируем по значению так как после thr.detach() klinesPackageSize перестанет сущетсвовать
    auto lambda = [this, packageSize = klinesPackageSize](){
        isUpdateFinished = false;
        emit updateProgressChanged(1);
        std::unordered_map<QString, QJsonArray> oneWeekTf;

        auto download = [&packageSize, this](std::unordered_map<QString, QJsonArray> *mapPtr, const QString &interval, const QString &limit){
            auto begin = symbol::utf8.begin();
            auto it = symbol::utf8.begin();
            auto end = symbol::utf8.end();
            auto counter{0};

            while(it != end){
                counter++;
                it++;
                if(counter == packageSize || it == end){
                    counter = 0;
                    auto pack = downloadKlinesPackage(interval, limit, begin, it);
                    mapPtr->insert(pack.begin(), pack.end());
                    begin = it;
                }
            }
        };

        download(&oneWeekTf, "W", "320");
        emit updateProgressChanged(99-oneWeekTf.size());
        auto currentProgress = 99-oneWeekTf.size();
        std::vector<std::thread> thr_vec;
        auto lambda = [&oneWeekTf, this, &currentProgress](std::unordered_map<QString, QJsonArray>::const_iterator it){
            updateLiquids(it->second, it->first);
            currentProgress++;
            emit updateProgressChanged(currentProgress);
        };
        for(auto it = oneWeekTf.cbegin(); it != oneWeekTf.cend(); it++){
            thr_vec.emplace_back(lambda, it);
        }
        for(auto &it : thr_vec){
            if(it.joinable()){
                it.join();
            }
        }
        emit updateProgressChanged(100);
        isUpdateFinished = true;
        emit(updated(QJsonArray()));
    };
    std::thread thr(lambda);
    thr.detach();
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

void SmartMoney::updateAreas(TradingWindow window, double filterPrice, double currentPrice)
{
    auto symbol = window.symbol;

    auto lowStart = window.lowWindowBegin.toMSecsSinceEpoch();
    auto lowEnd = window.lowWindowEnd.toMSecsSinceEpoch();

    auto highStart = window.highWindowBegin.toMSecsSinceEpoch();
    auto highEnd = window.highWindowEnd.toMSecsSinceEpoch();

    auto forHighTakeProfit = instruments::double_to_utf8(symbol.toUtf8(), instruments::Filter_type::price, window.lowWindowPriceHigh);
    auto forLowTakeProfit = instruments::double_to_utf8(symbol.toUtf8(), instruments::Filter_type::price, window.highWindowPriceLow);


    if(highStart > 0 && highEnd > 0 && window.isHighZoneAvaible) //закоментировано условие вхождения текущей цены в зону
        for(auto interval: symbol::intervals){
            auto klines = QJsonArray();
            while (klines.empty()){
                klines = Klines::downloadKlines(symbol, interval, "1000", instruments::timeToByteArray(window.highWindowBegin), instruments::timeToByteArray(window.highWindowEnd));
            }
            auto orders = updateOrders(klines, symbol, forHighTakeProfit, "Sell", currentPrice, filterPrice);

            if(!orders.empty())
                orderMap[symbol] = std::move(orders);

        }

        if(lowStart > 0 && lowEnd > 0 && !window.isHighZoneAvaible) //закоментировано условие вхождения текущей цены в зону
            for(auto interval: symbol::intervals){
                auto klines = QJsonArray();
                while (klines.empty()){
                    klines = Klines::downloadKlines(symbol, interval, "1000", instruments::timeToByteArray(window.lowWindowBegin), instruments::timeToByteArray(window.lowWindowEnd));
                }
                auto orders = updateOrders(klines, symbol, forLowTakeProfit, "Buy", currentPrice, filterPrice);

                if(!orders.empty())
                    orderMap[symbol] = std::move(orders);
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
        updateAreas(window, highestPrice, currentPrice);
    }

    if((currentPrice > window.lowWindowPriceLow && currentPrice < window.lowWindowPriceHigh)){
        window.isHighZoneAvaible = false;
        updateAreas(window, lowestPrice, currentPrice);
    }



}

QList<QJsonObject> SmartMoney::updateOrders(const QJsonArray &klines, const QString &symbol, const QString &takeProfit, const QString &side, const double currentPrice, const double filterPrice)
{

    QList<QJsonObject> reply;

    auto time =     [&klines](int i){return Klines::time(klines.at(i).toArray());};
    auto open =     [&klines](int i){return Klines::time(klines.at(i).toArray()); };
    auto high =     [&klines](int i){return Klines::time(klines.at(i).toArray()); };
    auto low =      [&klines](int i){return Klines::time(klines.at(i).toArray()); };
    auto close =    [&klines](int i){return Klines::time(klines.at(i).toArray()); };

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

    auto ORDERBLOOOOOOKSUUUUKAAA = [&time, &high, &low, &symbol, &side, &takeProfit, &reply, currentPrice, &findLow, &findHigh, filterPrice](int index){
        QJsonObject obj;
        auto h = high(index);
        auto l = low(index);
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

        if((side == "Buy" && price < filterPrice && price < lowest)
                || (side == "Sell" && price > filterPrice && price > highest)){
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

QJsonArray Klines::downloadKlines(const QString &symbol, const QString &interval, const QString &limit, const QString &begin, const QString &end)
{
    QByteArray query("category=linear&symbol=" + symbol.toUtf8() + "&interval=" + interval.toUtf8() + "&limit=1000");

    if(!begin.isEmpty() && !end.isEmpty())
        query.append("&start=" + begin.toUtf8() + "&end=" + end.toUtf8());
    else if((begin.isEmpty() && !end.isEmpty()) || (!begin.isEmpty() && end.isEmpty())){
        std::cout << "ERROR in downloadKlines: begin must not be empty when end is not empty or vice";
        return QJsonArray();
    }

    QUrl url("https://api.bybit.com/v5/market/mark-price-kline?" + std::move(query));
    url.setUserInfo("klines-" + symbol + "-" + interval);

    auto customReplyParser = [](const QUrl &url, const QByteArray &data) ->QJsonObject
        {
            auto obj = QJsonDocument::fromJson(data).object();
            auto retCode = obj["retCode"].toInt();
            if(retCode != 0){
                instruments::replyError(url, data);
                return QJsonObject();
            }
            return std::move(obj);
        };

    auto obj = requests::get(url, query, "klines-" + symbol + "-" + interval, 10000, customReplyParser);

    if(!obj.isEmpty()){
        auto result = obj["result"].toObject();
        auto list = result["list"].toArray();

        if(!list.empty()){
            //выдаем дальше
            return std::move(list);
        }
    }
    return QJsonArray();
}

QCandlestickSet *Klines::toQCandlestickSetPtr(const QJsonArray &kline)
{
    return new QCandlestickSet(open(kline), high(kline), low(kline), close(kline), time(kline));
}

QCandlestickSet Klines::toQCandlestickSet(const QJsonArray &kline)
{
    return QCandlestickSet(open(kline), high(kline), low(kline), close(kline), time(kline));
}

std::unordered_map<QString, QJsonArray> SmartMoney::downloadKlinesPackage(const QString &interval, const QString &limit, std::vector<QByteArray>::const_iterator begin, std::vector<QByteArray>::const_iterator end, const QString &timeBegin, const QString &timeEnd)
{

    std::vector<std::thread> thr_vec;
    std::mutex mtx;
    std::unordered_map<QString, QJsonArray> reply;
    if(begin < end){
        std::vector<QString> downloadDeque(begin, end);
        auto lambda = [&reply, &interval, &limit, &timeBegin, &timeEnd, this, &mtx](const QString &symbol){
            mtx.lock();
            reply[symbol] = Klines::downloadKlines(symbol, interval, limit, timeBegin, timeEnd);
            mtx.unlock();
        };
        auto sucscess = false;
        while (!sucscess){
            if(!thr_vec.empty()){
                thr_vec.clear();
            }

            auto dequeBegin = downloadDeque.begin();
            auto dequeEnd = downloadDeque.end();

            while(dequeBegin != dequeEnd){
                thr_vec.emplace_back(lambda, *dequeBegin);
                dequeBegin++;
            }

            for(auto &it : thr_vec){
                if(it.joinable())
                    it.join();
            }

            for(auto &it : reply){
                if(!it.second.isEmpty()){
                    auto finded = std::find(downloadDeque.begin(), downloadDeque.end(), it.first);
                    if(finded != downloadDeque.end()){
                        downloadDeque.erase(finded);
                    }
                }
            }

            if(downloadDeque.empty()){
                sucscess = true;
                break;
            }
        }
    }
    return reply;
}

QJsonObject requests::get(QUrl url, const QByteArray &query, const QString &userInfo, int recWindow_msec, std::function<QJsonObject (const QUrl &, const QByteArray &)> customReplyErrorParser)
{
    QEventLoop loop;
    QTimer timer;
    QNetworkAccessManager mgr;

    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    QNetworkRequest req(url);

    if(!userInfo.isEmpty())
        url.setUserInfo(userInfo);

    mgr.get(req);

    timer.start(recWindow_msec);
    QNetworkReply *reply = mgr.get(req);
    loop.exec();

    auto obj = customReplyErrorParser(url, reply->readAll());

    reply->deleteLater();

    return std::move(obj);
}
