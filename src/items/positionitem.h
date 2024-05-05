#ifndef POSITIONITEM_H
#define POSITIONITEM_H

#include "abstractitem.h"
#include "accountitem.h"

#include <QMap>



class PositionItem : public AbstractItem
{
    Q_OBJECT
public:
    explicit PositionItem(QWidget *parent = nullptr);
    /*
     *  пустой qjsonobject не обнуляет объект, а устанавливает значения из qjsonobject который хранится в классе
    */
    virtual void updateData(const QJsonObject &obj = QJsonObject()) override;
    virtual void updateData(const QJsonObject &obj, AccountItem *owner);
    virtual void updateData(PositionItem* item);
    virtual void addOwner(AccountItem *owner, const QJsonObject &obj = QJsonObject());
    virtual bool removeOwner(AccountItem* owner);
    virtual int ownersSize() const;

    virtual int itemHeight() override;

    //getters
    QString get_symbol() const;
    QString get_side() const;
    QString get_poe(AccountItem* owner = nullptr) const;
    QString get_tp(AccountItem* owner = nullptr) const;
    QString get_sl(AccountItem* owner = nullptr) const;
    QString get_mark() const;
    QString get_pnl() const;
    QString get_leverage(AccountItem* owner = nullptr) const;
    QString get_size(AccountItem* owner = nullptr) const;

    QList<AccountItem *>            get_owners() const;
    QMap<AccountItem*, QJsonObject> get_owners_data() const;

    bool isUpdated() const {return updated;}
    bool isEmpty() const {return data.isEmpty();}


protected slots:
    void buttonAccountsPressed();
    void buttonClosePressed();
    void buttonStopBUPressed();
    void setTpSL();

private:
    bool updated{false};

    QLabel *label_symbol;
    QLabel *label_side;

    QLabel *label_pnl;
    QPushButton *pb_stop_bu;
    QPushButton *pb_close_position;

    QLabel *label_poe;
    QLabel *label_markPrice;

    QDoubleSpinBox *dsb_sl;
    QDoubleSpinBox *dsb_tp;

    QLabel *label_leverage;
    QLabel *label_size;

    QPushButton *pb_accounts;

    QSpinBox *sb_ownersCount;

    QList<AccountItem *> acc_owners;
    QMap<AccountItem*, QJsonObject> owner_data;


    std::pair<double, double> showSlTpChooseDialog();
    double showRedusePercentDialog();


public:
    //setters
    //setters
    void setUpdated(bool trg = true){updated = trg;}

    void clear();



};

#endif // POSITIONITEM_H
