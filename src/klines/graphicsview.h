#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>


class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    GraphicsView();
    virtual void drawBackground(QPainter *p, const QRectF &crect) override;
    void drawKlines(const QJsonArray &arr);
private:
    int cellSize;
};

#endif // GRAPHICSVIEW_H
