#ifndef ACCOUNTITEM_H
#define ACCOUNTITEM_H

#include <QTabWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QCheckBox>

#include "abstractitem.h"

class AccountItem : public AbstractItem
{
    Q_OBJECT
public:
    explicit AccountItem(QWidget *parent = nullptr);

    /*
     *  пустой qjsonobject не обнуляет объект, а устанавливает значения из qjsonobject который хранится в классе
    */
    virtual void updateData(const QJsonObject &obj = QJsonObject()) override;

    //getters
    QString get_name() const;
    QString get_name_second() const;
    QString get_balance_str() const;
    QString get_api() const;
    QString get_secret() const;

    double get_balance() const;

    static QList<AccountItem *> showAccountChooseDialog (QList <AccountItem *> accounts, const QString &text = "", bool defaultCheckboxes = true);

public slots:
    //setters
    void set_balance(double balance);
    void set_balance(const QString &balance);
    void set_balance(QString &&balance);
private:
    QLabel *label_name;
    QLabel *label_name_second;
    QLabel *label_balance;
    QLabel *label_balance_info;


};

#endif // ACCOUNTITEM_H
