#include "orders.h"

Orders::Orders(const QString &__symbol, const QString &__timeframe) : symbol{__symbol}, timeframe{__timeframe}
{
    klines = std::make_shared<Klines>(symbol, timeframe, "1000");
    if(klines->get_klines()->size() != 0){
        liquidity = std::make_unique<Liquidity>(klines->get_klines(), symbol, timeframe);
        liquidity->init();
        imbalance = std::make_unique<Imbalance>(klines->get_klines());
        imbalance->init();
        selltobuy = std::make_unique<SellToBuy>(klines->get_klines());
        selltobuy->init();
        buytosell = std::make_unique<BuyToSell>(klines->get_klines());
        buytosell->init();
        fill_orderdata();
        set_all_status();
    }
}

Orders::Orders(Orders &&moved) noexcept:
    orders{std::move(moved.orders)}
{

}

void Orders::fill_orderdata()
{
    for(auto it : *(selltobuy->data())){
        orders.emplace_back(create_order(it));
    }

    for(auto it : *(buytosell->data())){
        orders.emplace_back(create_order(it));
    }

    for(auto it : *(liquidity->swickSk->data())){
        orders.emplace_back(create_order(it));
    }

    for(auto it : *(imbalance->data())){
        orders.emplace_back(create_order(it));
    }

    for(auto it : *(liquidity->data())){
        orders.emplace_back(std::move(it));
    }
}


OrderData Orders::create_order(const AreaPair &area)
{
    switch(area.area){
        case Area::null:
        {
            return OrderData();
            break;
        }
        case Area::bts:
        {
            return OrderData(Area::bts,
                             0.3 * (area.hight - area.low) + area.low,
                             area.hight,
                             rr_take_profit_below((0.3 * (area.hight - area.low) + area.low), area.hight, area.low),
                             area.time,
                             klines->get_klines()->back().close,
                             symbol,
                             timeframe
                             );
            break;
        }

        case Area::sk_up:
        {
            return OrderData(Area::sk_up,
                             0.3 * (area.hight - area.low) + area.low,
                             area.hight,
                             rr_take_profit_below((0.3 * (area.hight - area.low) + area.low), area.hight, area.low),
                             area.time,
                             klines->get_klines()->back().close,
                             symbol,
                             timeframe
                             );
        }
        case Area::sk_low:
        {
            return OrderData(Area::sk_low,
                             0.7 * (area.hight - area.low) + area.low,
                             area.low,
                             rr_take_profit_above((0.7 * (area.hight - area.low) + area.low), area.low, area.hight),
                             area.time,
                             klines->get_klines()->back().close,
                             symbol,
                             timeframe
                             );
            break;
        }
        case Area::swick_up:
        {
            if(area.hight > area.low){
                return OrderData(Area::swick_up,
                                 0.3 * (area.hight - area.low) + area.low,
                                 area.hight,
                                 rr_take_profit_below((0.3 * (area.hight - area.low) + area.low), area.hight, area.low),
                                 area.time,
                                 klines->get_klines()->back().close,
                                 symbol,
                                 timeframe
                                 );
            }

        }
        case Area::swick_low:
        {
            return OrderData(Area::swick_low,
                             0.7 * (area.hight - area.low) + area.low,
                             area.low,
                             rr_take_profit_above((0.7 * (area.hight - area.low) + area.low), area.low, area.hight),
                             area.time,
                             klines->get_klines()->back().close,
                             symbol,
                             timeframe
                             );
            break;
        }
        case Area::stb:
        {
            return OrderData(Area::stb,
                             0.7 * (area.hight - area.low) + area.low,
                             area.low,
                             rr_take_profit_above((0.7 * (area.hight - area.low) + area.low), area.low, area.hight),
                             area.time,
                             klines->get_klines()->back().close,
                             symbol,
                             timeframe
                             );
            break;
        }
        case Area::imbalance_up:
        {
            auto poe = area.hight;
            auto sl = nearest_below(area.low);
            auto tp = rr_take_profit_above(poe, sl, area.hight);

            return OrderData(Area::imbalance_up,
                                             poe,
                                             sl,
                                             tp,
                                             area.time,
                                             klines->get_klines()->back().close,
                                             symbol,
                                             timeframe
                                             );

            poe = area.low;
            return OrderData(Area::imbalance_up,
                                             poe,
                                             sl,
                                             tp,
                                             area.time,
                                             klines->get_klines()->back().close,
                                             symbol,
                                             timeframe
                                             );
            break;
        }
        case Area::imbalance_low:
        {
            auto poe = area.low;
            auto sl = nearest_above(area.hight);
            auto tp = rr_take_profit_below(poe, sl, area.low);

           return OrderData(Area::imbalance_low,
                                             poe,
                                             sl,
                                             tp,
                                             area.time,
                                             klines->get_klines()->back().close,
                                             symbol,
                                             timeframe
                                             );

            poe = area.hight;
            return OrderData(Area::imbalance_low,
                                             poe,
                                             sl,
                                             tp,
                                             area.time,
                                             klines->get_klines()->back().close,
                                             symbol,
                                             timeframe
                                             );
            break;
        }
        case Area::order_block:
        {
            return OrderData(Area::order_block,
                             0.7 * (area.hight - area.low) + area.low,
                             area.low,
                             rr_take_profit_above((0.7 * (area.hight - area.low) + area.low), area.low, area.hight),
                             area.time,
                             klines->get_klines()->back().close,
                             symbol,
                             timeframe
                             );
            break;
        }
        case Area::braker:
        {
            return OrderData(Area::braker,
                             0.3 * (area.hight - area.low) + area.low,
                             area.hight,
                             rr_take_profit_below((0.3 * (area.hight - area.low) + area.low), area.hight, area.low),
                             area.time,
                             klines->get_klines()->back().close,
                             symbol,
                             timeframe
                              );
            break;
        }
        case Area::liquid:
        {
            break;
        }
    }
    return OrderData();

}


double Orders::rr_take_profit_above(double __poe, double __sl, double __tp)
{
    if((__poe - __sl)*3 <= (__tp - __poe))
            return __tp;

    auto tp = nearest_above(__tp);

    if(__tp > __sl){
            while((__poe - __sl)*3 > (tp - __poe)){
                auto tmp = nearest_above(tp);

                if(tmp == -1.0)
                    return tp;

                tp = tmp;
            }
        return tp;
    }
    return -1;
}

double Orders::rr_take_profit_below(double __poe, double __sl, double __tp)
{
    if((__sl - __poe)*3 < (__poe - __tp))
        return __tp;

    auto tp = nearest_below(__tp);

    if(__tp < __sl){
        while((__sl - __poe)*3 > (__poe - tp)){
                auto tmp = nearest_below(tp);

                if(tmp == -1.0)
                        return tp;

                tp = tmp;
        }
        return tp;
    }
    return -1;
}

double Orders::nearest_above(double val)
{
    auto nearest{100000000.0};

    auto tmp = selltobuy->nearest_above(val);
    if(tmp < nearest && tmp > (0.0))
        nearest = tmp;

    tmp = buytosell->nearest_above(val);
    if(tmp < nearest && tmp > (0.0))
        nearest = tmp;

    tmp = liquidity->swickSk->nearest_above(val);
    if(tmp < nearest && tmp > (0.0))
        nearest = tmp;

    tmp = liquidity->nearest_above(val);
    if(tmp < nearest && tmp > (0.0))
        nearest = tmp;

    tmp = imbalance->nearest_above(val);
    if(tmp < nearest && tmp > (0.0))
        nearest = tmp;

    if(nearest == 100000000.0)
        return -1;

    return nearest;
}

double Orders::nearest_below(double val)
{
    auto nearest{0.0};

    auto tmp = selltobuy->nearest_below(val);
    if(tmp > nearest)
        nearest = tmp;

    tmp = buytosell->nearest_below(val);
    if(tmp > nearest)
        nearest = tmp;

    tmp = liquidity->swickSk->nearest_below(val);
    if(tmp > nearest)
        nearest = tmp;

    tmp = liquidity->nearest_below(val);
    if(tmp > nearest)
        nearest = tmp;

    tmp = imbalance->nearest_below(val);
    if(tmp > nearest)
        nearest = tmp;

    if(nearest == 0.0)
        return -1;

    return nearest;
}

void Orders::set_all_status()
{
    for(auto it = orders.begin(); it != orders.end();){
        if(!check_order_status(*it))
            it = orders.erase(it);
        else
            it++;
    }
    //расстановка трендов
    for(auto it = orders.begin(); it != orders.end();it++)
        it->set_thrand(*klines->get_thrand());
}

bool Orders::check_order_status(const OrderData &order)
{

    int start_iter{0};

    int i = klines->get_klines()->size() - 1;
    while(klines->get_klines()->at(i).open_time > order.time){
        i--;
    }
    start_iter = i + 1;

    bool flag_poe{false};

    for(int j = start_iter; j < klines->get_klines()->size(); j++){
        flag_poe = is_include(order.point_of_entry, klines->get_klines()->at(j));
        if(flag_poe)
            return false;
    }
    return true;
}


