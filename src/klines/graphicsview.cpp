#include "graphicsview.h"


GraphicsView::GraphicsView() : cellSize(20)
{
    setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);



    auto scene =  new QGraphicsScene(this);

    setScene(scene);

    scene->addRect(QRect(0, 0, 10, 10), QPen(Qt::green), QBrush(Qt::green));

    //drawBackground(new QPainter(), QRect(0,0,width(),height()));
    scene->setSceneRect(0,0,width(),height());

    QJsonObject obj;
    obj.insert("hui", "piza");


}

void GraphicsView::drawBackground(QPainter *p, const QRectF &crect)
{
    p->save();
    p->setPen(QPen(Qt::black,1));

    for (int x = (int)crect.left()/cellSize*cellSize; x < crect.right(); x += cellSize)
    for (int y = (int)crect.top()/cellSize*cellSize; y < crect.bottom(); y += cellSize)
    p->drawPoint(x, y);

    p->restore();
}

void GraphicsView::drawKlines(const QJsonArray &arr)
{
    auto a = arr;
    //1707163200000
    //1707134400000
    //1707105600000
    //scene()->setSceneRect(arr.at(arr.size()-1).toArray().at(0).toString().toDouble(), 40000, arr.at(arr.size()-1).toArray().at(0).toString().toDouble() - arr.at(0).toArray().at(0).toString().toDouble(), 15000);
    for(auto &it : arr){
        auto x1 = it.toArray()[0].toString().toDouble() / 10000 - 17070000;
        auto y1 = it.toArray()[1].toString().toDouble() - 42000;
        auto y2 = it.toArray()[4].toString().toDouble() - 42000;
        scene()->addLine(x1, y1, x1, y2,
                 QPen(Qt::black,100));
    }

    update();

    auto breakpoint = false;
}


