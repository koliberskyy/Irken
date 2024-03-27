#ifndef REQUESTS_H
#define REQUESTS_H

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include <QMessageAuthenticationCode>

#include "instrumentsinfo.h"
#include "settings.h"

namespace Requests {

    QJsonObject get(const QString &method,
                              const QByteArray &query,
                              const QString &userInfo = "",
                              int recWindow_msec = settings::recv_window.toInt(),
                              std::function<QJsonObject (const QUrl &, const QByteArray &)> customReplyErrorParser = [](const QUrl &url, const QByteArray &data) ->QJsonObject
        {
            auto obj = QJsonDocument::fromJson(data).object();
            auto retCode = obj["retCode"].toInt();
            if(retCode != 0 || data.isEmpty()){
                instruments::replyError(url, data);
                return QJsonObject();
            }
            return std::move(obj);
        });

    QJsonObject get(const QString &method,
                              const QByteArray &query,
                              const QString &api,
                              const QString &secret,
                              const QString &userInfo = "",
                              int recWindow_msec = settings::recv_window.toInt(),
                              std::function<QJsonObject (const QUrl &, const QByteArray &)> customReplyErrorParser = [](const QUrl &url, const QByteArray &data) ->QJsonObject
        {
            auto obj = QJsonDocument::fromJson(data).object();
            auto retCode = obj["retCode"].toInt();
            if(retCode != 0 || data.isEmpty()){
                instruments::replyError(url, data);
                return QJsonObject();
            }
            return std::move(obj);
        });
    QJsonObject get(QNetworkRequest req, int recWindow_msec,
                              std::function<QJsonObject (const QUrl &, const QByteArray &)> customReplyErrorParser = [](const QUrl &url, const QByteArray &data) ->QJsonObject
        {
            auto obj = QJsonDocument::fromJson(data).object();
            auto retCode = obj["retCode"].toInt();
            if(retCode != 0 || data.isEmpty()){
                instruments::replyError(url, data);
                return QJsonObject();
            }
            return std::move(obj);
        });


    QJsonObject post(QNetworkRequest req, const QByteArray &data, int recWindow_msec = settings::recv_window.toInt(),
                     std::function<QJsonObject (const QUrl &, const QByteArray &, const QByteArray &)> customReplyErrorParser = [](const QUrl &url, const QByteArray &data, const QByteArray &reply) ->QJsonObject
        {
           auto obj = QJsonDocument::fromJson(reply).object();
           auto retCode = obj["retCode"].toInt();
           if(retCode != 0 || reply.isEmpty()){
               instruments::replyError(url, data, reply);
               return QJsonObject();
           }
           return std::move(obj);
        });
    QJsonObject post(const QString &method,
                     const QByteArray &data,
                     const QString &api,
                     const QString &secret,
                     const QString &userInfo = "",
                     int recWindow_msec = settings::recv_window.toInt(),
                                std::function<QJsonObject (const QUrl &, const QByteArray &, const QByteArray &)> customReplyErrorParser = [](const QUrl &url, const QByteArray &data, const QByteArray &reply) ->QJsonObject
                   {
                      auto obj = QJsonDocument::fromJson(reply).object();
                      auto retCode = obj["retCode"].toInt();
                      if(retCode != 0 || reply.isEmpty()){
                          instruments::replyError(url, data, reply);
                          return QJsonObject();
                      }
                      return std::move(obj);
                   });

    namespace Private{
        std::vector<std::pair<QByteArray, QByteArray> > make_headers(const QByteArray &data, const QByteArray &api_key, const QByteArray &secret_key);
        QByteArray make_signature(const QByteArray &data, const QByteArray &ts, const QByteArray &api_key, const QByteArray &secret_key);
        QByteArray timestamp();
    }
}

#endif // REQUESTS_H
