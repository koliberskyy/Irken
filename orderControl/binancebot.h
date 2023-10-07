#ifndef BINANCEBOT_H
#define BINANCEBOT_H

#include <requests.h>
#include "orderdata.h"
#include <chrono>
#include <QMessageAuthenticationCode>
#include <vector>
#include "orders.h"
#include "managerEnums.h"
#include "binanceaccount.h"
#include "key.h"
#include <array>
#include <thread>
#include "logger.h"
using OrderBook = std::array<std::list<OrderData>, manager::pairs.size()>;

namespace binance {

    class BinanceBot
    {
    public:
        explicit BinanceBot();
        void update_orders(OrderBook *__orders);
        int update_control();
        bool ping();
        void cancel_all_open_orders();
        QString test_conectivity();
        QString ip(){return accounts.begin()->ip();}
        auto get_reply()const{
            return &reply_buf;
        }
        void close_all_positions();
        QString get_logs();
    private:
        OrderBook *orders;
        QString reply_buf;
        std::list<OrderData> sended_orders;
        std::list<BinanceAccount> accounts;
        OrderData *was_sended(const QString &id);
        bool is_canceled(const QString &id);
        void post_order(const OrderData &order);
        void cancel_order(const OrderData &order);
        Logger logger;

    };

}//namespace binance****************************

#endif // BINANCEBOT_H
