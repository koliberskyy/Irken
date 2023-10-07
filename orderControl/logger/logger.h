#ifndef LOGGER_H
#define LOGGER_H
#include <QString>
#include <list>
namespace binance {

struct positionLog{
    explicit positionLog(const QString &__id): id{__id}{}
    explicit positionLog(const QString &__id, double __max_rebound_plus, double __max_rebound_minus, bool __executed, bool __profit_taked): id{__id}, max_rebound_plus{__max_rebound_plus},
    max_rebound_minus{__max_rebound_minus}, executed{__executed}, profit_taked{__profit_taked}{}
    QString id;
    double max_rebound_plus{0.0};
    double max_rebound_minus{0.0};// пусть будет с минусом
    bool executed{false};
    bool profit_taked{false};
    void update_max_rebound_plus(double val){if(val > max_rebound_plus) max_rebound_plus = val;}
    void update_max_rebound_minus(double val){if(val < max_rebound_minus) max_rebound_minus = val;}
};
struct dailyPositionsLog{
    //explicit dailyPositionsLog(){}
    explicit dailyPositionsLog(std::list<positionLog> *__pos_log_list): pos_log_list{__pos_log_list}{}
    int total{0};
    int total_profit{0};
    int total_loss{0};
    double average_rebound_plus{0.0};
    double average_rebound_minus{0.0};
    std::list<positionLog> *pos_log_list;
    void update_daily_log(){
        int total_tmp{0};
        int total_profit_tmp{0};
        int total_loss_tmp{0};
        double average_rebound_minus_tmp{0.0};
        double average_rebound_plus_tmp{0.0};
        for(auto it = pos_log_list->cbegin(); it != pos_log_list->cend(); it++){
            total_tmp++;
            if(it->executed){
                if(it->profit_taked){
                    total_profit_tmp++;
                }
                else{
                    total_loss_tmp++;
                }
            }
            average_rebound_plus_tmp += it->max_rebound_plus;
            average_rebound_minus_tmp -= it->max_rebound_minus;
        }
        total = total_tmp;
        total_profit = total_profit_tmp;
        total_loss = total_loss_tmp;
        if(total_tmp != 0){
            average_rebound_plus = average_rebound_plus_tmp/total_tmp;
            average_rebound_minus = average_rebound_minus_tmp/total_tmp;
        }
    }
    QString get_log(){
        QString reply;
        reply.append("Всего позиций:");
        reply.append(QString::fromStdString(std::to_string(total)));
        reply.append("\nПозиций в плюс:");
        reply.append(QString::fromStdString(std::to_string(total_profit)));
        reply.append("\nПозиций в минус:");
        reply.append(QString::fromStdString(std::to_string(total_profit)));
        reply.append("\nСредний отскок в плюс:");
        reply.append(QString::fromStdString(std::to_string(average_rebound_plus)));
        reply.append("\nСредний отскок в минус:");
        reply.append(QString::fromStdString(std::to_string(average_rebound_minus)));
        reply.append('\n');
        return std::move(reply);
    }
};
    class Logger
    {
    public:
        Logger();
        void update_positions_log(const QString &id, double max_rebound_plus, double max_rebound_minus, bool executed, bool profit_taked);
        void update_daily_positions_log();
        QString get_positions_log();
    private:
        std::list<positionLog> positions;
        dailyPositionsLog daily_pos_log;
    };
}//namespace binanace ***************
#endif // LOGGER_H
