#include <QCoreApplication>
#include "manager.h"
#include "thread"
#include "binancebot.h"
void do_work(){
    Manager manager;
    try {
        auto flag{true};
        while(true){
            if(flag){
                auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::cout << "try to update...time: "<< std::ctime(&time) <<"\n";

                if(!manager.update())
                    throw "err";

                manager.send_result();
                std::cout << "\tpositions: " << manager.update_control() <<'\n';
                time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::cout << "updating complete. time: "<< std::ctime(&time) <<"\n";
                flag = false;
            }
            //обновление ордеров
            if(std::chrono::system_clock::now().time_since_epoch().count()/1'000'000 % (1000 * 60 * 5) == 0){
                auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::cout << "try to update...time: "<< std::ctime(&time) <<"\n";

                if(!manager.update())
                    throw "err";
                std::cout << "\tpositions: " << manager.update_control() <<'\n';
                time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::cout << "updating complete. time: "<< std::ctime(&time) <<"\n";
            }
            //отправка результатов в тг
            if(std::chrono::system_clock::now().time_since_epoch().count()/1'000'000 % (1000 * 60 * 58) == 0){
                manager.send_result();
            }
            //обновление контроля
            if(std::chrono::system_clock::now().time_since_epoch().count()/1'000'000 % (1000 * 58) == 0){
                auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::cout << "try to update control... time: "<< std::ctime(&time) <<"";
                std::cout << "\tpositions: " << manager.update_control() <<'\n';
                time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::cout << "updating complete. time: "<< std::ctime(&time) <<"\n";

            }
        }
    }  catch (...) {
        std::cout << "Либо инет сдох, либо заебись отфильтровалось" << '\n';
        auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::cout << "control... time: "<< std::ctime(&time) <<"";
        std::cout << "\tpositions: " << manager.update_control() <<'\n';
        time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::cout << "complete. time: "<< std::ctime(&time) <<" ";
        std::this_thread::sleep_for(std::chrono::seconds(60));
        time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::cout << "control... time: "<< std::ctime(&time) <<"";
        std::cout << "\tpositions: " << manager.update_control() <<'\n';

        do_work();
    }
}

void test();

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //test();
    do_work();
    return a.exec();
}


void test(){
//    binance::BinanceAccount acc(binance::key::API[0], binance::key::SECRET[0]);
//    //double usdt = acc.usdt_balance();
//    double hui = 10.0;
//    std::cout << "acc.balances(): \n";
//    //std::cout << acc.balances()->toStdString() <<"\n\n\n\n";
//    std::cout << "usdt balance(): RA \n";
//    //std::cout <<"\n usdt balance: " <<  usdt << '\n';
//    std::cout << "post_order: RABOTAET, NEHUI TUT SMOTRETI\n";
//    //длинная короткая зменяется параметром side(BUY, SELL);
//    //сумма сделки(маржа) равна выбранное плечо делить на количество умножить на цену
//    // м = (квонтити * прайс)/леверейдж
//    // м*л = к*п
//    //к = м*л/п
//    //std::cout << acc.post_order("BTCUSDT","BUY", "LIMIT", "GTC", "0.03900000","25000","TEST_ORDER","","")->toStdString() << '\n';
//    //std::this_thread::sleep_for(std::chrono::seconds(1));
//    //std::cout << "delete_order: \n";
//    //std::cout << acc.delete_order("BTCUSDT", "TEST_ORDER")->toStdString() << '\n';
//    //****DONE****
//    std::cout << "account: RABOTAET" << '\n';
//    //std::cout << acc.account()->toStdString() <<"\n\n\n\n";
//    std::cout << "positions: " << '\n';
//    acc.positions();
//    std::cout << "\n\n\n\n" << '\n';

    ///test close_position //RABOTAET DLYA LONG ORDEROV
//    auto qjarr = acc.positions();
//    for(auto i = 0; i < qjarr.size(); i++){
//        acc.close_position(qjarr.at(i).toObject(), false);
//    }

    Manager m;
    m.update();
    m.send_result();


}
