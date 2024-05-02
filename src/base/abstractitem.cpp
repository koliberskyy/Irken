#include "abstractitem.h"

AbstractItem::AbstractItem(QWidget *parent)
    : QWidget{parent},
      layout_main{new QHBoxLayout(this)}
{
    setFixedHeight(oneItemHeight);

}

QJsonObject AbstractItem::get_data() const
{
    return data;
}

int AbstractItem::itemHeight()
{
    return oneItemHeight;
}
