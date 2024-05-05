#include "abstractkunteynir.h"

AbstractKunteynir::AbstractKunteynir(QWidget *parent):
    QScrollArea(parent),
    layout_main{new QVBoxLayout()}
{
    wgt = new QWidget();
    wgt->setLayout(layout_main);

    setWidget(wgt);
    setWidgetResizable(true);

}

QList<AbstractItem *>::iterator AbstractKunteynir::findItemPosition(AbstractItem *item)
{
    return std::find(items.begin(), items.end(), item);
}

QList<AbstractItem *> AbstractKunteynir::get_items() const
{
    return items;
}

void AbstractKunteynir::addItem(AbstractItem *item)
{
    items.append(item);
    layout_main->addWidget(item);
    wgt->setFixedHeight(items.size() * item->itemHeight());
}

QList<AbstractItem* >::iterator AbstractKunteynir::removeItem(AbstractItem *item)
{
    if(item != nullptr){
        layout_main->removeWidget(item);
        wgt->setFixedHeight((items.size()-1) * item->itemHeight());
        return items.erase(findItemPosition(item));
    }
    return items.end();
}

void AbstractKunteynir::removeAllItems()
{
    for(auto it : items)
        removeItem(it);
}


