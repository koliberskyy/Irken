#include "spredcalculatorwgt.h"

SpredCalculatorWgt::SpredCalculatorWgt(QWidget *parent)
    : QWidget(parent),
      l_widgetName(new QLabel("Калькулятор спреда")),
      l_comission(new QLabel("Комиссия/СПРЕД")),
      l_buyPrice(new QLabel("Покупка/Продажа")),
      l_cashBefore(new QLabel("Фиат до/после")),
      l_priceUSDT(new QLabel("USDT/RUB")),
      l_USDTRUB_market(new QLabel("USDT/RUB"))
{
    // BUY PRICE
    dsb_buyPrice = new QDoubleSpinBox();
    dsb_buyPrice->setMinimum(0.0);
    dsb_buyPrice->setMaximum(10'000'000);
    dsb_buyPrice->setSingleStep(0.001);
    dsb_buyPrice->setDecimals(3);

    // SELL PRICE
    dsb_sellPrice = new QDoubleSpinBox();
    dsb_sellPrice->setMinimum(0.0);
    dsb_sellPrice->setMaximum(10'000'000);
    dsb_sellPrice->setSingleStep(0.001);
    dsb_sellPrice->setDecimals(3);

    // COMISSION
    dsb_comission = new QDoubleSpinBox();
    dsb_comission->setMinimum(0.0);
    dsb_comission->setMaximum(100.0);
    dsb_comission->setSingleStep(0.1);
    dsb_comission->setDecimals(1);
    dsb_comission->setValue(0.9);

    // SPRED
    dsb_spred = new QDoubleSpinBox();
    dsb_spred->setMinimum(-1000.0);
    dsb_spred->setMaximum(1000.0);
    dsb_spred->setDecimals(2);
    dsb_spred->setReadOnly(true);

    // CASH BEFORE / AFTER
    dsb_cashAfter = new QDoubleSpinBox();
    dsb_cashAfter->setMinimum(0.0);
    dsb_cashAfter->setMaximum(10'000'000);
    dsb_cashAfter->setDecimals(2);
    dsb_cashAfter->setValue(10'000.0);
    dsb_cashAfter->setReadOnly(true);
    //
    dsb_cashBefore = new QDoubleSpinBox();
    dsb_cashBefore->setMinimum(0.0);
    dsb_cashBefore->setMaximum(10'000'000);
    dsb_cashBefore->setSingleStep(100);
    dsb_cashBefore->setDecimals(2);
    dsb_cashBefore->setValue(10'000.0);

    // PRICE USDT / RUB
    dsb_priceUSDT = new QDoubleSpinBox();
    dsb_priceUSDT->setMinimum(0.0);
    dsb_priceUSDT->setMaximum(10'000'000);
    dsb_priceUSDT->setDecimals(6);
    dsb_priceUSDT->setReadOnly(true);
    //
    dsb_priceRUB = new QDoubleSpinBox();
    dsb_priceRUB->setMinimum(0.0);
    dsb_priceRUB->setMaximum(10'000'000);
    dsb_priceRUB->setDecimals(3);
    dsb_priceRUB->setReadOnly(true);
    //big action
    dsb_priceRUB->setContextMenuPolicy(Qt::ActionsContextMenu);
    auto bigAction = new QAction("Show BIG");
    dsb_priceRUB->addAction(bigAction);
    QObject::connect(bigAction, &QAction::triggered,
                     this, &SpredCalculatorWgt::showBigPrice);

    // USDT-RUB
    dsb_priceUSDT_RUB_market = new QDoubleSpinBox();
    dsb_priceUSDT_RUB_market->setMinimum(0.0);
    dsb_priceUSDT_RUB_market->setMaximum(10'000'000);
    dsb_priceUSDT_RUB_market->setDecimals(2);
    dsb_priceUSDT_RUB_market->setValue(90.0);



    // SHOW BIG BUTTON
    showBigButton = new QPushButton("BIG");

    // SELL PRICE FOLLOW CHECKBOX
    sellPriceFollowCheck = new QCheckBox("Follow");






    // ***************** MAPPING ***************

    // OTHER LAYOUT
    auto lay_other = new QVBoxLayout();
    lay_other->addWidget(l_USDTRUB_market);
    lay_other->addWidget(dsb_priceUSDT_RUB_market);
    lay_other->addWidget(sellPriceFollowCheck);

    // buy sell price
    auto vlayBuySell = new QVBoxLayout();
    vlayBuySell->addWidget(l_buyPrice);
    vlayBuySell->addWidget(dsb_buyPrice);
    vlayBuySell->addWidget(dsb_sellPrice);

    // comission, spred
    auto vlayComissionSpred = new QVBoxLayout();
    vlayComissionSpred->addWidget(l_comission);
    vlayComissionSpred->addWidget(dsb_comission);
    vlayComissionSpred->addWidget(dsb_spred );

    // PRICE USDT/RUB
    auto vlaypPrices = new QVBoxLayout();
    vlaypPrices->addWidget(l_priceUSDT);
    vlaypPrices->addWidget(dsb_priceUSDT);
    vlaypPrices->addWidget(dsb_priceRUB);

    // fiat
    auto vlayFiat= new QVBoxLayout();
    vlayFiat->addWidget(l_cashBefore);
    vlayFiat->addWidget(dsb_cashBefore);
    vlayFiat->addWidget(dsb_cashAfter);


    // MAIN LAYOUT
    vlay_main = new QVBoxLayout();
    hlay_main = new QHBoxLayout();

    hlay_main->addLayout(vlayBuySell);
    hlay_main->addLayout(vlayFiat);
    hlay_main->addLayout(vlayComissionSpred);
    hlay_main->addLayout(vlaypPrices);

    hlay_main->addLayout(lay_other);

    vlay_main->addWidget(l_widgetName);
    vlay_main->addLayout(hlay_main);

    this->setLayout(vlay_main);


    // CONNECTIONS
    QObject::connect(dsb_buyPrice, &QDoubleSpinBox::valueChanged,
                     this, &SpredCalculatorWgt::priceChanged);

    QObject::connect(dsb_sellPrice, &QDoubleSpinBox::valueChanged,
                     this, &SpredCalculatorWgt::priceChanged);

    QObject::connect(dsb_cashBefore, &QDoubleSpinBox::valueChanged,
                     this, &SpredCalculatorWgt::priceChanged);

    QObject::connect(dsb_comission, &QDoubleSpinBox::valueChanged,
                     this, &SpredCalculatorWgt::priceChanged);

    QObject::connect(dsb_priceUSDT_RUB_market, &QDoubleSpinBox::valueChanged,
                     this, &SpredCalculatorWgt::lastPriceChanged);

    QObject::connect(dsb_priceUSDT, &QDoubleSpinBox::valueChanged,
                     this, &SpredCalculatorWgt::lastPriceChanged);

    QObject::connect(sellPriceFollowCheck, SIGNAL(toggled(bool)),
                     this, SLOT(sellPriceFollowCheckToggled(bool)));


}


void SpredCalculatorWgt::priceChanged()
{
    if(dsb_buyPrice->value() > 0.0 && dsb_sellPrice->value() > 0.0){
        auto inverceComission = 1 - dsb_comission->value() / 100;
        auto before{dsb_cashBefore->value()};
        auto after{before / dsb_buyPrice->value() * inverceComission * dsb_sellPrice->value()};
        auto spred{100 * (after - before) / before};
        dsb_spred->setValue(spred);
        dsb_cashAfter->setValue(after);

    }
}

void SpredCalculatorWgt::lastPriceChanged()
{
    dsb_priceRUB->setValue(dsb_priceUSDT->value() *
                           (dsb_priceUSDT_RUB_market->value()));
}

void SpredCalculatorWgt::sellPriceFollowCheckToggled(bool check)
{
    if(check){
        QObject::connect(dsb_priceRUB, SIGNAL(valueChanged(double)),
                         dsb_sellPrice, SLOT(setValue(double)));
    }
    else {
        QObject::disconnect(dsb_priceRUB, SIGNAL(valueChanged(double)),
                         dsb_sellPrice, SLOT(setValue(double)));
    }
}

void SpredCalculatorWgt::updatePrice(double price)
{
    dsb_priceUSDT->setValue(price);
}

void SpredCalculatorWgt::showBigPrice()
{
    auto lcd = new QLCDNumber();
    lcd->setWindowFlag(Qt::WindowStaysOnTopHint);
    lcd->display(dsb_priceRUB->value());
    lcd->setDigitCount(16);

    QObject::connect(dsb_priceRUB, SIGNAL(valueChanged(double)),
                     lcd, SLOT(display(double)));

    lcd->setMinimumSize(800, 200);
    lcd->show();
}
