#include "liquidity.h"

void Liquidity::init()
{
    if(klines->size() > 0){
        add_hight(klines->at(0).hight);
        add_low(klines->at(0).low);
    }
    else{
        std::cout << "void Liquidity::init-ERROR-candles.size()=" << klines->size() <<"\n";
    }

    for(auto i = 1; i < klines->size(); i++){
        collect_liquidity(i);

        if(klines->at(i).hight > klines->at(i-1).hight){
            add_hight(klines->at(i).hight);
        }
        if(klines->at(i).low < klines->at(i-1).low){
            add_low(klines->at(i).low);
        }
    }
    fill_orderdata();

}

double Liquidity::nearest_above(double hight)
{
    auto nearest = set_hights.upper_bound(hight);
    if(nearest == set_hights.end()){
        return -1;
    }
    if(nearest == set_hights.begin()){
        return *nearest;
    }

    return *nearest;
}

double Liquidity::nearest_below(double low)
{
    //auto nearest = lower(&set_lows, low);
    auto nearest = set_lows.lower_bound(low);
    if(nearest == set_lows.end()){
        return *(--nearest);
    }
    if(nearest == set_lows.begin()){
        return -1;
    }

    nearest--;
    return *nearest;
}

void Liquidity::fill_orderdata()
{
    for(auto it = set_hights.begin(); it != set_hights.end(); it++){
        if(++it == set_hights.end())
            break;
        it--;

        Area area = Area::liquid;
        auto poe = *it;

        auto sl = *(++it);
        it--;

        auto tp = rr_take(poe, sl);

        auto candle_it = klines->rbegin();
        for(; candle_it != klines->rend(); candle_it++){
            if(candle_it->hight == poe)
                break;
        }

        if(candle_it != klines->rend() /*&& candle_it->open_time < klines->at(klines->size() - 24 - 1).open_time*/){
            orderdata.emplace_back(OrderData(area, poe, sl, tp, candle_it->open_time, klines->back().close, symbol, timeframe));
        }
    }


    for(auto it = set_lows.end(); it != set_lows.begin(); it--){
        Area area = Area::liquid;
        auto poe = *it;

        auto sl = *(--it);
        it++;

        auto tp = rr_take(poe, sl);

        auto candle_it = klines->rbegin();
        for(;candle_it != klines->rend(); candle_it++){
            if(candle_it->low == poe)
                break;
        }

        if(candle_it != klines->rend() /*&& candle_it->open_time < klines->at(klines->size() - 24 - 1).open_time*/){
            orderdata.emplace_back(OrderData(area, poe, sl, tp, candle_it->open_time, klines->back().close, symbol, timeframe));
        }
    }
}

double Liquidity::rr_take(double poe, double sl)
{
    if(poe > sl){
        auto red_zone = poe - sl;
        for(auto it = set_hights.begin(); it != set_hights.end(); it++){
            auto green_zone = *it - poe;
            if((green_zone / red_zone) > 5){
                return *it;
            }
        }
    }
    else{
        auto red_zone = sl - poe;
        for(auto it = set_lows.rbegin(); it != set_lows.rend(); it++){
            auto green_zone = poe - *it;
            if((green_zone / red_zone) > 5){
                return *it;
            }
        }
    }
    return 0.0;
}

void Liquidity::add_hight(double hight)
{
    set_hights.emplace(hight);
}

void Liquidity::add_low(double low)
{
    set_lows.emplace(low);
}

double Liquidity::erase_hights(double from_low, double to_hight)
{
    if(set_hights.size() < 1)
        return -1;
    if(from_low > to_hight)
        return -1;

    //*highest_it > to_hight (первый больший)
    //auto highest_it = upper(&set_hights, to_hight);
    auto highest_it = set_hights.upper_bound(to_hight);
    //*lower_it >= from_low (первый больший или равный)
    //auto lowest_it = lower(&set_hights, from_low);
    auto lowest_it = set_hights.lower_bound(from_low);

    //set_hights > to_hight
    if(highest_it == set_hights.begin())
        return -1;
    //set_hights < from_low
    if(lowest_it == set_hights.end())
        return -1;

    auto highest_d = *(--highest_it);
    highest_it++;

    auto lowest_d = *(lowest_it);

    if(highest_d > lowest_d)
    //удаляет элементы от lowest_it (вкл) до highest_it (не вкл)
        set_hights.erase(lowest_it, highest_it);
    else{
        set_hights.erase(lowest_it);
        //return lowest_d;
        return -1.0;//возвращаем -1 для того чтобы не осуществлять провреку на свик/ск (отсеиваем еруну)
    }
    return highest_d;
}

double Liquidity::erase_lows(double from_low, double to_hight)
{
    if(set_lows.size() < 1)
        return -1;
    if(from_low > to_hight)
        return -1;

    //*highest_it > to_hight
    //auto highest_it = upper(&set_lows, to_hight);
    auto highest_it = set_lows.upper_bound(to_hight);
    //*lower_it >= from_low
    //auto lowest_it = lower(&set_lows, from_low);
    auto lowest_it = set_lows.lower_bound(from_low);

    //set_hights > to_hight
    if(highest_it == set_lows.begin())
        return -1;
    //set_hights < from_low
    if(lowest_it == set_lows.end())
        return -1;

    auto highest_d = *(--highest_it);
    highest_it++;

    auto lowest_d = *(lowest_it);

    if(highest_d > lowest_d)
    //удаляет элементы от lowest_it (вкл) до highest_it (не вкл)
        set_lows.erase(lowest_it, highest_it);
    else{
        set_lows.erase(lowest_it);
        //return lowest_d;
        return -1.0;//возвращаем -1 для того чтобы не осуществлять провреку на свик/ск (отсеиваем еруну)
    }
    return lowest_d;
}

void Liquidity::collect_liquidity(int pos)
{
    if(pos > 0){
        auto hight_liquid = erase_hights(klines->at(pos).low, klines->at(pos).hight);
        auto low_liquid = erase_lows(klines->at(pos).low, klines->at(pos).hight);
        //верхний сбор
        if(hight_liquid != klines->at(pos-1).hight){
            if(hight_liquid > 0){
                swickSk->swick_check(klines->at(pos), hight_liquid);
                if(pos+1 < klines->size())
                    swickSk->sk_check(klines->at(pos), klines->at(pos+1), hight_liquid);
            }
        }
        //нижний сбор
        if(low_liquid != klines->at(pos-1).low){
            if(low_liquid > 0){
                swickSk->swick_check(klines->at(pos), low_liquid);
                if(pos+1 < klines->size())
                    swickSk->sk_check(klines->at(pos), klines->at(pos+1), low_liquid);
            }
        }
    }
}
