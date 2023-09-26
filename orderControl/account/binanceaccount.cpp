#include "binanceaccount.h"
namespace binance{
    BinanceAccount::BinanceAccount(const QString &__api_key, const QString &__api_secret):
        BinanceAPIClient(__api_key, __api_secret)
    {
    }

    void BinanceAccount::new_order(const OrderData &order)
    {
        // маржа = (квонтити * прайс)/леверейдж
        // м*л = к*п
        //к = м*л/п
        //std::cout << acc.post_order("BTCUSDT","BUY", "LIMIT", "GTC", "0.03900000","25000","TEST_ORDER","","")->toStdString() << '\n';
        std::cout <<"void BinanceAccount::new_order(const OrderData &order)\n" << order.pair.toStdString() << '\n';
        auto balance = usdt_balance();
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
        quantity = manager::filter(quantity, symbol);
        QString newClientOrderId = order.id();
        std::cout << (QJsonDocument::fromJson(*post_order(symbol, side, "LIMIT", "GTC", quantity, price, newClientOrderId,"",""))).toJson().toStdString() << '\n';
    }

    void BinanceAccount::cancel_order(const OrderData &order)
    {
        delete_order(order.pair, order.id());
    }

    double BinanceAccount::usdt_balance()
    {
        QJsonDocument doc(QJsonDocument::fromJson(*balances()));
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
        for(auto i = 0; i < pos.size(); i++){
            auto mark_price = pos.at(i).toObject()["markPrice"].toString().toDouble();
            auto entry_price = pos.at(i).toObject()["entryPrice"].toString().toDouble();
            //если ордер длинный
            if(pos.at(i).toObject()["positionAmt"].toString().at(0) != '-'){

                //если цена выше на пол процента (take_profit)
                if((((mark_price / entry_price) * 100) - 100) > 0.45){
                    close_position(pos.at(i).toObject(), true);
                }
                //если на пол процента ниже (stop_loss)
                else if((((mark_price / entry_price) * 100) - 100) < (-0.5)){
                    close_position(pos.at(i).toObject(), true);
                }
            }
            //если ордер короткий
            else if(pos.at(i).toObject()["positionAmt"].toString().at(0) == '-'){
                //если цена выше на пол процента (stop_loss)
                if((((mark_price / entry_price) * 100) - 100) > 0.5){
                    close_position(pos.at(i).toObject(), false);
                }
                //если на пол процента ниже (take_profit)
                else if((((mark_price / entry_price) * 100) - 100) < (-0.45)){
                    close_position(pos.at(i).toObject(), false);
                }
            }
        }
        return pos.size();
    }


}//namespace bianance
