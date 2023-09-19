#ifndef IMBALANCE_H
#define IMBALANCE_H

#include <areas.h>

class Imbalance : public Areas
{
public:
    Imbalance(const std::vector<CandleStick> *__klines);
    void init() override;
    double trueKeff{0.6};
    void imbalance_check(const CandleStick &first,
                         const CandleStick &second,
                         const CandleStick &third);
};

#endif // IMBALANCE_H
