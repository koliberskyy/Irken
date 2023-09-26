#include "buytosell.h"

BuyToSell::BuyToSell(const std::vector<CandleStick> *__klines)
{
    klines = __klines;
}

void BuyToSell::init()
{
    //поряд от 2х до 4х зеленых, дальше красные (без зеленых) с пробитием лоя первой зеленой
    const CandleStick *last_green{nullptr};
    const CandleStick *first_green{nullptr};
    int red_counter{0};
    int green_counter{0};
    bool probitije{false};

    for(auto i = 0; i < klines->size() - 1; i++){
        if(klines->at(i).color == KlineColor::green){
            if(red_counter > 0){
                last_green = nullptr;
                first_green = nullptr;
                green_counter = 0;
                red_counter = 0;
                probitije = false;
            }

            if(first_green == nullptr){
                first_green = &klines->at(i);
                green_counter++;
            }
            else if(green_counter < 4){
                last_green = &klines->at(i);
                green_counter++;
            }
            else{
                last_green = nullptr;
                first_green = nullptr;
                green_counter = 0;
                red_counter = 0;
                probitije = false;
            }
        }
        else if(green_counter > 1 && red_counter < 3){
            red_counter++;
            if(klines->at(i).close < first_green->low){
                if(probitije){
                    add(last_green->hight, first_green->low, last_green->open_time, Area::bts);
                    last_green = nullptr;
                    first_green = nullptr;
                    green_counter = 0;
                    red_counter = 0;

                    probitije = false;
                }
                else
                    probitije = true;
            }
        }
        else{
            last_green = nullptr;
            first_green = nullptr;
            probitije = false;
            green_counter = 0;
            red_counter = 0;
        }
    }
}
