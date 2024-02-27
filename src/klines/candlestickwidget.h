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
#include <QAreaSeries>
#include <QLine>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QDialog>
#include <QLabel>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QToolTip>
#include <unordered_map>
#include "smartmoney.h"

struct AbstractLiquid{
    QLineSeries *series{nullptr};
    qreal count;
    qreal timestamp;
    qreal endtimestamp;
    virtual bool isFarther(AbstractLiquid*) = 0;
    void set(QLineSeries* s, qreal c, qreal t, qreal et){series = s; count = c; timestamp = t; endtimestamp = et;}
    void clear(){series = nullptr; count = 0; timestamp = 0; endtimestamp = 0;}
};
struct HighLiquid : public AbstractLiquid{
    virtual bool isFarther(AbstractLiquid* other)override{ return count > other->count;}
};
struct LowLiquid : public AbstractLiquid{
    virtual bool isFarther(AbstractLiquid* other)override{ return count < other->count;}
};
struct TakeProfit : public AbstractLiquid{
    virtual bool isFarther(AbstractLiquid* other)override{ return false;}
};
struct AbstractArea{
    QAreaSeries *series;
    QLineSeries *stopLine;
    QLineSeries *_05Series;
    QLineSeries *_03Series;
    QLineSeries *_07Series;

    QString type;
    qreal high;
    qreal low;
    qreal timestamp;
    qreal endtimestamp;
    bool isBuyArea;

    static constexpr qreal stopStep{0.005};

    qreal particional(qreal count)const{return (high-low)*count + low;}
    qreal _07()const{return particional(0.7); }
    qreal _05()const{return particional(0.5);}
    qreal _03()const{return particional(0.3);}

    qreal priceFromRange(qreal price, qreal range)const{return price * range;}

    qreal _stop()const{if(isBuyArea) return priceFromRange(low, 1 - stopStep); else return priceFromRange(high, 1+stopStep);}
    void attachAxis(QAbstractAxis* axisX, QAbstractAxis* axisY){
        series->attachAxis(axisX);
        series->attachAxis(axisY);

        stopLine->attachAxis(axisX);
        stopLine->attachAxis(axisY);

        _05Series->attachAxis(axisX);
        _05Series->attachAxis(axisY);

        _03Series->attachAxis(axisX);
        _03Series->attachAxis(axisY);

        _07Series->attachAxis(axisX);
        _07Series->attachAxis(axisY);
    }
    void addToChart(QChart *chart){
        chart->addSeries(series);
        chart->addSeries(_07Series);
        chart->addSeries(_05Series);
        chart->addSeries(_03Series);
        chart->addSeries(stopLine);
    }
    void removeFromChart(QChart *chart){
        chart->removeSeries(series);
        chart->removeSeries(_07Series);
        chart->removeSeries(_05Series);
        chart->removeSeries(_03Series);
        chart->removeSeries(stopLine);    }
};

class CandleStickWidget : public QChartView
{
    Q_OBJECT
public:
    explicit CandleStickWidget(QWidget *parent = nullptr);
signals:
    void addOrderClicked(QJsonObject order);
public slots:
    void updateKlines(const QString &symbol, const QString &interval, const QString &limit = "1000");
    void autoDrawLiquidities();
    void autoDrawAreas();

protected:

    virtual void mouseMoveEvent(QMouseEvent *pEvent) override;
    virtual void mousePressEvent(QMouseEvent *pEvent) override;
    virtual void mouseReleaseEvent(QMouseEvent *pEvent) override;
    virtual void wheelEvent(QWheelEvent *pEvent) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:
    void klineClicked(QCandlestickSet *set);
    void areaClicked();
    void areaDoubleClicked();

private:
    QChart *chart {nullptr};
    QPoint m_oPrePos;
    bool m_bLeftButtonPressed{false};
    bool ctrlButtonPressed{false};
    QCandlestickSeries *klinesSeries {nullptr};

    QString currentSymbol;
    QString currentTimeframe;
    std::unordered_map<QString, QList<HighLiquid>> hightsList;
    std::unordered_map<QString, QList<LowLiquid>> lowsList;
    std::unordered_map<QString, QList<AbstractArea>> areas;
    TakeProfit takeProfit;
    const AbstractArea* findArea(QAreaSeries *series);

    QAbstractAxis *axisX;
    QAbstractAxis *axisY;

    bool areaAddModeActivated{false};
    bool hightsAddModeActivated{false};
    bool lowsAddModeActivated{false};
    bool delModeActivated{false};
    void deactivateAllMods(bool activate = false);
    bool isSomeModeActivated();

    void setKlines(const QString &symbol, const QString &interval, const QString &limit =  "1000");
    void setLiquidities(QAbstractAxis *axisX, QAbstractAxis *axisY);
    QAbstractAxis* setAxisX(QCandlestickSeries *klineSeries, QChart *chart);
    QAbstractAxis* setAxisY(QCandlestickSeries *klineSeries, QChart *chart);

    void addLow(qreal low, qreal beginTimeStamp, qreal endTimeStamp = -1);
    void addHigh(qreal high, qreal beginTimeStamp, qreal endTimeStamp = -1);
    void addArea(qreal high, qreal low, qreal beginTimeStamp, bool isBuyArea, qreal endTimeStamp = -1);
    void addTakeProfit(qreal price, qreal beginTimeStamp);
    void delLiquid(qreal liquid);

    void addExistSerieses();


};

#endif // CANDLESTICKWIDGET_H
