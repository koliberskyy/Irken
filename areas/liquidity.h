#ifndef LIQUIDITY_H
#define LIQUIDITY_H
#include <vector>
#include "candlestick.h"
#include <set>
#include "swicksk.h"
#include "orderdata.h"
class Liquidity
{
public:
    Liquidity(const std::vector<CandleStick> *__klines, const QString &sym, const QString &tf) {
        klines=__klines;
        symbol = sym;
        timeframe = tf;
        swickSk = std::make_unique<SwickSk>();
}
    void init();
    //наибольшее, но не больше hight
    double nearest_above(double hight);
    //наименьшее, но не меньше low
    double nearest_below(double low);

    std::unique_ptr<SwickSk> swickSk;
private:
    QString symbol;
    QString timeframe;
    std::set<double> set_hights;
    std::set<double> set_lows;
    const std::vector<CandleStick> *klines;
    std::vector<OrderData> orderdata;

    void fill_orderdata();
    double rr_take(double poe, double sl);

    void add_hight(double hight);
    void add_low(double low);

    /// удаляет хаи в диапазоне от from (включительно) до to (включительно),
    /// возвращает значение наибольшего удаленного элемента или -1 если не удалено ничего
    double erase_hights(double from_low, double to_hight);
    /// удаляет хаи в диапазоне от from (включительно) до to (включительно),
    /// возвращает значение наибольшего удаленного элемента или -1 если не удалено ничего
    double erase_lows(double from_low, double to_hight);

    void collect_liquidity(int pos);
public:

    auto data() {return &orderdata;}


};

#endif // LIQUIDITY_H
