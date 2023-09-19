#ifndef SWICKSK_H
#define SWICKSK_H

#include <areas.h>

class SwickSk : public Areas
{
public:
    SwickSk(){}
    void init() override{;}
    bool swick_check(const CandleStick &kline, double liquidity);
    bool sk_check(const CandleStick &kline_0, const CandleStick &kline_1, double liquidity);
private:
    double trueKeff{0.013};
};

#endif // SWICKSK_H
