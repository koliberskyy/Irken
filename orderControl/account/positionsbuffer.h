#ifndef POSITIONSBUFFER_H
#define POSITIONSBUFFER_H
#include <QJsonObject>
#include <list>
#include <memory>
#include <unordered_map>
#include <deque>
#include <QJsonArray>
#include <tuple>
#include <settings.h>

namespace binance{
    class PositionsBuffer
    {
    public:
        PositionsBuffer();
        void update(const QJsonArray &arr);

        //КЛИЕНТ, НЕ ВТАВЛЯЙ НИЧЕГО, ТОЛЬКО УДАЛЯЙ, после удаления не забудь выщелкнуть триггер isStopOrdersChanged(isTakeOrdersChanged) (мне лень писать интерфейсы)
        std::list<std::tuple<QString, double, bool>> stop_orders;
        std::list<std::tuple<QString, double, bool>> take_orders;
        bool isStopOrdersChanged;
        bool isTakeOrdersChanged;
        //**

        void clear_buf();
    private:
        struct position{
                QString symbol;
                double entry_price;
                bool isPosLong;
                double stop_price;
                double stop_percent;
                double take_price;
                double take_percent;
                bool isModifyed;// если тру: надо вкидывать ордер
                bool isFinded;//если фалсе: надо удалять
                bool isStopedBU;//если тру: стоп переставлен на точку входа
                bool isTakePosted;//если тру, тейк-профи выкинут на биржу
                explicit position
                (
                    const QString &__symbol,
                    double __entry_price,
                    bool __isPosLong,
                    double __stop_percent = settings::stop_percent,
                    double __take_percent = settings::take_percent
                )
                    :
                    symbol{__symbol},
                    entry_price{__entry_price},
                    isPosLong{__isPosLong},
                    stop_percent{__stop_percent},
                    isModifyed{true},
                    isFinded{true},
                    isStopedBU{false},
                    isTakePosted{false}
                {
                    set_stop_price(__stop_percent);
                    set_take_price(__take_percent);
                }
                position(){;}//по хорошему надо инциализировать все переменные чем-нибудь, но я лучше потрачу это время на что-нибудь более полезное, естественно в коммерческом коде я бы такого себе не позволил
                void set_stop_price(double __stop_percent = settings::stop_percent){
                    if(__stop_percent > (-0.01))
                        isStopedBU = true;
                    stop_percent = __stop_percent;
                    stop_price = percent_to_price(__stop_percent);
                }
                void set_take_price(double __take_percent = settings::take_percent){
                    take_percent = __take_percent;
                    take_price = percent_to_price(__take_percent);
                }
                double percent_to_price(double percent){
                    if(isPosLong)
                        return entry_price * (1.0 + percent/100);
                    else
                        return entry_price * (1.0 - percent/100);
                }
        };
        using Positions_table = std::unordered_map<QString, position>;
        Positions_table positions_tab;
        void add_position(const QJsonObject &pos, const QString &id);
        void modify_position(const QJsonObject &pos, const QString &id);
        void get_new_orders();
    };
}

#endif // POSITIONSBUFFER_H
