#include "candlestickwidget.h"

CandleStickWidget::CandleStickWidget(QWidget *parent)
    : QChartView{parent}
{
    setKlines("BTCUSDT", "D", "1000");
    setKlines("ETHUSDT", "120");
}

void CandleStickWidget::mouseMoveEvent(QMouseEvent *pEvent)
{

    if(m_bLeftButtonPressed){
        auto oDeltaPos = pEvent->pos() - m_oPrePos;
        this->chart->scroll(-oDeltaPos.x(), oDeltaPos.y());
        m_oPrePos = pEvent->pos();
    }

    //QChartView::mouseMoveEvent(pEvent);

}

void CandleStickWidget::mousePressEvent(QMouseEvent *pEvent)
{

    if (pEvent->button() == Qt::LeftButton)
    {
        m_bLeftButtonPressed = true;
        m_oPrePos = pEvent->pos();
        this->setCursor(Qt::OpenHandCursor);
    }

    //QChartView::mousePressEvent(pEvent);

}

void CandleStickWidget::mouseReleaseEvent(QMouseEvent *pEvent)
{

    if (pEvent->button() == Qt::LeftButton)
    {
        m_bLeftButtonPressed = false;
        this->setCursor(Qt::ArrowCursor);
    }

    //QChartView::mouseReleaseEvent(pEvent);

}

void CandleStickWidget::wheelEvent(QWheelEvent *pEvent)
{

    if(ctrlButtonPressed && pEvent->angleDelta().y() > 0)
        chart->zoomOut();
    else if(ctrlButtonPressed && pEvent->angleDelta().y() < 0)
        chart->zoomIn();

}

void CandleStickWidget::keyPressEvent(QKeyEvent *event)
{
    if(Qt::Key_Control == event->key()){
        ctrlButtonPressed = true;
    }
}

void CandleStickWidget::keyReleaseEvent(QKeyEvent *event)
{
    if(Qt::Key_Control == event->key()){
        ctrlButtonPressed = false;
    }
}

void CandleStickWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        chart->zoomReset();
    }
}

void CandleStickWidget::setKlines(const QString &symbol, const QString &interval, const QString &limit)
{

    if(klinesSeries != nullptr)
        klinesSeries->clear();


    if(chart == nullptr)
        chart = new QChart();
    else{
        chart->removeAllSeries();
        chart->removeAxis(chart->axes(Qt::Horizontal).at(0));
        chart->removeAxis(chart->axes(Qt::Vertical).at(0));
    }


    klinesSeries = new QCandlestickSeries();

    klinesSeries->setUseOpenGL(true);
    klinesSeries->setName("Acme Ltd");
    klinesSeries->setIncreasingColor(QColor(Qt::green));
    klinesSeries->setDecreasingColor(QColor(Qt::red));

    QJsonArray klines;

    while(klines.isEmpty())
            klines = Klines::downloadKlines(symbol, interval, limit);

    int i = klines.size()-1;
    while (i > (-1) ){
        auto set = Klines::toQCandlestickSetPtr(klines.at(i).toArray());
        if (set) {
            klinesSeries->append(set);
        }
        i--;
    }

    chart->addSeries(klinesSeries);
    chart->setTitle(symbol + ", " + interval);
    //chart->setAnimationOptions(QChart::SeriesAnimations);

    axisX = setAxisX(klinesSeries, chart);
    axisY = setAxisY(klinesSeries, chart);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);


    setLiquidities(axisX, axisY);

    setChart(chart);

}

void CandleStickWidget::setLiquidities(QAbstractAxis *axisX, QAbstractAxis *axisY)
{
    auto lineSeries = new QLineSeries();

    chart->addSeries(lineSeries);

    lineSeries->setColor(Qt::blue);
    lineSeries->attachAxis(axisX);
    lineSeries->attachAxis(axisY);

    lineSeries->append(klinesSeries->sets().at(867)->timestamp(), klinesSeries->sets().at(867)->high());
    lineSeries->append(klinesSeries->sets().last()->timestamp(), klinesSeries->sets().at(867)->high());
    lineSeries->setVisible(true);

    for(auto &it : klinesSeries->sets()){
    //работать здесь
    }
}

QAbstractAxis* CandleStickWidget::setAxisX(QCandlestickSeries *klineSeries, QChart *chart)
{
    auto axisX = new QDateTimeAxis();
    chart->addAxis(axisX, Qt::AlignBottom);
    klinesSeries->attachAxis(axisX);

    axisX->setFormat("dd\nMM\nyyyy");
    axisX->setMax(QDateTime::fromMSecsSinceEpoch(klinesSeries->sets().last()->timestamp()));
    axisX->setMin(QDateTime::fromMSecsSinceEpoch(klinesSeries->sets().at(0)->timestamp()));
    axisX->setRange(axisX->min(),
                    axisX->max());

    return axisX;
}

QAbstractAxis *CandleStickWidget::setAxisY(QCandlestickSeries *klineSeries, QChart *chart)
{
    auto axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    klinesSeries->attachAxis(axisY);

    axisY->setMax(axisY->max() * 1.01);
    axisY->setMin(axisY->min() * 0.99);

    return axisY;
}
