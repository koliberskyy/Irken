#ifndef BINANCEAPICLIENT_H
#define BINANCEAPICLIENT_H

#include <requests.h>
#include <QMessageAuthenticationCode>
#include <QSignalSpy>
#include <memory>
#include "key.h"
#include "source.h"
namespace binance{

    class BinanceAPIClient : public Requests
    {
        Q_OBJECT
    public:
        explicit BinanceAPIClient(const QString &__api_key, const QString &__secret_key);
        ~BinanceAPIClient(){;}//для удаления неявносоздаваемых конструкторов

        //methods
        QByteArray* account();
        QByteArray* post_order(const QString &symbol,
                        const QString &side,
                        const QString &type,
                        const QString &timeInForce,
                        const QString &quantity,
                        const QString &price,
                        const QString &newClientOrderId,
                        const QString &stopPrice,
                        const QString &positionSide);
        QByteArray* balances();
        QByteArray* positionRisk();
        QByteArray* positionRisk(const QString &symbol);
        QByteArray* delete_order(const QString &symbol, const QString &origClientOrderId);
        QString normalize_digit(QString digit);
        QString normalize_qty(QString qty, const QString &symbol);
    public slots:
        virtual void replyFinished(QNetworkReply* reply) override;
    signals:
        void reply_complete();
    protected:
        //http
        void get(const QUrl &url) final;
        void post(const QUrl &url,  const QString &data) final;
        void delete_request(const QString &url);
    private:
        const QString api_key;
        const QString secret_key;
        std::unique_ptr<QByteArray> buffer;
        std::unique_ptr<QSignalSpy> spy;
        QString timestamp();
        QString signature_sha256(const QString &message, const QString &key);
    public:
        auto read_buffer() const{
            return buffer.get();
        }
    };

}//namespace binance
#endif // BINANCEAPICLIENT_H
