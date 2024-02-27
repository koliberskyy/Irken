#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QToolBox>
#include <QDateTimeEdit>
#include <QStatusBar>
#include <QTimer>
#include <QProgressBar>
#include <QComboBox>

#include <memory>

#include "candlestickwidget.h"

#include "account.h"
#include "smartmoney.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QList<Account*> accountList;
    SmartMoney *smartMoney;

public:
    MainWindow(QWidget *parent = nullptr);

    ~MainWindow();
public slots:
    void updateAccTree();
    void updatePosTree(QJsonArray positions);
    void updateOrdTree(QJsonArray orders);
    void updateSmmTree(QJsonArray orders);
    void ordItemDoubleClicked(QTreeWidgetItem *item, int column);
private slots:
    void timerChanged();
    void graphicControlComboChanged();

private:
    QToolBox *toolBox;

    QDateTimeEdit *dateTimeEdit;
    QTimer *timer;
    QProgressBar *smmUpdateprogressBar;

    std::unique_ptr<QDateTime> posUpdateTime;
    std::unique_ptr<QDateTime> ordUpdateTime;
    std::unique_ptr<QDateTime> smmUpdateTime;

    std::unique_ptr<qint64> posUpdateFluencySec;
    std::unique_ptr<qint64> ordUpdateFluencySec;
    std::unique_ptr<qint64> smmUpdateFluencySec;

    QTreeWidget *accTree;
    QTreeWidget *posTree;
    QTreeWidget *ordTree;
    QTreeWidget *smmTree;

    //graphics control pannel
    CandleStickWidget * candlestickWidget;

    QComboBox *symbolComboBox;
    QComboBox *timeframeComboBox;

    QPushButton *liquidityButton;
    QPushButton *smartMoneyButton;

    void updateAccounts();
    void createConnctions();


};
#endif // MAINWINDOW_H
