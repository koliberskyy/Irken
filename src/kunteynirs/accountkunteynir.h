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
    void updateBalance();
private:
    QString fileName;




};

#endif // ACCOUNTKUNTEYNIR_H
