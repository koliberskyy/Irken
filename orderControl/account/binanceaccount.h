#ifndef BINANCEACCOUNT_H
#define BINANCEACCOUNT_H

#include "binanceapiclient.h"
#include <memory>
#include <QJsonObject>
#include "orderdata.h"
#include "managerEnums.h"

namespace binance{
    class BinanceAccount : public BinanceAPIClient
    {
        Q_OBJECT
    public:
        BinanceAccount(const QString &__api_key, const QString &__api_secret);
        void new_order(const OrderData &order);
        void cancel_order(const OrderData &order);
        double usdt_balance();
        QJsonArray positions();
        bool close_position(QJsonObject position, bool long_position);
        int update_control();

    };
}//nsmespace binance*************************
#endif // BINANCEACCOUNT_H
