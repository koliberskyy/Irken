#include "areas.h"

double Areas::nearest_above(double val)
{
    auto nearest{100000000.0};
    for(auto it : areas){
        if(it.low < nearest && it.low > val)
            nearest = it.low;
    }
    return nearest;
}

double Areas::nearest_below(double val)
{
    auto nearest{0.0};

    for(auto it : areas){
        if(it.hight > nearest && it.hight < val)
            nearest = it.hight;
    }
    return nearest;
}

void Areas::add(double hight, double low, double time, Area area)
{
    areas.emplace_back(AreaPair(hight, low, time, area));
}
