#include "binanceapiclient.h"
namespace binance{

BinanceAPIClient::BinanceAPIClient(const QString &__api_key, const QString &__secret_key):
    api_key{__api_key}, secret_key{__secret_key}
    {
        connect_manager(this);
        spy = std::make_unique<QSignalSpy>(this, SIGNAL(reply_complete()));
        buffer = std::make_unique<QByteArray>();
}




//*********************************METHODS***********************************

/*
 * account()
GET /fapi/v2/account?

Parameters:
Name		Type	Mandatory	Description
recvWindow	LONG	NO
timestamp	LONG	YES
*/
QByteArray *BinanceAPIClient::account()
{
    QString url = binance::FAPI_HOST + "/fapi/v2/account?";
    QString data;
    data.append(timestamp());
    data.append(signature_sha256(data, secret_key));

    url.append(data);
    get(url);

    return read_buffer();
}

/*
 * post_order(...)
POST /fapi/v1/order?

Name				Type		Mandatory	Description
symbol				STRING		YES
side				ENUM		YES
type				ENUM		YES
timeInForce			ENUM		YES
quantity			DECIMAL		YES
price				DECIMAL		YES
newClientOrderId    STRING		NO		A unique id for the order. Automatically generated by default.
stopPrice			DECIMAL		NO		Used with STOP orders
positionSide        ENUM        NO      надо разобраться что за ерунда

*/
    QByteArray *BinanceAPIClient::post_order(	const QString &symbol,
                                        const QString &side,
                                        const QString &type,
                                        const QString &timeInForce,
                                        const QString &quantity,
                                        const QString &price,
                                        const QString &newClientOrderId = "",
                                        const QString &stopPrice = "",
                                        const QString &positionSide = ""
                                        )
    {
        const QString url = binance::FAPI_HOST + "/fapi/v1/order?";
        QString data;

        data.append("symbol=");
        data.append(symbol);

        if(!side.isEmpty()){
            data.append("&side=");
            data.append(side);
        }
        if(!type.isEmpty()){
            data.append("&type=");
            data.append(type);
        }
        if(!quantity.isEmpty()){
            data.append("&quantity=");
            data.append(quantity);
        }
        if(!price.isEmpty()){
            data.append("&price=");
            data.append(price);
        }
        if(!timeInForce.isEmpty()){
            data.append("&timeInForce=");
            data.append(timeInForce);
        }
        if(!newClientOrderId.isEmpty()){
            data.append("&newClientOrderId=");
            data.append(newClientOrderId);
        }
        if(!stopPrice.isEmpty()){
            data.append("&stopPrice=");
            data.append(stopPrice);
        }
        if(!positionSide.isEmpty()){
            data.append("&positionSide=");
            data.append(positionSide);
        }

        data.append(timestamp());
        data.append(signature_sha256(data, secret_key));

        post(url, data);

        return read_buffer();
    }
    //*********************************
    /*
     * balances()
    GET /fapi/v2/balance?

    Name				Type		Mandatory	Description
    -                   -           -           -

    */
    QByteArray* BinanceAPIClient::balances()
    {
        QString url = binance::FAPI_HOST + "/fapi/v2/balance?";
        QString data;
        //data.append("&recvWindow=100000");
        data.append(timestamp());
        data.append(signature_sha256(data, secret_key));

        url.append(data);
        get(url);

        return read_buffer();
    }

    QByteArray *BinanceAPIClient::positionRisk()
    {
        QString url = binance::FAPI_HOST + "/fapi/v2/positionRisk?";
        QString data;
        //data.append("&recvWindow=100000");

        data.append(timestamp());
        data.append(signature_sha256(data, secret_key));

        url.append(data);
        get(url);

        return read_buffer();
    }

    QByteArray *BinanceAPIClient::positionRisk(const QString &symbol)
    {
        QString url = binance::FAPI_HOST + "/fapi/v2/positionRisk?";
        QString data;
        //data.append("&recvWindow=100000");

        data.append("symbol=");
        data.append(symbol);

        data.append(timestamp());
        data.append(signature_sha256(data, secret_key));

        url.append(data);
        get(url);

        return read_buffer();
    }

    //------------
    /*
     * cancel_order(...)
    DELETE /fapi/v1/order?

    symbol				STRING	YES
    orderId				LONG	NO
    origClientOrderId	STRING	NO
    newClientOrderId	STRING	NO	Used to uniquely identify this cancel. Automatically generated by default.
    recvWindow			LONG	NO
    timestamp			LONG	YES

    */
    QByteArray *BinanceAPIClient::delete_order(const QString &symbol, const QString &origClientOrderId)
    {
        QString url = binance::FAPI_HOST + "/fapi/v1/order?";
        QString data;

        data.append("&symbol=");
        data.append(symbol);

        if(!origClientOrderId.isEmpty()){
            data.append("&origClientOrderId=");
            data.append(origClientOrderId);
        }
        data.append(timestamp());
        data.append(signature_sha256(data, secret_key));

        url.append(data);
        delete_request(url);
        //get(url);
        return read_buffer();
    }

    QString BinanceAPIClient::normalize_digit(QString digit)
    {
       QString result;
        if(!digit.isEmpty()){
           //поиск точки
            int i{0};
                do{
                    if(i == digit.size())
                        return result;
                    result.append(digit.at(i));
                    i++;
            }while(digit.at(i) != ',');
            //если точки нет, добавляем точку
            if(i == digit.size()){
                result.append('.');
                //добавляем нули
                for(int j = 0; j < 8; j++){
                    result.append('0');
                }
            }
            else{
                //запомнили позицию точки
                result.append('.');
                int point = i;
                i++;
                //добавляем нули
                for(; i < point + 9; i++){
                    if(i == digit.size()){
                        digit.append('0');
                    }
                    result.append(digit.at(i));
                }
            }
       }
        return result;
    }


//*********************************METHODS_END***********************************
    void BinanceAPIClient::get(const QUrl &url)
    {
        QNetworkRequest req(url);
        req.setRawHeader(QByteArray("X-MBX-APIKEY"), api_key.toUtf8());
        manager.get(req);
        spy->wait();
    }

    void BinanceAPIClient::post(const QUrl &url, const QString &data)
    {
        QNetworkRequest req(url);
        req.setRawHeader(QByteArray("X-MBX-APIKEY"), api_key.toUtf8());
        manager.post(req, data.toUtf8());
        spy->wait();
    }

    void BinanceAPIClient::delete_request(const QString &url)
    {
        QNetworkRequest req(url);
        req.setRawHeader(QByteArray("X-MBX-APIKEY"), api_key.toUtf8());
        manager.deleteResource(req);
        spy->wait();
    }

    void BinanceAPIClient::replyFinished(QNetworkReply *reply)
    {
        reply->deleteLater();

        if(!buffer->isEmpty())
            buffer->clear();

        buffer->append(reply->readAll());
        emit reply_complete();
    }

    QString BinanceAPIClient::timestamp()
    {
        return "&timestamp=" + QString::fromStdString(std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()));//ахуеть
    }

    QString BinanceAPIClient::signature_sha256(const QString &message, const QString &key)
    {
        QString return_value{"&signature="};
        auto hash = QMessageAuthenticationCode::hash(message.toUtf8(), key.toUtf8(), QCryptographicHash::Sha256).toHex();
        return_value.append(hash);
        return return_value;
    }

}//namespace binance
