#include "spredcalculatorwgt.h"

SpredCalculatorWgt::SpredCalculatorWgt(QWidget *parent)
    : QWidget(parent),
      l_widgetName(new QLabel("Калькулятор спреда")),
      l_comission(new QLabel("Комиссия %")),
      l_buyPrice(new QLabel("Цена покупки")),
      l_sellPrice(new QLabel("Цена продажи")),
      l_spred(new QLabel("Spred %")),
      l_cashBefore(new QLabel("Фиат до")),
      l_cashAfter(new QLabel("После")),
      l_priceUSDT(new QLabel("USDT")),
      l_priceRUB(new QLabel("RUB")),
      l_USDTRUB_market(new QLabel("USDT/RUB рын.")),
      l_USDTRUB_real(new QLabel("USDT/RUB реал."))

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
    dsb_priceUSDT->setDecimals(8);
    dsb_priceUSDT->setReadOnly(true);
    //
    dsb_priceRUB = new QDoubleSpinBox();
    dsb_priceRUB->setMinimum(0.0);
    dsb_priceRUB->setMaximum(10'000'000);
    dsb_priceRUB->setDecimals(3);
    dsb_priceRUB->setReadOnly(true);

    // USDT-RUB
    dsb_priceUSDT_RUB_market = new QDoubleSpinBox();
    dsb_priceUSDT_RUB_market->setMinimum(0.0);
    dsb_priceUSDT_RUB_market->setMaximum(10'000'000);
    dsb_priceUSDT_RUB_market->setDecimals(2);
    dsb_priceUSDT_RUB_market->setValue(90.0);
    //
    dsb_priceUSDT_RUB_real = new QDoubleSpinBox();
    dsb_priceUSDT_RUB_real->setMinimum(0.0);
    dsb_priceUSDT_RUB_real->setMaximum(10'000'000);
    dsb_priceUSDT_RUB_real->setDecimals(2);
    dsb_priceUSDT_RUB_real->setValue(90.0);


    // FORM
    lform = new QFormLayout();
    lform->addRow(l_buyPrice, l_sellPrice);
    lform->addRow(dsb_buyPrice, dsb_sellPrice);
    lform->addRow(l_comission, l_spred);
    lform->addRow(dsb_comission, dsb_spred);
    rform = new QFormLayout();
    rform->addRow(l_cashBefore, l_cashAfter);
    rform->addRow(dsb_cashBefore, dsb_cashAfter);
    rform->addRow(l_priceUSDT, l_priceRUB);
    rform->addRow(dsb_priceUSDT, dsb_priceRUB);

    // OTHER LAYOUT
    auto lay_other = new QVBoxLayout();
    lay_other->addWidget(l_USDTRUB_market);
    lay_other->addWidget(dsb_priceUSDT_RUB_market);
    lay_other->addWidget(l_USDTRUB_real);
    lay_other->addWidget(dsb_priceUSDT_RUB_real);

   // rform->addRow(l_USDTRUB, dsb_priceUSDT_RUB);


    // MAIN LAYOUT
    vlay_main = new QVBoxLayout();
    hlay_main = new QHBoxLayout();

    hlay_main->addLayout(lform);
    hlay_main->addLayout(rform);
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

    QObject::connect(dsb_priceUSDT_RUB_real, &QDoubleSpinBox::valueChanged,
                     this, &SpredCalculatorWgt::lastPriceChanged);

    QObject::connect(dsb_priceUSDT, &QDoubleSpinBox::valueChanged,
                     this, &SpredCalculatorWgt::lastPriceChanged);


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
                           (dsb_priceUSDT_RUB_market->value() + dsb_priceUSDT_RUB_real->value()) / 2);
}

void SpredCalculatorWgt::updatePrice(double price)
{
    dsb_priceUSDT->setValue(price);
}

