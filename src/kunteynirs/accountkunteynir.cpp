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
    updateBalance(nullptr);
}

void AccountKunteynir::updateBalance(AccountItem *acc)
{
    if(acc == nullptr){
        for(auto it : items){
            auto item = (AccountItem*)it;

            if(item->get_whenBalanceWasUpdated().secsTo(QDateTime::currentDateTime()) > settings::accBalanceUpdateFluencySec){
                auto balance = Methods::getBalance(item->get_api(), item->get_secret());

                if(!balance.isEmpty())
                    item->set_balance(std::move(balance));
            }
        }
    }
    else if(acc->get_whenBalanceWasUpdated().secsTo(QDateTime::currentDateTime()) > settings::accBalanceUpdateFluencySec){
        auto balance = Methods::getBalance(acc->get_api(), acc->get_secret());
        while(balance.isEmpty()){
            balance = Methods::getBalance(acc->get_api(), acc->get_secret());
        }
        acc->set_balance(std::move(balance));
    }
}

void AccountKunteynir::shareOrder(QJsonObject order, int leverage)
{
    shareOrder(order, leverage, list());
}

void AccountKunteynir::shareOrder(QJsonObject order, int leverage, QList<AccountItem *> accList)
{
    QList<AccountItem* > currentAccList;
    if(!accList.isEmpty()){
         currentAccList = accList;
    }
    else{
        currentAccList = list();
    }

    auto isMarketOrder = order["orderType"].toString() == "Market";
    auto qtyPercent = order["qty"].toString().toDouble();
    //choosed accounts
    auto choosed = AccountItem::showAccountChooseDialog(currentAccList, "Выбор аккаунтов");

    //progess dialog
    auto pd = new QProgressDialog("Прогресс",  "Остановить", 0, choosed.size(), this);

    //after order will placed on every acc, signal of increase progress of this func must generates
    for(auto it : choosed){
        pd->setLabelText(it->get_name() + " " + it->get_name_second());
        //update_accBalance HARD(if time before last upadate > settings::accBalanceUpdate...)
        updateBalance(it);
        //if balance > 10 usdt
        if(it->get_balance() > 5.0){
            //use one info for all operations, needs to be cleared after every operation
            bybitInfo info;

            //get exist (placed before) orders HARD
            bool existTrigger{false};
            if(!isMarketOrder){
                info = Methods::getOpenOrders(it->get_api(), it->get_secret());
                while (info.retCode() != 0){
                    info = Methods::getOpenOrders(it->get_api(), it->get_secret());
                    if(pd->wasCanceled()){
                        break;
                    }
                }
                auto exist = info.list();
                info.clear();

                //if odrer's symbol && order's price != same parameters of one of exist orders
                for(int i = 0; i < exist.size(); i++){
                    auto obj = exist[i].toObject();
                    if(obj["symbol"].toString() == order["symbol"].toString()
                            && obj["price"].toDouble() == order["price"].toDouble()){
                        existTrigger = true;
                        break;
                    }
                }
            }
            //if order was not paced before
            if(!existTrigger){
                //calculate qty
                auto qty = Methods::qty_to_post(it->get_balance(), order["price"].toString().toDouble(), qtyPercent);
                order["qty"] =  QString::fromUtf8(instruments::double_to_utf8(order["symbol"].toString().toUtf8(), instruments::Filter_type::lotSize, qty, leverage));

                //do place order while retcode != 0 ("OK") HARD
                do{
                    info = Methods::placeOrder(order, it->get_api(), it->get_secret());
                    if(pd->wasCanceled()){
                        break;
                    }
                }while(info.retCode() != 0);
            }
        }
        if(pd->wasCanceled()){
            break;
            pd->deleteLater();
        }
        //increase progress
        pd->setValue(pd->value() + 1);

    }
    pd->deleteLater();
}
