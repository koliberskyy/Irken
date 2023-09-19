#include "requests.h"

Requests::Requests(QObject *parent)
    : QObject{parent}
{

}

void Requests::get(const QUrl &url)
{
    manager.get(QNetworkRequest(url));
}

void Requests::post(const QUrl &url, QByteArray *data)
{
    manager.post(QNetworkRequest(url), *data);
}

void Requests::connect_manager(Requests *requests)
{
    connect(&requests->manager, SIGNAL(finished(QNetworkReply*)),
            requests, SLOT(replyFinished(QNetworkReply*)));

}

void Requests::replyFinished(QNetworkReply *reply)
{
    reply->deleteLater();
}
