#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      dateTimeEdit(new QDateTimeEdit()),
      smmUpdateprogressBar(new QProgressBar),
      timer(new QTimer(this)),
      ordUpdateTime(new QDateTime(QDateTime::currentDateTime())),
      posUpdateTime(new QDateTime(QDateTime::currentDateTime())),
      smmUpdateTime(new QDateTime(QDateTime::currentDateTime())),
      posUpdateFluencySec(new qint64(3)),
      ordUpdateFluencySec(new qint64(10)),
      smmUpdateFluencySec(new qint64(300)),
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

    setMinimumSize(1280, 720);
    updateAccounts();
    createConnctions();

    updateAccTree();
    updatePosTree(QJsonArray());
    updateOrdTree(QJsonArray());
    updateSmmTree(QJsonArray());

    toolBox->addItem(accTree, "Аккаунты");
    toolBox->addItem(posTree,  "Позиции");
    toolBox->addItem(ordTree, "Ордера");
    toolBox->addItem(smmTree, "SmartMoney");

    setCentralWidget(toolBox);

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

                childList.append(obj["orderLinkId"].toString());


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

void MainWindow::timerChanged()
{
    auto current = QDateTime::currentDateTime();
    dateTimeEdit->setDateTime(current);

    //update pos
    if(posUpdateTime->secsTo(current) > *posUpdateFluencySec){
        for(auto &it : accountList){
            it->updatePositions();
        }
        posUpdateTime->setDate(current.date());
        posUpdateTime->setTime(current.time());
    }

    //update ord
    if(ordUpdateTime->secsTo(current) > *ordUpdateFluencySec){
        for(auto &it : accountList){
            it->updateOrders();
        }
        ordUpdateTime->setDate(current.date());
        ordUpdateTime->setTime(current.time());
    }
    //smartmoney update
    if(smmUpdateTime->secsTo(current) > *smmUpdateFluencySec || smartMoney->firstRun()){
        if(smartMoney->isUpdateFinished)
            smartMoney->updateSmartMoney(6);

        smmUpdateTime->setDate(current.date());
        smmUpdateTime->setTime(current.time());
    }
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
    }
    connect(smartMoney, SIGNAL(updated(QJsonArray)), this, SLOT(updateSmmTree(QJsonArray)));

    connect(timer, &QTimer::timeout, this, &MainWindow::timerChanged);

}

