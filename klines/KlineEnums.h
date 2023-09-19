#ifndef KLINEENUMS_H
#define KLINEENUMS_H
#include <iostream>


enum class KlineColor{red, green};

inline std::ostream& operator<<(std::ostream &out, KlineColor color){
    switch(color){
    case KlineColor::green:
        out << "green";
        break;
    case KlineColor::red:
        out << "red";
        break;
    }
    return out;
}

enum class Thrand{up, down, range};


inline std::ostream& operator<<(std::ostream &out, Thrand tr){
    switch(tr){
    case Thrand::up:
        out << "up";
        break;
    case Thrand::down:
        out << "down";
        break;
    case Thrand::range:
        out << "range";
        break;
    }
    return out;
}
#endif // KLINEENUMS_H
