#include "swicksk.h"

bool SwickSk::swick_check(const CandleStick &kline, double liquidity)
{
    switch (kline.color) {
        case KlineColor::green:
        {
            auto shadow(0.0);
            auto body = kline.close - kline.open;
            bool upTrigger{false};
            auto middleOfBody = 0.5 * (kline.close + kline.open);

            if(liquidity <= kline.hight && liquidity > kline.close){
                shadow = kline.hight - kline.close;
                upTrigger = true;
            }
            else if(liquidity >= kline.low && liquidity < kline.open){
                shadow = kline.open - kline.low;
            }
            else{
                return false;
            }

            if((shadow > 1.5*body) && (shadow/middleOfBody > trueKeff)){
                if(upTrigger)
                    add(kline.hight, kline.close, kline.open_time, Area::swick_up);
                else
                    add(kline.open, kline.low, kline.open_time, Area::swick_low);
                return true;
            }

            return false;
            break;
        }

        case KlineColor::red:
        {
            auto shadow(0.0);
            auto body = kline.open - kline.close;
            bool upTrigger{false};
            auto middleOfBody = 0.5 * (kline.open + kline.close);

            if(liquidity <= kline.hight && liquidity > kline.open){
                shadow = kline.hight - kline.open;
                upTrigger = true;
            }
            else if(liquidity >= kline.low && liquidity < kline.close){
                shadow = kline.close - kline.low;
            }
            else{
                return false;
            }

            if((shadow > 1.5*body) && (shadow/middleOfBody > trueKeff)){
                if(upTrigger)
                    add(kline.hight, kline.open, kline.open_time, Area::swick_up);
                else
                    add(kline.close, kline.low, kline.open_time, Area::swick_low);
                return true;
            }

            return false;
            break;
        }
    }
    return false;
}

bool SwickSk::sk_check(const CandleStick &kline_0, const CandleStick &kline_1, double liquidity)
{
        //верхний ск (ликвидность находится между закрытием зеленой и закрытием красной)
    if(kline_0.color == KlineColor::green
        && kline_1.color == KlineColor::red
        //тени меньше тел
        && (kline_0.hight - kline_0.close) * 4 < (kline_0.close - kline_0.open)
        && (kline_1.hight - kline_1.open) * 4 < (kline_1.open - kline_1.close)
        //ликвидность собрана двумя телами
        && liquidity < kline_0.close
        && liquidity > kline_1.close
        //размел тел почти одинаковый
        && (kline_0.open / kline_1.close ) > 0.99
        && (kline_0.open / kline_1.close ) < 1.01
        //отсечение
        && ((kline_0.close - kline_0.open) / kline_0.close) > trueKeff
    )
    {
        add(kline_0.close, kline_0.open, kline_1.open_time, Area::sk_up);
        return true;
    }

    //нижний ск (ликвидность находится между закрытием красной и закрытием зеленой)
    if(kline_0.color == KlineColor::red
        && kline_1.color == KlineColor::green
        //тени меньше тел
        && (kline_0.close - kline_0.low) * 4 < (kline_0.open - kline_0.close)
        && (kline_1.open - kline_1.low) * 4< (kline_1.close - kline_1.open)
        //ликвидность собрана двумя телами
        && liquidity > kline_0.close
        && liquidity < kline_1.close
        //размел тел почти одинаковый
        && (kline_0.open / kline_1.close ) > (0.99)
        && (kline_0.open / kline_1.close ) < 1.01
        //отсечение
        && ((kline_0.open - kline_0.close) / kline_0.close) > trueKeff
    )
    {
        add(kline_0.open, kline_0.close, kline_1.open_time, Area::sk_low);
        return true;
    }

    return false;
}
