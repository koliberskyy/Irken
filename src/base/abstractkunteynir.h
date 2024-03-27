#ifndef ABSTRACTKUNTEYNIR_H
#define ABSTRACTKUNTEYNIR_H

#include <QScrollArea>
#include <QVBoxLayout>
#include <QList>

#include "abstractitem.h"

class AbstractKunteynir : public QScrollArea
{
    Q_OBJECT
public:
    explicit AbstractKunteynir(QWidget *parent);
    virtual QList<AbstractItem *>::iterator findItemPosition(AbstractItem *item);
    virtual QList<AbstractItem *>::iterator findItem(const QJsonObject &item) = 0;
    virtual QList<AbstractItem *> get_items() const;

public slots:
    virtual void addItem(AbstractItem* item);
    virtual QList<AbstractItem* >::iterator removeItem(AbstractItem* item);

    virtual void removeAllItems();

protected:
    QVBoxLayout *layout_main;
    QList<AbstractItem *> items;

private:
    QWidget *wgt;

};

#endif // ABSTRACTKUNTEYNIR_H
