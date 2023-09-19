#ifndef REQUESTS_H
#define REQUESTS_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>

class Requests : public QObject
{
    Q_OBJECT
public:
    explicit Requests(QObject *parent = nullptr);
    void get(const QUrl &url);
    void post(const QUrl &url, QByteArray *data);
    void connect_manager(Requests *requests);
private slots:
    virtual void replyFinished(QNetworkReply* reply);

protected:
    QNetworkAccessManager manager;
signals:

};

#endif // REQUESTS_H
