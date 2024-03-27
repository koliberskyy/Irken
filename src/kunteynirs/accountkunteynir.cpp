#include "accountkunteynir.h"

AccountKunteynir::AccountKunteynir(QWidget *parent, const QString &__filename):
    AbstractKunteynir(parent),
    fileName{__filename}
{
    updateAccounts();
}

QList<AbstractItem *>::iterator AccountKunteynir::findItem(const QJsonObject &item)
{
    return items.end();
}

QList<AccountItem *> AccountKunteynir::list()
{
    QList<AccountItem *> reply;
    auto items = get_items();
    for(auto item : items){
        reply.append((AccountItem*) item);
    }
    return reply;
}

void AccountKunteynir::updateAccounts()
{
    if(!items.isEmpty())
        removeAllItems();

    QFile qfile(fileName);
    qfile.open(QIODevice::OpenModeFlag::ReadOnly);
    auto jsonArr = QJsonDocument::fromJson(qfile.readAll()).array();
    qfile.close();
    for (auto it : jsonArr){
        addItem(new AccountItem());
        items.last()->updateData(it.toObject());
    }
}

void AccountKunteynir::updateBalance()
{
    for(auto it : items){
        auto item = (AccountItem*)it;
        auto balance = Methods::getBalance(item->get_api(), item->get_secret());

        if(!balance.isEmpty())
            item->set_balance(std::move(balance));
    }

}
