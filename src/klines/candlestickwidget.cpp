#include "candlestickwidget.h"

CandleStickWidget::CandleStickWidget(QWidget *parent)
    : QChartView{parent}
{
    setKlines("BTCUSDT", "W", "1000");
}

void CandleStickWidget::updateKlines(const QString &symbol, const QString &interval, const QString &limit)
{
    setKlines(symbol, interval, limit);
}

void CandleStickWidget::autoDrawLiquidities()
{

    if(klinesSeries != nullptr){

        QList<HighLiquid> hights;

        auto list = klinesSeries->sets();

        auto current = list.begin();
        auto prev = current;
        current++;
        auto next = current;

        QCandlestickSet *currentHigh{nullptr};
        QCandlestickSet *currentLow{nullptr};


        for(; current != list.end(); current++){
            next++;

            if((*current)->high() > (*prev)->high()){
                currentHigh = (*current);
            }
            else if(next != list.end() && currentHigh != nullptr){
                    if((*next)->high() < currentHigh->high()){
                        HighLiquid hl;
                        hl.set(nullptr, currentHigh->high(), currentHigh->timestamp(), 0);
                        hights.push_back(hl);
                        currentHigh = nullptr;
                    }
            }
            prev=current;
        }
        auto high = hights.begin();
        while(high != hights.end()){
            auto set = list.begin();
            while((*set)->timestamp() <= high->timestamp){
                set++;
                if(set == list.end()){
                    break;
                }
            }
            if(set != list.end()){
                while((*set)->high() < (*high).count){
                    set++;
                    if(set == list.end()){
                        break;
                    }
                }
                if(set != list.end()){
                    addHigh((*high).count, (*high).timestamp, (*set)->timestamp());
                    high = hights.erase(high);
                }
                else
                    high++;
            }
            else
                high++;
        }
        for(auto high : hights){
            addHigh(high.count, high.timestamp);
        }

    }
}

void CandleStickWidget::autoDrawAreas()
{

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

    if (pEvent->button() == Qt::LeftButton && !isSomeModeActivated())
    {
        m_bLeftButtonPressed = true;
        m_oPrePos = pEvent->pos();
        this->setCursor(Qt::OpenHandCursor);
    }


    QChartView::mousePressEvent(pEvent);

}

void CandleStickWidget::mouseReleaseEvent(QMouseEvent *pEvent)
{

    if (pEvent->button() == Qt::LeftButton && !isSomeModeActivated())
    {
        m_bLeftButtonPressed = false;
        this->setCursor(Qt::ArrowCursor);
    }


    QChartView::mouseReleaseEvent(pEvent);

}

void CandleStickWidget::wheelEvent(QWheelEvent *pEvent)
{

    if(ctrlButtonPressed && pEvent->angleDelta().y() < 0)
        chart->zoom(0.9);
    else if(ctrlButtonPressed && pEvent->angleDelta().y() > 0)
        chart->zoom(1.1);
    else if((pEvent->angleDelta().x() != 0 ) && !ctrlButtonPressed)
        this->chart->scroll(-pEvent->angleDelta().x(), 0);
    else if(pEvent->angleDelta().y() > 0 && !ctrlButtonPressed){
        auto ptr = (QValueAxis*)axisY;
        ptr->setMax(ptr->max() * 1.01);
        ptr->setMin(ptr->min() * 0.99);
    }
    else if(pEvent->angleDelta().y() < 0 && !ctrlButtonPressed){
        auto ptr = (QValueAxis*)axisY;
        ptr->setMax(ptr->max() * 0.99);
        ptr->setMin(ptr->min() * 1.01);
    }
}

void CandleStickWidget::keyPressEvent(QKeyEvent *event)
{
    if(Qt::Key_Control == event->key()){
        ctrlButtonPressed = true;
    }
    if(event->modifiers() == Qt::CTRL && event->key() == Qt::Key_H){
        this->setCursor(Qt::CrossCursor);
        deactivateAllMods();
        hightsAddModeActivated = true;
    }
    if(event->modifiers() == Qt::CTRL && event->key() == Qt::Key_L){
        this->setCursor(Qt::CrossCursor);
        deactivateAllMods();
        lowsAddModeActivated = true;
    }
    if(event->modifiers() == Qt::CTRL && event->key() == Qt::Key_D){
        this->setCursor(Qt::CrossCursor);
        deactivateAllMods();
        for(auto area : areas[currentSymbol]){
            QObject::connect(area.series, SIGNAL(clicked(const QPointF)), this, SLOT(areaClicked()));
        }
        delModeActivated = true;
    }
    if(event->modifiers() == Qt::CTRL && event->key() == Qt::Key_A){
        this->setCursor(Qt::CrossCursor);
        deactivateAllMods();
        areaAddModeActivated = true;
    }
    if(QKeyCombination(Qt::Modifiers({Qt::CTRL, Qt::SHIFT}), Qt::Key_D) == event->keyCombination()){
        this->setCursor(Qt::ArrowCursor);
        deactivateAllMods();
        hightsList.clear();
        lowsList.clear();
        areas.clear();
        for(auto series : chart->series()){
            if(series->type() != QCandlestickSeries::SeriesTypeCandlestick){
                chart->removeSeries(series);
            }
        }
    }
    if(event->key() == Qt::Key_Escape){
        if(this->cursor() == Qt::CrossCursor){
            this->setCursor(Qt::ArrowCursor);
            for(auto area : areas[currentSymbol]){
                QObject::disconnect(area.series, SIGNAL(clicked(const QPointF)), this, SLOT(areaClicked()));
            }
            deactivateAllMods();
        }
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
        auto ptr = (QValueAxis*)axisY;
        qreal max{0};
        qreal min{100000000};
        for (auto set : klinesSeries->sets()){
            if(set->high() > max)
                max = set->high();
            if(set->low() < min)
                min = set->low();
        }
        if(max != 0)
            ptr->setMax(max);
        if(min != 100000000)
            ptr->setMin(min);

        chart->zoomReset();
    }
}

void CandleStickWidget::klineClicked(QCandlestickSet *set)
{
    if(hightsAddModeActivated){
        addHigh(set->high(), set->timestamp());
    }

    if(lowsAddModeActivated){
        addLow(set->low(), set->timestamp());
    }

    if(areaAddModeActivated){
        QDialog dlg(this);
        dlg.setWindowTitle(tr("My dialog"));

        auto layout = new QVBoxLayout();
        auto comboBox = new QComboBox();
        auto buySellComboBox = new QComboBox();
        std::vector<QString> btnText{
            "1. Свеча",
            "2. Тело",
            "3. Верхняя тень",
            "4. Нижняя тень"
        };
        for(auto buttonText : btnText){
            comboBox->addItem(buttonText);
        }
        buySellComboBox->addItem("Buy");
        buySellComboBox->addItem("Sell");

        QDialogButtonBox *btn_box = new QDialogButtonBox(&dlg);
        btn_box->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

        connect(btn_box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        connect(btn_box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

        layout->addWidget(comboBox);
        layout->addWidget(buySellComboBox);
        layout->addWidget(btn_box);

        dlg.setLayout(layout);

        // В случае, если пользователь нажал "Ok".
        if(dlg.exec() == QDialog::Accepted) {
            auto triggered = (comboBox->currentText().at(0));
            int value = QString(triggered).toInt();
            auto green = set->close() > set->open();

            switch (value) {
                case 1:
                    addArea(set->high(), set->low(), set->timestamp(), buySellComboBox->currentText() == "Buy");
                    break;

                case 2:
                    if(green)
                        addArea(set->close(), set->open(), set->timestamp(), buySellComboBox->currentText() == "Buy");
                    else
                        addArea(set->open(), set->close(), set->timestamp(), buySellComboBox->currentText() == "Buy");
                    break;

                case 3:
                    if(green)
                        addArea(set->high(), set->close(), set->timestamp(), buySellComboBox->currentText() == "Buy");
                    else
                        addArea(set->high(), set->open(), set->timestamp(), buySellComboBox->currentText() == "Buy");
                    break;

                case 4:
                    if(green)
                        addArea(set->open(), set->low(), set->timestamp(), buySellComboBox->currentText() == "Buy");
                    else
                        addArea(set->close(), set->low(), set->timestamp(), buySellComboBox->currentText() == "Buy");
                    break;
            }
        }
    }

    if(delModeActivated){
        delLiquid(set->high());
        delLiquid(set->low());
    }


}

void CandleStickWidget::areaClicked()
{
    auto snd = (QAbstractSeries*)sender();

    if(snd != nullptr){
        auto area = areas[currentSymbol].begin();
        while(area->series != snd){
            area++;
        }
        if(area != areas[currentSymbol].end()){
            areas[currentSymbol].erase(area);
        }

        area->removeFromChart(chart);
    }

}


void CandleStickWidget::deactivateAllMods(bool activate)
{
    hightsAddModeActivated = activate;
    lowsAddModeActivated = activate;
    delModeActivated = activate;
    areaAddModeActivated = activate;

}

bool CandleStickWidget::isSomeModeActivated()
{
    return
            hightsAddModeActivated == true ||
            delModeActivated == true ||
            areaAddModeActivated == true ||
            lowsAddModeActivated == true;
}

void CandleStickWidget::setKlines(const QString &symbol, const QString &interval, const QString &limit)
{
    if(symbol != currentSymbol){
        currentSymbol = symbol;
        currentTimeframe = interval;

        if(klinesSeries != nullptr){
            QObject::disconnect(klinesSeries, SIGNAL(clicked(QCandlestickSet*)), this, SLOT(klineClicked(QCandlestickSet *)));
            klinesSeries->clear();
        }


        if(chart == nullptr)
            chart = new QChart();
        else{
            chart->zoomReset();
            chart->removeAllSeries();
            chart->removeAxis(chart->axes(Qt::Horizontal).at(0));
            chart->removeAxis(chart->axes(Qt::Vertical).at(0));
        }


        klinesSeries = new QCandlestickSeries();
        auto connection = QObject::connect(klinesSeries, SIGNAL(clicked(QCandlestickSet*)), this, SLOT(klineClicked(QCandlestickSet *)));

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
        chart->legend()->hide();
        chart->setTitle(symbol + ", " + interval);
        //chart->setAnimationOptions(QChart::SeriesAnimations);

        axisX = setAxisX(klinesSeries, chart);
        axisY = setAxisY(klinesSeries, chart);

        setLiquidities(axisX, axisY);

        addExistSerieses();

        setChart(chart);
    }
    else{
        chart->zoomReset();
        klinesSeries->clear();
        QJsonArray klines;

        chart->removeAxis(chart->axes(Qt::Horizontal).at(0));
        chart->removeAxis(chart->axes(Qt::Vertical).at(0));

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

        chart->setTitle(symbol + ", " + interval);
        chart->legend()->hide();


        axisX = setAxisX(klinesSeries, chart);
        axisY = setAxisY(klinesSeries, chart);

        for(auto it : hightsList[currentSymbol]){
            it.series->attachAxis(axisX);
            it.series->attachAxis(axisY);
        }
        for(auto it : lowsList[currentSymbol]){
            it.series->attachAxis(axisX);
            it.series->attachAxis(axisY);
        }
        for(auto &it : areas[currentSymbol]){
            it.attachAxis(axisX, axisY);
        }

        //this->update();

    }

}

void CandleStickWidget::setLiquidities(QAbstractAxis *axisX, QAbstractAxis *axisY)
{
   // auto lineSeries = new QLineSeries();



    for(auto &it : klinesSeries->sets()){
    //работать здесь
        auto tmpAreas = areas;
        auto tmpHigh = hightsList;
        auto tmpLows = lowsList;

    }
}

QAbstractAxis* CandleStickWidget::setAxisX(QCandlestickSeries *klineSeries, QChart *chart)
{
    auto axisX = new QDateTimeAxis();
    chart->addAxis(axisX, Qt::AlignBottom);
    klinesSeries->attachAxis(axisX);

    axisX->setFormat("dd.MM-hh:mm");
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

void CandleStickWidget::addLow(qreal low, qreal beginTimeStamp, qreal endTimeStamp)
{
    auto lineSeries = new QLineSeries();
    if(endTimeStamp < 0){
        endTimeStamp = klinesSeries->sets().last()->timestamp();
    }

    lineSeries->append(beginTimeStamp, low);
    lineSeries->append(endTimeStamp, low);

    chart->addSeries(lineSeries);

    lineSeries->setColor(QColor(0, 0, 255, 255/2));
    lineSeries->attachAxis(axisX);
    lineSeries->attachAxis(axisY);

    lineSeries->setVisible(true);

    LowLiquid ll;
    ll.set(lineSeries, low, beginTimeStamp, endTimeStamp);
    lowsList[currentSymbol].append(std::move(ll));
}

void CandleStickWidget::addHigh(qreal high, qreal beginTimeStamp, qreal endTimeStamp)
{
    auto lineSeries = new QLineSeries();

    if(endTimeStamp < 0){
        endTimeStamp = klinesSeries->sets().last()->timestamp();
    }

    lineSeries->append(beginTimeStamp, high);
    lineSeries->append(endTimeStamp, high);

    chart->addSeries(lineSeries);

    lineSeries->setColor(QColor(0, 0, 255, 50));
    lineSeries->attachAxis(axisX);
    lineSeries->attachAxis(axisY);

    lineSeries->setVisible(true);

    HighLiquid hl;
    hl.set(lineSeries, high, beginTimeStamp, endTimeStamp);
    hightsList[currentSymbol].append(std::move(hl));
}

void CandleStickWidget::addArea(qreal high, qreal low, qreal beginTimeStamp, bool isBuyArea)
{
    AbstractArea area;
    area.high = high;
    area.low = low;
    area.timestamp = beginTimeStamp;
    area.isBuyArea = isBuyArea;

    auto series0 = new QLineSeries();
    auto series1 = new QLineSeries();
    auto series07 = new QLineSeries();
    auto series05 = new QLineSeries();
    auto series03 = new QLineSeries();
    auto seriesStop = new QLineSeries();

    series0->append(beginTimeStamp, high);
    series0->append(klinesSeries->sets().last()->timestamp(), high);

    series1->append(beginTimeStamp, low);
    series1->append(klinesSeries->sets().last()->timestamp(), low);

    series07->append(beginTimeStamp, area._07());
    series07->append(klinesSeries->sets().last()->timestamp(), area._07());
    series07->setColor(QColor(255, 116, 23));

    series05->append(beginTimeStamp, area._05());
    series05->append(klinesSeries->sets().last()->timestamp(), area._05());
    series05->setColor(QColor(255, 116, 23));

    series03->append(beginTimeStamp, area._03());
    series03->append(klinesSeries->sets().last()->timestamp(), area._03());
    series03->setColor(QColor(255, 116, 23));

    seriesStop->append(beginTimeStamp, area._stop());
    seriesStop->append(klinesSeries->sets().last()->timestamp(), area._stop());
    seriesStop->setColor(Qt::red);

    auto series = new QAreaSeries(series0, series1);

    area.series = series;
    area._03Series = series03;
    area._05Series = series05;
    area._07Series = series07;
    area.stopLine = seriesStop;

    QPen pen(QColor(153, 0, 255));
    pen.setWidth(1);
    pen.setCosmetic(true);
    series->setPen(pen);
    series->setBrush(Qt::Dense7Pattern);

    area.addToChart(chart);

    area.attachAxis(axisX, axisY);

    areas[currentSymbol].append(area);

}

void CandleStickWidget::delLiquid(qreal liquid)
{
    for(auto liq = hightsList[currentSymbol].begin(); liq != hightsList[currentSymbol].end(); liq++){
        if(liq->count == liquid){
            auto series = liq->series;
            hightsList[currentSymbol].erase(liq);
            chart->removeSeries(series);
            break;
        }
    }
    for(auto liq = lowsList[currentSymbol].begin(); liq != lowsList[currentSymbol].end(); liq++){
        if(liq->count == liquid){
            auto series = liq->series;
            lowsList[currentSymbol].erase(liq);
            chart->removeSeries(series);
            break;
        }
    }
}

void CandleStickWidget::addExistSerieses()
{
    auto highCopy = hightsList[currentSymbol];
    hightsList[currentSymbol].clear();
    for(auto it : highCopy){
        addHigh(it.count, it.timestamp);
    }

    auto lowCopy = lowsList[currentSymbol];
    lowsList[currentSymbol].clear();
    for(auto it : lowCopy){
        addLow(it.count, it.timestamp);
    }

    auto areasCopy = areas[currentSymbol];
    areas[currentSymbol].clear();
    for(auto it : areasCopy){
        addArea(it.high, it.low, it.timestamp, it.isBuyArea);
    }
}
