#ifndef ORDERS_H
#define ORDERS_H

#include <vector>
#include <list>
#include <memory>
#include "orderdata.h"
#include "liquidity.h"
#include "selltobuy.h"
#include "buytosell.h"
#include "imbalance.h"
#include "klines.h"


//спроектирован ТОЛЬКО для конструирования в контейнерах
class Orders
{
public:
    Orders():symbol{""}, timeframe{""}{};
    //конструктор выполняет вычисления (инициализирует все переменные)
    Orders(const QString &__symbol, const QString &__timeframe);
    //перемещяет только список ордеров
    Orders(Orders&& moved) noexcept;

private:
    std::list<OrderData> orders;
    QString symbol;
    QString timeframe;
    std::shared_ptr<Klines> klines;
    std::unique_ptr<Liquidity> liquidity;
    std::unique_ptr<SellToBuy> selltobuy;
    std::unique_ptr<BuyToSell> buytosell;
    std::unique_ptr<Imbalance> imbalance;


    void fill_orderdata();
    OrderData create_order(const AreaPair &area);
    double rr_take_profit_above(double __poe, double __sl, double __tp);
    double rr_take_profit_below(double __poe, double __sl, double __tp);
    double nearest_above(double val);
    double nearest_below(double val);

    void set_all_status();
    bool check_order_status(const OrderData &order);

    bool is_include(double price, const CandleStick &candle)
    {
        if(price >= candle.low && price <= candle.hight)
            return true;

        return false;
    }
public:
    auto get_symbol() const{
        return &symbol;
    }
    auto get_timeframe() const{
        return &timeframe;
    }
    auto data() {
        return &orders;
    }
    auto const_data() const{
        return &orders;
    }
};




inline std::ostream& operator<<(std::ostream &out, const Orders &od){


    out << od.get_symbol()->toStdString() << '\n'
        << "timeframe: " << od.get_symbol()->toStdString() << '\n';
    for(auto it : *od.const_data()){
        if(it.actuality < 1.5 && it.actuality > (0.0) && it.shoulder < 250)
        {
            out << it;
        }
        out <<"_________________\n\n";
    }
    return out;
}

#endif // ORDERS_H
