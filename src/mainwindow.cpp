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
      chartUpdateFluencySec(new qint64(1)),
      accTree(new QTreeWidget()),
      ordTree(new QTreeWidget()),
      tabWgt(new QTabWidget())

{
    auto progress = new QProgressDialog("", "Не жми сюда", 0, 10, this, Qt::WindowStaysOnTopHint);
    progress->setMinimumSize(300, 100);
    progress->setWindowTitle("Требуется интернет. Запуск.");
    progress->show();

    //обновляем фильтры перед началом работы приложения
    instruments::double_to_utf8("BTCUSDT", instruments::Filter_type::lotSize, 10);
    progress->setValue(1);

    progress->setLabelText("Создаю статус бар.");

    //status bar
    auto bar = statusBar();

    auto close_btn = new QPushButton("Выход");
    close_btn->setPalette(QPalette(Qt::red));
    QObject::connect(close_btn, &QPushButton::clicked, this, [this](bool){emit this->closeButtonPressed();});
    bar->addWidget(close_btn);

    auto hide_btn = new QPushButton("Свернуть");
    hide_btn->setPalette(QPalette(Qt::blue));
    QObject::connect(hide_btn, &QPushButton::clicked, this, &MainWindow::showMinimized);
    bar->addWidget(hide_btn);

    dateTimeEdit->setReadOnly(true);
    dateTimeEdit->setDisplayFormat("dd-MM-yyyy HH:mm:ss");
    bar->addWidget(dateTimeEdit);

    bar->show();
    timer->start(1000);

    updateAccounts();

    updateAccTree();
    updateOrdTree(QJsonArray());

    progress->setValue(progress->value() + 1);
    progress->setLabelText("Обновляю позиции.");

    //positions
    auto accList_abs = accounts->get_items();
    QList<AccountItem *> accList;
    for(auto it : accList_abs){
        accList.push_back((AccountItem*)it);
    }
    positions->set_accounts(accList);
    connect(this, &MainWindow::timeToUpdatePositions, positions, &PositionKunteynir::download);
    connect(this, SIGNAL(timeToUpdateBalances()), accounts, SLOT(updateBalance()));

    progress->setValue(progress->value() + 1);

    //klines work spaces
    for (auto i = 0; i < 3; i++){
        progress->setLabelText("Создаю рабочее пространство №" + QString::fromStdString(std::to_string(i+1)));
        progress->setValue(progress->value() + 1);
        auto ptr = new KlinesWorkingSpace(accounts);
        tabWgt->addTab(ptr, "Гафик" + QString::fromStdString(std::to_string(i+1)));
        connect(positions, SIGNAL(updatingComplete(QList<AbstractItem*>)), ptr, SLOT(updatePosition(QList<AbstractItem*>)));
        connect(ptr->candlestickWidget, SIGNAL(addOrderClicked(QJsonObject,int)), accounts, SLOT(shareOrder(QJsonObject,int)));
        connect(this, &MainWindow::timeToUpdateKlines, ptr->candlestickWidget, &CandleStickWidget::updateCurrentChart);
    }
    createConnctions();

    tabWgt->addTab(accTree, "Аккаунты");
    tabWgt->addTab(ordTree, "Ордера");
    tabWgt->addTab(accounts, "Аккаунты(бета)");
    tabWgt->addTab(positions, "Позиции (бета)");

    setCentralWidget(tabWgt);

    progress->setValue(progress->maximum());
    delete  progress;
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
    }

    connect(ordTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(ordItemDoubleClicked(QTreeWidgetItem*,int)));
    connect(timer, &QTimer::timeout, this, &MainWindow::timerChanged);


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
    str.append("Для работы программы треуется интернет. Для вытавления(промотра) ордеров и позиций необходим API key загруженый в json формате в файл AccountList.txt.\n");
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
    str.append("ctrl+r - Режим выставления произвольной Зоны\n");
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



