#include "requests.h"

QJsonObject Requests::get(const QString &method, const QByteArray &query, const QString &userInfo, int recWindow_msec, std::function<QJsonObject (const QUrl &, const QByteArray &)> customReplyErrorParser)
{
    QUrl url(method + query);

    url.setUserInfo(userInfo);

    QNetworkRequest req(url);
    return get(req, recWindow_msec, customReplyErrorParser);
}

QJsonObject Requests::get(const QString &method, const QByteArray &query, const QString &api, const QString &secret, const QString &userInfo, int recWindow_msec, std::function<QJsonObject (const QUrl &, const QByteArray &)> customReplyErrorParser)
{
    QUrl url(method + query);

    url.setUserInfo(userInfo);

    QNetworkRequest req(url);

    if(!api.isEmpty() && !secret.isEmpty()){
        auto headers = Private::make_headers(query, api.toUtf8(), secret.toUtf8());
        for(auto &it : headers){
            req.setRawHeader(it.first, it.second);
        }
    }
    else{
        std::cout << "request error: missing api or secret";
    }

    return get(req, recWindow_msec, customReplyErrorParser);
}

QJsonObject Requests::get(QNetworkRequest req, int recWindow_msec, std::function<QJsonObject (const QUrl &, const QByteArray &)> customReplyErrorParser)
{
    QEventLoop loop;
    QTimer timer;
    QNetworkAccessManager mgr;

    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(recWindow_msec);
    QNetworkReply *reply = mgr.get(req);
    loop.exec();

    auto obj = customReplyErrorParser(req.url(), reply->readAll());

    reply->deleteLater();

    return std::move(obj);
}

std::vector<std::pair<QByteArray, QByteArray> > Requests::Private::make_headers(const QByteArray &data, const QByteArray &api_key, const QByteArray &secret_key)
{
    auto ts = timestamp();
    return std::vector<std::pair<QByteArray, QByteArray> >( {
                                                                (std::pair<QByteArray, QByteArray>("X-BAPI-SIGN", make_signature(data, ts, api_key, secret_key))),
                                                                (std::pair<QByteArray, QByteArray>("X-BAPI-API-KEY", api_key)),
                                                                (std::pair<QByteArray, QByteArray>("X-BAPI-TIMESTAMP", std::move(ts))),
                                                                (std::pair<QByteArray, QByteArray>("X-BAPI-RECV-WINDOW", settings::recv_window))
                                                            });
}

QByteArray Requests::Private::make_signature(const QByteArray &data, const QByteArray &ts, const QByteArray &api_key, const QByteArray &secret_key)
{
    return QMessageAuthenticationCode::hash(ts + api_key + settings::recv_window + data, secret_key, QCryptographicHash::Sha256).toHex();
}

QByteArray Requests::Private::timestamp()
{
    return  QByteArray::fromStdString(std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()));//ахуеть
}


QJsonObject Requests::post(QNetworkRequest req, const QByteArray &data, int recWindow_msec, std::function<QJsonObject (const QUrl &, const QByteArray &, const QByteArray &)> customReplyErrorParser)
{
    QEventLoop loop;
    QTimer timer;
    QNetworkAccessManager mgr;

    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(recWindow_msec);
    QNetworkReply *reply = mgr.post(req, data);
    loop.exec();

    auto obj = customReplyErrorParser(req.url(), data, reply->readAll());

    reply->deleteLater();

    return std::move(obj);
}

QJsonObject Requests::post(const QString &method, const QByteArray &data, const QString &api, const QString &secret, const QString &userInfo, int recWindow_msec, std::function<QJsonObject (const QUrl &, const QByteArray &, const QByteArray &)> customReplyErrorParser)
{
    QUrl url(method);

    url.setUserInfo(userInfo);

    QNetworkRequest req(url);

    if(!api.isEmpty() && !secret.isEmpty()){
        auto headers = Private::make_headers(data, api.toUtf8(), secret.toUtf8());
        for(auto &it : headers){
            req.setRawHeader(it.first, it.second);
        }
    }
    else{
        std::cout << "request error: missing api or secret";
    }

    return post(req, data, recWindow_msec, customReplyErrorParser);
}
