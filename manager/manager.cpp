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

    if(orders[0].size() == 0)
        return false;
    send_result();
    return true;
}

void Manager::updatePair(const std::pair<int, QString> &pair)
{

    orders[pair.first].clear();
    for(int i = 0; i < manager::timefraemes.size(); i++){
        orders[pair.first].splice(orders[pair.first].end(), *Orders(pair.second, manager::timefraemes[i]).data());
    }
    compress(pair);

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
            if(list_it->area == Area::liquid)
            {
                if(list_it->timeframe.size() > 3
                        && (list_it->time < (std::chrono::system_clock::now().time_since_epoch().count()/1000000) - 1000/*ms*/ * 60 /*s*/ * 60 /*m*/ * 24/*h*/ * 1/*d*/))
                    ss << *list_it;

                //ебля с временем
                //std::cout <<"________"<< tg::convertTime(/*list_it->time*/1677024000000).toStdString() << "\n";
                //std::cout <<"________"<< tg::convertTime(/*list_it->time*/1677024000000 - 1000/*ms*/ * 60 /*s*/ * 60 /*m*/ * 24/*h*/ * 1/*d*/).toStdString() << "\n";
                //std::cout <<"________"<< std::chrono::system_clock::now().time_since_epoch().count()/1000000 << "\n";
            }
            else
                ss << *list_it;
            if(ss.str().size() > 3500){
                tg.send_message(QString::fromStdString(ss.str()));
                ss.str("");
            }
        }
    }
    if(!ss.str().empty())
        tg.send_message(QString::fromStdString(ss.str()));

}
