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
    /*
        :param desired_amount:
        :param base_currency_code: TON | BTC
        :param quote_currency_code: RUB | KZT etc.
        :param offer_type: SALE | PURCHASE
        :param limit:
    */
    QJsonArray getAvaibleOrders(const QString &baseCoin, const QString &quoteCoin, const QString &offerType, const QString &limit = "20", double minVolume = 0.0, double maxVolume = 0.0) const;
private:
    QCheckBox *enabledCheck;
    double alertDifference{0.9};
    long long lastSendedTimestamp{0};

    const QString authToken;

public slots:
    void updateKlines(QString symbol, QString interval, QJsonArray klines);

};

#endif // TELEGRAMALERT_H
