#ifndef KLINES_H
#define KLINES_H

#include <requests.h>
#include <QObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSignalSpy>
#include <vector>
#include "candlestick.h"

class Klines : public Requests
{
    Q_OBJECT
public:
    explicit Klines(const QString &__symbol, const QString &__timeframe, const QString &__limit);
    Klines(Klines &&moved) noexcept;
    ~Klines(){}
private:
    std::vector<CandleStick> klines;
    QString symbol;
    QString timeframe;
    QString limit;
    Thrand thrand;

    void download();
    void create(QJsonArray &&arr);
    void set_thrand();

signals:
    void reply_complete();
private slots:
    virtual void replyFinished(QNetworkReply* reply) override;

public:
    auto get_klines() const{
        return &klines;
    }

    auto get_symbol() const{
        return &symbol;
    }
    auto get_thrand()const{
        return &thrand;
    }
};

#endif // KLINES_H
