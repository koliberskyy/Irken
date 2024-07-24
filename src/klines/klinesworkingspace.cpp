#include "klinesworkingspace.h"

KlinesWorkingSpace::KlinesWorkingSpace(QWidget *parent)
    : QWidget(parent)
{
    ///***candlestickWidget

    candlestickWidget = new CandleStickWidget();

    ///***end of candlestickWidget




    ///***symbol timeframe group
    gb_symbolTF = new QGroupBox("График");
    gb_symbolTF_layout = new QVBoxLayout();

    //symbol combo box
    symbolComboBox = new QComboBox();
    for(auto &symbol : symbol::utf8){
        symbolComboBox->addItem(symbol);
    }
    QObject::connect(symbolComboBox, &QComboBox::currentTextChanged, this, &KlinesWorkingSpace::graphicControlComboChanged);

    //timeframe combo box
    timeframeComboBox = new QComboBox();
    for(auto &timeframe : symbol::timeframes){
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




    ///***other group
    gb_other = new QGroupBox("Другие");
    gb_other_layout = new QVBoxLayout();

    //trading sessions button
    TradingSessionsButton = new QPushButton("Торговые сессии");
    QObject::connect(TradingSessionsButton, &QPushButton::pressed, candlestickWidget, &CandleStickWidget::autoDrawTradingSessions);

    //HLTS button
    HLTSButton = new QPushButton("HLTS");
    QObject::connect(HLTSButton, &QPushButton::pressed, candlestickWidget, &CandleStickWidget::autoDrawHLTS);

    //NSL Liquids button
    NSLLiquids = new QPushButton("NSL Liquid");
    QObject::connect(NSLLiquids, &QPushButton::pressed, candlestickWidget, &CandleStickWidget::autoDrawNSLLiquds);

    gb_other_layout->addWidget(TradingSessionsButton);
    gb_other_layout->addWidget(HLTSButton);
    gb_other_layout->addWidget(NSLLiquids);


    gb_other->setLayout(gb_other_layout);
    ///***end of other group

    // SPRED WGT
    spredWgt = new SpredCalculatorWgt();
    QObject::connect(candlestickWidget, &CandleStickWidget::currentPriceChanged, spredWgt, &SpredCalculatorWgt::updatePrice);
    // EOF SPRED WGT


    // TG ALERT WGT
    tgAlert = new TelegramAlert();
    QObject::connect(candlestickWidget, SIGNAL(klineAdded(QString,QString,QJsonArray)),
                     tgAlert, SLOT(updateKlines(QString,QString,QJsonArray)));
    // EOF TG ALERT WGT

    //РАЗМЕТКА
    controlUnitVBL = new QHBoxLayout();
    controlUnitVBL->addWidget(gb_symbolTF);
    controlUnitVBL->addWidget(gb_smartmoney);
    controlUnitVBL->addWidget(gb_other);
    controlUnitVBL->addWidget(tgAlert);
    controlUnitVBL->addWidget(spredWgt);


    //main VBoxLayout
    mainVBL = new QVBoxLayout();
    mainVBL->addLayout(controlUnitVBL);
    mainVBL->addWidget(candlestickWidget);

    setLayout(mainVBL);
}

void KlinesWorkingSpace::graphicControlComboChanged()
{
    candlestickWidget->updateKlines(symbolComboBox->currentText(), timeframeComboBox->currentText());
}
