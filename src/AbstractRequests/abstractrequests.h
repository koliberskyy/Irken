#ifndef ABSTRACTREQUESTS_H
#define ABSTRACTREQUESTS_H

#include <memory>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

/*
 * Обертка над QNetworkRequests действительно позволяющая использовать 1 QNetworkAccessManager на весь проект
 * Наследуем куда надо, в конструктор передаем shared_ptr на созданный QNetworkAccessManager (или нет)
*/
class AbstractRequests : public QObject
{
    Q_OBJECT
    std::shared_ptr<QNetworkAccessManager> manager;

public:
    explicit AbstractRequests(std::shared_ptr<QNetworkAccessManager> __manager = nullptr, QObject *parent = nullptr)
	: QObject(parent)
    {
        if(__manager == nullptr){
            manager = std::make_shared<QNetworkAccessManager>();
        }
        else{
            manager = __manager;
        }
    }


    //GET
    /*
     * Отправляет GET запрос, в идеале тип аргументов - QString
    */
    template<typename Host_str, typename Path_str, typename Query_str>
    void sendGet(Host_str host, Path_str path, Query_str query, QString scheme = "https")
    {
        QUrl url;

        url.setScheme(scheme);
        url.setHost(std::forward<Host_str>(host));
        url.setPath(std::forward<Path_str>(path));
        url.setQuery(std::forward<Query_str>(query));

        GET(QNetworkRequest(std::move(url)));
    }

    //POST
    /*
     * Отправляет POST запрос, в идеале тип аргументов - QString
    */
    template<typename Host_str, typename Path_str>
    void sendPost(Host_str host, Path_str path, QByteArray &&data, QString scheme = "https")
    {
        QUrl url;

        url.setScheme(scheme);
        url.setHost(std::forward<Host_str>(host));
        url.setPath(std::forward<Path_str>(path));

        POST(QNetworkRequest(std::move(url)), std::move(data));
    }

    template<typename Host_str, typename Path_str>
    void sendPost(Host_str host, Path_str path, QByteArray &&data, QString scheme,
				 const std::vector<std::pair<QByteArray, QByteArray>> &headers)
    {
        QUrl url;

        url.setScheme(scheme);
        url.setHost(std::forward<Host_str>(host));
        url.setPath(std::forward<Path_str>(path));

		QNetworkRequest request(std::move(url));
		for(const auto &it : headers)
		{
			request.setRawHeader(it.first, it.second);
		}

        POST(std::move(request), std::move(data));
    }
signals:
    /*
    * Отправляется после окончания обработки ответа от сервера
    */
    void networkReplyFinished();

protected:
    virtual void parceNetworkReply(QNetworkRequest &&request, QNetworkReply::NetworkError &&error, QByteArray &&reply) = 0;

protected slots:
    void replyFinished()
    {
        auto reply = static_cast<QNetworkReply*>(sender());
        parceNetworkReply(reply->request(), reply->error(), reply->readAll());
        reply->deleteLater();
        emit networkReplyFinished();
    }

private:
    void GET(QNetworkRequest &&request)
    {
        auto reply = manager->get(std::move(request));
        QObject::connect(reply, &QNetworkReply::finished,
                         this, &AbstractRequests::replyFinished);
    }
    
	void POST(QNetworkRequest &&request, QByteArray &&data) 
    {
        auto reply = manager->post(std::move(request), std::move(data));
        QObject::connect(reply, &QNetworkReply::finished,
                         this, &AbstractRequests::replyFinished);
    }


};

#endif // ABSTRACTREQUESTS_H
