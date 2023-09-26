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
    int update_control();
signals:
private:
    std::array<std::list<OrderData>, manager::pairs.size()> orders;
    tg::TelegramBot tg;
    binance::BinanceBot binance_bot;

public:
    auto telegram(){
        return &tg;
    }


};

#endif // MANAGER_H
