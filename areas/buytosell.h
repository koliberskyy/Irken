#ifndef BUYTOSELL_H
#define BUYTOSELL_H

#include <areas.h>

class BuyToSell : public Areas
{
public:
    BuyToSell(const std::vector<CandleStick> *__klines);
    virtual void init() override;
};
#endif // BUYTOSELL_H
