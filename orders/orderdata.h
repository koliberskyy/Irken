#ifndef ORDERDATA_H
#define ORDERDATA_H
#include <QJsonArray>
#include "AeasEnums.h"
#include "telegrambot.h"
#include "KlineEnums.h"
struct OrderData
{
    QString pair;
    QString timeframe;
    Thrand thrand;
    double point_of_entry;
    double stop_loss;
    double take_profit;
    double stopudoviy_take;//пол процента в нужную сторону
    double take_profit_0_75_pc;
    double take_profit_1_pc;
    double stop_loss_05_pc;
    double stop_percent;
    bool long_order;
    double shoulder;
    qint64 time;
    double actuality; // характеризует близость точки входа к текущей цене актива
    double last_price;
    Area area;
    void set_thrand(Thrand thr){
        thrand = thr;
    }
    void set_actuality(double price){
        if(price > point_of_entry){
            actuality = 100*(price - point_of_entry)/point_of_entry;
            if(!long_order)
                actuality *= -1;
        }
        else if(price < point_of_entry){
            actuality = 100*(point_of_entry - price)/price;
            if(long_order)
                actuality *= -1;
        }
    }
    void set_stopudoviy_take(){
        if(long_order){
            stopudoviy_take = point_of_entry * 1.005;
            stop_loss_05_pc = point_of_entry / 1.005;
            take_profit_1_pc = point_of_entry * 1.01;
            take_profit_0_75_pc = point_of_entry * 1.0075;
        }
        else{
            stopudoviy_take = point_of_entry / 1.005;
            stop_loss_05_pc = point_of_entry * 1.005;
            take_profit_1_pc = point_of_entry / 1.01;
            take_profit_0_75_pc = point_of_entry / 1.0075;
        }
    }
    QString id() {
        return QString(pair + QString::fromStdString(std::to_string(time)));
    }
    QString id() const{
        return QString(pair + QString::fromStdString(std::to_string(time)));
    }
    void update_actuality(double act){
        actuality = act;
    }
    OrderData():
        point_of_entry{0.0}, stop_loss{0.0}, take_profit{0.0}, stop_percent{0.0}, long_order{true}, shoulder{0.0},
        time{0}, actuality{0.0}, area{Area::null}{}
    OrderData(Area a, double poe, double sl, double tp, qint64 __time, double price, const QString &pr, const QString &tf):
        area{a}, point_of_entry{poe}, stop_loss{sl}, take_profit{tp}, time{__time}, long_order{true},
        actuality{0.0}, last_price{price}, pair{pr}, timeframe{tf}{
        if(poe > tp)
            long_order = false;
        if(!long_order){
            stop_percent = (100 * (sl-poe) ) / poe;
            set_stopudoviy_take();
        }

        else{
            stop_percent = (100 * (poe-sl) ) / poe;
            set_stopudoviy_take();
        }
        shoulder = 100 / stop_percent;
        set_actuality(last_price);

    }
    OrderData(Area &&a, double &&poe, double &&sl, double &&tp, qint64 __time, double price, const QString &pr, const QString &tf):
        area{a}, point_of_entry{poe}, stop_loss{sl}, take_profit{tp}, time{__time}, long_order{true}, actuality{0.0}, last_price{price}, pair{pr}, timeframe{tf}{
        if(poe > tp)
            long_order = false;
        if(!long_order){
            stop_percent = (100 * (sl-poe) ) / poe;
            set_stopudoviy_take();
        }
        else{
            stop_percent = (100 * (poe-sl) ) / poe;
            set_stopudoviy_take();
        }
        shoulder = 100 / stop_percent;
        set_actuality(last_price);
    }
};

inline std::ostream& operator<<(std::ostream &out, OrderData od){

    if(od.actuality < 1.5 && od.actuality > (0.0) && od.shoulder < 250){
        out << od.pair.toStdString() << ' '
            << od.timeframe.toStdString() << '(' << od.thrand << ')'<< '\n';
        if(od.long_order)
            out << "LONG-";
        else
            out << "SHORT-";

        out << od.area << "\n"
            << tg::convertTime(od.time).toStdString()<< "\n"
            <<"\n\tPOE: \t\t" << od.point_of_entry << '\n'
           << "\tTP: \t\t" << od.take_profit << '\n'
           << "\t\tTP 0.5%: \t\t" << od.stopudoviy_take << '\n'
           << "\t\tTP 0.75%: \t\t" << od.take_profit_0_75_pc << '\n'
           << "\t\tTP 1.0%: \t\t" << od.take_profit_1_pc << '\n'
           << "\tSL: \t\t" << od.stop_loss << "\n"
           <<"\t\tSL 0.5%: \t\t" << od.stop_loss_05_pc<<"\n\n"
          << "Lev: " << od.shoulder<< ", "
          << "Act: " << od.actuality << "\n"
          << "--------------\n";
    }
    return out;
}
#endif // ORDERDATA_H
