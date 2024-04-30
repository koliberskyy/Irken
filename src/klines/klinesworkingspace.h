#ifndef KLINESWORKINGSPACE_H
#define KLINESWORKINGSPACE_H

#include <QWidget>
#include <QGroupBox>
#include "positionkunteynir.h"
#include "candlestickwidget.h"
#include "accountkunteynir.h"

class KlinesWorkingSpace : public QWidget
{
    Q_OBJECT
public:
    explicit KlinesWorkingSpace(QWidget *parent = nullptr);
    explicit KlinesWorkingSpace(AccountKunteynir *accKunt, QWidget *parent = nullptr);
    CandleStickWidget * candlestickWidget;

private slots:
    void graphicControlComboChanged();
    void setLeverage();

private:
    QComboBox *symbolComboBox;
    QComboBox *timeframeComboBox;
    QLabel *maxLeverageLabel;
    QDoubleSpinBox *maxLeverageDSB;

    QPushButton *liquidityButton;
    QPushButton *imbalanceButton;
    QPushButton *orderblockButton;
    QPushButton *setLeverageButton;
    QPushButton *NSLButton;
    QPushButton *NSLRGButton;
    QPushButton *NSLLiquids;
    QPushButton *HLTSButton;
    QPushButton *TradingSessionsButton;


    QVBoxLayout *mainVBL;

    QHBoxLayout *controlUnitVBL;
    QGroupBox *gb_symbolTF;
    QVBoxLayout *gb_symbolTF_layout;
    QGroupBox *gb_smartmoney;
    QVBoxLayout *gb_smartmoney_layout;
    QGroupBox *gb_NSL;
    QVBoxLayout *gb_NSL_layout;
    QGroupBox *gb_leverage;
    QVBoxLayout *gb_leverage_layout;
    QGroupBox *gb_other;
    QVBoxLayout *gb_other_layout;

    AccountKunteynir *accounts;


    double showLeverageChooseDialog();

};

#endif // KLINESWORKINGSPACE_H
