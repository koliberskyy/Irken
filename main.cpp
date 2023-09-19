#include <QCoreApplication>
#include "manager.h"
#include "thread"
void do_work(){
    Manager manager;
    try {
        auto flag{true};
        while(true){
            if(flag){
                auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::cout << "try to update...time: "<< std::ctime(&time) <<"\n";

                manager.telegram()->send_message("Ну что, ручки почесали и поконем");

                if(!manager.update())
                    throw "err";

                time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::cout << "updating complete. time: "<< std::ctime(&time) <<"\n";
                manager.telegram()->send_message("Чики брики и в дамки");

                flag = false;
            }
            if(std::chrono::system_clock::now().time_since_epoch().count()/1'000'000 % (1000 * 60 * 60) == 0){
                auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::cout << "try to update...time: "<< std::ctime(&time) <<"\n";

                manager.telegram()->send_message("Ну что, ручки почесали и поконем");

                if(!manager.update())
                    throw "err";

                time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::cout << "updating complete. time: "<< std::ctime(&time) <<"\n";
                manager.telegram()->send_message("Чики брики и в дамки");
            }
        }
    }  catch (...) {
        std::cout << "Kажется пора включить интернет..." << '\n';
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
