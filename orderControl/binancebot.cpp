#include "binancebot.h"
namespace binance {

    BinanceBot::BinanceBot():
    orders{nullptr}
    {
        for(int i = 0; i < key::API_SIZE; i++){
            accounts.emplace_back(key::API[i], key::SECRET[i]);
        }
    }

    OrderData* BinanceBot::was_sended(const QString &id)
    {
        for(auto it = sended_orders.begin(); it != sended_orders.end(); it++){
            if(it->id() == id)
                return &(*it);
        }
        return nullptr;
    }

    bool BinanceBot::is_canceled(const QString &id)
    {
        for(auto i = 0; i < manager::pairs.size(); i++){
            for(auto it = orders->at(i).begin(); it != orders->at(i).end(); it++){
                if(it->id() == id)
                    return false;
            }
        }
        return true;
    }

    void BinanceBot::post_order(const OrderData &order)
    {
        std::cout << "void BinanceBot::post_order(const OrderData &order)";

        for(auto it = accounts.begin(); it != accounts.end(); it++){
            it->new_order(order);
        }
    }

    void BinanceBot::cancel_order(const OrderData &order)
    {
        for(auto it = accounts.begin(); it != accounts.end(); it++){
            it->cancel_order(order);
        }
    }

    void BinanceBot::update_orders(OrderBook *__orders)
    {
        orders = __orders;
        std::cout << "void BinanceBot::update_orders(OrderBook *__orders)";
        //update sended
        for(auto i = 0; i < manager::pairs.size(); i++){
            for(auto it = orders->at(i).begin(); it != orders->at(i).end(); it++){
                auto sended = was_sended(it->id());
                if(sended == nullptr){
                    post_order(*it);
                    //задержка для того чтобы бинанс не ругался
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    sended_orders.emplace_back(*it);
                }
                else{
                    sended->update_actuality(it->actuality);
                }
            }
        }

        //remove_no_actual
        for(auto it = sended_orders.begin(); it != sended_orders.end();){
            if(is_canceled(it->id())){
                cancel_order(*it);
                it = sended_orders.erase(it);
            }
            else
                it++;
        }

    }

    int BinanceBot::update_control()
    {
        int positions_count(0);
        for(auto it = accounts.begin(); it != accounts.end(); it++){
           positions_count = it->update_control();
        }
        return positions_count;
    }
}//namespace binance****************************
