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
#include <QSpacerItem>


#include "klinesworkingspace.h"


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;

private slots:
    void timerChanged();

signals:
    void timeToUpdateKlines();
    void closeButtonPressed();

private:
    QTabWidget *tabWgt;

    QDateTimeEdit *dateTimeEdit;
    QTimer *timer;

    std::unique_ptr<QDateTime> chartUpdateTime;

    std::unique_ptr<qint64> chartUpdateFluencySec;

    void createConnctions();
    void displayInfo();



};
#endif // MAINWINDOW_H
