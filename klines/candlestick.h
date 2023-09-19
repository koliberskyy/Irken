#ifndef CANDLESTICK_H
#define CANDLESTICK_H
#include <QJsonArray>
#include "KlineEnums.h"
class CandleStick
{
public:
    CandleStick(const CandleStick &copy) = default;
    CandleStick& operator=(const CandleStick &copy) = default;
    CandleStick(CandleStick &&moved) = default;
    CandleStick& operator=(CandleStick &&moved) = default;
    ~CandleStick() = default;

    CandleStick(): open_time(0), open(0.0), hight(0.0),
        low(0.0), close(0.0), close_time(0.0), color(KlineColor::red){}

    CandleStick(qint64 o_t, double o, double h,
                 double l, double c, qint64 c_t) :
        open_time(o_t), open(o), hight(h),
        low(l), close(c), close_time(c_t)
    {
        if(open < close)
            color = KlineColor::green;
        else
            color = KlineColor::red;
    }

    qint64    open_time; //miliseconds
    double   open;
    double   hight;
    double   low;
    double   close;
    qint64    close_time; //miliseconds
    KlineColor color;
};

#endif // CANDLESTICK_H
