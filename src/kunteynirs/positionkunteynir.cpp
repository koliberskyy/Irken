#include "positionkunteynir.h"

PositionKunteynir::PositionKunteynir(QWidget *parent):
    AbstractKunteynir(parent)
{

}

QList<AbstractItem *>::iterator PositionKunteynir::findItem(const QJsonObject &item)
{
    auto it = items.begin();

    while(it != items.end()){
        auto curr_item = (PositionItem*)*it;
        if(curr_item->get_symbol() == item["symbol"].toString()
                && curr_item->get_side() == item["side"].toString() ){
            return it;
        }
        else{
            it++;
        }
    }

    return it;
}

void PositionKunteynir::set_accounts(QList<AccountItem *> accList)
{
    accounts = accList;
}

void PositionKunteynir::download(AccountItem *acc)
{
    updateFinished = false;
    if(acc == nullptr){
        for(auto it : accounts){
            auto info = Methods::getPositionList(it->get_api(), it->get_secret());
            while(info.retCode() != 0){
                info = Methods::getPositionList(it->get_api(), it->get_secret());
            }

            updatePositions(info.list(), it);
        }
    }
    else
    {
        auto info = Methods::getPositionList(acc->get_api(), acc->get_secret());
        while(info.retCode() != 0){
            info = Methods::getPositionList(acc->get_api(), acc->get_secret());
        }

        updatePositions(info.list(), acc);

    }

    emit updatingComplete(items);
    updateFinished = true;
}

void PositionKunteynir::updatePositions(const QJsonArray &arr, AccountItem *owner)
{
    for(auto it : arr){
        auto obj = it.toObject();
        auto exist = findItem(obj);

        QString sym("");
        if(exist != items.end())
             sym = ((PositionItem *)*exist)->get_symbol();

        if(exist == items.end()){
            addItem(new PositionItem());
            auto last = (PositionItem *)items.last();
            last->updateData(obj, owner);
            last->setUpdated();

            if(owner != nullptr)
                last->addOwner(owner);
        }
        else{
            ((PositionItem *)*exist)->updateData(obj, owner);
            ((PositionItem *)*exist)->setUpdated();
            if(owner != nullptr)
                ((PositionItem *)*exist)->addOwner(owner);
        }
    }

    //remove closed positions
    auto it = items.begin();
    while(it != items.end()){
        auto item = (PositionItem *)*it;
        if(!item->isUpdated()){
            if(owner != nullptr)
                item->removeOwner(owner);

            if(item->ownersSize() == 0)
                it = removeItem(item);
            else
                it++;
        }
        else{
            item->setUpdated(false);
            it++;
        }
    }
}
