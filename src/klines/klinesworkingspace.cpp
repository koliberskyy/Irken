#include "klinesworkingspace.h"

KlinesWorkingSpace::KlinesWorkingSpace(QWidget *parent)
    : QWidget{parent}, accounts{nullptr}, pos{new PositionItem()}
{
    ///***candlestickWidget

    candlestickWidget = new CandleStickWidget();

    ///***end of candlestickWidget




    ///***symbol timeframe group
    gb_symbolTF = new QGroupBox("График");
    gb_symbolTF_layout = new QVBoxLayout();

    //symbol combo box
    symbolComboBox = new QComboBox();
    for(auto symbol : symbol::utf8){
        symbolComboBox->addItem(symbol);
    }
    QObject::connect(symbolComboBox, &QComboBox::currentTextChanged, this, &KlinesWorkingSpace::graphicControlComboChanged);

    //timeframe combo box
    timeframeComboBox = new QComboBox();
    for(auto timeframe : symbol::timeframes){
        timeframeComboBox->addItem(timeframe);
    }
    QObject::connect(timeframeComboBox, &QComboBox::currentTextChanged, this, &KlinesWorkingSpace::graphicControlComboChanged);

    gb_symbolTF_layout->addWidget(symbolComboBox);
    gb_symbolTF_layout->addWidget(timeframeComboBox);

    gb_symbolTF->setLayout(gb_symbolTF_layout);

    ///***End of symbol timeframe group




    ///***smartmoney indicators group
    gb_smartmoney = new QGroupBox("smart money");
    gb_smartmoney_layout = new QVBoxLayout();

    //liquidity button
    liquidityButton = new QPushButton("Ликвидности");
    QObject::connect(liquidityButton, &QPushButton::pressed, candlestickWidget, &CandleStickWidget::autoDrawLiquidities);

    //imbalance button
    imbalanceButton = new QPushButton("Имбалансы");
    QObject::connect(imbalanceButton, &QPushButton::pressed, candlestickWidget, &CandleStickWidget::autoDrawImbalance);

    //orderblock button
    orderblockButton = new QPushButton("Ордер Блоки");
    QObject::connect(orderblockButton, &QPushButton::pressed, candlestickWidget, &CandleStickWidget::autoDrawOrderBlocks);

    gb_smartmoney_layout->addWidget(liquidityButton);
    gb_smartmoney_layout->addWidget(imbalanceButton);
    gb_smartmoney_layout->addWidget(orderblockButton);

    gb_smartmoney->setLayout(gb_smartmoney_layout);

    ///***End of smartmoney timeframe group





    ///***NSL indicators group
    gb_NSL = new QGroupBox("NSL");
    gb_NSL_layout = new QVBoxLayout();

    //NSL button
    NSLButton = new QPushButton("NSL");
    QObject::connect(NSLButton, &QPushButton::pressed, candlestickWidget, &CandleStickWidget::autoDrawNSL);

    //NSL RG button
    NSLRGButton = new QPushButton("NSL RG");
    QObject::connect(NSLRGButton, &QPushButton::pressed, candlestickWidget, &CandleStickWidget::autoDrawNSLRG);

    //NSL Liquids button
    NSLLiquids = new QPushButton("NSL Liquid");
    QObject::connect(NSLLiquids, &QPushButton::pressed, candlestickWidget, &CandleStickWidget::autoDrawNSLLiquds);

    gb_NSL_layout->addWidget(NSLButton);
    gb_NSL_layout->addWidget(NSLRGButton);
    gb_NSL_layout->addWidget(NSLLiquids);

    gb_NSL->setLayout(gb_NSL_layout);

    ///***end of NSL indicators group




    ///***leverage group
    gb_leverage = new QGroupBox("Плечо");
    gb_leverage_layout = new QVBoxLayout();

    //max leverage info
    maxLeverageLabel = new QLabel("Максимальное плечо");
    maxLeverageDSB = new QDoubleSpinBox();
    maxLeverageDSB->setReadOnly(true);
    maxLeverageDSB->setValue(instruments::maxLeverage(symbolComboBox->currentText().toUtf8()));

    //set leverage button
    setLeverageButton = new QPushButton("Установить плечо");
    QObject::connect(setLeverageButton, &QPushButton::pressed, this, &KlinesWorkingSpace::setLeverage);

    gb_leverage_layout->addWidget(maxLeverageLabel);
    gb_leverage_layout->addWidget(maxLeverageDSB);
    gb_leverage_layout->addWidget(setLeverageButton);

    gb_leverage->setLayout(gb_leverage_layout);

    ///*** end of leverage group




    ///***other group
    gb_other = new QGroupBox("Другие");
    gb_other_layout = new QVBoxLayout();

    //trading sessions button
    TradingSessionsButton = new QPushButton("Торговые сессии");
    QObject::connect(TradingSessionsButton, &QPushButton::pressed, candlestickWidget, &CandleStickWidget::autoDrawTradingSessions);

    //HLTS button
    HLTSButton = new QPushButton("HLTS");
    QObject::connect(HLTSButton, &QPushButton::pressed, candlestickWidget, &CandleStickWidget::autoDrawHLTS);

    gb_other_layout->addWidget(TradingSessionsButton);
    gb_other_layout->addWidget(HLTSButton);

    gb_other->setLayout(gb_other_layout);
    ///***end of other group




    //РАЗМЕТКА

    controlUnitVBL = new QHBoxLayout();
    controlUnitVBL->addWidget(gb_symbolTF);
    controlUnitVBL->addWidget(gb_smartmoney);
    controlUnitVBL->addWidget(gb_NSL);
    controlUnitVBL->addWidget(gb_leverage);
    controlUnitVBL->addWidget(gb_other);



    //main VBoxLayout
    mainVBL = new QVBoxLayout();
    mainVBL->addLayout(controlUnitVBL);
    mainVBL->addWidget(candlestickWidget);
    mainVBL->addWidget(pos);

    setLayout(mainVBL);
}

KlinesWorkingSpace::KlinesWorkingSpace(AccountKunteynir *accKunt, QWidget *parent):
    KlinesWorkingSpace()
{
    accounts = accKunt;
}

void KlinesWorkingSpace::graphicControlComboChanged()
{
    candlestickWidget->updateKlines(symbolComboBox->currentText(), timeframeComboBox->currentText());
    maxLeverageDSB->setValue(instruments::maxLeverage(symbolComboBox->currentText().toUtf8()));
}

void KlinesWorkingSpace::setLeverage()
{
    auto choosed = AccountItem::showAccountChooseDialog(accounts->list(), "Аккаунты на которых необходимо изменить Плечо");

    if(!choosed.isEmpty()){
        auto leverage = showLeverageChooseDialog();
        if(leverage > 0){
            auto pd = new QProgressDialog("Прогресс",  "Остановить", 0, choosed.size(), this);
            for (auto it : choosed){
                auto info = bybitInfo();
                pd->setLabelText(it->get_name() + " " + it->get_name_second());

                do{
                    info = Methods::setLeverage(symbolComboBox->currentText(), leverage,  it->get_api(), it->get_secret());
                    if(pd->wasCanceled()){
                        break;
                    }
                }while(info.retCode() != 0 && info.retCode() != 110043);
                pd->setValue(pd->value() + 1);
            }
            pd->deleteLater();
        }
    }
}

double KlinesWorkingSpace::showLeverageChooseDialog()
{
    QDialog dlg(this);
    auto form = new QFormLayout();


    auto btn_box = new QDialogButtonBox();
    btn_box->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);


    connect(btn_box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btn_box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    form->addRow(new QLabel("Выбор плеча"));

    //sb_qty
    auto sb_lev = new QSpinBox();
    sb_lev->setRange(1, instruments::maxLeverage(symbolComboBox->currentText().toUtf8()));
    sb_lev->setValue(100);
    sb_lev->setSingleStep(1);

    //sld_qty
    auto sld_lev = new QSlider(Qt::Horizontal);
    sld_lev->setRange(sb_lev->minimum(), sb_lev->maximum());
    sld_lev->setValue(sb_lev->value());
    sld_lev->setSingleStep(sb_lev->singleStep());
    QObject::connect(sld_lev, SIGNAL(valueChanged(int)), sb_lev, SLOT(setValue(int)));
    QObject::connect(sb_lev, SIGNAL(valueChanged(int)), sld_lev, SLOT(setValue(int)));

    form->addRow(sb_lev);
    form->addRow(sld_lev);
    form->addRow(btn_box);

    dlg.setLayout(form);

    // В случае, если пользователь нажал "Ok".
    if(dlg.exec() == QDialog::Accepted) {
        return sb_lev->value();
    }

    return -1;
}
