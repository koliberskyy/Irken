#include "accountitem.h"

AccountItem::AccountItem(QWidget *parent):
    AbstractItem(parent),
    balanceWasUpdatedTime{new QDateTime(QDateTime(QDate(2012, 7, 6), QTime(23, 55, 0)))}
{
    //name label
    label_name = new QLabel();
    label_name_second = new QLabel();
    auto vbl_n = new QVBoxLayout();
    vbl_n->addWidget(label_name);
    vbl_n->addWidget(label_name_second);

    //balance label
    label_balance_info = new QLabel("Баланс");
    label_balance = new QLabel("0.0");
    auto vbl_b = new QVBoxLayout();
    vbl_b->addWidget(label_balance_info);
    vbl_b->addWidget(label_balance);


    //main layout
    layout_main->addLayout(vbl_n);
    layout_main->addLayout(vbl_b);
}

void AccountItem::updateData(const QJsonObject &obj)
{
    if(!obj.isEmpty())
        data = obj;

    //name
    label_name->setText(get_name());
    label_name_second->setText(get_name_second());

    label_balance->setText(get_balance_str());

}

QString AccountItem::get_name() const
{
    if(data.isEmpty())
        return "";

    return data["name"].toString();
}

QString AccountItem::get_name_second() const
{
    if(data.isEmpty())
        return "";

    return data["secondName"].toString();
}

QString AccountItem::get_balance_str() const
{
    if(data.isEmpty())
        return "";

    return data["balance"].toString();
}

QString AccountItem::get_api() const
{
    if(data.isEmpty())
        return "";

    return data["api"].toString();
}

QString AccountItem::get_secret() const
{
    if(data.isEmpty())
        return "";

    return data["secret"].toString();
}

double AccountItem::get_balance() const
{
    return get_balance_str().toDouble();
}

QDateTime AccountItem::get_whenBalanceWasUpdated()
{
    return *balanceWasUpdatedTime;
}

QList<AccountItem *> AccountItem::showAccountChooseDialog(QList<AccountItem *> accounts, const QString &text, bool defaultCheckboxes)
{
    auto dlg = new QDialog();

    auto layout = new QVBoxLayout();
    auto label = new QLabel();

    label->setText(text);

    QDialogButtonBox *btn_box = new QDialogButtonBox();
    btn_box->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(btn_box, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    connect(btn_box, &QDialogButtonBox::rejected, dlg, &QDialog::reject);

    layout->addWidget(label);

    QList <QCheckBox *> boxes;
    for(auto it : accounts){
        auto check = new QCheckBox(it->get_name() + " " + it->get_name_second());
        check->setChecked(defaultCheckboxes);
        boxes.append(check);
        layout->addWidget(check);

    }

    layout->addWidget(btn_box, Qt::AlignCenter);

    dlg->setLayout(layout);

    if(dlg->exec() == QDialog::Accepted){
        for(auto box : boxes){
            if(!box->isChecked()){
                for(auto acc = accounts.begin(); acc != accounts.end(); acc++){
                    if(((*acc)->get_name() + " " + (*acc)->get_name_second()) == box->text()){
                        accounts.erase(acc);
                        break;
                    }
                }
            }
        }

        dlg->deleteLater();
        return accounts;
    }

    dlg->deleteLater();
    return QList<AccountItem* >();
}


void AccountItem::set_balance(double balance)
{
    auto str = std::to_string(balance);

    auto it = std::find(str.begin(), str.end(), ',');
    if(it != str.end())
        *it = '.';

    set_balance(QString::fromStdString(std::move(str)));
}

void AccountItem::set_balance(const QString &balance)
{
    data["balance"] = balance;
    label_balance->setText(get_balance_str());
    set_balanceWasUpdatedNow();
}

void AccountItem::set_balance(QString &&balance)
{
    data["balance"] = std::forward<QString>(balance);
    label_balance->setText(get_balance_str());
    set_balanceWasUpdatedNow();
}

void AccountItem::set_balanceWasUpdatedNow()
{
    balanceWasUpdatedTime->setDate(QDate::currentDate());
    balanceWasUpdatedTime->setTime(QTime::currentTime());
}
