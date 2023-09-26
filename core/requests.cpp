#include "requests.h"

Requests::Requests(QObject *parent)
    : QObject{parent}
{

}

void Requests::get(const QUrl &url)
{
    manager.get(QNetworkRequest(url));
}

void Requests::post(const QUrl &url, const QString &data)
{
    manager.post(QNetworkRequest(url), data.toUtf8());
}

void Requests::connect_manager(Requests *requests)
{
    connect(&requests->manager, SIGNAL(finished(QNetworkReply*)),
            requests, SLOT(replyFinished(QNetworkReply*)));

}

void Requests::replyFinished(QNetworkReply *reply)
{
    QString answer = QString::fromUtf8(reply->readAll());

    std::cout << answer.toStdString() << '\n';

    reply->deleteLater();
}
