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
        auto reply = items.erase(findItemPosition(item));
        layout_main->removeWidget(item);

        //debug info
        std::cout << "remeove item from kunteynir result /n (after layout_main->removeWidget(item);(item == nullptr)): " << (item == nullptr);

        wgt->setFixedHeight((items.size()) * item->itemHeight());
        return reply;
    }
    return items.end();
}

void AbstractKunteynir::removeAllItems()
{
    for(auto it : items)
        removeItem(it);
}


