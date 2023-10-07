#include "logger.h"
namespace binance{
    Logger::Logger():daily_pos_log(&positions)
    {
        //daily_pos_log = dailyPositionsLog(&positions);
    }

    void Logger::update_positions_log(const QString &id, double max_rebound_plus, double max_rebound_minus, bool executed, bool profit_taked)
    {
        auto iter = positions.end();
        for(auto it = positions.begin(); it !=positions.end(); it++){
            if(it->id == id){
                iter = it;
            }
        }
        if(iter == positions.end()){
            positions.emplace_back(positionLog(id, max_rebound_plus, max_rebound_minus, executed, profit_taked));
        }
        else{
            iter->update_max_rebound_minus(max_rebound_minus);
            iter->update_max_rebound_plus(max_rebound_plus);
            iter->executed = executed;
            iter->profit_taked = profit_taked;
        }
    }

    void Logger::update_daily_positions_log()
    {
        daily_pos_log.update_daily_log();
    }

    QString Logger::get_positions_log()
    {
        return std::move(daily_pos_log.get_log());
    }
}// namespace biance *********
