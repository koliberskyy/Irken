#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLCDNumber>
#include <QCheckBox>
#include <QAction>

class SpredCalculatorWgt : public QWidget
{
    Q_OBJECT

    QLabel *l_widgetName;
    QLabel *l_buyPrice;
    QLabel *l_comission;
    QLabel *l_cashBefore;
    QLabel *l_priceUSDT;
    QLabel *l_USDTRUB_market;

    QDoubleSpinBox *dsb_buyPrice;
    QDoubleSpinBox *dsb_sellPrice;
    QDoubleSpinBox *dsb_comission;
    QDoubleSpinBox *dsb_spred;
    QDoubleSpinBox *dsb_cashBefore;
    QDoubleSpinBox *dsb_cashAfter;
    QDoubleSpinBox *dsb_priceUSDT;
    QDoubleSpinBox *dsb_priceRUB;
    QDoubleSpinBox *dsb_priceUSDT_RUB_market;

    QVBoxLayout *vlay_main;
    QHBoxLayout *hlay_main;
    QFormLayout *lform;
    QFormLayout *rform;

    QPushButton *showBigButton;
    QCheckBox *sellPriceFollowCheck;

public:
    SpredCalculatorWgt(QWidget *parent = nullptr);
    ~SpredCalculatorWgt() = default;

private slots:
    void priceChanged();
    void lastPriceChanged();
    void sellPriceFollowCheckToggled(bool check);
public slots:
    void updatePrice(double price);
    void showBigPrice();

};
#endif // WIDGET_H
