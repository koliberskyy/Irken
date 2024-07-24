#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      dateTimeEdit(new QDateTimeEdit()),
      timer(new QTimer(this)),
      chartUpdateTime(new QDateTime(QDateTime::currentDateTime())),
      chartUpdateFluencySec(new qint64(1)),
      tabWgt(new QTabWidget())

{
    auto progress = new QProgressDialog("", "Не жми сюда", 0, 10, this, Qt::WindowStaysOnTopHint);
    progress->setMinimumSize(300, 100);
    progress->setWindowTitle("Требуется интернет. Запуск.");
    progress->show();

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

    //klines work spaces
    for (auto i = 0; i < 3; i++){
        progress->setLabelText("Создаю рабочее пространство №" + QString::fromStdString(std::to_string(i+1)));
        progress->setValue(progress->value() + 1);
        auto ptr = new KlinesWorkingSpace();
        tabWgt->addTab(ptr, "Гафик" + QString::fromStdString(std::to_string(i+1)));
        connect(this, &MainWindow::timeToUpdateKlines, ptr->candlestickWidget, &CandleStickWidget::updateCurrentChart);
    }
    createConnctions();
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


void MainWindow::timerChanged()
{
    auto current = QDateTime::currentDateTime();
    dateTimeEdit->setDateTime(current);

    //update charts
    if(chartUpdateTime->secsTo(current) > *chartUpdateFluencySec){
        emit timeToUpdateKlines();
        chartUpdateTime->setDate(current.date());
        chartUpdateTime->setTime(current.time());
    }
}



void MainWindow::createConnctions()
{
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



