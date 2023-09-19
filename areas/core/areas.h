#ifndef AREAS_H
#define AREAS_H
#include "candlestick.h"
#include "areapair.h"

class Areas
{
public:
    virtual void init() = 0;
    double nearest_above(double val);
    double nearest_below(double val);
protected:
    void add(double hight, double low, double time, Area area);
    const std::vector<CandleStick> *klines;
    std::vector<AreaPair> areas;
public:
    auto data() const{
        return &areas;
    }
};

#endif // AREAS_H
