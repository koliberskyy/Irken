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
#include <QCheckBox>

#include <memory>

#include "candlestickwidget.h"

#include "account.h"
#include "smartmoney.h"
#include "accountkunteynir.h"
#include "positionkunteynir.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QList<Account*> accountList;
    SmartMoney *smartMoney;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;

public slots:
    void updateAccTree();
    void updatePosTree(QJsonArray positions);
    void updateOrdTree(QJsonArray orders);
    void updateSmmTree(QJsonArray orders);
    void ordItemDoubleClicked(QTreeWidgetItem *item, int column);
    void posItemDoubleClicked(QTreeWidgetItem *item, int column);
    void setLeverage();


private slots:
    void timerChanged();
    void graphicControlComboChanged();

signals:
    void timeToUpdatePositions(AccountItem*);
    void timeToUpdateBalances();
    void timeToUpdateKlines();

private:

    AccountKunteynir *accounts;
    PositionKunteynir *positions;

    //для отключения контроя позиций вместо true написать false
    bool positionControlActivated{true};

    QToolBox *toolBox;

    QDateTimeEdit *dateTimeEdit;
    QTimer *timer;
    QProgressBar *smmUpdateprogressBar;

    std::unique_ptr<QDateTime> posUpdateTime;
    std::unique_ptr<QDateTime> ordUpdateTime;
    std::unique_ptr<QDateTime> smmUpdateTime;
    std::unique_ptr<QDateTime> chartUpdateTime;


    std::unique_ptr<qint64> posUpdateFluencySec;
    std::unique_ptr<qint64> ordUpdateFluencySec;
    std::unique_ptr<qint64> smmUpdateFluencySec;
    std::unique_ptr<qint64> chartUpdateFluencySec;

    QTreeWidget *accTree;
    QTreeWidget *posTree;
    QTreeWidget *ordTree;
    QTreeWidget *smmTree;

    //graphics control pannel
    CandleStickWidget * candlestickWidget;

    QComboBox *symbolComboBox;
    QComboBox *timeframeComboBox;
    QLabel *maxLeverageLabel;
    QDoubleSpinBox *maxLeverageDSB;

    QPushButton *liquidityButton;
    QPushButton *imbalanceButton;
    QPushButton *orderblockButton;
    QPushButton *setLeverageButton;
    QPushButton *NSLButton;
    QPushButton *NSLRGButton;
    QPushButton *NSLLiquids;
    QPushButton *HLTSButton;

    QCheckBox *autocontrolcheckbox;

    void updateAccounts();
    void createConnctions();
    void displayInfo();

    double showLeverageChooseDialog();


};
#endif // MAINWINDOW_H
