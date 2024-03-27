#include "abstractitem.h"

AbstractItem::AbstractItem(QWidget *parent)
    : QWidget{parent},
      layout_main{new QHBoxLayout(this)}
{
    setFixedHeight(oneItemHeight);

}

int AbstractItem::itemHeight()
{
    return oneItemHeight;
}
