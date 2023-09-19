#ifndef AEASENUMS_H
#define AEASENUMS_H
#include <iostream>
enum class Area{null, stb, bts, order_block, braker, swick_up, swick_low, sk_up, sk_low, liquid, imbalance_up, imbalance_low};

inline std::ostream& operator<<(std::ostream &out, Area area){
    switch(area){
    case Area::null:
        out << "null";
        break;
    case Area::braker:
        out << "braker";
        break;
    case Area::bts:
        out << "buy to sell";
        break;
    case Area::liquid:
        out << "liquidity";
        break;
    case Area::order_block:
        out << "order block";
        break;
    case Area::sk_up:
        out << "sk_up";
        break;
    case Area::sk_low:
        out << "sk_low";
        break;
    case Area::swick_up:
        out << "swick_up";
        break;
    case Area::swick_low:
        out << "swick_low";
        break;
    case Area::stb:
        out << "sell to buy";
        break;
    case Area::imbalance_up:
        out << "imbalance_up";
        break;
    case Area::imbalance_low:
        out << "imbalance_low";
        break;

    }
    return out;
}

#endif // AEASENUMS_H
