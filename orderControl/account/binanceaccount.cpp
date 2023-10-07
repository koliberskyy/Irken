#include "binanceaccount.h"
namespace binance{
    BinanceAccount::BinanceAccount(const QString &__api_key, const QString &__api_secret, const QString &acc_name, Logger *__logger):
        BinanceAPIClient(__api_key, __api_secret), account_name{acc_name}, logger{__logger}
    {
    }

    QString BinanceAccount::new_order(const OrderData &order)
    {
        // маржа = (квонтити * прайс)/леверейдж
        // м*л = к*п
        //к = м*л/п
        //std::cout << acc.post_order("BTCUSDT","BUY", "LIMIT", "GTC", "0.03900000","25000","TEST_ORDER","","")->toStdString() << '\n';
        std::cout <<"void BinanceAccount::new_order(const OrderData &order)\n" << order.pair.toStdString() << '\n';
        auto balance = usdt_balance();
        if(balance < 0){
            QString reply(account_name);
            reply.append("- неверный ip\n");
            return reply;
        }
        auto margin = balance * 0.05;
        auto quantity_d = (margin * 20/*пока везде 20е плечи*/)/order.point_of_entry;
        QString symbol = order.pair;
        QString side;
        if(order.long_order)
            side = "BUY";
        else
            side = "SELL";

        QString price(normalize_digit(QString::fromStdString(std::to_string(order.point_of_entry))));
        QString quantity(normalize_digit(QString::fromStdString(std::to_string(quantity_d))));
        //фильтрацию не пропускаем
        quantity = manager::filter_minQty(quantity, symbol);
        price = manager::filter_price(price, symbol);

        QString newClientOrderId = order.id();
        std::cout << (QJsonDocument::fromJson(*post_order(symbol, side, "LIMIT", "GTC", quantity, price, newClientOrderId,"",""))).toJson().toStdString() << '\n';

        return "";
    }

    void BinanceAccount::cancel_order(const OrderData &order)
    {
        delete_order(order.pair, order.id());
    }

    void BinanceAccount::new_stop_loss(const QString &sybmol, double stop_price, bool long_pos)
    {
        auto order_type = [long_pos](){if(long_pos) return QString("LONG");else return QString("SHORT");};
        delete_order(sybmol, sybmol + QString("STOP") + order_type());

        QString type = "STOP_MARKET";
        QString stop_price_mod(normalize_digit(QString::fromStdString(std::to_string(stop_price))));
        stop_price_mod = manager::filter_price(stop_price_mod, sybmol);

        QString side;
        if(long_pos) side = "SELL";
        else side = "BUY";

        std::cout << (QJsonDocument::fromJson( *post_order
                                               (
                                                   sybmol,
                                                   side,
                                                   type,
                                                   "",
                                                   "",
                                                   "",
                                                   (sybmol + QString("STOP") + order_type()),
                                                   stop_price_mod,
                                                   "",
                                                   true
                                                )
                                             )
                      ).toJson().toStdString() << '\n';

    }

    void BinanceAccount::new_take_profit_trailing(const QString &sybmol, double take_price, bool long_pos)
    {
        auto order_type = [long_pos](){if(long_pos) return QString("LONG");else return QString("SHORT");};

        QString type = "TAKE_PROFIT_MARKET";
        QString take_price_mod(normalize_digit(QString::fromStdString(std::to_string(take_price))));
        take_price_mod = manager::filter_price(take_price_mod, sybmol);

        QString side;
        if(long_pos) side = "SELL";
        else side = "BUY";

        std::cout << (QJsonDocument::fromJson( *post_order
                                               (
                                                   sybmol,
                                                   side,
                                                   type,
                                                   "",
                                                   "",
                                                   "",
                                                   (sybmol + QString("TAKE") + order_type()),
                                                   take_price_mod,
                                                   "",
                                                   true,
                                                   settings::traling_callbackRate
                                               )
                                             )
                      ).toJson().toStdString() << '\n';
    }
    double BinanceAccount::usdt_balance()
    {
        QJsonDocument doc(QJsonDocument::fromJson(*balances()));
        if(!doc.object()["msg"].toString().isEmpty()){
            return -2015.0;
        }
        return doc.array().at(5).toObject()["balance"].toString().toDouble();//обожаю qt, всё понятно, а главное так просто;
    }

    QJsonArray BinanceAccount::positions()
    {
        QJsonDocument doc(QJsonDocument::fromJson(*positionRisk()));
        //std::cout << doc.toJson().toStdString();
        auto tmp = doc.array();
        QJsonArray array;
        for(auto i = 0; i < tmp.size(); i++){
            if(tmp.at(i).toObject()["entryPrice"].toString().toDouble() > 0.0)
                array.push_back(tmp.at(i));
        }
        return std::move(array);
    }

    bool BinanceAccount::close_position(QJsonObject position, bool long_position)
    {
        QString symbol = position["symbol"].toString();
        QString side;
        QString type = "MARKET";
        QString quantity = position["positionAmt"].toString();
        //if(position["positionAmt"].toString().at(0) == '-') <-так тоже можно и даже более универсально
        if(long_position){
            side = "SELL";
        }
        else{
            side = "BUY";
            //удаляем минус
            quantity.erase(quantity.begin(), quantity.begin()+1);
        }

        post_order(symbol, side, type, "", quantity, "" ,"" , "", "");
        return true;
    }

    int BinanceAccount::update_control()
    {
        const auto pos = positions();

        if(pos.size() > 0)
            posBuf.update(pos);
        else
            posBuf.clear_buf();

        if(posBuf.isStopOrdersChanged){
            for(auto it = posBuf.stop_orders.begin(); it != posBuf.stop_orders.end(); it++){
                new_stop_loss(std::get<0>(*it), std::get<1>(*it), std::get<2>(*it));
            }
            posBuf.stop_orders.clear();
            posBuf.isStopOrdersChanged = false;
        }

        if(posBuf.isTakeOrdersChanged){
            for(auto it = posBuf.take_orders.begin(); it != posBuf.take_orders.end(); it++){
                new_take_profit_trailing(std::get<0>(*it), std::get<1>(*it), std::get<2>(*it));
            }
            posBuf.take_orders.clear();
            posBuf.isTakeOrdersChanged = false;
        }

        return pos.size();
    }

    void BinanceAccount::close_all_positions()
    {
        const auto pos = positions();
        for(auto i = 0; i < pos.size(); i++){
            close_position(pos.at(i).toObject(), false);
        }
    }

    QString BinanceAccount::test_conectivity()
    {
        QString reply(account_name);
        QJsonDocument doc(QJsonDocument::fromJson(*balances()));
        auto hui      = doc.array();
        auto arr = doc.object();
        auto obj = doc.object()["msg"].toString();
        if(!doc.object()["msg"].toString().isEmpty()){
            reply.append("- неверный ip\n");
        }
        else
        {
            reply.append("-OK\n");
            if(usdt_balance() < 200.0)
                reply.append("Маловато у тебя деньжат\n");
        }
        return std::move(reply);
    }

    QString BinanceAccount::ip()
    {
        get(QString::fromStdString("https://api.ipify.org/?format=json"));
        QJsonDocument doc(QJsonDocument::fromJson(*read_buffer()));
        auto reply = doc.object()["ip"].toString();
        reply.append('\n');
        return reply;
    }


}//namespace bianance
