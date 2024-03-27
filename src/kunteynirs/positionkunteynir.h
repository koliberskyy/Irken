#ifndef POSITIONKUNTEYNIR_H
#define POSITIONKUNTEYNIR_H

#include <QThread>

#include "abstractkunteynir.h"
#include "positionitem.h"



class PositionKunteynir : public AbstractKunteynir
{
    Q_OBJECT
public:
    explicit PositionKunteynir(QWidget *parent = nullptr);
    virtual QList<AbstractItem *>::iterator findItem(const QJsonObject &item) override;
    void set_accounts(QList<AccountItem *> accounts = QList<AccountItem*>());

    bool isUpdated(){return updateFinished;}

public slots:
    /*
     * при не нулевом аккаунте качает только с него
     */
    void download(AccountItem * acc = nullptr);
private:
    void updatePositions(const QJsonArray &arr, AccountItem* owner = nullptr);
    QList<AccountItem *> accounts;
    bool updateFinished{true};
};

#endif // POSITIONKUNTEYNIR_H
