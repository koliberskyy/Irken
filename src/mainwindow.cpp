#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      accounts{new AccountKunteynir()},
      positions{new PositionKunteynir()},
      dateTimeEdit(new QDateTimeEdit()),
      timer(new QTimer(this)),
      ordUpdateTime(new QDateTime(QDateTime::currentDateTime())),
      posUpdateTime(new QDateTime(QDateTime::currentDateTime())),
      chartUpdateTime(new QDateTime(QDateTime::currentDateTime())),
      posUpdateFluencySec(new qint64(3)),
      ordUpdateFluencySec(new qint64(10)),
      chartUpdateFluencySec(new qint64(3)),
      accTree(new QTreeWidget()),
      ordTree(new QTreeWidget()),
      toolBox(new QToolBox())

{
    //обновляем фильтры перед началом работы приложения
    auto filters = instruments::double_to_utf8("BTCUSDT", instruments::Filter_type::lotSize, 10);

    //status bar
    auto bar = statusBar();
    dateTimeEdit->setReadOnly(true);
    dateTimeEdit->setDisplayFormat("dd-MM-yyyy HH:mm:ss");
    bar->addWidget(dateTimeEdit);
    bar->show();
    timer->start(1000);

    setMinimumSize(800, 600);
    updateAccounts();

    updateAccTree();
    updateOrdTree(QJsonArray());

    toolBox->addItem(accTree, "Аккаунты");
    toolBox->addItem(ordTree, "Ордера");

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

    NSLButton = new QPushButton("NSL");
    NSLRGButton = new QPushButton("NSL RG");
    NSLLiquids = new QPushButton("NSL Liquid");
    HLTSButton = new QPushButton("HLTS");
    TradingSessionsButton = new QPushButton("Торговые сессии");
    QObject::connect(NSLButton, &QPushButton::pressed, candlestickWidget, &CandleStickWidget::autoDrawNSL);
    QObject::connect(NSLRGButton, &QPushButton::pressed, candlestickWidget, &CandleStickWidget::autoDrawNSLRG);
    QObject::connect(NSLLiquids, &QPushButton::pressed, candlestickWidget, &CandleStickWidget::autoDrawNSLLiquds);
    QObject::connect(HLTSButton, &QPushButton::pressed, candlestickWidget, &CandleStickWidget::autoDrawHLTS);
    QObject::connect(TradingSessionsButton, &QPushButton::pressed, candlestickWidget, &CandleStickWidget::autoDrawTradingSessions);

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
    graphicGrid->addWidget(NSLButton, 9, 1, Qt::AlignCenter);
    graphicGrid->addWidget(NSLRGButton, 10, 1, Qt::AlignCenter);
    graphicGrid->addWidget(NSLLiquids, 11, 1, Qt::AlignCenter);
    graphicGrid->addWidget(HLTSButton, 12, 1, Qt::AlignCenter);
    graphicGrid->addWidget(TradingSessionsButton, 13, 1, Qt::AlignCenter);

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
        connect(it, SIGNAL(ordersUpdated(QJsonArray)), this, SLOT(updateOrdTree(QJsonArray)));
        //legacy orders
        //connect(candlestickWidget, SIGNAL(addOrderClicked(QJsonObject, int)), it, SLOT(placeOrder(QJsonObject, int)));
    }
    //new orders
    connect(candlestickWidget, SIGNAL(addOrderClicked(QJsonObject, int)), accounts, SLOT(shareOrder(QJsonObject, int)));

    connect(ordTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int )), this, SLOT(ordItemDoubleClicked(QTreeWidgetItem *, int)));

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

