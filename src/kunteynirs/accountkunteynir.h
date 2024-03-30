#ifndef ACCOUNTKUNTEYNIR_H
#define ACCOUNTKUNTEYNIR_H

#include <QFile>
#include <QTabWidget>

#include "accountitem.h"
#include "abstractkunteynir.h"

class AccountKunteynir : public AbstractKunteynir
{
    Q_OBJECT
public:
    explicit AccountKunteynir(QWidget *parent = nullptr, const QString &__filename = "AccountList.txt");
    virtual QList<AbstractItem *>::iterator findItem(const QJsonObject &item) override;
    QList<AccountItem *> list();

public slots:
    void updateAccounts();
    /*
    * При передаче аккаунта будет пытаться обновить на нем баланс пока не получится, при acc == nullptr делает одну попытку загузки баланса на каждый акк
    */
    void updateBalance();
    void updateBalance(AccountItem * acc);
    void shareOrder(QJsonObject order, int leverage);
    void shareOrder(QJsonObject order, int leverage, QList<AccountItem*> accList);

signals:
    void allAccountsSucscessfulyOperated();
private:
    QString fileName;

};

#endif // ACCOUNTKUNTEYNIR_H
