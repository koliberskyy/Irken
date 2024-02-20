#ifndef CANDLESTICKWIDGET_H
#define CANDLESTICKWIDGET_H

#include <QWidget>
#include <QCandlestickSeries>
#include <QCandlestickSet>
#include <QChart>
#include <QChartView>
#include <QBarCategoryAxis>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QLineSeries>
#include "smartmoney.h"

class CandleStickWidget : public QChartView
{
    Q_OBJECT
public:
    explicit CandleStickWidget(QWidget *parent = nullptr);

protected:

    virtual void mouseMoveEvent(QMouseEvent *pEvent) override;
    virtual void mousePressEvent(QMouseEvent *pEvent) override;
    virtual void mouseReleaseEvent(QMouseEvent *pEvent) override;
    virtual void wheelEvent(QWheelEvent *pEvent) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
private:

    QChart *chart {nullptr};
    QPoint m_oPrePos;
    bool m_bLeftButtonPressed{false};
    bool ctrlButtonPressed{false};
    QCandlestickSeries *klinesSeries {nullptr};
    QAbstractAxis *axisX;
    QAbstractAxis *axisY;

    void setKlines(const QString &symbol, const QString &interval, const QString &limit =  "1000");
    void setLiquidities(QAbstractAxis *axisX, QAbstractAxis *axisY);
    QAbstractAxis* setAxisX(QCandlestickSeries *klineSeries, QChart *chart);
    QAbstractAxis* setAxisY(QCandlestickSeries *klineSeries, QChart *chart);




};

#endif // CANDLESTICKWIDGET_H
