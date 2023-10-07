#ifndef BINANCEACCOUNT_H
#define BINANCEACCOUNT_H
#include "binanceapiclient.h"
#include <memory>
#include <QJsonObject>
#include "orderdata.h"
#include "managerEnums.h"
#include "logger.h"
#include "positionsbuffer.h"
#include "settings.h"
namespace binance{
    class BinanceAccount : public BinanceAPIClient
    {
        Q_OBJECT
    public:
        BinanceAccount(const QString &__api_key, const QString &__api_secret, const QString &account_name, Logger *__logger = nullptr);
        QString new_order(const OrderData &order);
        void cancel_order(const OrderData &order);
        void new_stop_loss(const QString &sybmol, double stop_price, bool long_pos);
        void new_take_profit_trailing(const QString &sybmol, double take_price, bool long_pos);
        double usdt_balance();
        QJsonArray positions();
        bool close_position(QJsonObject position, bool long_position = true);
        int update_control();
        void close_all_positions();
        QString test_conectivity();
        QString ip();
    private:
        QString account_name;
        Logger *logger;
        PositionsBuffer posBuf;

    };
}//nsmespace binance*************************
#endif // BINANCEACCOUNT_H
