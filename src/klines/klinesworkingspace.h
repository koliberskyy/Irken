#ifndef KLINESWORKINGSPACE_H
#define KLINESWORKINGSPACE_H

#include <QWidget>
#include <QGroupBox>
#include "candlestickwidget.h"
#include "spredcalculatorwgt.h"
#include "telegramalert.h"

class KlinesWorkingSpace : public QWidget
{
    Q_OBJECT
public:
    explicit KlinesWorkingSpace(QWidget *parent = nullptr);
    CandleStickWidget *candlestickWidget;

private slots:
    void graphicControlComboChanged();

private:
    QComboBox *symbolComboBox;
    QComboBox *timeframeComboBox;

    QPushButton *liquidityButton;
    QPushButton *imbalanceButton;
    QPushButton *orderblockButton;
    QPushButton *NSLLiquids;
    QPushButton *HLTSButton;
    QPushButton *TradingSessionsButton;

    QVBoxLayout *mainVBL;

    QHBoxLayout *controlUnitVBL;
    QGroupBox *gb_symbolTF;
    QVBoxLayout *gb_symbolTF_layout;
    QGroupBox *gb_smartmoney;
    QVBoxLayout *gb_smartmoney_layout;
    QGroupBox *gb_other;
    QVBoxLayout *gb_other_layout;

    SpredCalculatorWgt *spredWgt;
    TelegramAlert *tgAlert;
};

#endif // KLINESWORKINGSPACE_H
