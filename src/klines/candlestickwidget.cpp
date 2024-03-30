#include "candlestickwidget.h"

CandleStickWidget::CandleStickWidget(QWidget *parent)
    : QChartView{parent}
{
    updateKlines("BTCUSDT", "W", "1000");
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
        for(auto high : hights){
            addHigh(high.count, high.timestamp);
        }
        for(auto low : lows){
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
            //middle = (*prev)->high() + voidImbalance/2;
            middle = (*curr)->open() + body/2;

            if(voidImbalance > size_prev /*&& middle < (*next)->low()*/
            || voidImbalance > size_next /*&& middle > (*prev)->high()*/){
            //if(voidImbalance > size_prev && middle < (*next)->low()){

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
            //middle = (*next)->low() + voidImbalance/2;
            middle = (*curr)->close() + body/2;

            if(voidImbalance > size_prev /*&& middle > (*next)->high()*/
            || voidImbalance > size_next /*&& middle < (*prev)->low()*/){
            //if(voidImbalance > size_prev && middle > (*next)->high()){
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
                addArea(*imbalance);
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
        addArea(imbalance);
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
            //middle = (*prev)->high() + voidImbalance/2;
            middle = (*curr)->open() + body/2;

            if(voidImbalance > size_prev /*&& middle < (*next)->low()*/
            || voidImbalance > size_next /*&& middle > (*prev)->high()*/){
            //if(voidImbalance > size_prev && middle < (*next)->low()){

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
                addArea(orderblock->high, orderblock->low, orderblock->timestamp, orderblock->isBuyArea, (*set)->timestamp(), QColor(75, 0, 130));
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
        addArea(orderblock.high, orderblock.low, orderblock.timestamp, orderblock.isBuyArea, -1, QColor(75, 0, 130));
    }
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
        stopLoss.clear();
        takeProfit.clear();
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

    if(event->modifiers() == Qt::CTRL && event->key() == Qt::Key_B){
        this->setCursor(Qt::ArrowCursor);
        deactivateAllMods();
        marketBuySellInit("Buy");
    }

    if(event->modifiers() == Qt::CTRL && event->key() == Qt::Key_S){
        this->setCursor(Qt::ArrowCursor);
        deactivateAllMods();
        marketBuySellInit("Sell");
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
        if(stopLoss.timestamp == set->timestamp()){
            chart->removeSeries(stopLoss.series);
            stopLoss.clear();
        }
        if(takeProfit.timestamp == set->timestamp()){
            chart->removeSeries(takeProfit.series);
            takeProfit.clear();
        }
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
            for(auto area : areas[currentSymbol]){
                QObject::disconnect(area.series, SIGNAL(clicked(const QPointF)), this, SLOT(areaClicked()));
            }
            deactivateAllMods();
            this->setCursor(Qt::ArrowCursor);
        }
    }

}

void CandleStickWidget::areaDoubleClicked()
{
    auto snd = (QAreaSeries*)sender();
    if(snd != nullptr){
        auto area = findArea(snd);

        QDialog dlg(this);
        dlg.setWindowTitle(tr("My dialog"));

        auto *form = new QFormLayout();

        //dsb_price
        auto dsb_price = new QDoubleSpinBox();
        dsb_price->setDecimals(instruments::dap(currentSymbol));
        dsb_price->setRange(instruments::minPrice(currentSymbol), instruments::maxPrice(currentSymbol));
        dsb_price->setSingleStep(instruments::stepPrice(currentSymbol));
        if(area->type == "im"){
            dsb_price->setValue(area->_05());
        }
        else if(area->isBuyArea){
            dsb_price->setValue(area->_07());
        }
        else{
            dsb_price->setValue(area->_03());
        }

        //dsb_tp
        auto dsb_tp = new QDoubleSpinBox();
        dsb_tp->setDecimals(instruments::dap(currentSymbol));
        dsb_tp->setRange(0, instruments::maxPrice(currentSymbol));
        dsb_tp->setSingleStep(instruments::stepPrice(currentSymbol));
        if(takeProfit.series != nullptr){
            dsb_tp->setValue(takeProfit.count);
        }

        //dsb_sl
        auto dsb_sl = new QDoubleSpinBox();
        dsb_sl->setDecimals(instruments::dap(currentSymbol));
        dsb_sl->setRange(0, instruments::maxPrice(currentSymbol));
        dsb_sl->setSingleStep(instruments::stepPrice(currentSymbol));

        if(stopLoss.series != nullptr){
            dsb_sl->setValue(stopLoss.count);
        }
        else{
            dsb_sl->setValue(area->_stop());
        }

        //sb_qty
        auto sb_qty = new QSpinBox();
        sb_qty->setRange(0, 100);
        sb_qty->setValue(sb_qty->maximum() / 20);
        sb_qty->setSingleStep(1);

        //sld_qty
        auto sld_qty = new QSlider(Qt::Horizontal);
        sld_qty->setRange(sb_qty->minimum(), sb_qty->maximum());
        sld_qty->setValue(sb_qty->value());
        sld_qty->setSingleStep(sb_qty->singleStep());
        QObject::connect(sld_qty, SIGNAL(valueChanged(int)), sb_qty, SLOT(setValue(int)));
        QObject::connect(sb_qty, SIGNAL(valueChanged(int)), sld_qty, SLOT(setValue(int)));


        //sb_lev
        auto sb_lev = new QSpinBox();
        sb_lev->setRange(1, instruments::maxLeverage(currentSymbol.toUtf8()));
        sb_lev->setValue(sb_lev->maximum());
        sb_lev->setSingleStep(1);

        //sld_lev
        auto sld_lev = new QSlider(Qt::Horizontal);
        sld_lev->setRange(sb_lev->minimum(), sb_lev->maximum());
        sld_lev->setValue(sb_lev->value());
        sld_lev->setSingleStep(sb_lev->singleStep());
        QObject::connect(sld_lev, SIGNAL(valueChanged(int)), sb_lev, SLOT(setValue(int)));
        QObject::connect(sb_lev, SIGNAL(valueChanged(int)), sld_lev, SLOT(setValue(int)));

        //buySellComboBox
        auto buySellComboBox = new QComboBox();
        buySellComboBox->addItem("Buy");
        buySellComboBox->addItem("Sell");
        if(area->isBuyArea){
            buySellComboBox->setCurrentIndex(0);
        }
        else{
            buySellComboBox->setCurrentIndex(1);
        }


        QDialogButtonBox *btn_box = new QDialogButtonBox();
        btn_box->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

        connect(btn_box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
        connect(btn_box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

        form->addRow(new QLabel("Side"), buySellComboBox);
        form->addRow(new QLabel("ТВХ"), dsb_price);
        form->addRow(new QLabel("ТП"), dsb_tp);
        form->addRow(new QLabel("СЛ"), dsb_sl);
        form->addRow(new QLabel("%"), sb_qty);
        form->addRow(sld_qty);
        form->addRow(new QLabel("Плечо"), sb_lev);
        form->addRow(sld_lev);
        form->addRow(btn_box);

        dlg.setLayout(form);

        // В случае, если пользователь нажал "Ok".
        if(dlg.exec() == QDialog::Accepted) {
            QJsonObject order;
            order.insert("orderType", "Limit");
            order.insert("category", "linear");
            order.insert("symbol", currentSymbol);
            order.insert("price", QString::fromUtf8(instruments::double_to_utf8(currentSymbol.toUtf8(), instruments::Filter_type::price, dsb_price->value())));
            order.insert("side", buySellComboBox->currentText());
            order.insert("stopLoss", QString::fromUtf8(instruments::double_to_utf8(currentSymbol.toUtf8(), instruments::Filter_type::price, dsb_sl->value())));
            order.insert("takeProfit", QString::fromUtf8(instruments::double_to_utf8(currentSymbol.toUtf8(), instruments::Filter_type::price, dsb_tp->value())));
            order.insert("qty", QString::fromStdString(std::to_string(sb_qty->value())));

            if(!order["price"].toString().isEmpty())
                emit addOrderClicked(order, sb_lev->value());
            else
                std::cout << "order price isEmpty\n";

            dlg.deleteLater();
        }
    }
}

void CandleStickWidget::marketBuySellInit(const QString &side)
{
    QDialog dlg(this);
    dlg.setWindowTitle(tr("My dialog"));

    auto *form = new QFormLayout();


    //dsb_tp
    auto dsb_tp = new QDoubleSpinBox();
    dsb_tp->setDecimals(instruments::dap(currentSymbol));
    dsb_tp->setRange(instruments::minPrice(currentSymbol), instruments::maxPrice(currentSymbol));
    dsb_tp->setSingleStep(instruments::stepPrice(currentSymbol));
    if(takeProfit.series != nullptr){
        dsb_tp->setValue(takeProfit.count);
    }

    //dsb_sl
    auto dsb_sl = new QDoubleSpinBox();
    dsb_sl->setDecimals(instruments::dap(currentSymbol));
    dsb_sl->setRange(instruments::minPrice(currentSymbol), instruments::maxPrice(currentSymbol));
    dsb_sl->setSingleStep(instruments::stepPrice(currentSymbol));
    dsb_sl->setValue(stopLoss.count);

    //sb_qty
    auto sb_qty = new QSpinBox();
    sb_qty->setRange(0, 20);
    sb_qty->setValue(5);
    sb_qty->setSingleStep(1);

    //sld_qty
    auto sld_qty = new QSlider(Qt::Horizontal);
    sld_qty->setRange(sb_qty->minimum(), sb_qty->maximum());
    sld_qty->setValue(sb_qty->value());
    sld_qty->setSingleStep(sb_qty->singleStep());
    QObject::connect(sld_qty, SIGNAL(valueChanged(int)), sb_qty, SLOT(setValue(int)));
    QObject::connect(sb_qty, SIGNAL(valueChanged(int)), sld_qty, SLOT(setValue(int)));

    //sb_lev
    auto sb_lev = new QSpinBox();
    sb_lev->setRange(1, instruments::maxLeverage(currentSymbol.toUtf8()));
    sb_lev->setValue(sb_lev->maximum());
    sb_lev->setSingleStep(1);

    //sld_lev
    auto sld_lev = new QSlider(Qt::Horizontal);
    sld_lev->setRange(sb_lev->minimum(), sb_lev->maximum());
    sld_lev->setValue(sb_lev->value());
    sld_lev->setSingleStep(sb_lev->singleStep());
    QObject::connect(sld_lev, SIGNAL(valueChanged(int)), sb_lev, SLOT(setValue(int)));
    QObject::connect(sb_lev, SIGNAL(valueChanged(int)), sld_lev, SLOT(setValue(int)));


    QDialogButtonBox *btn_box = new QDialogButtonBox();
    btn_box->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(btn_box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btn_box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    form->addRow(new QLabel(side));
    form->addRow(new QLabel("ТП"), dsb_tp);
    form->addRow(new QLabel("СЛ"), dsb_sl);
    form->addRow(new QLabel("%"), sb_qty);
    form->addRow(sld_qty);
    form->addRow(new QLabel("Плечо"), sb_lev);
    form->addRow(sld_lev);
    form->addRow(btn_box);

    dlg.setLayout(form);

    // В случае, если пользователь нажал "Ok".
    if(dlg.exec() == QDialog::Accepted) {
        QJsonObject order;
        order.insert("category", "linear");
        order.insert("symbol", currentSymbol);
        order.insert("orderType", "Market");
        order.insert("side", side);
        order.insert("stopLoss", QString::fromUtf8(instruments::double_to_utf8(currentSymbol.toUtf8(), instruments::Filter_type::price, dsb_sl->value())));
        order.insert("takeProfit", QString::fromUtf8(instruments::double_to_utf8(currentSymbol.toUtf8(), instruments::Filter_type::price, dsb_tp->value())));
        order.insert("qty", QString::fromStdString(std::to_string(sb_qty->value())));
        order.insert("price", QString::fromUtf8(instruments::double_to_utf8(currentSymbol.toUtf8(), instruments::Filter_type::price, currentPrice.count)));
        emit addOrderClicked(order, sb_lev->value());

        dlg.deleteLater();
    }
}

void CandleStickWidget::updateCurrentChart()
{
    if(!klinesSeries->sets().isEmpty() && klinesUpdated){

        //last kline update
        auto do_nothing = true;
        QJsonArray klines;
        while(klines.isEmpty())
                klines = Klines::downloadKlines(currentSymbol, currentTimeframe, "1");

        auto set = Klines::toQCandlestickSetPtr(klines.begin()->toArray());

        if(set->timestamp() == klinesSeries->sets().back()->timestamp() && klinesUpdated){
            klinesSeries->remove(klinesSeries->sets().back());
            klinesSeries->append(set);

        }
        else if(klinesUpdated){
            klinesSeries->append(set);
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
            stopLoss.clear();
            takeProfit.clear();
            chart->removeAxis(chart->axes(Qt::Horizontal).at(0));
            chart->removeAxis(chart->axes(Qt::Vertical).at(0));
        }


        klinesSeries = new QCandlestickSeries();
        auto connection = QObject::connect(klinesSeries, SIGNAL(clicked(QCandlestickSet*)), this, SLOT(klineClicked(QCandlestickSet *)));

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
            QJsonArray tmp;
            auto i_limit = limit.toInt() - 1000;

            while(tmp.isEmpty())
                    tmp = Klines::downloadKlines(symbol, interval, "1000");

            auto timeBegin = Klines::toQCandlestickSet(tmp.last().toArray()).timestamp();
            auto timeEnd = Klines::toQCandlestickSet(tmp.begin()->toArray()).timestamp();
            klines.append(tmp);

            while(i_limit > 0){
                QJsonArray tmpppppppppppppp;
                while(tmpppppppppppppp.isEmpty()){
                    tmpppppppppppppp = Klines::downloadKlines(symbol, interval, "1000", QString::fromStdString(std::to_string(timeBegin - 2*(timeEnd - timeBegin))), QString::fromStdString(std::to_string(timeBegin)));
                }
                timeBegin = Klines::toQCandlestickSet(tmpppppppppppppp.last().toArray()).timestamp();
                timeEnd = Klines::toQCandlestickSet(tmpppppppppppppp.begin()->toArray()).timestamp();
                i_limit -= 1000;
                klines.push_back(tmp);
            }
            for(auto it:klines){
                auto arr = it.toArray();
                int i = arr.size() -1;
                while (i > (-1) ){
                    auto set = Klines::toQCandlestickSetPtr(arr.at(i).toArray());
                    if (set) {
                        klinesSeries->append(set);
                    }
                    i--;
                }

            }

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
        currentTimeframe = interval;
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

    chart->addSeries(lineSeries);

    lineSeries->setColor(QColor(0, 0, 255, 50));
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

void CandleStickWidget::addArea(qreal high, qreal low, qreal beginTimeStamp, bool isBuyArea, qreal endTimeStamp, const QColor &color)
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

    series1->append(beginTimeStamp, low);
    series1->append(endTimeStamp, low);

    series07->append(beginTimeStamp, area._07());
    series07->append(endTimeStamp, area._07());
    series07->setColor(QColor(255, 116, 23));

    series05->append(beginTimeStamp, area._05());
    series05->append(endTimeStamp, area._05());
    series05->setColor(QColor(255, 116, 23));

    series03->append(beginTimeStamp, area._03());
    series03->append(endTimeStamp, area._03());
    series03->setColor(QColor(255, 116, 23));

    seriesStop->append(beginTimeStamp, area._stop());
    seriesStop->append(endTimeStamp, area._stop());
    seriesStop->setColor(Qt::red);

    auto series = new QAreaSeries(series0, series1);

    area.series = series;
    area._03Series = series03;
    area._05Series = series05;
    area._07Series = series07;
    area.stopLine = seriesStop;

    QPen pen(color);
    pen.setWidth(1);
    pen.setCosmetic(true);
    series->setPen(pen);
    series->setBrush(Qt::Dense7Pattern);

    area.addToChart(chart);

    area.attachAxis(axisX, axisY);

    areas[currentSymbol].append(area);
    QObject::connect(area.series, SIGNAL(doubleClicked(const QPointF &)), this, SLOT(areaDoubleClicked()));


}

void CandleStickWidget::addArea(AbstractArea area, const QColor &color)
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

    series1->append(area.timestamp, area.low);
    series1->append(area.endtimestamp, area.low);

    series07->append(area.timestamp, area._07());
    series07->append(area.endtimestamp, area._07());
    series07->setColor(QColor(255, 116, 23));

    series05->append(area.timestamp, area._05());
    series05->append(area.endtimestamp, area._05());
    series05->setColor(QColor(255, 116, 23));

    series03->append(area.timestamp, area._03());
    series03->append(area.endtimestamp, area._03());
    series03->setColor(QColor(255, 116, 23));

    seriesStop->append(area.timestamp, area._stop());
    seriesStop->append(area.endtimestamp, area._stop());
    seriesStop->setColor(Qt::red);

    auto series = new QAreaSeries(series0, series1);

    area.series = series;
    area._03Series = series03;
    area._05Series = series05;
    area._07Series = series07;
    area.stopLine = seriesStop;

    QPen pen(color);
    pen.setWidth(1);
    pen.setCosmetic(true);
    series->setPen(pen);
    series->setBrush(Qt::Dense7Pattern);

    area.addToChart(chart);

    area.attachAxis(axisX, axisY);

    areas[currentSymbol].append(area);
    QObject::connect(area.series, SIGNAL(doubleClicked(const QPointF &)), this, SLOT(areaDoubleClicked()));

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
