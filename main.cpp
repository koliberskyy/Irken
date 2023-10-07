#include <QCoreApplication>
#include "manager.h"
#include "thread"
#include "binancebot.h"
void do_work(){
    Manager manager;
    manager.telegram()->send_message("Irken 3.2.0ocb\nПроверяю ключи API... \nСписок пользователей:");
    manager.test_conectivity();
    manager.telegram()->send_message("Начинаю работать...");

    manager.update_control();

    try {
        auto flag{true};
        while(true){
            if(flag){
                auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::cout << "try to update...time: "<< std::ctime(&time) <<"\n";

                if(!manager.update())
                    throw "err";

                manager.update_control();
                flag = false;
            }

            for(int i = 0; i < 40; i++){
                manager.update_control();



                                /*ЗАЛУПА*/
        int k =         /*ЗАЛУПА*/ 3000 /*ЗАЛУПА*/       ;
                                /*ЗАЛУПА*/




                std::this_thread::sleep_for(std::chrono::milliseconds(k));
            }

            auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::cout << "\ntry to update...time: "<< std::ctime(&time) <<"\n";
            if(!manager.update())
                throw "err";

        }
    }  catch (...) {
        std::cout << "Инет сдох. Включите пожалуйста интернет." << '\n';
        std::this_thread::sleep_for(std::chrono::seconds(60));
        do_work();
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    do_work();
    return a.exec();
}
