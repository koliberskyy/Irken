#include "selltobuy.h"

SellToBuy::SellToBuy(const std::vector<CandleStick> *__klines)
{
    klines = __klines;
}
void SellToBuy::init()
{
//поряд от 2х до 4х зеленых, дальше красные (без зеленых) с пробитием лоя первой зеленой
    const CandleStick *last_red{nullptr};
    const CandleStick *first_red{nullptr};
    int red_counter{0};
    int green_counter{0};
    bool probitije{false};

    for(auto i = 0; i < klines->size() - 1; i++){
        if(klines->at(i).color == KlineColor::red){
            if(green_counter > 0){
                last_red = nullptr;
                first_red = nullptr;
                green_counter = 0;
                red_counter = 0;
                probitije = false;
            }

            if(first_red == nullptr){
                first_red = &klines->at(i);
                red_counter++;
            }
            else if(red_counter < 4){
                last_red = &klines->at(i);
                red_counter++;
            }
            else{
                last_red = nullptr;
                first_red = nullptr;
                green_counter = 0;
                red_counter = 0;
                probitije = false;
            }
        }
        else if(red_counter > 1 && green_counter < 3){
            green_counter++;
            if(klines->at(i).close > first_red->hight){
                if(probitije){
                    add(first_red->hight, last_red->low, last_red->open_time, Area::stb);
                    last_red = nullptr;
                    first_red = nullptr;
                    green_counter = 0;
                    red_counter = 0;

                    probitije = false;
                }
                else
                    probitije = true;
            }
        }
        else{
            last_red = nullptr;
            first_red = nullptr;
            probitije = false;
            green_counter = 0;
            red_counter = 0;
        }
    }
}
