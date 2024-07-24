#include "candlestickwidget.h"

CandleStickWidget::CandleStickWidget(QWidget *parent)
    : QChartView{parent}
{
    updateKlines("NOTUSDT", "W", "1000");

}

QString CandleStickWidget::get_currentSymbol() const{
    return currentSymbol;
}

void CandleStickWidget::updateKlines(const QString &symbol, const QString &interval, const QString &limit)
{
    klinesUpdated = false;
    setKlines(symbol, interval, limit);
    klinesUpdated = true;
}

void CandleStickWidget::autoDrawLiquidities()
{

    if(klinesSeries != nullptr){

        QList<HighLiquid> hights;
        QList<LowLiquid> lows;

        auto list = klinesSeries->sets();

        auto current = list.begin();
        auto prev = current;
        current++;
        auto next = current;

        QCandlestickSet *currentHigh{nullptr};
        QCandlestickSet *currentLow{nullptr};


        for(; current != list.end(); current++){
            next++;
            //hights
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
            //lows
            if((*current)->low() < (*prev)->low()){
                currentLow = (*current);
            }
            else if(next != list.end() && currentLow != nullptr){
                    if((*next)->low() > currentLow->low()){
                        LowLiquid ll;
                        ll.set(nullptr, currentLow->low(), currentLow->timestamp(), 0);
                        lows.push_back(ll);
                        currentLow = nullptr;
                    }
            }
            prev=current;
        }

        //сбор hight
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
                while((*set)->high() < high->count){
                    set++;
                    if(set == list.end()){
                        break;
                    }
                }
                if(set != list.end()){
                    addHigh(high->count, high->timestamp, (*set)->timestamp());
                    high = hights.erase(high);
                }
                else
                    high++;
            }
            else
                high++;
        }

        //сбор lows
        auto low = lows.begin();
        while(low != lows.end()){
            auto set = list.begin();
            while((*set)->timestamp() <= low->timestamp){
                set++;
                if(set == list.end()){
                    break;
                }
            }
            if(set != list.end()){
                while((*set)->low() > low->count){
                    set++;
                    if(set == list.end()){
                        break;
                    }
                }
                if(set != list.end()){
                    addLow(low->count, low->timestamp, (*set)->timestamp());
                    low = lows.erase(low);
                }
                else
                    low++;
            }
            else
                low++;
        }

        //добавлеие несобранных
        for(auto &high : hights){
            addHigh(high.count, high.timestamp);
        }
        for(auto &low : lows){
            addLow(low.count, low.timestamp);
        }
    }
}

void CandleStickWidget::autoDrawImbalance()
{
    //imbalance
    auto list = klinesSeries->sets();
    QList<AbstractArea> imbalances;

    auto prev = list.begin();
    auto curr = prev + 1;
    auto next = curr + 1;
    while(next != list.end()){

        //войд имбаланса больше размера одной из соседних свечей, а другая не прекрывает половину его тела
        auto isBuyArea = (*curr)->close() > (*curr)->open();

        auto size_prev = (*prev)->high() - (*prev)->low();
        auto size_next = (*next)->high() - (*next)->low();

        qreal voidImbalance;
        qreal middle;
        if(isBuyArea){
            voidImbalance = (*next)->low() - (*prev)->high();

            if(voidImbalance > size_prev
            || voidImbalance > size_next){


                AbstractArea imba;
                imba.isBuyArea = isBuyArea;
                imba.high = (*next)->low();
                imba.low = (*prev)->high();
                imba.timestamp = (*curr)->timestamp();
                imba.type = "im";

                imbalances.push_back(std::move(imba));

            }


        }
        else{
            voidImbalance = (*prev)->low() - (*next)->high();

            if(voidImbalance > size_prev
            || voidImbalance > size_next ){
                AbstractArea imba;
                imba.isBuyArea = isBuyArea;
                imba.high = (*prev)->low();
                imba.low = (*next)->high();
                imba.timestamp = (*curr)->timestamp();
                imba.type = "im";

                imbalances.push_back(std::move(imba));

            }

        }

        prev++;
        curr++;
        next++;
    }

    //сбор имбалансов
    auto imbalance = imbalances.begin();
    while(imbalance != imbalances.end()){
        auto set = list.begin();
        while((*set)->timestamp() <= imbalance->timestamp){
            set++;
            if(set == list.end()){
                break;
            }
        }
        if(set != list.end()){
                //тут стоит логическое не ВНИМАНИЕ
            while(!((*set)->low() <= imbalance->_05() && (*set)->high() >= imbalance->_05())){
                set++;
                if(set == list.end()){
                    break;
                }
            }
            if(set != list.end()){
                imbalance->endtimestamp = (*set)->timestamp();
                addArea(*imbalance, QColor(153, 0, 255), "im " + currentTimeframe);
                //addArea(imbalance->high, imbalance->low, imbalance->timestamp, imbalance->isBuyArea, (*set)->timestamp());
                imbalance = imbalances.erase(imbalance);
            }
            else
                imbalance++;
        }
        else
            imbalance++;
    }



    //добавление несобранных  imba
    for(auto &imbalance : imbalances){
        //addArea(imbalance.high, imbalance.low, imbalance.timestamp, imbalance.isBuyArea);
        imbalance.endtimestamp = -1;
        addArea(imbalance, QColor(153, 0, 255), "im " + currentTimeframe);
    }

}

void CandleStickWidget::autoDrawOrderBlocks()
{
    //imbalance
    auto list = klinesSeries->sets();
    QList<AbstractArea> orderblocks;

    auto prev = list.begin();
    auto curr = prev + 1;
    auto next = curr + 1;
    while(next != list.end()){

        //войд имбаланса больше размера одной из соседних свечей, а другая не прекрывает половину его тела
        auto body = (*curr)->close() - (*curr)->open();
        auto isBuyArea = body > 0;

        if(!isBuyArea)
            body *= (-1);

        auto size_prev = (*prev)->high() - (*prev)->low();
        auto size_next = (*next)->high() - (*next)->low();

        qreal voidImbalance;
        qreal middle;
        if(isBuyArea){
            voidImbalance = (*next)->low() - (*prev)->high();

            if(voidImbalance > size_prev
            || voidImbalance > size_next ){

                //orderblock
                auto it_ob = curr;
                it_ob--;
                auto ob_size = (*it_ob)->high() - (*it_ob)->low();
                if((*it_ob)->low() < (*curr)->low() && it_ob != list.begin() && ob_size < voidImbalance){
                    auto it_ob_prev = it_ob - 1;
                    if(it_ob_prev != list.begin()){
                        auto it_ob_prev_prev = it_ob_prev - 1;
                        if((*it_ob)->low() < (*it_ob_prev)->low() && (*it_ob)->low() < (*it_ob_prev_prev)->low()){
                            AbstractArea ob;
                            ob.isBuyArea = isBuyArea;
                            ob.high = (*it_ob)->high();
                            ob.low = (*it_ob)->low();
                            ob.timestamp = (*it_ob)->timestamp();
                            ob.type = "ob";

                            orderblocks.push_back(std::move(ob));
                        }
                    }
                }

            }
        }
        else{
            voidImbalance = (*prev)->low() - (*next)->high();
            //middle = (*next)->low() + voidImbalance/2;
            middle = (*curr)->close() + body/2;

            if(voidImbalance > size_prev /*&& middle > (*next)->high()*/
            || voidImbalance > size_next /*&& middle < (*prev)->low()*/){
            //if(voidImbalance > size_prev && middle > (*next)->high()){

                //orderblock
                auto it_ob = curr;
                it_ob--;
                auto ob_size = (*it_ob)->high() - (*it_ob)->low();
                if((*it_ob)->high() > (*curr)->high() && it_ob != list.begin() && ob_size < voidImbalance){
                    auto it_ob_prev = it_ob - 1;
                    if(it_ob_prev != list.begin()){
                        auto it_ob_prev_prev = it_ob_prev - 1;
                        if((*it_ob)->high() > (*it_ob_prev)->high() && (*it_ob)->high() > (*it_ob_prev_prev)->high()){
                            AbstractArea ob;
                            ob.isBuyArea = isBuyArea;
                            ob.high = (*it_ob)->high();
                            ob.low = (*it_ob)->low();
                            ob.timestamp = (*it_ob)->timestamp();
                            ob.type = "ob";

                            orderblocks.push_back(std::move(ob));
                        }
                    }
                }
            }

        }

        prev++;
        curr++;
        next++;
    }


    //сбор ордерблоков
    auto orderblock = orderblocks.begin();
    while(orderblock != orderblocks.end()){
        auto set = list.begin();
        while((*set)->timestamp() <= orderblock->timestamp){
            set++;
            if(set == list.end()){
                break;
            }
        }
        if(set != list.end()){
            //плюсуемся во избежании сбора ордерблока его же имбалансом
            set++;
                //тут стоит логическое не ВНИМАНИЕ
            while(!((*set)->low() <= orderblock->_05() && (*set)->high() >= orderblock->_05())){
                set++;
                if(set == list.end()){
                    break;
                }
            }
            if(set != list.end()){
                addArea(orderblock->high, orderblock->low, orderblock->timestamp, orderblock->isBuyArea, (*set)->timestamp(), QColor(75, 0, 130), "ob " + currentTimeframe);
                orderblock = orderblocks.erase(orderblock);
            }
            else
                orderblock++;
        }
        else
            orderblock++;
    }

    //добавление несобранных  ob
    for(auto &orderblock : orderblocks){
        addArea(orderblock.high, orderblock.low, orderblock.timestamp, orderblock.isBuyArea, -1, QColor(75, 0, 130), "ob " + currentTimeframe);
    }
}

void CandleStickWidget::autoDrawNSL()
{

    auto min = ((QValueAxis*)axisY)->min();


    auto NSLList = NSL();


    for(auto it : NSLList){
        auto NSL = it.first;
        auto set = it.second;

        QColor colorNSL;
        switch(NSL){
        case 0:
            colorNSL = Qt::red;
            break;
        case 1:
            colorNSL = Qt::blue;
            break;
        case 2:
            colorNSL = Qt::green;
            break;
        }

        auto lineSeries1 = new QLineSeries();

        lineSeries1->append(set->timestamp(), set->low());
        lineSeries1->append(set->timestamp(), min);
        lineSeries1->setUseOpenGL(true);

        chart->addSeries(lineSeries1);

        lineSeries1->setPen(Qt::DotLine);
        lineSeries1->setColor(colorNSL);
        lineSeries1->attachAxis(axisX);
        lineSeries1->attachAxis(axisY);

        lineSeries1->setVisible(true);

    }

}

void CandleStickWidget::autoDrawNSLRG()
{
    auto min = ((QValueAxis*)axisY)->min();
    auto NSLList = NSL();
    auto NSLRGList = NSLRG(NSLList);

    for(auto it : NSLRGList){
        auto NSL = it.first;
        auto set = it.second;

        QColor colorNSL;
        switch(NSL){
        case 0:
            colorNSL = Qt::red;
            break;
        case 1:
            colorNSL = Qt::blue;
            break;
        case 2:
            colorNSL = Qt::green;
            break;
        }

        if(colorNSL != Qt::blue){
            auto lineSeries1 = new QLineSeries();

            lineSeries1->append(set->timestamp(), set->low());
            lineSeries1->append(set->timestamp(), min);
            lineSeries1->setUseOpenGL(true);

            chart->addSeries(lineSeries1);

            lineSeries1->setPen(Qt::DotLine);
            lineSeries1->setColor(colorNSL);
            lineSeries1->attachAxis(axisX);
            lineSeries1->attachAxis(axisY);

            lineSeries1->setVisible(true);

        }
    }
}

void CandleStickWidget::autoDrawNSLLiquds()
{
    auto NSLList = NSL();
    auto NSLRGList = NSLRG(NSLList);
    auto list = klinesSeries->sets();

    auto tmp = NSLLiquids(NSLRGList);
    auto hights = tmp.first;
    auto lows = tmp.second;

    //сбор hight
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
            while((*set)->high() < high->count){
                set++;
                if(set == list.end()){
                    break;
                }
            }
            if(set != list.end()){
                addHigh(high->count, high->timestamp, (*set)->timestamp());
                high = hights.erase(high);
            }
            else
                high++;
        }
        else
            high++;
    }

    //сбор lows
    auto low = lows.begin();
    while(low != lows.end()){
        auto set = list.begin();
        while((*set)->timestamp() <= low->timestamp){
            set++;
            if(set == list.end()){
                break;
            }
        }
        if(set != list.end()){
            while((*set)->low() > low->count){
                set++;
                if(set == list.end()){
                    break;
                }
            }
            if(set != list.end()){
                addLow(low->count, low->timestamp, (*set)->timestamp());
                low = lows.erase(low);
            }
            else
                low++;
        }
        else
            low++;
    }

    //добавлеие несобранных
    for(auto &high : hights){
        addHigh(high.count, high.timestamp);
    }
    for(auto &low : lows){
        addLow(low.count, low.timestamp);
    }
}

void CandleStickWidget::autoDrawHLTS()
{
    std::set<HighLiquid> hights;
    std::set<LowLiquid> lows;
    auto list = klinesSeries->sets();

    for(auto &it : hightsList[currentSymbol]){
        hights.insert(it);
    }
    for(auto &it : lowsList[currentSymbol]){
        chart->removeSeries(it.series);
    }
    lowsList[currentSymbol].clear();

    if(!hights.empty()){

        auto currHighIt = hights.begin();
        auto nextHighIt = hights.begin();
        nextHighIt++;

        auto currSetIt = list.begin();
        if(!lowsList.empty()){
            lowsList.clear();
        }
        //add lows
        while (nextHighIt != hights.end()){
            //смещение итератора на позицию первого хая
            while((*currSetIt)->timestamp() != currHighIt->timestamp){
                currSetIt++;
            }
            // поиск минимаьного лоя между хаями
            auto count = (*currSetIt)->low();
            auto ts = (*currSetIt)->timestamp();
            currSetIt++;
            while((*currSetIt)->timestamp() != nextHighIt->timestamp){
                if((*currSetIt)->low() <= count){
                    count = (*currSetIt)->low();
                    ts = (*currSetIt)->timestamp();
                }
                currSetIt++;
            }

            addLow(count, ts);
            currHighIt++;
            nextHighIt++;
        }

        for(auto it : lowsList[currentSymbol]){
            lows.insert(it);
        }

        //рисуем треугольники
        auto lowIt = lows.begin();
        currHighIt = hights.begin();
        nextHighIt = hights.begin();
        nextHighIt++;

        while(lowIt != lows.end() && nextHighIt != hights.end()){
            auto greenLS = new QLineSeries();
            auto redLS = new QLineSeries();

            greenLS->append(lowIt->timestamp, lowIt->count);
            greenLS->append(nextHighIt->timestamp, nextHighIt->count);

            QPen greenPen(Qt::green);
            greenPen.setWidth(3);
            greenLS->setPen(greenPen);
            greenLS->setUseOpenGL(true);

            redLS->append(currHighIt->timestamp, currHighIt->count);
            redLS->append(lowIt->timestamp, lowIt->count);

            QPen redPen(Qt::red);
            redPen.setWidth(3);
            redLS->setPen(redPen);
            redLS->setUseOpenGL(true);

            chart->addSeries(greenLS);
            greenLS->attachAxis(axisX);
            greenLS->attachAxis(axisY);

            chart->addSeries(redLS);
            redLS->attachAxis(axisX);
            redLS->attachAxis(axisY);


            lowIt++;
            currHighIt++;
            nextHighIt++;
        }

        //чистим старые лои
        for(auto &it: lowsList[currentSymbol]){
            chart->removeSeries(it.series);
        }
        lowsList[currentSymbol].clear();

        //соединяем  xaи
        auto gls = new QLineSeries();
        for(auto &it : hights){
            gls->append(it.timestamp, it.count);
        }

        //соединяем новые лои
        auto rls = new QLineSeries();
        for(auto &it : lows){
            rls->append(it.timestamp, it.count);
        }
        QPen pen(Qt::blue);
        pen.setWidth(2);
        gls->setPen(pen);
        gls->setUseOpenGL(true);

        chart->addSeries(gls);
        gls->attachAxis(axisX);
        gls->attachAxis(axisY);

        rls->setPen(pen);
        rls->setUseOpenGL(true);

        chart->addSeries(rls);
        rls->attachAxis(axisX);
        rls->attachAxis(axisY);

        klinesSeries->setIncreasingColor(Qt::white);
        klinesSeries->setDecreasingColor(Qt::black);


        //удаление хаев

        for(auto &it : hightsList[currentSymbol]){
            chart->removeSeries(it.series);
        }
        hightsList[currentSymbol].clear();
    }


}

void CandleStickWidget::autoDrawTradingSessions()
{
    auto dlg = new QDialog();

    auto layout = new QVBoxLayout();
    auto label = new QLabel();

    label->setText("Выбор торговой сессии");

    QDialogButtonBox *btn_box = new QDialogButtonBox();
    btn_box->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(btn_box, &QDialogButtonBox::accepted, dlg, &QDialog::accept);
    connect(btn_box, &QDialogButtonBox::rejected, dlg, &QDialog::reject);

    layout->addWidget(label);

    QList <QCheckBox *> boxes;

    boxes.append(new QCheckBox("1.Passific (Blue)"));
    boxes.append(new QCheckBox("2.Asian (Yellow)"));
    boxes.append(new QCheckBox("3.European (Green)"));
    boxes.append(new QCheckBox("4.American (Red)"));

    for(auto box : boxes){
        layout->addWidget(box);
    }

    layout->addWidget(btn_box, Qt::AlignCenter);
    dlg->setLayout(layout);

    if(dlg->exec() == QDialog::Accepted){

        int activeTradingSessionCount{0};

        if(!activeTradingSessions.isEmpty()){
            for(auto session : activeTradingSessions){
                chart->removeSeries(session);
            }
            activeTradingSessions.clear();
        }

        for(auto box : boxes){

            if(box->isChecked()){

                auto tmp = box->text().begin();
                auto symbol = tmp->toLatin1();
                auto activeTradingSessionCount = std::atoi(&symbol);
                switch (activeTradingSessionCount) {
                    case 1:
                    //passificoceanic
                    activeTradingSessions.append(drawTradingSession(0, 9, Qt::blue));
                    break;
                    case 2:
                    //asian
                    activeTradingSessions.append(drawTradingSession(3, 12, Qt::yellow));
                    break;
                    case 3:
                    //european
                    activeTradingSessions.append(drawTradingSession(10, 19, Qt::green));
                    break;
                    case 4:
                    //american
                    activeTradingSessions.append(drawTradingSession(15, 0, Qt::red));
                    break;
                }
            }
        }
    }
    else{
        if(!activeTradingSessions.isEmpty()){
            for(auto &session : activeTradingSessions){
                chart->removeSeries(session);
            }
            activeTradingSessions.clear();
        }
    }
    dlg->deleteLater();

}

void CandleStickWidget::mouseMoveEvent(QMouseEvent *pEvent)
{

    if(m_bLeftButtonPressed){
        auto oDeltaPos = pEvent->pos() - m_oPrePos;
        this->chart->scroll(-oDeltaPos.x(), oDeltaPos.y());
        m_oPrePos = pEvent->pos();
    }

}

void CandleStickWidget::mousePressEvent(QMouseEvent *pEvent)
{

    if (pEvent->button() == Qt::LeftButton && !isSomeModeActivated())
    {
        m_bLeftButtonPressed = true;
        m_oPrePos = pEvent->pos();
        this->setCursor(Qt::OpenHandCursor);
    }

    if(pEvent->button() == Qt::LeftButton && rangeModeActivated && !rangeModeFrirstClicked){
        rangeModeFirstClickMapToValue = chart->mapToValue(pEvent->pos(), klinesSeries);
        rangeModeFrirstClicked = true;
    }
    else if(pEvent->button() == Qt::LeftButton && rangeModeActivated && rangeModeFrirstClicked){
        deactivateAllMods();
        rangeModeFrirstClicked = false;
        this->setCursor(Qt::ArrowCursor);

        auto rangeModeSecondClickMapToValue = chart->mapToValue(pEvent->pos(), klinesSeries);

        qreal high;
        qreal low;
        qreal beginTS;
        qreal endTs;

        if(rangeModeFirstClickMapToValue.y() > rangeModeSecondClickMapToValue.y()){
            high = rangeModeFirstClickMapToValue.y();
            low = rangeModeSecondClickMapToValue.y();
        }
        else{
            low = rangeModeFirstClickMapToValue.y();
            high = rangeModeSecondClickMapToValue.y();
        }

        if(rangeModeFirstClickMapToValue.x() > rangeModeSecondClickMapToValue.x()){
            endTs = rangeModeFirstClickMapToValue.x();
            beginTS = rangeModeSecondClickMapToValue.x();
        }
        else{
            beginTS = rangeModeFirstClickMapToValue.x();
            endTs = rangeModeSecondClickMapToValue.x();
        }

        addArea(high, low, beginTS, true, endTs);

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
    if(Qt::Key_Shift == event->key()){
        shiftButtonPressed = true;
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
        for(auto &area : areas[currentSymbol]){
            QObject::connect(area.series, SIGNAL(clicked(QPointF)), this, SLOT(areaClicked()));
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

        auto serieses = chart->series();
        for(auto series : serieses){
            if(series->type() != QCandlestickSeries::SeriesTypeCandlestick){
                chart->removeSeries(series);
            }
        }
        hightsList.clear();
        lowsList.clear();
        areas.clear();
        stopLoss.clear();
        takeProfit.clear();

        //костыль для удаления серий использующих opengl
        chart->resize(chart->size() + QSize(1, 1));
        chart->resize(chart->size() - QSize(1, 1));

        klinesSeries->setIncreasingColor(Qt::green);
        klinesSeries->setDecreasingColor(Qt::red);
    }
    if(event->key() == Qt::Key_Escape){
        if(this->cursor() == Qt::CrossCursor){
            this->setCursor(Qt::ArrowCursor);
            for(auto &area : areas[currentSymbol]){
                QObject::disconnect(area.series, SIGNAL(clicked(QPointF)), this, SLOT(areaClicked()));
            }
            deactivateAllMods();
        }
    }

    if(event->modifiers() == Qt::CTRL && event->key() == Qt::Key_R){
        deactivateAllMods();
        this->setCursor(Qt::CrossCursor);
        rangeModeActivated = true;
    }

}

void CandleStickWidget::keyReleaseEvent(QKeyEvent *event)
{
    if(Qt::Key_Control == event->key()){
        ctrlButtonPressed = false;
    }
    if(Qt::Key_Shift == event->key()){
        shiftButtonPressed = false;
    }
}

void CandleStickWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        auto ptr = (QValueAxis*)axisY;
        qreal max{0};
        qreal min{100000000};
        for (auto &set : klinesSeries->sets()){
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
    QChartView::mouseDoubleClickEvent(event);

}

void CandleStickWidget::klineClicked(QCandlestickSet *set)
{
    if(hightsAddModeActivated){
        if(ctrlButtonPressed){
            addTakeProfit(set->high(), set->timestamp());
        }
        else if(shiftButtonPressed){
            shiftButtonPressed = false;
            addStopLoss(set->high(), set->timestamp());
        }
        else{
        addHigh(set->high(), set->timestamp());
        }

    }

    if(lowsAddModeActivated){
        if(ctrlButtonPressed){
            addTakeProfit(set->low(), set->timestamp());
        }
        else if(shiftButtonPressed){
            shiftButtonPressed = false;
            addStopLoss(set->low(), set->timestamp());
        }
        else{
            addLow(set->low(), set->timestamp());
        }
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
        for(auto &buttonText : btnText){
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
        if(stopLoss.timestamp == set->timestamp()){
            chart->removeSeries(stopLoss.series);
            stopLoss.clear();
        }
        if(takeProfit.timestamp == set->timestamp()){
            chart->removeSeries(takeProfit.series);
            takeProfit.clear();
        }

        //костыль для удаления серий использующих opengl
        chart->resize(chart->size() + QSize(1, 1));
        chart->resize(chart->size() - QSize(1, 1));
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
            area->removeFromChart(chart);
            areas[currentSymbol].erase(area);
            for(auto &area_it : areas[currentSymbol]){
                QObject::disconnect(area_it.series, SIGNAL(clicked(QPointF)), this, SLOT(areaClicked()));
            }
            deactivateAllMods();

            //костыль для удаления серий использующих opengl
            chart->resize(chart->size() + QSize(1, 1));
            chart->resize(chart->size() - QSize(1, 1));

            this->setCursor(Qt::ArrowCursor);
        }
    }

}


void CandleStickWidget::showToolTip(const QPointF &point)
{
    if(!isSomeModeActivated()){
        // поймать сендера, преобразовать в указатель на абстрактную серию, вытащить поле нейм и вставить в тул тип
        // а в series->name() можно запихнуть любые данные в json формате, а здесь их парсить
        auto snd = (QAreaSeries*)sender();

        QToolTip::showText(this->cursor().pos(), snd->name(), nullptr, {}, 1000 * 60);
    }
}

void CandleStickWidget::updateCurrentChart()
{
    if(!klinesSeries->sets().isEmpty() && klinesUpdated){

        //last kline update
        auto do_nothing = true;
        QJsonArray klines;
        while(klines.isEmpty())
                klines = Klines::downloadKlines(currentSymbol, currentTimeframe, "2");

        auto set = Klines::toQCandlestickSetPtr(klines.begin()->toArray());

        if(set->timestamp() == klinesSeries->sets().back()->timestamp() && klinesUpdated){
            klinesSeries->remove(klinesSeries->sets().back());
            klinesSeries->append(set);

        }
        else if(klinesUpdated){
            klinesSeries->append(set);
            emit klineAdded(currentSymbol, currentTimeframe, klines);
        }

        //curr price line
        auto oldCurrPriceSer = currentPrice.series;
        auto newCurrPriceSer = new QLineSeries();
        auto endTimeStamp = klinesSeries->sets().last()->timestamp() * 2;
        auto beginTimeStamp = (*klinesSeries->sets().begin())->timestamp();

        newCurrPriceSer->append(beginTimeStamp, klinesSeries->sets().back()->close());
        newCurrPriceSer->append(endTimeStamp, klinesSeries->sets().back()->close());

        if(oldCurrPriceSer != nullptr)
            chart->removeSeries(oldCurrPriceSer);

        currentPrice.series = newCurrPriceSer;
        currentPrice.count = klinesSeries->sets().back()->close();
        chart->addSeries(newCurrPriceSer);

        auto pen = newCurrPriceSer->pen();
        pen.setWidth(0);
        pen.setStyle(Qt::DashLine);



        if(klinesSeries->sets().back()->close() > klinesSeries->sets().back()->open())
            pen.setColor(QColor(0, 180, 0));
        else
            pen.setColor(Qt::red);

        QList<qreal> dashes;
        qreal space = 10;
        dashes << 1 << space;
        pen.setDashPattern(dashes);

        newCurrPriceSer->setPen(pen);
        newCurrPriceSer->attachAxis(axisX);
        newCurrPriceSer->attachAxis(axisY);

        newCurrPriceSer->setVisible(true);

        emit currentPriceChanged(currentPrice.count);


    }
}

const AbstractArea *CandleStickWidget::findArea(QAreaSeries *series)
{
    for(auto it = areas[currentSymbol].begin(); it != areas[currentSymbol].end(); it++){
        if(it->series == series){
            return &(*it);
        }
    }

    return nullptr;
}


void CandleStickWidget::deactivateAllMods(bool activate)
{
    hightsAddModeActivated = activate;
    lowsAddModeActivated = activate;
    delModeActivated = activate;
    areaAddModeActivated = activate;
    rangeModeActivated = activate;
    rangeModeFrirstClicked = activate;
}

bool CandleStickWidget::isSomeModeActivated()
{
    return
            hightsAddModeActivated == true ||
            delModeActivated == true ||
            areaAddModeActivated == true ||
            lowsAddModeActivated == true ||
            rangeModeActivated == true;
}

void CandleStickWidget::setKlines(const QString &symbol, const QString &interval, const QString &limit)
{
    if(symbol != currentSymbol){
        currentSymbol = symbol;
        currentTimeframe = interval;

        if(klinesSeries != nullptr){
            QObject::disconnect(klinesSeries, SIGNAL(clicked(QCandlestickSet*)), this, SLOT(klineClicked(QCandlestickSet*)));
            klinesSeries->clear();
        }

        if(chart == nullptr)
            chart = new QChart();
        else{
            chart->zoomReset();
            chart->removeAllSeries();
            stopLoss.clear();
            takeProfit.clear();
            hightsList.clear();
            lowsList.clear();
            chart->removeAxis(chart->axes(Qt::Horizontal).at(0));
            chart->removeAxis(chart->axes(Qt::Vertical).at(0));
        }


        klinesSeries = new QCandlestickSeries();
        auto connection = QObject::connect(klinesSeries, SIGNAL(clicked(QCandlestickSet*)), this, SLOT(klineClicked(QCandlestickSet*)));
        klinesSeries->setUseOpenGL(true);
        klinesSeries->setIncreasingColor(QColor(Qt::green));
        klinesSeries->setDecreasingColor(QColor(Qt::red));

        QJsonArray klines;

        if(limit.toInt() <= 1000){
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
        }
        else{
            std::cout << "\n download klines error: limit must be less then 1001\n";
        }

        chart->addSeries(klinesSeries);
        chart->legend()->hide();
        chart->setTitle(symbol + ", " + interval);

        axisX = setAxisX(klinesSeries, chart);
        axisY = setAxisY(klinesSeries, chart);

        addExistSerieses();
        setChart(chart);

    }
    else{
        chart->removeAllSeries();
        chart->removeAxis(chart->axes(Qt::Horizontal).at(0));
        chart->removeAxis(chart->axes(Qt::Vertical).at(0));
        currentTimeframe = interval;
        chart->zoomReset();

        klinesSeries = new QCandlestickSeries();
        auto connection = QObject::connect(klinesSeries, SIGNAL(clicked(QCandlestickSet*)), this, SLOT(klineClicked(QCandlestickSet*)));
        QJsonArray klines;
        klinesSeries->setUseOpenGL(true);
        klinesSeries->setIncreasingColor(QColor(Qt::green));
        klinesSeries->setDecreasingColor(QColor(Qt::red));

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
        chart->legend()->hide();

        axisX = setAxisX(klinesSeries, chart);
        axisY = setAxisY(klinesSeries, chart);

        addExistSerieses();

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
    chart->addAxis(axisY, Qt::AlignRight);
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
    lineSeries->setUseOpenGL(true);

    chart->addSeries(lineSeries);

    lineSeries->setColor(QColor(139, 0, 139));
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
    lineSeries->setUseOpenGL(true);

    chart->addSeries(lineSeries);

    lineSeries->setColor(QColor(139, 0, 139));
    lineSeries->attachAxis(axisX);
    lineSeries->attachAxis(axisY);

    lineSeries->setVisible(true);

    HighLiquid hl;
    hl.set(lineSeries, high, beginTimeStamp, endTimeStamp);
    hightsList[currentSymbol].append(std::move(hl));
}

void CandleStickWidget::addArea(qreal high, qreal low, qreal beginTimeStamp, bool isBuyArea, qreal endTimeStamp, const QColor &color, const QString &areaName)
{
    AbstractArea area;
    area.high = high;
    area.low = low;
    area.timestamp = beginTimeStamp;
    area.isBuyArea = isBuyArea;

    if(endTimeStamp < 0){
        endTimeStamp = klinesSeries->sets().last()->timestamp() + (klinesSeries->sets().last()->timestamp() - klinesSeries->sets().at(klinesSeries->sets().size() - 2)->timestamp()) * 5;
    }

    auto series0 = new QLineSeries();
    auto series1 = new QLineSeries();
    auto series07 = new QLineSeries();
    auto series05 = new QLineSeries();
    auto series03 = new QLineSeries();
    auto seriesStop = new QLineSeries();

    series0->append(beginTimeStamp, high);
    series0->append(endTimeStamp, high);
    series0->setUseOpenGL(true);

    series1->append(beginTimeStamp, low);
    series1->append(endTimeStamp, low);
    series1->setUseOpenGL(true);

    series07->append(beginTimeStamp, area._07());
    series07->append(endTimeStamp, area._07());
    series07->setColor(QColor(255, 116, 23));
    series07->setUseOpenGL(true);

    series05->append(beginTimeStamp, area._05());
    series05->append(endTimeStamp, area._05());
    series05->setColor(QColor(255, 116, 23));
    series05->setUseOpenGL(true);

    series03->append(beginTimeStamp, area._03());
    series03->append(endTimeStamp, area._03());
    series03->setColor(QColor(255, 116, 23));
    series03->setUseOpenGL(true);

    seriesStop->append(beginTimeStamp, area._stop());
    seriesStop->append(endTimeStamp, area._stop());
    seriesStop->setColor(Qt::red);
    seriesStop->setUseOpenGL(true);

    auto series = new QAreaSeries(series0, series1);

    area.series = series;
    area._03Series = series03;
    area._05Series = series05;
    area._07Series = series07;
    area.stopLine = seriesStop;

    QJsonObject seriesName;
    if(!areaName.isEmpty()){
        seriesName.insert("area", areaName);
    }
    seriesName.insert("high", area.high);
    seriesName.insert("07", area._07());
    seriesName.insert("05", area._05());
    seriesName.insert("03", area._03());
    seriesName.insert("low", area.low);
    seriesName.insert("rangeHL", area.high - area.low);
    seriesName.insert("rangeHLpc", 100 * (area.high - area.low) / area.low );
    series->setName(QJsonDocument(seriesName).toJson(QJsonDocument::JsonFormat::Indented));

    QPen pen(color);
    pen.setWidth(1);
    pen.setCosmetic(true);
    series->setPen(pen);
    series->setBrush(Qt::Dense7Pattern);
    series->setUseOpenGL(true);

    area.addToChart(chart);

    area.attachAxis(axisX, axisY);

    areas[currentSymbol].append(area);
    QObject::connect(area.series, SIGNAL(clicked(const QPointF &)), this, SLOT(showToolTip(const QPointF &)));


}

void CandleStickWidget::addArea(AbstractArea area, const QColor &color, const QString &areaName)
{

    if(area.endtimestamp < 0){
        area.endtimestamp = klinesSeries->sets().last()->timestamp() + (klinesSeries->sets().last()->timestamp() - klinesSeries->sets().at(klinesSeries->sets().size() - 2)->timestamp()) * 5;
    }

    auto series0 = new QLineSeries();
    auto series1 = new QLineSeries();
    auto series07 = new QLineSeries();
    auto series05 = new QLineSeries();
    auto series03 = new QLineSeries();
    auto seriesStop = new QLineSeries();

    series0->append(area.timestamp, area.high);
    series0->append(area.endtimestamp, area.high);
    series0->setUseOpenGL(true);

    series1->append(area.timestamp, area.low);
    series1->append(area.endtimestamp, area.low);
    series1->setUseOpenGL(true);

    series07->append(area.timestamp, area._07());
    series07->append(area.endtimestamp, area._07());
    series07->setColor(QColor(255, 116, 23));
    series07->setUseOpenGL(true);

    series05->append(area.timestamp, area._05());
    series05->append(area.endtimestamp, area._05());
    series05->setColor(QColor(255, 116, 23));
    series05->setUseOpenGL(true);

    series03->append(area.timestamp, area._03());
    series03->append(area.endtimestamp, area._03());
    series03->setColor(QColor(255, 116, 23));
    series03->setUseOpenGL(true);

    seriesStop->append(area.timestamp, area._stop());
    seriesStop->append(area.endtimestamp, area._stop());
    seriesStop->setColor(Qt::red);
    seriesStop->setUseOpenGL(true);

    auto series = new QAreaSeries(series0, series1);

    area.series = series;
    area._03Series = series03;
    area._05Series = series05;
    area._07Series = series07;
    area.stopLine = seriesStop;

    QJsonObject seriesName;
    if(!areaName.isEmpty()){
        seriesName.insert("area", areaName);
    }
    seriesName.insert("high", area.high);
    seriesName.insert("07", area._07());
    seriesName.insert("05", area._05());
    seriesName.insert("03", area._03());
    seriesName.insert("low", area.low);
    seriesName.insert("rangeHL", area.high - area.low);
    seriesName.insert("rangeHLpc", 100 * (area.high - area.low) / area.low );
    series->setName(QJsonDocument(seriesName).toJson(QJsonDocument::JsonFormat::Indented));

    QPen pen(color);
    pen.setWidth(1);
    pen.setCosmetic(true);
    series->setPen(pen);
    series->setBrush(Qt::Dense7Pattern);
    series->setUseOpenGL(true);

    area.addToChart(chart);

    area.attachAxis(axisX, axisY);

    areas[currentSymbol].append(area);

    QObject::connect(area.series, SIGNAL(clicked(const QPointF &)), this, SLOT(showToolTip(const QPointF &)));


}

void CandleStickWidget::addTakeProfit(qreal price, qreal beginTimeStamp)
{
    if(takeProfit.series != nullptr){
        chart->removeSeries(takeProfit.series);
        takeProfit.clear();
    }
    auto lineSeries = new QLineSeries();
    auto endTimeStamp = klinesSeries->sets().last()->timestamp() + (klinesSeries->sets().last()->timestamp() - klinesSeries->sets().at(klinesSeries->sets().size() - 2)->timestamp()) * 5;

    lineSeries->append(beginTimeStamp, price);
    lineSeries->append(endTimeStamp, price);

    chart->addSeries(lineSeries);

    lineSeries->setColor(QColor(0, 255, 0));
    lineSeries->attachAxis(axisX);
    lineSeries->attachAxis(axisY);

    lineSeries->setVisible(true);

    takeProfit.series = lineSeries;
    takeProfit.count = price;
    takeProfit.timestamp = beginTimeStamp;

}

void CandleStickWidget::addStopLoss(qreal price, qreal beginTimeStamp)
{
    //dialog

    QDialog dlg(this);
    auto form = new QFormLayout();


    auto btn_box = new QDialogButtonBox();
    btn_box->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(btn_box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btn_box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    form->addRow(new QLabel("Смещение стопа"));

    //sb_qty
    auto sb_qty = new QSpinBox();
    sb_qty->setRange(-10, 10);
    sb_qty->setValue(0);
    sb_qty->setSingleStep(1);

    //sld_qty
    auto sld_qty = new QSlider(Qt::Horizontal);
    sld_qty->setRange(sb_qty->minimum(), sb_qty->maximum());
    sld_qty->setValue(sb_qty->value());
    sld_qty->setSingleStep(sb_qty->singleStep());
    QObject::connect(sld_qty, SIGNAL(valueChanged(int)), sb_qty, SLOT(setValue(int)));
    QObject::connect(sb_qty, SIGNAL(valueChanged(int)), sld_qty, SLOT(setValue(int)));

    form->addRow(new QLabel("Смещение %"), sb_qty);
    form->addRow(sld_qty);
    form->addRow(btn_box);

    dlg.setLayout(form);

    // В случае, если пользователь нажал "Ok".
    if(dlg.exec() == QDialog::Accepted) {
        if(sb_qty->value() != 0){
            price = price + price * sb_qty->value() / 1000;
        }
        if(stopLoss.series != nullptr){
            chart->removeSeries(stopLoss.series);
            stopLoss.clear();
        }
        auto lineSeries = new QLineSeries();
        auto endTimeStamp = klinesSeries->sets().last()->timestamp() + (klinesSeries->sets().last()->timestamp() - klinesSeries->sets().at(klinesSeries->sets().size() - 2)->timestamp()) * 5;

        lineSeries->append(beginTimeStamp, price);
        lineSeries->append(endTimeStamp, price);

        chart->addSeries(lineSeries);

        lineSeries->setColor(QColor(255, 0, 0));
        lineSeries->attachAxis(axisX);
        lineSeries->attachAxis(axisY);

        lineSeries->setVisible(true);

        stopLoss.series = lineSeries;
        stopLoss.count = price;
        stopLoss.timestamp = beginTimeStamp;
    }

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
    for(auto &it : highCopy){
        addHigh(it.count, it.timestamp, it.endtimestamp);
    }

    auto lowCopy = lowsList[currentSymbol];
    lowsList[currentSymbol].clear();
    for(auto &it : lowCopy){
        addLow(it.count, it.timestamp, it.endtimestamp);
    }

    auto areasCopy = areas[currentSymbol];
    areas[currentSymbol].clear();
    for(auto &it : areasCopy){
        addArea(it.high, it.low, it.timestamp, it.isBuyArea, it.endtimestamp);
    }
}

QList<std::pair<int, QCandlestickSet *> > CandleStickWidget::NSL()
{
    auto list = klinesSeries->sets();
    QList<std::pair<int, QCandlestickSet*>> NSLList;


    for(auto set : list){
        auto size = set->high() - set->low();

        int NSL{-1};
        //нижняя треть
        if(set->close() < set->low() + size/3){
            NSL = 0;
        }
        //средняя треть
        else if(set->close() <= set->low() + 2*size/3){
            NSL = 1;
        }
        //верхняя треть
        else if(set->close() > set->low() + 2*size/3){
            NSL = 2;
        }

        NSLList.append(std::pair(NSL, set));
    }

    return NSLList;
}

QList<std::pair<int, QCandlestickSet *> > CandleStickWidget::NSLRG(QList<std::pair<int, QCandlestickSet *> > NSL)
{
    auto prev = NSL.begin();
    auto curr = NSL.begin() + 1;
    auto next = NSL.begin() + 2;


    while(next != NSL.end()){
        if(curr->first == 1){
            //red blue blue ... red
            if(prev->first == 0){
                auto lastBlue = next;
                while (lastBlue->first == 1){
                    lastBlue++;
                    if(lastBlue == NSL.end()){
                        break;
                    }
                }
                if(lastBlue != NSL.end()){
                    auto firstBlue = curr;
                    while (firstBlue != lastBlue){
                        firstBlue->first = 0;
                        firstBlue++;
                    }
                }
            }

            //green blue blue ... green
            if(prev->first == 2){
                auto lastBlue = next;
                while (lastBlue->first == 1){
                    lastBlue++;
                    if(lastBlue == NSL.end()){
                        break;
                    }
                }
                if(lastBlue != NSL.end()){
                    auto firstBlue = curr;
                    while (firstBlue != lastBlue){
                        firstBlue->first = 2;
                        firstBlue++;
                    }
                }
            }
        }

        // пропуски красная зеленая красная (зеленую красит в красную) и наоборот
        if(prev->first == 0 && next->first == 0 && curr->first == 2){
            curr->first = 0;
        }
        else
            if(prev->first == 2 && next->first == 2 && curr->first == 0){
            curr->first = 2;
        }


        curr++;
        prev++;
        next++;
    }

    return NSL;
}

std::pair<QList<HighLiquid>, QList<LowLiquid> > CandleStickWidget::NSLLiquids(QList<std::pair<int, QCandlestickSet *> > NSLRG)
{
    QList<HighLiquid> hights;
    QList<LowLiquid> lows;

    auto setPair = NSLRG.begin();
    while(setPair != NSLRG.end()){
        auto zoneBegin = setPair;
        auto zoneEnd = setPair;
        QCandlestickSet *highest{nullptr};
        QCandlestickSet *lowest{nullptr};

        while(setPair->first == zoneBegin->first){
            setPair++;
            if(setPair == NSLRG.end())
                break;
        }
        if(setPair != NSLRG.end()){

            zoneEnd = setPair;
            highest = zoneBegin->second;
            lowest = zoneBegin->second;
            auto highestIt = zoneBegin;
            auto lowestIt = zoneBegin;

            zoneBegin++;

            while(zoneBegin != zoneEnd){
                    if(zoneBegin->second->low() < lowest->low()){
                        lowest = zoneBegin->second;
                        lowestIt = zoneBegin;
                    }

                    if(zoneBegin->second->high() > highest->high()){
                        highest = zoneBegin->second;
                        highestIt = zoneBegin;
                    }

                zoneBegin++;
            }


            //смещение
            //high
            if(highestIt != NSLRG.begin() && highest != nullptr){

                auto prev = --highestIt;

                while(prev != NSLRG.begin()){
                    if(prev->second->high() > highest->high()){
                        highest = prev->second;
                        prev--;
                    }
                    else
                        break;
                }
            }
            //low
            if(lowestIt != NSLRG.begin() && lowest != nullptr){

                auto prev = --lowestIt;

                while(prev != NSLRG.begin()){
                    if(prev->second->low() < lowest->low()){
                        lowest = prev->second;
                        prev--;
                    }
                    else
                        break;
                }
            }



            if(highest != nullptr){
                HighLiquid hl;
                hl.set(nullptr, highest->high(), highest->timestamp(), 0);
                hights.push_back(hl);
            }
            if(lowest != nullptr){
                LowLiquid ll;
                ll.set(nullptr, lowest->low(), lowest->timestamp(), 0);
                lows.push_back(ll);
            }

            highest = nullptr;
            lowest = nullptr;

        }
    }

    return std::pair(hights, lows);
}

QList<QAreaSeries *> CandleStickWidget::drawTradingSession(int hourBegin, int hourEnd, QColor color)
{

    QList<QAreaSeries *> reply;
    auto sets = klinesSeries->sets();

    auto high = (*sets.begin())->high();
    auto low = (*sets.begin())->low();
    for(auto set : sets){
        if(set->high() > high)
            high = set->high();
        if(set->low() < low)
            low = set->low();
    }

    auto tsBegin = (*sets.begin())->timestamp();
    auto tsEnd = (*sets.begin())->timestamp();
    for(auto set : sets){
        auto dateTime = QDateTime::fromMSecsSinceEpoch(set->timestamp());
        if(dateTime.time().hour() == hourBegin && tsBegin == tsEnd){
            tsBegin = set->timestamp();
        }
        if(dateTime.time().hour() == hourEnd && tsBegin != tsEnd){
            tsEnd = set->timestamp();

            auto upper = new QLineSeries();
            upper->append(tsBegin, high);
            upper->append(tsEnd, high);
            upper->setUseOpenGL(true);

            auto lower = new QLineSeries();
            lower->append(tsBegin, low);
            lower->append(tsEnd, low);
            lower->setUseOpenGL(true);

            auto area = new QAreaSeries(upper, lower);
            QPen pen(color);
            pen.setWidth(3);
            pen.setCosmetic(true);
            area->setPen(pen);
            area->setBrush(Qt::Dense7Pattern);
            area->setUseOpenGL(true);

            chart->addSeries(area);

            area->attachAxis(axisX);
            area->attachAxis(axisY);

            tsEnd = tsBegin;

            reply.append(area);

        }
    }

    return reply;
}

double CandleStickWidget::getRiskRatio(double poe, double sl, double tp)
{
    //уловие лонг позиции
    if(poe > sl){
        return (tp - poe) / (poe-sl);
    }
    else{
        return (poe - tp) / (sl - poe);
    }
}

double CandleStickWidget::getStopLossPercent(double poe, double sl)
{
    //уловие лонг позиции
    if(poe > sl){
        return 100 * (poe-sl) / sl;
    }
    else{
        return 100 * (sl - poe) / poe;
    }
}

double CandleStickWidget::leverageFromSLP(double slp)
{
    if(slp <= 0)
        return 0;

    return 100 / slp;
}

bool AbstractLiquid::operator <(const AbstractLiquid &other) const
{
    return timestamp < other.timestamp;
}

bool AbstractLiquid::operator ==(const AbstractLiquid &other) const
{
    return timestamp == other.timestamp;
}



//klines**************************
QJsonArray Klines::downloadKlines(const QString &symbol, const QString &interval, const QString &limit, const QString &begin, const QString &end)
{
    QByteArray query("category=spot&symbol=" + symbol.toUtf8() + "&interval=" + interval.toUtf8() + "&limit=" +limit.toUtf8());

    if(!begin.isEmpty() && !end.isEmpty())
        query.append("&start=" + begin.toUtf8() + "&end=" + end.toUtf8());
    else if((begin.isEmpty() && !end.isEmpty()) || (!begin.isEmpty() && end.isEmpty())){
        std::cout << "ERROR in downloadKlines: begin must not be empty when end is not empty or vice";
        return QJsonArray();
    }

    //QString url("https://api.bybit.com/v5/market/mark-price-kline?");

    QString url("https://api.bybit.com/v5/market/kline?");


    auto customReplyParser = [](const QUrl &url, const QByteArray &data) ->QJsonObject
        {
            auto obj = QJsonDocument::fromJson(data).object();
            auto retCode = obj["retCode"].toInt();
            if(retCode != 0){
                instruments::replyError(url, data);
                return QJsonObject();
            }
            return std::move(obj);
        };

    auto obj = Requests::get(url, query, "klines-" + symbol + "-" + interval, 10000, customReplyParser);

    if(!obj.isEmpty()){
        auto result = obj["result"].toObject();
        auto list = result["list"].toArray();

        if(!list.empty()){
            //выдаем дальше
            return std::move(list);
        }
    }
    return QJsonArray();
}

QCandlestickSet *Klines::toQCandlestickSetPtr(const QJsonArray &kline)
{
    return new QCandlestickSet(open(kline), high(kline), low(kline), close(kline), time(kline));
}

QCandlestickSet Klines::toQCandlestickSet(const QJsonArray &kline)
{
    return QCandlestickSet(open(kline), high(kline), low(kline), close(kline), time(kline));
}
