#ifndef MANAGER_H
#define MANAGER_H

#include <QObject>
#include "orders.h"
#include "managerEnums.h"
#include "vector"
#include <fstream>
#include "telegrambot.h"
#include "binancebot.h"
class Manager : public QObject
{
    Q_OBJECT
public:
    explicit Manager();
    bool update();
    void updatePair(const std::pair<int, QString> &pair);
    void compress(const std::pair<int, QString> &pair);
    void filter(const std::pair<int, QString> &pair);
    void print_result_file();
    void send_result();
    void update_control();
signals:
private:
    std::array<std::list<OrderData>, manager::pairs.size()> orders;
    tg::TelegramBot tg;
    binance::BinanceBot binance_bot;

public:
    auto telegram(){
        return &tg;
    }
    void cancel_all_open_orders(){
        binance_bot.cancel_all_open_orders();
    }
    void test_conectivity(){
        tg.send_message(binance_bot.test_conectivity());
    }
    void close_all_positions(){
        binance_bot.close_all_positions();
    }
    void send_logs(){
        tg.send_message(binance_bot.get_logs());
    }


};

#endif // MANAGER_H
