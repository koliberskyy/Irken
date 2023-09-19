#ifndef AREAPAIR_H
#define AREAPAIR_H
#include <QJsonArray>
#include "AeasEnums.h"
class AreaPair{
public:
    double hight;
    double low;
    qint64 time;
    Area area;
    AreaPair():hight{0.0}, low{0.0}{}
    AreaPair(double h, double l, qint64 t, Area a): hight{h}, low{l}, time{t}, area{a}{}

};
#endif // AREAPAIR_H
