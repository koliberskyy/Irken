#include "positionitem.h"

PositionItem::PositionItem(QWidget *parent):
    AbstractItem(parent)

{
    //symbol, side
    label_symbol = new QLabel();
    label_side = new QLabel();
    auto vbl_ss = new QVBoxLayout();
    vbl_ss->addWidget(label_symbol);
    vbl_ss->addWidget(label_side);

    //pnl, stopBU, close
    label_pnl = new QLabel();
    pb_stop_bu = new QPushButton("Стоп б/у");
    pb_close_position = new QPushButton("Закрыть");

    auto grid_pnl = new QGridLayout();
    grid_pnl->addWidget(new QLabel("pnl"), 0, 0);
    grid_pnl->addWidget(label_pnl, 0, 1);
    grid_pnl->addWidget(pb_stop_bu, 1, 0);
    grid_pnl->addWidget(pb_close_position, 1, 1);
    connect(pb_close_position, &QPushButton::pressed, this, &PositionItem::buttonClosePressed);
    connect(pb_stop_bu, &QPushButton::pressed, this, &PositionItem::buttonStopBUPressed);

    //poe, mark
    label_poe = new QLabel();
    label_markPrice = new QLabel();
    auto grid_pm = new QGridLayout();
    grid_pm->addWidget(new QLabel("ТВХ"), 0, 0);
    grid_pm->addWidget(label_poe, 0, 1);
    grid_pm->addWidget(new QLabel("Марк."), 1, 0);
    grid_pm->addWidget(label_markPrice, 1, 1);

    //tp,sl
    dsb_tp = new QDoubleSpinBox();
    dsb_sl = new QDoubleSpinBox();
    dsb_sl->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
    dsb_tp->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
    //dsb_sl->setReadOnly(true);
    //dsb_tp->setReadOnly(true);
    auto grid_st = new QGridLayout();
    grid_st->addWidget(new QLabel("ТП"), 0, 0);
    grid_st->addWidget(dsb_tp, 0, 1);
    grid_st->addWidget(new QLabel("СЛ"), 1, 0);
    grid_st->addWidget(dsb_sl, 1, 1);
    auto pb_changeTpSl = new QPushButton("Изм.");
    grid_st->addWidget(pb_changeTpSl, 0, 2, 2, 1);
    connect(pb_changeTpSl, &QPushButton::pressed, this, &PositionItem::setTpSL);


    //leverage, size
    label_size = new QLabel();
    label_leverage = new QLabel();
    auto grid_ls = new QGridLayout();
    grid_ls->addWidget(new QLabel("Плечо"), 0, 0);
    grid_ls->addWidget(label_leverage, 0, 1);
    grid_ls->addWidget(new QLabel("Разм."), 1, 0);
    grid_ls->addWidget(label_size, 1, 1);

    //accounts
    pb_accounts = new QPushButton("Аккаунты");
    connect(pb_accounts, &QPushButton::pressed, this, &PositionItem::buttonAccountsPressed);
    auto vbl_ac = new QVBoxLayout();
    sb_ownersCount = new QSpinBox();
    sb_ownersCount->setValue(0);
    sb_ownersCount->setReadOnly(true);
    sb_ownersCount->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
    vbl_ac->addWidget(sb_ownersCount);
    vbl_ac->addWidget(pb_accounts);

    //main layout
    layout_main->addLayout(vbl_ss);
    layout_main->addLayout(grid_pnl);
    layout_main->addLayout(grid_pm);
    layout_main->addLayout(grid_st);
    layout_main->addLayout(grid_ls);
    layout_main->addLayout(vbl_ac);

}

void PositionItem::updateData(const QJsonObject &obj, AccountItem *owner)
{
    updateData(obj);
    owner_data[owner] = obj;

}

void PositionItem::updateData(PositionItem *item)
{
    updateData(item->get_data());
    acc_owners = item->get_owners();
    owner_data = item->get_owners_data();
}

void PositionItem::updateData(const QJsonObject &obj)
{
    if(!obj.isEmpty())
        data = obj;

    label_symbol->setText(get_symbol());
    label_side->setText(get_side());

    label_pnl->setText(get_pnl());

    label_poe->setText(get_poe());
    label_markPrice->setText(get_mark());

    dsb_tp->setRange(instruments::minPrice(get_symbol()), instruments::maxPrice(get_symbol()));
    dsb_tp->setSingleStep(instruments::stepPrice(get_symbol()));
    dsb_tp->setDecimals(instruments::dap(get_symbol()));
    dsb_tp->setValue(get_tp().toDouble());

    dsb_sl->setRange(instruments::minPrice(get_symbol()), instruments::maxPrice(get_symbol()));
    dsb_sl->setSingleStep(instruments::stepPrice(get_symbol()));
    dsb_sl->setDecimals(instruments::dap(get_symbol()));
    dsb_sl->setValue(get_sl().toDouble());

    label_leverage->setText(get_leverage());
    label_size->setText(get_size());
}

void PositionItem::addOwner(AccountItem *owner, const QJsonObject &obj)
{
    if(owner != nullptr){
        if(!obj.isEmpty()){
            owner_data[owner] = obj;
        }

        auto addTrigger{true};
        for(auto it : acc_owners){
            //сравниваю именно указатели, т.к. буду использовать одни и те же
            if(it == owner){
                addTrigger = false;
                break;
            }
        }

        if(addTrigger){
            acc_owners.append(owner);
            sb_ownersCount->setValue(acc_owners.size());
        }
    }
}

bool PositionItem::removeOwner(AccountItem *owner)
{
    if(owner != nullptr){
        auto trigger{false};
        for(auto it = acc_owners.begin(); it !=  acc_owners.end(); it++){
            //сравниваю именно указатели, т.к. буду использовать одни и те же
            auto item = *it;
            if(item == owner){
                acc_owners.erase(it);
                trigger = true;
                break;
            }
        }

        owner_data.remove(owner);

        if(trigger){
            sb_ownersCount->setValue(acc_owners.size());
        }
        return trigger;
    }
    return false;
}

int PositionItem::ownersSize() const
{
    return acc_owners.size();
}

int PositionItem::itemHeight()
{
    return 60;
}

QString PositionItem::get_symbol() const
{
    if(data.isEmpty())
        return "";

    return data["symbol"].toString();
}

QString PositionItem::get_side() const
{
    if(data.isEmpty())
        return "";

    return data["side"].toString();
}

QString PositionItem::get_poe(AccountItem *owner) const
{
    if(owner != nullptr){
        return owner_data[owner]["avgPrice"].toString();
    }

    if(data.isEmpty())
        return "";

    return data["avgPrice"].toString();
}

QString PositionItem::get_tp(AccountItem *owner) const
{
    if(owner != nullptr){
        return owner_data[owner]["takeProfit"].toString();
    }

    if(data.isEmpty())
        return "";

    return data["takeProfit"].toString();
}

QString PositionItem::get_sl(AccountItem *owner) const
{
    if(owner != nullptr){
        return owner_data[owner]["stopLoss"].toString();
    }

    if(data.isEmpty())
        return "";

    return data["stopLoss"].toString();
}

QString PositionItem::get_mark() const
{
    if(data.isEmpty())
        return "";

    return data["markPrice"].toString();
}

QString PositionItem::get_pnl() const
{
    if(data.isEmpty())
        return "";

    auto avg = get_poe().toDouble();
    auto mark = get_mark().toDouble();
    auto leverage = get_leverage().toDouble();
    auto side = get_side();

    double reply;

    if(side == "Buy"){
        reply = 100 * leverage * (mark-avg) / avg ;
    }
    else{
        reply = 100 * leverage * (avg-mark) / mark;
    }

    return QString::fromStdString(std::to_string(reply));

}

QString PositionItem::get_leverage(AccountItem *owner) const
{
    if(owner != nullptr){
        return owner_data[owner]["leverage"].toString();
    }

    if(data.isEmpty())
        return "";

    return data["leverage"].toString();
}

QString PositionItem::get_size(AccountItem *owner) const
{
    if(owner != nullptr){
        return owner_data[owner]["size"].toString();
    }

    if(data.isEmpty())
        return "";

    return data["size"].toString();
}

QList<AccountItem *> PositionItem::get_owners() const
{
    return acc_owners;
}

QMap<AccountItem *, QJsonObject> PositionItem::get_owners_data() const
{
    return owner_data;
}

void PositionItem::buttonAccountsPressed()
{
    AccountItem::showAccountChooseDialog(acc_owners, "Аккаунты на которых эта позциция есть");
}

void PositionItem::buttonClosePressed()
{
    auto choosed = AccountItem::showAccountChooseDialog(acc_owners, "Закрытие позиции");
    if(!choosed.isEmpty()){
        auto reduce_percent = showRedusePercentDialog();
        if(reduce_percent > 0 ){
            //order prepare
            QJsonObject order;
            order.insert("category", "linear");
            order.insert("symbol", get_symbol());
            //side
            auto side = [this]()->QString{if(get_side() == "Buy")return "Sell";else return "Buy";}();
            order.insert("side", side);
            order.insert("orderType", "Market");
            order.insert("reduceOnly", true);

            auto pd = new QProgressDialog("Прогресс",  "Остановить", 0, choosed.size(), this);
            pd->show();

            for (auto it : choosed){
                pd->setLabelText(it->get_name() + " " + it->get_name_second());
                auto info = bybitInfo();

                //qty
                if(reduce_percent >= 100)
                    order["qty"] = get_size(it);
                else
                    order["qty"] =  QString(instruments::double_to_utf8(get_symbol().toUtf8(), instruments::Filter_type::lotSize_without_leverage_multipy, get_size(it).toDouble() * reduce_percent / 100));

                if(order["qty"].toString().toDouble() > 0)
                    do{
                        info = Methods::placeOrder(order, it->get_api(), it->get_secret());
                    }while(info.retCode() != 0);

                pd->setValue(pd->value() + 1);
            }
            pd->close();
            pd->deleteLater();

        }
    }
}

void PositionItem::buttonStopBUPressed()
{
    auto choosed = AccountItem::showAccountChooseDialog(acc_owners, "Аккаунты на которых необходимо изменить ТП/СЛ");

    if(!choosed.isEmpty()){
        auto pd = new QProgressDialog("Прогресс",  "Остановить", 0, choosed.size(), this);
        pd->show();

            for (auto it : choosed){
                auto info = bybitInfo();
                pd->setLabelText(it->get_name() + " " + it->get_name_second());

                auto sl = get_poe(it).toDouble();

                if(sl > 0)
                    do{
                       info = Methods::setTradingStop(get_symbol(), it->get_api(), it->get_secret(),
                                                sl, get_tp(it).toDouble());
                    }while(info.retCode() != 0 && info.retCode() != 34040);
                pd->setValue(pd->value() + 1);
            }

        pd->close();
        pd->deleteLater();
    }
}

void PositionItem::setTpSL()
{
    auto choosed = AccountItem::showAccountChooseDialog(acc_owners, "Аккаунты на которых необходимо изменить ТП/СЛ");

    if(!choosed.isEmpty()){
        auto sltp = showSlTpChooseDialog();

        if(sltp.first > 0 && sltp.second > 0){
            auto pd = new QProgressDialog("Прогресс",  "Остановить", 0, choosed.size(), this);
            pd->show();

            for (auto it : choosed){
                auto info = bybitInfo();
                pd->setLabelText(it->get_name() + " " + it->get_name_second());

                do{
                   info = Methods::setTradingStop(get_symbol(), it->get_api(), it->get_secret(),
                                            sltp.first, sltp.second);
                }while(info.retCode() != 0);
                pd->setValue(pd->value() + 1);
            }
            pd->close();
            pd->deleteLater();
        }
    }
}

void PositionItem::clear()
{
    data = QJsonObject();
    updateData(QJsonObject());
}

std::pair<double, double> PositionItem::showSlTpChooseDialog()
{
    QDialog dlg(this);

    auto layout = new QVBoxLayout();
    auto label = new QLabel();

    label->setText("Изменить ТП/СЛ");

    auto btn_box = new QDialogButtonBox();
    btn_box->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    auto tp = new QDoubleSpinBox();
    auto sl = new QDoubleSpinBox();

    auto grid_st = new QGridLayout();
    grid_st->addWidget(new QLabel("ТП"), 0, 0);
    grid_st->addWidget(tp, 0, 1);
    grid_st->addWidget(new QLabel("СЛ"), 1, 0);
    grid_st->addWidget(sl, 1, 1);

    tp->setRange(instruments::minPrice(get_symbol()), instruments::maxPrice(get_symbol()));
    tp->setSingleStep(instruments::stepPrice(get_symbol()));
    tp->setDecimals(instruments::dap(get_symbol()));
    tp->setValue(get_tp().toDouble());

    sl->setRange(instruments::minPrice(get_symbol()), instruments::maxPrice(get_symbol()));
    sl->setSingleStep(instruments::stepPrice(get_symbol()));
    sl->setDecimals(instruments::dap(get_symbol()));
    sl->setValue(get_sl().toDouble());

    connect(btn_box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btn_box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    layout->addWidget(label);
    layout->addLayout(grid_st);
    layout->addWidget(btn_box, Qt::AlignCenter);

    dlg.setLayout(layout);

    if(dlg.exec() == QDialog::Accepted){
        auto stop = sl->value();
        auto take = tp->value();

        dlg.deleteLater();
        return std::pair(stop, take);
    }

    dlg.deleteLater();
    return std::pair(-1, -1);
}

double PositionItem::showRedusePercentDialog()
{
    QDialog dlg(this);
    auto form = new QFormLayout();


    auto btn_box = new QDialogButtonBox();
    btn_box->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);


    connect(btn_box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btn_box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    form->addRow(new QLabel("Закрыть позицию?"));

    //sb_qty
    auto sb_qty = new QSpinBox();
    sb_qty->setRange(0, 100);
    sb_qty->setValue(100);
    sb_qty->setSingleStep(25);

    //sld_qty
    auto sld_qty = new QSlider(Qt::Horizontal);
    sld_qty->setRange(sb_qty->minimum(), sb_qty->maximum());
    sld_qty->setValue(sb_qty->value());
    sld_qty->setSingleStep(sb_qty->singleStep());
    QObject::connect(sld_qty, SIGNAL(valueChanged(int)), sb_qty, SLOT(setValue(int)));
    QObject::connect(sb_qty, SIGNAL(valueChanged(int)), sld_qty, SLOT(setValue(int)));

    form->addRow(new QLabel("%"), sb_qty);
    form->addRow(sld_qty);
    form->addRow(btn_box);

    dlg.setLayout(form);

    // В случае, если пользователь нажал "Ok".
    if(dlg.exec() == QDialog::Accepted) {
        return sb_qty->value();
    }

    return -1;
}
