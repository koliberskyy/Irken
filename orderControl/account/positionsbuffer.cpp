#include "positionsbuffer.h"
namespace binance{
    PositionsBuffer::PositionsBuffer(): isStopOrdersChanged{false}
    {

    }

    void PositionsBuffer::update(const QJsonArray &arr)
    {
        for(auto i = 0; i < arr.size(); i++){
            auto object = arr.at(i).toObject();
            auto id = object["symbol"].toString() + object["positionAmt"].toString();
            auto it = positions_tab.find(id);
            if(it == positions_tab.end()){
                add_position(object, id);
            }
            else{
                it->second.isFinded = true;
                modify_position(object, id);
            }
        }

        get_new_orders();
    }

    void PositionsBuffer::clear_buf()
    {
        positions_tab.clear();
        stop_orders.clear();
        take_orders.clear();
    }

    void PositionsBuffer::add_position(const QJsonObject &pos, const QString &id)
    {
        auto symbol = pos["symbol"].toString();
        double entry_price = pos["entryPrice"].toString().toDouble();
        auto long_pos = [&pos]()->bool {if(pos["positionAmt"].toString().at(0) != '-') return true; else return false;};

        positions_tab[id] = position(symbol, entry_price, long_pos());
    }

    void PositionsBuffer::modify_position(const QJsonObject &pos, const QString &id)
    {
#ifdef LEGACY
        auto mark_price = pos["markPrice"].toString().toDouble();
        auto entry_price = pos["entryPrice"].toString().toDouble();
        double profit = ((mark_price / entry_price) * 100)- 100;

        if(positions_tab[id].isPosLong){
            if(profit > (positions_tab[id].stop_percent + 0.5/*дипазон проторговки*/ + 0.25/*шаг перестановки*/)){
                positions_tab[id].set_stop_price(positions_tab[id].stop_percent + 0.25);
                positions_tab[id].isModifyed = true;
            }
        }
        else{
            if((-1 * profit) > (positions_tab[id].stop_percent + 0.5/*дипазон проторговки*/ + 0.25/*шаг перестановки*/)){
                positions_tab[id].set_stop_price(positions_tab[id].stop_percent + 0.25);
                positions_tab[id].isModifyed = true;
            }
        }
#endif
        auto mark_price = pos["markPrice"].toString().toDouble();
        auto entry_price = pos["entryPrice"].toString().toDouble();
        double profit = ((mark_price / entry_price) * 100) - 100;

        //если шорт то профит будет отрицатльным, а для адекватного восприятия методам класса position  необходимо положителньое число
        if(!positions_tab[id].isPosLong)
            profit *= (-1);
        // стоп без убытка по точке входа
        if(profit >= settings::stop_BU){
            positions_tab[id].set_stop_price(0.0);
            positions_tab[id].isModifyed = true;
        }
    }

    void PositionsBuffer::get_new_orders()
    {
        if(!positions_tab.empty()){
            for(auto it = positions_tab.begin(); it != positions_tab.end(); it++){
                if(it->second.isModifyed){
                    //stop loss
                    stop_orders.emplace_back(it->second.symbol, it->second.stop_price, it->second.isPosLong);
                    it->second.isModifyed = false;
                    if(!isStopOrdersChanged) isStopOrdersChanged = true;
                    //take profit
                    if(!it->second.isTakePosted){
                        take_orders.emplace_back(it->second.symbol, it->second.take_price, it->second.isPosLong);
                        it->second.isTakePosted = true;
                        if(!isTakeOrdersChanged) isTakeOrdersChanged = true;
                    }
                }
                //удаление исполненых
                if(!it->second.isFinded){
                    if(positions_tab.size() > 1){
                        it = positions_tab.erase(it);
                        if(it == positions_tab.end())
                            break;
                    }
                    else{
                        positions_tab.clear();
                        break;
                    }
                }
            }

            if(!positions_tab.empty())
                for(auto it = positions_tab.begin(); it != positions_tab.end(); it++){
                    it->second.isFinded = false;
            }
        }
    }
}//namesapce binance****************
