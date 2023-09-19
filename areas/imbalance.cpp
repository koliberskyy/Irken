#include "imbalance.h"


Imbalance::Imbalance(const std::vector<CandleStick> *__klines)
{
    klines = __klines;
}

void Imbalance::init()
{
    for(int i = 2; i < klines->size(); i++){
        imbalance_check(klines->at(i-2), klines->at(i-1), klines->at(i));
    }
}

void Imbalance::imbalance_check(const CandleStick &first, const CandleStick &second, const CandleStick &third)
{
    if(second.color == KlineColor::green){
        auto distance = third.low - first.hight;
        if(distance > 0){
            auto body = second.close - second.open;
            auto imbalance = distance / body;

            auto imba_killer = 100*distance / first.hight; // размер имбаланса в процентах по аналогии в нижнем

            if(/*imbalance > trueKeff && imbalance < 1.0 && */imba_killer > 1.0){
                add(0.5 * distance + first.hight, first.hight, third.open_time, Area::imbalance_up);
                if(first.color == KlineColor::red)
                    add(first.hight, first.low, first.open_time, Area::order_block);
            }
        }
    }
    else if(second.color == KlineColor::red){
        auto distance = first.low - third.hight;
        if(distance > 0){
            auto body = second.open - second.close;
            auto imbalance = distance / body;

            auto imba_killer = 100*distance / third.hight;

            if(/*imbalance > trueKeff && imbalance < 1.0 && */ imba_killer > 1.0){
                add(first.low, 0.5 * distance + third.hight, third.open_time, Area::imbalance_low);
                if(first.color == KlineColor::green)
                    add(first.hight, first.low, first.open_time, Area::braker);
            }
        }
    }
}
