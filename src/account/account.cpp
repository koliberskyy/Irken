#include "account.h"

Account::Account(QJsonObject &&acc_json, QObject *parent)
    : QObject{parent}, data{std::move(acc_json)}, netManager(new QNetworkAccessManager(this))
{
    connect(netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    data["balance"] = "-1.0";

    //************** ПЛЕЧИ
    //setLeverage();

    reduceTable.emplace_back(reducePair{20, 0, 25});
    reduceTable.emplace_back(reducePair{40, 5, 50});
    reduceTable.emplace_back(reducePair{200005, 15, 25.0});
    //reduceTable.emplace_back(reducePair{35, 25, 25.0});
    //reduceTable.emplace_back(reducePair{45, 35, 25.0});

    vec_positionControlLambdas.push_back([](std::vector<reducePair> table, const QJsonObject &pos)->int{
        auto isLongPos = pos["side"].toString() == "Buy";
        auto sl = pos["stopLoss"].toString().toDouble();
        auto pnl = Position::unrealizedPnlPercent(pos);
        auto avg = pos["avgPrice"].toString().toDouble();

        if(isLongPos){
            if(sl < avg && pnl >= table.at(0).reducePnl)
                return 0;
        }
        else{
            if(sl > avg && pnl >= table.at(0).reducePnl)
                return 0;
        }
        return -1;});

    vec_positionControlLambdas.push_back([](std::vector<reducePair> table, const QJsonObject &pos)->int{
        auto isLongPos = pos["side"].toString() == "Buy";
        auto sl = pos["stopLoss"].toString().toDouble();
        auto pnl = Position::unrealizedPnlPercent(pos);
        auto avg = pos["avgPrice"].toString().toDouble();

        if(sl == avg && pnl >= table.at(1).reducePnl)
            return 1;
        return -1;});

    vec_positionControlLambdas.push_back([](std::vector<reducePair> table, const QJsonObject &pos)->int{
        auto isLongPos = pos["side"].toString() == "Buy";
        auto sl = pos["stopLoss"].toString().toDouble();
        auto pnl = Position::unrealizedPnlPercent(pos);
        auto avg = pos["avgPrice"].toString().toDouble();

        for(int i = 2; i < table.size(); i++){
            if(pnl > 0){
                if(isLongPos){
                    if(sl >= Position::markPriceFromPnl(pos, table.at(i-1).stopPnl - 1)
                            && sl < Position::markPriceFromPnl(pos, table.at(i).stopPnl - 1)
                            && pnl >= table.at(i).reducePnl)
                        return i;
                }
                else{
                    if(sl >= Position::markPriceFromPnl(pos, table.at(i-1).stopPnl - 1)
                            && sl < Position::markPriceFromPnl(pos, table.at(i).stopPnl - 1)
                            && pnl >= table.at(i).reducePnl)
                        return i;
                }

            }
        }
        return -1;});
}

QTreeWidgetItem Account::toTreeItem() const
{
    return QTreeWidgetItem(QStringList{
                                       data["name"].toString(),
                                       data["secondName"].toString(),
                                       data["balance"].toString(),
                                       data["startingDate"].toString(),
                                       data["pnlBeforeStartingDate"].toString(),
                                       data["verified"].toString(),
                                       "",
                                       api()
                           });
}

QTreeWidgetItem *Account::toTreeItemPtr() const
{
    return new QTreeWidgetItem(QStringList{
                                       data["name"].toString(),
                                       data["secondName"].toString(),
                                       data["balance"].toString(),
                                       data["startingDate"].toString(),
                                       data["pnlBeforeStartingDate"].toString(),
                                       data["verified"].toString(),
                                       "",
                                       api()
                               });
}

void Account::moveStopAndReduse(QJsonObject &&pos)
{
    auto stopLoss = pos["stopLoss"].toString();
    auto takeProfit = pos["takeProfit"].toString();
    auto avgPrice = pos["avgPrice"].toString();


    //kostil'
    if(stopLoss.isEmpty())
        Position::setTradingStop(pos, api(), secret(), avgPrice, takeProfit);

    int reduceIndex = -1;
    for(auto it : vec_positionControlLambdas){
        auto tmp = it(reduceTable, pos);
        if(tmp >= 0)
            reduceIndex = tmp;
    }

    if (reduceIndex == 0){
        if(Position::setTradingStop(pos, api(), secret(), avgPrice, takeProfit))
            Position::reducePosition(pos, reduceTable.at(reduceIndex).reducePercent, api(), secret());
    }
    else if(reduceIndex > 0){
        QString updatedStop = instruments::double_to_utf8(pos["symbol"].toString().toUtf8(), instruments::Filter_type::price, Position::markPriceFromPnl(pos, reduceTable.at(reduceIndex).stopPnl));

        if(Position::setTradingStop(pos, api(), secret(), updatedStop, takeProfit))
            Position::reducePosition(pos, reduceTable.at(reduceIndex).reducePercent, api(), secret());
    }
}

void Account::refreshOrderList(const QList<QJsonObject> &ordersToPost)
{

    QEventLoop eventLoop;
    QTimer timer;
    connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
    QObject::connect(this, SIGNAL(ordersUpdated(QJsonArray)), &eventLoop, SLOT(quit()));
    updateOrders();
    // ждем пока ордера обновятся
    timer.start(100000);
    eventLoop.exec();

    auto exist = orders;
    auto post = ordersToPost;

    for(auto ex = exist.begin(); ex != exist.end();){


        bool finded{false};
        for(auto po =   post.begin(); po != post.end(); po++){
            if(ex->operator[]("symbol") == po->operator[]("symbol")/*
                    && ex->operator[]("price").toString().toDouble() == po->operator[]("price").toString().toDouble()
                    && ex->operator[]("stopLoss").toString().toDouble() == po->operator[]("stopLoss").toString().toDouble()*/){

                //закоменил эти условия для того чтобы не выставлялись ордера на манеты по которым открыты позиции
                finded = true;
                //контрольная точка для проверки
                auto reduce = ex->operator[]("reduceOnly").toBool();
                post.erase(po);
                break;
            }
        }
        if(finded){
                ex = exist.erase(ex);
        }
        else
            ex++;
    }

    for(auto ex = exist.begin(); ex != exist.end();){
        //если это сл/тп позиции
        if(ex->operator[]("reduceOnly").toBool()){
            ex = exist.erase(ex);
        }
        else{
            ex++;
        }
    }

    //после этого цикла имеем два списка ордеров:
    // пост - ордера которые необходимо разместить
    // екзист - ордера которые необходимо закрыть
    auto breakpoint  = true;




    Order::batch_place(post, api(), secret(), balance());
    Order::batch_cancel(exist, api(), secret());

}

bool Position::setTradingStop(const QJsonObject &pos, const QString &api, const QString &secret, const QString &sl, const QString &tp)
{

    QEventLoop eventLoop;
    QNetworkAccessManager mgr;
    QTimer timer;

    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    QObject::connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);


    QJsonObject obj;
    obj.insert("category", "linear");
    obj.insert("symbol", pos["symbol"]);
    obj.insert("takeProfit", tp);
    obj.insert("stopLoss", sl);

    auto data = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    auto headers = Account::make_headers(data, api.toUtf8(), secret.toUtf8());
    QUrl url("https://api.bybit.com/v5/position/trading-stop");

    QNetworkRequest req(url);

    for(auto &it : headers){
        req.setRawHeader(it.first, it.second);
    }

    QNetworkReply *reply = mgr.post(req, data);

    timer.start(10000);
    eventLoop.exec();


    if (reply->error() == QNetworkReply::NoError){
        auto obj = QJsonDocument::fromJson(reply->readAll()).object();
        auto retCode = obj["retCode"].toInt();
        if(retCode == 0){
            reply->deleteLater();
            return true;
        }
        else if(retCode == 10001){
            std::cout << "Failure: request - " << url.toString().toStdString() << "\ndata:" << QJsonDocument::fromJson(data).toJson().toStdString() << "\nreply:\n" <<  QJsonDocument(obj).toJson().toStdString();
            reply->deleteLater();
            return true;
        }
        else{
            std::cout << "Failure: request - " << url.toString().toStdString() << "\ndata:" << QJsonDocument::fromJson(data).toJson().toStdString() << "\nreply:\n" <<  QJsonDocument(obj).toJson().toStdString();
        }
    }
    else {
        //failure
        instruments::replyError(url, data);
    }
    reply->deleteLater();
    return false;
}

void Position::reducePosition(const QJsonObject &pos, double reducePercent, const QString &api, const QString &secret)
{
    if(reducePercent > 0){
        QJsonObject order;
        order.insert("category", "linear");
        order.insert("symbol", pos["symbol"]);
        //side
        auto side = [pos]()->QString{if(pos["side"].toString() == "Buy")return "Sell";else return "Buy";}();
        order.insert("side", side);

        order.insert("orderType", "Market");

        //qty
        if(reducePercent >= 100)
            order.insert("qty", pos["size"]);
        else
            order.insert("qty", QString(instruments::double_to_utf8(pos["symbol"].toString().toUtf8(), instruments::Filter_type::lotSize_without_leverage_multipy, pos["size"].toString().toDouble() * reducePercent / 100)));

        order.insert("reduceOnly", true);

        auto trigger = false;
        do{
        trigger = Order::place(order, api, secret);
        }while(!trigger);
    }
}

double Position::unrealizedPnlPercent(const QJsonObject &pos)
{
#ifdef LEGACY

    return (pos["unrealisedPnl"].toString().toDouble() * pos["leverage"].toString().toDouble() * 100)
            / (pos["markPrice"].toString().toDouble() * pos["size"].toString().toDouble());
#endif
#ifndef LEGACY
    auto avg = pos["avgPrice"].toString().toDouble();
    auto mark = pos["markPrice"].toString().toDouble();
    auto leverage = pos["leverage"].toString().toDouble();
    auto side = pos["side"].toString();

    if(side == "Buy"){
        return 100 * leverage * (mark-avg) / avg ;
    }
    else{
        return 100 * leverage * (avg-mark) / mark;
    }

#endif
}

QString Position::unrealizedPnlPercentQString(const QJsonObject &pos)
{
    return QString::fromStdString(std::to_string(unrealizedPnlPercent(pos)));
}

double Position::markPriceFromPnl(const QJsonObject &pos, double pnl)
{
    auto avg = pos["avgPrice"].toString().toDouble();
    auto leverage = pos["leverage"].toString().toDouble();
    auto side = pos["side"].toString();

    if(side == "Buy"){
        return  avg + (pnl/100 * avg / leverage);
    }
    else{
        return  (100 * leverage * avg) / (pnl + (100 * leverage));
    }
}

void Account::updateBalance()
{
    std::thread thr([this](){
        QEventLoop eventLoop;
        QNetworkAccessManager mgr;
        QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
        QByteArray query("accountType=UNIFIED&coin=USDT");
        auto headers = make_headers(query, api(), secret());
        QUrl url("https://api.bybit.com/v5/account/wallet-balance?" + std::move(query));

        QNetworkRequest req(url);

        for(auto &it : headers){
            req.setRawHeader(it.first, it.second);
        }

        QNetworkReply *reply = mgr.get(req);
        eventLoop.exec();

        if (reply->error() == QNetworkReply::NoError){
            auto obj = QJsonDocument::fromJson(reply->readAll()).object();
            auto retCode = obj["retCode"].toString().toUInt();
            if(retCode == 0){
                auto result = obj["result"].toObject();
                auto b = result["list"].toArray()[0].toObject()["coin"].toArray()[0].toObject()["walletBalance"].toString();

                data["balance"] = b;
                emit balanceUpdated(std::move(b));
            }
            else{
                qDebug() << "Failure" << QJsonDocument(obj).toJson();
            }
        }
        else {
            //failure
            qDebug() << "Failure" << QJsonDocument::fromJson(reply->readAll()).toJson();
            emit balanceUpdated("");
        }
        reply->deleteLater();
    });

    thr.detach();

}

void Account::updatePositions(bool autocontrol)
{
#ifdef LEGACY
    std::thread thr([this](){
        QEventLoop eventLoop;
        QNetworkAccessManager mgr;
        QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
        QByteArray query("category=linear&settleCoin=USDT");
        auto headers = make_headers(query, api(), secret());
        QUrl url("https://api.bybit.com/v5/position/list?" + std::move(query));

        QNetworkRequest req(url);

        for(auto &it : headers){
            req.setRawHeader(it.first, it.second);
        }

        QNetworkReply *reply = mgr.get(req);
        QTimer::singleShot(3000, &eventLoop, &QEventLoop::quit);
        eventLoop.exec();

        if (reply->error() == QNetworkReply::NoError) {
            auto obj = QJsonDocument::fromJson(reply->readAll()).object();
            auto retCode = obj["retCode"].toString().toInt();
            if(retCode == 0){
                auto arr = obj["result"].toObject()["list"].toArray();

                if(!arr.isEmpty()){
                    for (auto it : arr){
                        moveStopAndReduse(it.toObject());
                    }
                    emit positionsUpdated(arr);
                }

            }
            else{
                qDebug() << "Failure" << QJsonDocument(obj).toJson();
            }
        }
        else {
            //failure
            qDebug() << "Failure" << QJsonDocument::fromJson(reply->readAll()).toJson();
        }
        reply->deleteLater();


    });
///////
    thr.detach();
#endif
#ifndef LEGACY
    autoControlActivated = autocontrol;

    QByteArray query("category=linear&settleCoin=USDT");
    auto headers = make_headers(query, api(), secret());
    QUrl url("https://api.bybit.com/v5/position/list?" + std::move(query));
    url.setUserInfo("pos");

    QNetworkRequest req(url);

    for(auto &it : headers){
        req.setRawHeader(it.first, it.second);
    }

    netManager->get(req);
#endif
}

void Account::updateOrders()
{
#ifdef LEGACY
    std::thread thr([this](){
        QEventLoop eventLoop;
        QNetworkAccessManager mgr;
        QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
        QByteArray query("category=linear&settleCoin=USDT");
        auto headers = make_headers(query, api(), secret());
        QUrl url("https://api.bybit.com/v5/order/realtime?" + std::move(query));

        QNetworkRequest req(url);

        for(auto &it : headers){
            req.setRawHeader(it.first, it.second);
        }

        QNetworkReply *reply = mgr.get(req);
        QTimer::singleShot(3000, &eventLoop, &QEventLoop::quit);
        eventLoop.exec();

        if (reply->error() == QNetworkReply::NoError) {
            auto obj = QJsonDocument::fromJson(reply->readAll()).object();
            auto retCode = obj["retCode"].toString().toInt();

            if(retCode == 0){
                auto arr = obj["result"].toObject()["list"].toArray();

                if(!arr.isEmpty()){
                    for (auto it : arr){
                    }
                    emit ordersUpdated(arr);
                }
            }
            else{
                qDebug() << "Failure: request - " << url << "\nquery: " << query << "\nreply:\n" <<  QJsonDocument(obj).toJson();
            }
        }
        else {
            //failure
            qDebug() << "Failure" << QJsonDocument::fromJson(reply->readAll()).toJson();
        }
        reply->deleteLater();


    });

    thr.detach();
#endif
#ifndef LEGACY
    QByteArray query("category=linear&settleCoin=USDT");
    auto headers = make_headers(query, api(), secret());
    QUrl url("https://api.bybit.com/v5/order/realtime?" + std::move(query));
    url.setUserInfo("ord");

    QNetworkRequest req(url);

    for(auto &it : headers){
        req.setRawHeader(it.first, it.second);
    }

    netManager->get(req);
#endif
}

void Account::placeOrder(QJsonObject order)
{
    if(balance() > 10.0){
        QEventLoop eventLoop;
        QTimer timer;

        QObject::connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
        QObject::connect(this, &Account::ordersUpdated, &eventLoop, &QEventLoop::quit);

        updateOrders();

        timer.start(10000);
        eventLoop.exec();

        auto exist = orders;
        bool exist_trigger = false;

        for(auto ex : exist){
            if(ex["symbol"] == order["symbol"] && ex["price"].toString().toDouble() == order["price"].toString().toDouble()){
                exist_trigger = true;
            }
        }
        if(!exist_trigger)
            while (!Order::place(order, api(), secret(), Order::qty_to_post(balance(), order["price"].toString().toDouble(), order["qty"].toString().toDouble())));
    }
}

void Account::replyFinished(QNetworkReply *reply)
{
    auto url = reply->request().url();
    auto data = reply->readAll();
    auto obj = QJsonDocument::fromJson(data).object();
    auto retCode = obj["retCode"].toString().toInt();

    if(retCode != 0 || data.isEmpty() || url.userInfo() == ""){
        std::cout << QJsonDocument(this->data).toJson().toStdString();
        instruments::replyError(url, data);
    }

    parceReply(obj, url);

    reply->deleteLater();
}

void Account::cancelOrder(QJsonObject order)
{
    Order::batch_cancel(QList<QJsonObject>{order}, api(), secret());
}

void Account::setTradingStop(const QJsonObject &pos, const QString &sl, const QString &tp)
{
    if(sl.isEmpty())
        while(!Position::setTradingStop(pos, api(), secret(), pos["avgPrice"].toString(), pos["takeProfit"].toString()));
    else
        while(!Position::setTradingStop(pos, api(), secret(), sl, tp));
}

void Account::reducePosition(const QJsonObject &pos, double reducePercent)
{
    Position::reducePosition(pos, reducePercent, api(), secret());
}

double Order::qty_to_post(double acc_balance, double price)
{
    return settings::one_order_qty_pc_from_balance * acc_balance / (100 * price);
}

double Order::qty_to_post(double acc_balance, double price, double percent_from_balance)
{
    return percent_from_balance * acc_balance / (100 * price);
}


void Account::parceReply(const QJsonObject &obj, const QUrl &url)
{
    if(url.userInfo() == "pos"){
        posDownloaded(obj);
    }
    else if(url.userInfo() == "ord"){
        ordDownloaded(obj);
    }
}

void Account::posDownloaded(const QJsonObject &obj)
{
    auto arr = obj["result"].toObject()["list"].toArray();

    if(!arr.isEmpty() && autoControlActivated){
        for (auto it : arr){
            moveStopAndReduse(it.toObject());
        }
    }

    emit positionsUpdated(arr);
}

void Account::ordDownloaded(const QJsonObject &obj)
{
    auto arr = obj["result"].toObject()["list"].toArray();

    orders.clear();

    if(!arr.isEmpty()){
        for (auto it : arr){
           orders.emplace_back(it.toObject());
        }
    }

    emit ordersUpdated(arr);
}

void Account::setLeverage(double leverage)
{
    std::cout << QJsonDocument(this->data).toJson().toStdString();
    for(auto &symbol : symbol::utf8){
        QEventLoop eventLoop;
        QNetworkAccessManager mgr;
        QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

        //#request body
        auto leverage = instruments::maxLeverage(symbol);

        if(leverage > instruments::tradingLeverage(symbol)){
            leverage = instruments::tradingLeverage(symbol);
        }

        auto levStr = QString(QByteArray::fromStdString(std::to_string(leverage)));
        auto it = std::find(levStr.begin(), levStr.end(), ',');
        levStr.erase(it, levStr.end());

        QJsonObject obj;
        obj.insert("symbol", QString::fromUtf8(symbol));
        obj.insert("category", "linear");
        obj.insert("buyLeverage", levStr);
        obj.insert("sellLeverage", levStr);

        auto data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
        auto headers = Account::make_headers(data, api(), secret());
        QUrl url("https://api.bybit.com/v5/position/set-leverage");

        QNetworkRequest req(url);

        for(auto &it : headers){
            req.setRawHeader(it.first, it.second);
        }
        //#request body

        QTimer timer;
        QObject::connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);

        auto reply = mgr.post(req, data);

        timer.start(10000);
        eventLoop.exec();

        if (reply->error() == QNetworkReply::NoError){
            auto obj = QJsonDocument::fromJson(reply->readAll()).object();
            auto retCode = obj["retCode"].toInt();
            if(retCode == 0 || retCode == 110043){
                auto breakpoint = true;
                std::cout << symbol.toStdString() << " - sucscess\n";
            }
            else{
                instruments::replyError(url, data);
            }
        }
        else {
            //failure
            instruments::replyError(url, data);
        }
        reply->deleteLater();
    }
    std::cout << "-----------------------------------\n\n\n";
}

QByteArray Account::api() const
{
    return data["api"].toString().toUtf8();
}

QByteArray Account::secret() const
{
    return data["secret"].toString().toUtf8();
}

double Account::balance() const
{
    return data["balance"].toString().toDouble();
}



std::vector<std::pair<QByteArray, QByteArray> > Account::make_headers(const QByteArray &data, const QByteArray &api_key, const QByteArray &secret_key)
{
    auto ts = timestamp();
    return std::vector<std::pair<QByteArray, QByteArray> >( {
                                                                (std::pair<QByteArray, QByteArray>("X-BAPI-SIGN", make_signature(data, ts, api_key, secret_key))),
                                                                (std::pair<QByteArray, QByteArray>("X-BAPI-API-KEY", api_key)),
                                                                (std::pair<QByteArray, QByteArray>("X-BAPI-TIMESTAMP", std::move(ts))),
                                                                (std::pair<QByteArray, QByteArray>("X-BAPI-RECV-WINDOW", settings::recv_window))
                                                            });
}

QByteArray Account::make_signature(const QByteArray &data, const QByteArray &ts, const QByteArray &api_key, const QByteArray &secret_key)
{
    return QMessageAuthenticationCode::hash(ts + api_key + settings::recv_window + data, secret_key, QCryptographicHash::Sha256).toHex();
}

QByteArray Account::timestamp()
{
    return  QByteArray::fromStdString(std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()));//ахуеть
}


bool Order::place(const QJsonObject &order, const QString &api, const QString &secret)
{
    QEventLoop eventLoop;
    QNetworkAccessManager mgr;
    QTimer timer;

    QObject::connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

    auto data = QJsonDocument(order).toJson(QJsonDocument::Compact);
    auto headers = Account::make_headers(data, api.toUtf8(), secret.toUtf8());
    QUrl url("https://api.bybit.com/v5/order/create?");

    QNetworkRequest req(url);

    for(auto &it : headers){
        req.setRawHeader(it.first, it.second);
    }

    QNetworkReply *reply = mgr.post(req, data);
    timer.start(10000);
    eventLoop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        auto obj = QJsonDocument::fromJson(reply->readAll()).object();
        auto retCode = obj["retCode"].toInt();
        if(retCode != 0){
            std::cout << "Failure: request - " << url.toString().toStdString() << "\ndata:" << QJsonDocument::fromJson(data).toJson().toStdString() << "\nreply:\n" <<  QJsonDocument(obj).toJson().toStdString();
        }
        else{
            reply->deleteLater();
            return true;
        }

    }
    else {
        //failure
        qDebug() << "Failure" << QJsonDocument::fromJson(reply->readAll()).toJson();
    }
    reply->deleteLater();
    return false;

}

bool Order::place(const QJsonObject &order, const QString &api, const QString &secret, double qty)
{
    if(qty > 0){
        auto obj = order;
        obj["qty"] =  QString::fromUtf8(instruments::double_to_utf8(obj["symbol"].toString().toUtf8(), instruments::Filter_type::lotSize, qty));
        obj.insert("category", "linear");

        if(obj["orderType"].toString().isEmpty())
            obj.insert("orderType", "Limit");


        QEventLoop eventLoop;
        QTimer timer;
        QNetworkAccessManager mgr;

        QObject::connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
        QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

        auto data = QJsonDocument(obj).toJson(QJsonDocument::Compact);

        auto headers = Account::make_headers(data, api.toUtf8(), secret.toUtf8());
        QUrl url("https://api.bybit.com/v5/order/create");

        QNetworkRequest req(url);

        for(auto &it : headers){
            req.setRawHeader(it.first, it.second);
        }

        QNetworkReply *reply = mgr.post(req, data);

        timer.start(10000);
        eventLoop.exec();

        if (reply->error() == QNetworkReply::NoError){
            auto obj = QJsonDocument::fromJson(reply->readAll()).object();
            auto retCode = obj["retCode"].toInt();
            if(retCode == 0){
                reply->deleteLater();
                return true;
            }
            else{
                instruments::replyError(url, QJsonDocument(obj).toJson());
                std::cout << "data:\n" << QJsonDocument::fromJson(data).toJson().toStdString() << '\n';
            }
        }
        else {
            //failure
            instruments::replyError(url, QJsonDocument(obj).toJson());
            std::cout << "data:\n" << QJsonDocument::fromJson(data).toJson().toStdString() << '\n';
        }
        reply->deleteLater();
        return false;
    }
    else{
        std::cout << "place order qty < 0\n";
    }
    return true;
}

void Order::batch_place(const QList<QJsonObject> orders, const QString &api, const QString &secret, const double balance)
{
    QList<QJsonObject> orders_copy;

    if(balance > 1){
        for(auto i = 0; i != orders.size(); i++){
            orders_copy.emplaceBack(orders.at(i));
            if((i+1) % 10 == 0 || i+1 == orders.size()){
                QJsonObject obj;
                QJsonArray arr;
                obj.insert("category", "linear");
                for(auto &it : orders_copy){
                    auto symbol = it["symbol"].toString().toUtf8();
                    auto price = it["price"].toString().toUtf8().toDouble();
                    auto qty = QString::fromUtf8(instruments::double_to_utf8(std::move(symbol), instruments::Filter_type::lotSize, qty_to_post(std::move(balance), std::move(price))));
                    it.insert("qty", std::move(qty));
                    arr.push_back(it);
                }
                obj.insert("request", arr);

                QEventLoop eventLoop;
                QTimer timer;
                QNetworkAccessManager mgr;

                QObject::connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
                QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

                auto data = QJsonDocument(obj).toJson(QJsonDocument::Compact);

                auto headers = Account::make_headers(data, api.toUtf8(), secret.toUtf8());
                QUrl url("https://api.bybit.com/v5/order/create-batch");

                QNetworkRequest req(url);

                for(auto &it : headers){
                    req.setRawHeader(it.first, it.second);
                }

                QNetworkReply *reply = mgr.post(req, data);

                timer.start(10000);
                eventLoop.exec();

                if (reply->error() == QNetworkReply::NoError){
                    auto obj = QJsonDocument::fromJson(reply->readAll()).object();
                    auto retCode = obj["retCode"].toInt();
                    if(retCode == 0){
                        auto breakpoint = true;
                    }
                    else{
                        instruments::replyError(url, data);
                    }
                }
                else {
                    //failure
                    instruments::replyError(url, data);
                }
                reply->deleteLater();

                orders_copy.clear();
            }
        }
    }
}

void Order::batch_cancel(const QList<QJsonObject> orders, const QString &api, const QString &secret)
{
    QList<QJsonObject> orders_copy;

    for(auto i = 0; i != orders.size(); i++){
        orders_copy.emplaceBack(orders.at(i));
        if((i+1) % 10 == 0 || i+1 == orders.size()){
            QJsonObject obj;
            QJsonArray arr;
            obj.insert("category", "linear");
            for(auto &it : orders_copy){
                QJsonObject tmp;

                auto symbol = it["symbol"].toString();
                auto orderId = it["orderId"].toString();

                tmp.insert("symbol", std::move(symbol));
                tmp.insert("orderId", std::move(orderId));

                arr.push_back(tmp);
            }

            obj.insert("request", arr);

            QEventLoop eventLoop;
            QTimer timer;
            QNetworkAccessManager mgr;

            QObject::connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
            QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

            auto data = QJsonDocument(obj).toJson(QJsonDocument::Compact);

            auto headers = Account::make_headers(data, api.toUtf8(), secret.toUtf8());
            QUrl url("https://api.bybit.com/v5/order/cancel-batch");

            QNetworkRequest req(url);

            for(auto &it : headers){
                req.setRawHeader(it.first, it.second);
            }

            QNetworkReply *reply = mgr.post(req, data);

            timer.start(10000);
            eventLoop.exec();

            if (reply->error() == QNetworkReply::NoError){
                auto obj = QJsonDocument::fromJson(reply->readAll()).object();
                auto retCode = obj["retCode"].toInt();
                if(retCode == 0){
                    auto breakpoint = true;
                }
                else{
                    instruments::replyError(url, data);
                }
            }
            else {
                //failure
                instruments::replyError(url, data);
            }
            reply->deleteLater();

            orders_copy.clear();
        }
    }
}

