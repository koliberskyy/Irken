#include "manager.h"

Manager::Manager()
{
}

bool Manager::update()
{
    std::vector<std::thread> thr_vec;
    for(int i = 0; i < manager::pairs.size(); i++){
        thr_vec.emplace_back(std::thread(&Manager::updatePair, this, manager::pairs[i]));
    }

    for(int i = 0; i < manager::pairs.size(); i++){
        if(thr_vec.at(i).joinable())
            thr_vec.at(i).join();
    }

    //проверка отсутствия интернета
    if(!binance_bot.ping())
        return false;

    binance_bot.update_orders(&orders);
    if(!binance_bot.get_reply()->isEmpty()){
        tg.send_message(*binance_bot.get_reply());
        tg.send_message(QString("Актуальный ip: ") + binance_bot.ip());
    }
    return true;
}

void Manager::updatePair(const std::pair<int, QString> &pair)
{

    orders[pair.first].clear();
    for(int i = 0; i < manager::timefraemes.size(); i++){
        orders[pair.first].splice(orders[pair.first].end(), *Orders(pair.second, manager::timefraemes[i]).data());
    }
    compress(pair);
    filter(pair);
}

void Manager::compress(const std::pair<int, QString> &pair)
{
    //смысл в том чтобы слепить ордера из разных таймфреймов в один ордер
    for(auto it = orders[pair.first].begin(); it != orders[pair.first].end(); it++){
        if(it->area == Area::liquid || it->area == Area::imbalance_low || it->area == Area::imbalance_up || it->area == Area::swick_low || it->area == Area::swick_up){
            auto jit = it;
            jit++;//смотрим со следующего за исследуемым
            for(; jit != orders[pair.first].end(); jit++){
                if(jit->point_of_entry == it->point_of_entry){
                    it->timeframe += jit->timeframe;
                    jit = orders[pair.first].erase(jit);
                    jit--;//так как эрейз вернет итератор на элемент следующий за удаленным а фор его плюсплюснет
                }
            }
        }
    }
}

void Manager::filter(const std::pair<int, QString> &pair)
{
    //фильтры
    for(auto it = orders[pair.first].begin(); it != orders[pair.first].end();){
        //фильтр по времени возникновения зоны
        if(it->time > (std::chrono::system_clock::now().time_since_epoch().count()/1000000) - 1000/*ms*/ * 60 /*s*/ * 60 /*m*/ * 12/*h*/ * 1/*d*/
                //фильтр по актуальности
                || it->actuality < 0.0 || it->actuality > 0.5
                //фильтр для мелочной ликвиности
                || (it->area == Area::liquid && it->shoulder > 196)
                )
        {
            it = orders[pair.first].erase(it);
        }
        else
            it++;
    }
}


void Manager::print_result_file()
{
    std::ofstream os;
    os.open("result.txt");
    os.close();
}

void Manager::send_result()
{
    //tg.send_document("result.txt");
    //костыль
    std::stringstream ss;
    for(auto i = 0; i < manager::pairs.size(); i++){
        for(auto list_it = orders[i].begin(); list_it != orders[i].end(); list_it++){
                ss << *list_it;
            if(ss.str().size() > 3500){
                tg.send_message(QString::fromStdString(ss.str()));
                ss.str("");
            }
        }
        if(!ss.str().empty()){
            tg.send_message(QString::fromStdString(ss.str()));
            ss.str("");
        }
    }
}



void Manager::update_control()
{
    auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout <<std::ctime(&time)<< "--->positions: " << binance_bot.update_control() <<", ";
}
