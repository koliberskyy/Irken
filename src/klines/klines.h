#ifndef KLINES_H
#define KLINES_H

#include <QObject>

class Klines : public QObject
{
    Q_OBJECT
public:
    explicit Klines(QObject *parent = nullptr);

signals:

};

#endif // KLINES_H
