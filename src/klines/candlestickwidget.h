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
#include <QCheckBox>
#include <QToolTip>
#include <set>
#include <unordered_map>
#include "requests.h"

struct AbstractLiquid{
    QLineSeries *series{nullptr};
    qreal count;
    qreal timestamp;
    qreal endtimestamp;
    virtual bool isFarther(AbstractLiquid*) = 0;
    void set(QLineSeries* s, qreal c, qreal t, qreal et){series = s; count = c; timestamp = t; endtimestamp = et;}
    void clear(){series = nullptr; count = 0; timestamp = 0; endtimestamp = 0;}
    bool operator < (const AbstractLiquid & other) const;
    bool operator == (const AbstractLiquid & other) const;
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
struct StopLoss : public AbstractLiquid{
    virtual bool isFarther(AbstractLiquid* other)override{ return false;}
};
struct CurrentPrice : public AbstractLiquid{
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

    static constexpr qreal stopStep{0.001};

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

class Klines{
public:
    /*!
    Возвращает массив свечей ВСЕХ!!!!!!!
    */
    static QJsonArray downloadKlines(const QString &symbol, const QString &interval, const QString &limit, const QString &begin = "", const QString &end = "");

    /*!
    принимает ОДНУ свечу!!!!!!!
    */
    static QCandlestickSet *toQCandlestickSetPtr(const QJsonArray &kline);
    static QCandlestickSet toQCandlestickSet(const QJsonArray &kline);


    static long long    time    (const QJsonArray &kline)   {return kline[0].toString().toLongLong(); }
    static double       open    (const QJsonArray &kline)   {return kline[1].toString().toDouble(); }
    static double       high    (const QJsonArray &kline)   {return kline[2].toString().toDouble(); }
    static double       low     (const QJsonArray &kline)   {return kline[3].toString().toDouble(); }
    static double       close   (const QJsonArray &kline)   {return kline[4].toString().toDouble(); }

};


// ********************************************* class CandleStickWidget ******************************************************
class CandleStickWidget : public QChartView
{
    Q_OBJECT
public:
    explicit CandleStickWidget(QWidget *parent = nullptr);
    QString get_currentSymbol() const;
signals:
    void currentPriceChanged(double price);
    void klineAdded(QString symbol, QString interval, QJsonArray klines);
public slots:
    void updateKlines(const QString &symbol, const QString &interval, const QString &limit = "1000");
    /*
    * SmartMoney indicator
    */
    void autoDrawLiquidities();
    /*
    * SmartMoney indicator
    */
    void autoDrawImbalance();
    /*
    * SmartMoney indicator
    */
    void autoDrawOrderBlocks();
    /*
    * Neutral/short/long indicator
    */
    void autoDrawNSL();
    /*
    * Neutral/short/long indicator
    */
    void autoDrawNSLRG();
    /*
    * Neutral/short/long indicator
    */
    void autoDrawNSLLiquds();
    /*
    * High Low Triange Strip indicator
    */
    void autoDrawHLTS();
    /*
    * Trading sessions indicator
    */
    void autoDrawTradingSessions();

    void updateCurrentChart();

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
    void showToolTip(const QPointF &point);

private:
    QChart *chart {nullptr};
    QPoint m_oPrePos;
    bool m_bLeftButtonPressed{false};
    bool ctrlButtonPressed{false};
    bool shiftButtonPressed{false};
    bool klinesUpdated{false};
    QCandlestickSeries *klinesSeries {nullptr};
    QList<QAreaSeries *>activeTradingSessions{nullptr};

    QString currentSymbol;
    QString currentTimeframe;
    std::unordered_map<QString, QList<HighLiquid>> hightsList;
    std::unordered_map<QString, QList<LowLiquid>> lowsList;
    std::unordered_map<QString, QList<AbstractArea>> areas;
    TakeProfit takeProfit;
    StopLoss stopLoss;
    CurrentPrice currentPrice;
    const AbstractArea* findArea(QAreaSeries *series);

    QAbstractAxis *axisX;
    QAbstractAxis *axisY;

    bool areaAddModeActivated{false};
    bool hightsAddModeActivated{false};
    bool lowsAddModeActivated{false};
    bool delModeActivated{false};
    bool rangeModeActivated{false};
    bool rangeModeFrirstClicked{false};
    QPointF rangeModeFirstClickMapToValue{0.0, 0.0};

    void deactivateAllMods(bool activate = false);
    bool isSomeModeActivated();

    void setKlines(const QString &symbol, const QString &interval, const QString &limit =  "1000");
    QAbstractAxis* setAxisX(QCandlestickSeries *klineSeries, QChart *chart);
    QAbstractAxis* setAxisY(QCandlestickSeries *klineSeries, QChart *chart);

    void addLow(qreal low, qreal beginTimeStamp, qreal endTimeStamp = -1);
    void addHigh(qreal high, qreal beginTimeStamp, qreal endTimeStamp = -1);
    void addArea(qreal high, qreal low, qreal beginTimeStamp, bool isBuyArea, qreal endTimeStamp = -1, const QColor &color = QColor(153, 0, 255), const QString &areaName = "");
    void addArea(AbstractArea area, const QColor &color = QColor(153, 0, 255), const QString &areaName = "");
    void addTakeProfit(qreal price, qreal beginTimeStamp);
    void addStopLoss(qreal price, qreal beginTimeStamp);
    void delLiquid(qreal liquid);

    void addExistSerieses();

    QList<std::pair<int, QCandlestickSet*>> NSL();
    QList<std::pair<int, QCandlestickSet*>> NSLRG(QList<std::pair<int, QCandlestickSet*>> NSL);
    std::pair<QList<HighLiquid>, QList<LowLiquid>> NSLLiquids(QList<std::pair<int, QCandlestickSet*>> NSLRG);

    QList<QAreaSeries*> drawTradingSession(int hourBegin, int hourEnd, QColor color);

    //risk ratio
    static double getRiskRatio(double poe, double sl, double tp);
    static double getStopLossPercent(double poe, double sl);
    static double leverageFromSLP(double slp); //SLP - stop loss percent

};

#endif // CANDLESTICKWIDGET_H
