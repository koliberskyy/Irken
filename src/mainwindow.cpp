#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      accounts{new AccountKunteynir()},
      positions{new PositionKunteynir()},
      dateTimeEdit(new QDateTimeEdit()),
      smmUpdateprogressBar(new QProgressBar),
      timer(new QTimer(this)),
      ordUpdateTime(new QDateTime(QDateTime::currentDateTime())),
      posUpdateTime(new QDateTime(QDateTime::currentDateTime())),
      smmUpdateTime(new QDateTime(QDateTime::currentDateTime())),
      chartUpdateTime(new QDateTime(QDateTime::currentDateTime())),
      posUpdateFluencySec(new qint64(3)),
      ordUpdateFluencySec(new qint64(10)),
      smmUpdateFluencySec(new qint64(300)),
      chartUpdateFluencySec(new qint64(3)),
      accTree(new QTreeWidget()),
      posTree(new QTreeWidget()),
      ordTree(new QTreeWidget()),
      smmTree(new QTreeWidget()),
      toolBox(new QToolBox())

{


    //обновляем фильтры перед началом работы приложения
    auto filters = instruments::double_to_utf8("BTCUSDT", instruments::Filter_type::lotSize, 10);

    smartMoney= (new SmartMoney());

    //status bar
    auto bar = statusBar();
    dateTimeEdit->setReadOnly(true);
    dateTimeEdit->setDisplayFormat("dd-MM-yyyy HH:mm:ss");
    bar->addWidget(dateTimeEdit);

    bar->addWidget(smmUpdateprogressBar);
    smmUpdateprogressBar->setRange(0, 100);
    connect(smartMoney, SIGNAL(updateProgressChanged(int)), smmUpdateprogressBar, SLOT(setValue(int)));

    bar->show();
    timer->start(1000);

    setMinimumSize(800, 600);
    updateAccounts();

    updateAccTree();
    updatePosTree(QJsonArray());
    updateOrdTree(QJsonArray());
    updateSmmTree(QJsonArray());

    toolBox->addItem(accTree, "Аккаунты");
    toolBox->addItem(posTree,  "Позиции");
    toolBox->addItem(ordTree, "Ордера");
    toolBox->addItem(smmTree, "SmartMoney");

    //graphics control
    candlestickWidget = new CandleStickWidget();

    symbolComboBox = new QComboBox();
    for(auto symbol : symbol::utf8){
        symbolComboBox->addItem(symbol);
    }
    QObject::connect(symbolComboBox, &QComboBox::currentTextChanged, this, &MainWindow::graphicControlComboChanged);

    timeframeComboBox = new QComboBox();
    for(auto timeframe : symbol::timeframes){
        timeframeComboBox->addItem(timeframe);
    }
    QObject::connect(timeframeComboBox, &QComboBox::currentTextChanged, this, &MainWindow::graphicControlComboChanged);

    liquidityButton = new QPushButton("Ликвидности");
    QObject::connect(liquidityButton, &QPushButton::pressed, candlestickWidget, &CandleStickWidget::autoDrawLiquidities);

    imbalanceButton = new QPushButton("Имбалансы");
    QObject::connect(imbalanceButton, &QPushButton::pressed, candlestickWidget, &CandleStickWidget::autoDrawImbalance);

    orderblockButton = new QPushButton("Ордер Блоки");
    QObject::connect(orderblockButton, &QPushButton::pressed, candlestickWidget, &CandleStickWidget::autoDrawOrderBlocks);

    setLeverageButton = new QPushButton("Установить плечо");
    QObject::connect(setLeverageButton, &QPushButton::pressed, this, &MainWindow::setLeverage);

    autocontrolcheckbox = new QCheckBox("Автоконтроль");
    autocontrolcheckbox->setChecked(false);

    maxLeverageLabel = new QLabel("Максимальное плечо");
    maxLeverageDSB = new QDoubleSpinBox();
    maxLeverageDSB->setMaximum(150);
    maxLeverageDSB->setMinimum(1);
    maxLeverageDSB->setReadOnly(true);
    maxLeverageDSB->setValue(instruments::maxLeverage(symbolComboBox->currentText().toUtf8()));


    auto graphicGrid = new QGridLayout();
    graphicGrid->addWidget(symbolComboBox, 0, 1, Qt::AlignCenter);
    graphicGrid->addWidget(timeframeComboBox, 1, 1, Qt::AlignCenter);
    graphicGrid->addWidget(liquidityButton, 2, 1, Qt::AlignCenter);
    graphicGrid->addWidget(imbalanceButton, 3, 1, Qt::AlignCenter);
    graphicGrid->addWidget(orderblockButton, 4, 1, Qt::AlignCenter);
    graphicGrid->addWidget(autocontrolcheckbox, 5, 1, Qt::AlignCenter);
    graphicGrid->addWidget(maxLeverageLabel, 6, 1, Qt::AlignCenter);
    graphicGrid->addWidget(maxLeverageDSB, 7, 1, Qt::AlignCenter);
    graphicGrid->addWidget(setLeverageButton, 8, 1, Qt::AlignCenter);

    graphicGrid->addWidget(candlestickWidget, 0, 0, graphicGrid->rowCount() + 10, 1);

    auto graphicWgt = new QWidget();
    graphicWgt->setLayout(graphicGrid);

    toolBox->addItem(graphicWgt, "График");
    toolBox->addItem(accounts, "Аккаунты(бета)");

    //positions

    auto accList_abs = accounts->get_items();
    QList<AccountItem *> accList;
    for(auto it : accList_abs){
        accList.push_back((AccountItem*)it);
    }
    positions->set_accounts(accList);
    connect(this, &MainWindow::timeToUpdatePositions, positions, &PositionKunteynir::download);
    connect(this, SIGNAL(timeToUpdateBalances()), accounts, SLOT(updateBalance()));

    toolBox->addItem(positions, "Позиции (бета)");

    createConnctions();

    setCentralWidget(toolBox);

}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(Qt::Key_F1 == event->key()){
        displayInfo();
    }

    QMainWindow::keyPressEvent(event);
}

MainWindow::~MainWindow()
{

}

void MainWindow::updateAccTree()
{
    auto accPtr = (Account*)sender();

    if(accPtr != nullptr){
        auto itemNew = accPtr->toTreeItemPtr();
        auto list = accTree->findItems(itemNew->text(itemNew->columnCount() - 1), Qt::MatchFixedString, itemNew->columnCount() - 1);
        auto a = *list.begin();
        a->setText(2, itemNew->text(2));
    }
    else{
        accTree->clear();
        QStringList accColumns{"Имя", "Фамилия", "Баланс", "Дата подкл.", "Пнл. с подкл.", "Верифицирован", "Прим", "api"};
        accTree->setColumnCount(accColumns.size());
        accTree->setHeaderLabels(accColumns);
        accTree->hideColumn(accTree->columnCount()-1);
        for(auto &it : accountList){
            accTree->addTopLevelItem(it->toTreeItemPtr());
        }
        for(auto &it : accountList){
            it->updateBalance();
        }
    }
}

void MainWindow::updatePosTree(QJsonArray positions)
{
    auto accPtr = (Account*)sender();
    if(accPtr == nullptr){
        posTree->clear();
        QStringList posColumns{"Имя", "Фамилия", "Символ", "Лонг/шорт", "Pnl", "ТВХ", "Марк.", "ТП", "СЛ", "Маржа", "Плечо", "Обновлено", "api"};
        posTree->setColumnCount(posColumns.size());
        posTree->setHeaderLabels(posColumns);
        posTree->hideColumn(posTree->columnCount()-1);
        for(auto &it : accountList){
            auto tmp = it->toTreeItem();
            posTree->addTopLevelItem(new QTreeWidgetItem(QStringList{tmp.text(0), tmp.text(1), "" , "", "", "", "", "", "", "", "", "", tmp.text(tmp.columnCount()-1)}));
        }
        for(auto &it : accountList){
            it->updatePositions();
        }
    }
    else{
        auto itemNew = accPtr->toTreeItem();
        auto list = posTree->findItems(itemNew.text(itemNew.columnCount() - 1), Qt::MatchFixedString, posTree->columnCount() - 1);
        if(list.size() > 0){
            auto item = (*list.begin());

            QList<QTreeWidgetItem *> undefinedChildrens;
            for(auto i = 0; i < item->childCount(); i++){
                undefinedChildrens.append(item->child(i));
            }
            for(auto it : positions){

                auto obj = it.toObject();
                QStringList childList;
                childList.append("");//пропускаем колонки имя, фамилия
                childList.append("");
                childList.append(obj["symbol"].toString());
                childList.append(obj["side"].toString());
                childList.append(Position::unrealizedPnlPercentQString(obj));
                childList.append(obj["avgPrice"].toString());
                childList.append(obj["markPrice"].toString());
                childList.append(obj["takeProfit"].toString());
                childList.append(obj["stopLoss"].toString());
                childList.append(obj["size"].toString());
                childList.append(obj["leverage"].toString());

                time_t timer{std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())};
                std::tm bt = *std::localtime(&timer);
                std::stringstream ss;
                ss << std::put_time(&bt, "%H:%M:%S");
                childList.append(QString::fromStdString(ss.str()));

                childList.append(accPtr->api());

                auto isFinded = false;
                //изменяем существующие
                for(auto it_undef = undefinedChildrens.begin(); it_undef != undefinedChildrens.end(); it_undef++){
                    auto child = *it_undef;
                    //еслм ордер существует
                    if(obj["avgPrice"] == child->text(5)){
                        isFinded = true;
                        int j = 0;
                        //заменяем отличающиеся поля
                        for(auto &cl_it : childList){
                            if(cl_it != child->text(j)){
                                child->setText(j, cl_it);
                            }
                            j++;
                        }
                        //удаляем из списка устаревших
                        it_undef = undefinedChildrens.erase(it_undef);
                        break;
                    }
                }


                //если одрдер с сервера не найден среди ордеров из памяти добавляем его
                if(!isFinded){
                    item->addChild(new QTreeWidgetItem(childList));
                }
            }
            //удаляем устаревшие
            for(auto it : undefinedChildrens){
                item->removeChild(it);
            }
            //accPtr->updatePositions();
        }
    }
}

void MainWindow::updateOrdTree(QJsonArray orders)
{
    auto accPtr = (Account*)sender();
    if(accPtr == nullptr){
        ordTree->clear();
        QStringList posColumns{"Имя", "Фамилия", "Символ", "Лонг/шорт", "ТВХ", "ТП", "СЛ", "Маржа", "Обновлено", "id",  "api"};
        ordTree->setColumnCount(posColumns.size());
        ordTree->setHeaderLabels(posColumns);
        ordTree->hideColumn(ordTree->columnCount()-1);
        for(auto &it : accountList){
            auto tmp = it->toTreeItem();
            ordTree->addTopLevelItem(new QTreeWidgetItem(QStringList{tmp.text(0), tmp.text(1), "" , "", "", "", "", "", "", "", tmp.text(tmp.columnCount()-1)}));
        }
        for(auto &it : accountList){
            it->updateOrders();
        }
    }
    else{
        auto itemNew = accPtr->toTreeItem();
        auto list = ordTree->findItems(itemNew.text(itemNew.columnCount() - 1), Qt::MatchFixedString, ordTree->columnCount() - 1);
        if(list.size() > 0){
            auto item = (*list.begin());

            QList<QTreeWidgetItem *> undefinedChildrens;
            for(auto i = 0; i < item->childCount(); i++){
                undefinedChildrens.append(item->child(i));
            }
            for(auto it : orders){

                auto obj = it.toObject();
                QStringList childList;
                childList.append("");//пропускаем колонки имя, фамилия
                childList.append("");
                childList.append(obj["symbol"].toString());
                childList.append(obj["side"].toString());
                childList.append(obj["price"].toString());
                childList.append(obj["takeProfit"].toString());
                childList.append(obj["stopLoss"].toString());
                childList.append(obj["qty"].toString());

                time_t timer{std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())};
                std::tm bt = *std::localtime(&timer);
                std::stringstream ss;
                ss << std::put_time(&bt, "%H:%M:%S");
                childList.append(QString::fromStdString(ss.str()));

                childList.append(obj["orderId"].toString());
                childList.append(accPtr->api());


                auto isFinded = false;
                //изменяем существующие
                for(auto it_undef = undefinedChildrens.begin(); it_undef != undefinedChildrens.end(); it_undef++){
                    auto child = *it_undef;
                    //еслм ордер существует
                    if(obj["orderLinkId"] == child->text(item->columnCount() -2)){
                        isFinded = true;
                        int j = 0;
                        //заменяем отличающиеся поля
                        for(auto &cl_it : childList){
                            if(cl_it != child->text(j)){
                                child->setText(j, cl_it);
                            }
                            j++;
                        }
                        //удаляем из списка устаревших
                        it_undef = undefinedChildrens.erase(it_undef);
                        break;
                    }
                }

                //если одрдер с сервера не найден среди ордеров из памяти добавляем его
                if(!isFinded){
                    item->addChild(new QTreeWidgetItem(childList));
                }


            }
            //удаляем устаревшие
            for(auto it : undefinedChildrens){
                item->removeChild(it);
            }
            //accPtr->updateOrders();
        }
    }
}

void MainWindow::updateSmmTree(QJsonArray orders)
{
    auto senderPtr = (SmartMoney*)sender();
    if(senderPtr == nullptr){
        smmTree->clear();
        QStringList posColumns{"Символ", "Лонг/шорт", "ТВХ", "ТП", "СЛ", "Обновлено", "id"};
        smmTree->setColumnCount(posColumns.size());
        smmTree->setHeaderLabels(posColumns);

    }
    else{
        auto tmp = smartMoney->getOrders();

        QList<QJsonObject> orderList;
        for(auto it: *tmp){
            orderList.append(it.second);
        }

        smmTree->clear();
        for (auto it : orderList){
            QStringList list;
            list.append(it["symbol"].toString());
            list.append(it["side"].toString());
            list.append(it["price"].toString());
            list.append(it["takeProfit"].toString());
            list.append(it["stopLoss"].toString());

            time_t timer{std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())};
            std::tm bt = *std::localtime(&timer);
            std::stringstream ss;
            ss << std::put_time(&bt, "%H:%M:%S");
            list.append(QString::fromStdString(ss.str()));

            smmTree->addTopLevelItem(new QTreeWidgetItem(list));
        }
        if(orderList.isEmpty()){
            QStringList list;
            list.append("");
            list.append("");
            list.append("");
            list.append("");
            list.append("");

            time_t timer{std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())};
            std::tm bt = *std::localtime(&timer);
            std::stringstream ss;
            ss << std::put_time(&bt, "%H:%M:%S");
            list.append(QString::fromStdString(ss.str()));

            smmTree->addTopLevelItem(new QTreeWidgetItem(list));
        }


        for(auto &it:accountList){
            it->refreshOrderList(orderList);
        }
    }
}

void MainWindow::ordItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    auto symbol = item->text(2);
    QDialog dlg(this);
    auto form = new QFormLayout();


    QDialogButtonBox *btn_box = new QDialogButtonBox();
    btn_box->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(btn_box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btn_box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    form->addRow(new QLabel("Отменить ордер?"));
    form->addRow(btn_box);
    dlg.setLayout(form);

    // В случае, если пользователь нажал "Ok".
    if(dlg.exec() == QDialog::Accepted) {
        QJsonObject order;
        order.insert("symbol", item->text(2));

        auto list = ordTree->findItems(item->text(4), Qt::MatchRecursive, 4);


        for(auto it : accountList){
            for(auto curr_item : list){
            if(curr_item->text(curr_item->columnCount() - 1) == it->api()){
                order["orderId"] = curr_item->text(ordTree->columnCount()-2);
                it->cancelOrder(order);
            }
            }
        }
    }
}

void MainWindow::posItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    //        QStringList posColumns{"Имя", "Фамилия", "Символ", "Лонг/шорт", "Pnl", "ТВХ", "Марк.", "ТП", "СЛ", "Маржа", "Плечо", "Обновлено", "api"};

    //prepare pos
    auto symbol = item->text(2);
    auto side = item->text(3);
    auto size = item->text(9);//different for every acc
    auto tmp = item->text(4);
    auto iter = std::find(tmp.begin(), tmp.end(), ',');
    if(iter != tmp.end())
        *iter = '.';

    auto pnl = tmp.toDouble();


    QJsonObject pos;

    pos.insert("symbol", symbol);
    pos.insert("side", side);

    QDialog dlg(this);
    auto form = new QFormLayout();


    auto btn_box = new QDialogButtonBox();
    btn_box->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    auto cb_stopBU = new QCheckBox("Стоп б/у");
    auto cb_close = new QCheckBox("Закрыть");
    cb_stopBU->setChecked(true);
    cb_close->setChecked(false);

    connect(btn_box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btn_box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    form->addRow(new QLabel("Закрыть позицию?"));

    //sb_qty
    auto sb_qty = new QSpinBox();
    sb_qty->setRange(0, 100);
    sb_qty->setValue(25);
    sb_qty->setSingleStep(1);

    //sld_qty
    auto sld_qty = new QSlider(Qt::Horizontal);
    sld_qty->setRange(sb_qty->minimum(), sb_qty->maximum());
    sld_qty->setValue(sb_qty->value());
    sld_qty->setSingleStep(sb_qty->singleStep());
    QObject::connect(sld_qty, SIGNAL(valueChanged(int)), sb_qty, SLOT(setValue(int)));
    QObject::connect(sb_qty, SIGNAL(valueChanged(int)), sld_qty, SLOT(setValue(int)));

    form->addRow(cb_stopBU);
    form->addRow(cb_close);
    form->addRow(new QLabel("%"), sb_qty);
    form->addRow(sld_qty);
    form->addRow(btn_box);

    dlg.setLayout(form);

    // В случае, если пользователь нажал "Ok".
    if(dlg.exec() == QDialog::Accepted) {

        auto list = posTree->findItems(item->text(2), Qt::MatchRecursive, 2);

        for(auto it : accountList){
            for(auto curr_item : list){
            if(curr_item->text(curr_item->columnCount() - 1) == it->api()){
                pos["size"] = curr_item->text(9);
                pos["avgPrice"] = curr_item->text(5);
                pos["stopLoss"] = curr_item->text(8);
                pos["takeProfit"] = curr_item->text(7);

                double close_pc;

                if(cb_close->isChecked())
                    close_pc = 100;
                else
                    close_pc = sb_qty->value();

                if(cb_stopBU->isChecked() && pos["avgPrice"].toString().toDouble() != pos["stopLoss"].toString().toDouble() && pnl > 0){
                    it->setTradingStop(pos);
                }

                if(close_pc != 0){
                    it->reducePosition(pos, close_pc);
                }
            }
            }
        }
    }
}

void MainWindow::setLeverage()
{

    auto choosed = AccountItem::showAccountChooseDialog(accounts->list(), "Аккаунты на которых необходимо изменить Плечо");

    if(!choosed.isEmpty()){
        auto leverage = showLeverageChooseDialog();
        if(leverage > 0){
            auto pd = new QProgressDialog("Прогресс",  "Остановить", 0, choosed.size(), this);
            for (auto it : choosed){
                auto info = bybitInfo();
                pd->setLabelText(it->get_name() + " " + it->get_name_second());

                do{
                   info = Methods::setLeverage(symbolComboBox->currentText(), leverage,  it->get_api(), it->get_secret());
                   if(pd->wasCanceled()){
                       break;
                   }
                }while(info.retCode() != 0 && info.retCode() != 110043);
                pd->setValue(pd->value() + 1);
            }
            pd->deleteLater();
        }
    }
}

void MainWindow::timerChanged()
{
    auto current = QDateTime::currentDateTime();
    dateTimeEdit->setDateTime(current);

    //update pos
    if(posUpdateTime->secsTo(current) > *posUpdateFluencySec && positionControlActivated){
//        for(auto &it : accountList){
//            it->updatePositions(autocontrolcheckbox->isChecked());
//        }
        posUpdateTime->setDate(current.date());
        posUpdateTime->setTime(current.time());

        if(positions->isUpdated()){
            emit timeToUpdatePositions(nullptr);
        }
    }

    //update ord + balance
    if(ordUpdateTime->secsTo(current) > *ordUpdateFluencySec){
        for(auto &it : accountList){
            it->updateOrders();
            it->updateBalance();
        }
        ordUpdateTime->setDate(current.date());
        ordUpdateTime->setTime(current.time());
        //emit timeToUpdateBalances();
    }

    //update charts
    if(chartUpdateTime->secsTo(current) > *chartUpdateFluencySec){
        emit timeToUpdateKlines();
        chartUpdateTime->setDate(current.date());
        chartUpdateTime->setTime(current.time());
    }
}

void MainWindow::graphicControlComboChanged()
{
    candlestickWidget->updateKlines(symbolComboBox->currentText(), timeframeComboBox->currentText());
    maxLeverageDSB->setValue(instruments::maxLeverage(symbolComboBox->currentText().toUtf8()));
}




void MainWindow::updateAccounts()
{
    if(!accountList.isEmpty())
        accountList.clear();

    QFile qfile("AccountList.txt");
    qfile.open(QIODevice::OpenModeFlag::ReadOnly);
    auto jsonArr = QJsonDocument::fromJson(qfile.readAll()).array();
    qfile.close();
    for (auto it : jsonArr){
        accountList.emplaceBack(new Account(it.toObject(), this));
    }
}

void MainWindow::createConnctions()
{
    for(auto &it : accountList){
        connect(it, &Account::balanceUpdated, this, &MainWindow::updateAccTree);
        connect(it, SIGNAL(positionsUpdated(QJsonArray)), this, SLOT(updatePosTree(QJsonArray)));
        connect(it, SIGNAL(ordersUpdated(QJsonArray)), this, SLOT(updateOrdTree(QJsonArray)));
        //legacy orders
        //connect(candlestickWidget, SIGNAL(addOrderClicked(QJsonObject, int)), it, SLOT(placeOrder(QJsonObject, int)));
    }
    //new orders
    connect(candlestickWidget, SIGNAL(addOrderClicked(QJsonObject, int)), accounts, SLOT(shareOrder(QJsonObject, int)));
    connect(smartMoney, SIGNAL(updated(QJsonArray)), this, SLOT(updateSmmTree(QJsonArray)));

    connect(ordTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int )), this, SLOT(ordItemDoubleClicked(QTreeWidgetItem *, int)));
    connect(posTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int )), this, SLOT(posItemDoubleClicked(QTreeWidgetItem *, int)));


    connect(timer, &QTimer::timeout, this, &MainWindow::timerChanged);

    connect(this, &MainWindow::timeToUpdateKlines, candlestickWidget, &CandleStickWidget::updateCurrentChart);


}

void MainWindow::displayInfo()
{
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Справка"));

    auto layout = new QVBoxLayout();
    auto label = new QLabel();
    QString str;
    str.append("Для вызова этой справки нажми f1 в любое время\n");
    str.append("\n");
    str.append("Список горячих клавиш:\n");
    str.append("ctrl+b - Купить ПО РЫНКУ\n");
    str.append("ctrl+s - Продать ПО РЫНКУ\n");
    str.append("ctrl+a - Переключить в режим выставления зоны\n");
    str.append("ctrl+d - Переключить в режим удаления элементов рисования\n");
    str.append("ctrl+l - Переключить в режим выставления лоёв\n");
    str.append("ctrl+h - Переключить в режим выставления хуёв\n");
    str.append("\t в режимах лоёв и хуёв: \n");
    str.append("\t\t держи ctrl для выставления тейк-профита \n");
    str.append("\t\t держи shift для выставления стоп-лосса \n");
    str.append("Esc - выключить все режимы\n");
    str.append("\n");
    str.append("Два раза кликни по позиции чтобы закрыть её или выставить стоп б/у\n");
    str.append("Два раза кликни по ордеру чтобы закрыть его\n");
    str.append("Два раза кликни зоне чтобы выложить ордер\n");
    str.append("\n");
    str.append("Если что-то идет не по плану то проверь интернет, а затем выключи и включи снова.\n");
    str.append("Если это не помогло вынь голову из жопы и попробуй еще раз.\n");

    label->setText(str);

    QDialogButtonBox *btn_box = new QDialogButtonBox(&dlg);
    btn_box->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(btn_box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btn_box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    layout->addWidget(label);
    layout->addWidget(btn_box, Qt::AlignCenter);

    dlg.setLayout(layout);

    if(dlg.exec() == QDialog::Accepted){}
}

double MainWindow::showLeverageChooseDialog()
{
    QDialog dlg(this);
    auto form = new QFormLayout();


    auto btn_box = new QDialogButtonBox();
    btn_box->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);


    connect(btn_box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btn_box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    form->addRow(new QLabel("Выбор плеча"));

    //sb_qty
    auto sb_lev = new QSpinBox();
    sb_lev->setRange(1, instruments::maxLeverage(symbolComboBox->currentText().toUtf8()));
    sb_lev->setValue(100);
    sb_lev->setSingleStep(1);

    //sld_qty
    auto sld_lev = new QSlider(Qt::Horizontal);
    sld_lev->setRange(sb_lev->minimum(), sb_lev->maximum());
    sld_lev->setValue(sb_lev->value());
    sld_lev->setSingleStep(sb_lev->singleStep());
    QObject::connect(sld_lev, SIGNAL(valueChanged(int)), sb_lev, SLOT(setValue(int)));
    QObject::connect(sb_lev, SIGNAL(valueChanged(int)), sld_lev, SLOT(setValue(int)));

    form->addRow(sb_lev);
    form->addRow(sld_lev);
    form->addRow(btn_box);

    dlg.setLayout(form);

    // В случае, если пользователь нажал "Ok".
    if(dlg.exec() == QDialog::Accepted) {
        return sb_lev->value();
    }

    return -1;
}

