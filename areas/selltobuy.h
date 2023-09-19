#ifndef SELLTOBUY_H
#define SELLTOBUY_H

#include <areas.h>

class SellToBuy : public Areas
{
public:
    SellToBuy(const std::vector<CandleStick> *__klines);
    virtual void init() override;
};

#endif // SELLTOBUY_H
