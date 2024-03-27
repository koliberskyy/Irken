#ifndef ABSTRACTITEM_H
#define ABSTRACTITEM_H

#include <QWidget>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QFormLayout>
#include <QSlider>

#include <memory>
#include <unordered_map>

#include "methods.h"

class AbstractItem : public QWidget
{
    Q_OBJECT
public:
    explicit AbstractItem(QWidget *parent = nullptr);
    virtual void updateData(const QJsonObject &obj = QJsonObject()) = 0;

    virtual int itemHeight();
    const static int oneItemHeight{60};
protected:
    QJsonObject data;
    QHBoxLayout *layout_main;

};

#endif // ABSTRACTITEM_H
