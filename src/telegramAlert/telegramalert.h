#ifndef TELEGRAMALERT_H
#define TELEGRAMALERT_H

#include <QWidget>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QJsonArray>
#include "requests.h"
class TelegramAlert : public QWidget
{
    Q_OBJECT
public:
    explicit TelegramAlert(QWidget *parent = nullptr);
private:
    QCheckBox *enabledCheck;
    double alertDifference{0.5};
    long long lastSendedTimestamp{0};
public slots:
    void updateKlines(QString symbol, QString interval, QJsonArray klines);

};

#endif // TELEGRAMALERT_H
