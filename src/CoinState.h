#ifndef COIN_STATE_H
#define COIN_STATE_H

#define DEBUG_UNDEF

#include <QObject>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>


#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <vector>
#include <QString>
#include <QByteArray>
#include <iostream>
#include <QFile>
#include <mutex>

#include <memory>

#include "abstractrequests.h"
#include "config.h"

//map of comissions
inline const std::unordered_map<QString, double> comission
{
    {"TON", 0.1},
    {"NOT", 100.0},
    {"BTC", 0.00012},
    {"DOGS", 1600.0},
    {"HMSTR", 0.0}

};

namespace market
{
	inline const QString bb{"bybit"};
	inline const QString tg{"tgWallet"};
}// EOF namespace market

	struct State
	{
		double tgBuy{0.0};	// best buy price rub from tg
		double tgSell{0.0}; // best sell price rub from tg
		double bbUsdt{0.0};	// market COINUSDT pair price from bybit[another market]
        QString coin;

        auto buy()const{if (tgBuy < tgSell) return tgBuy; else return tgSell;}
        auto sell()const{if (tgBuy > tgSell) return tgBuy; else return tgSell;}
        auto mid()const{return (buy() + sell())/2;}

	};

	struct Chain
	{
		QString marketBuy;
		double priceBuy;
		QString marketSell;
		double priceSell;

        QString coin;

		double spred() const
		{
            auto reply = 100 * (priceSell - priceBuy) / priceBuy;
            return reply - 0.9;
		}
        QString toUserNative(const State &usdtState) const
		{
#ifdef DEBUG_NULL
			QString reply;
            reply.append(coin);
            reply.append(" SPRED: ");
			reply.append(QString::fromStdString(std::to_string(spred())));
			reply.append(".\n");

			reply.append("Buy on  ");
			reply.append(marketBuy);
			reply.append(".\nWith price - ");
			reply.append(QString::fromStdString(std::to_string(priceBuy)));
			reply.append(" RUB\n");

			reply.append("Sell on  ");
			reply.append(marketSell);
			reply.append(".\nWith price - ");
			reply.append(QString::fromStdString((std::to_string(priceSell))));
            reply.append(" RUB\n");

			return std::move(reply);
#else
            QString reply;
            reply.append(coin);


            if(marketBuy == market::bb)
            {
                auto buySellRangePC = 100 * (usdtState.sell() - usdtState.buy()) / usdtState.buy();

                reply.append(" Спред \nот: ");
                reply.append(QString::fromStdString(std::to_string(spred() - buySellRangePC)));
                reply.append(" %\n");
                reply.append("до: ");
                reply.append(QString::fromStdString(std::to_string(spred())));
                reply.append(" %\n");

                reply.append("1. Купи USDT в tgWallet\n(Как можно дешевле)\nот:\n\t");
                reply.append(QString::fromStdString(std::to_string(usdtState.sell())));
                reply.append(" RUB\n");
                reply.append("до:\n\t");
                reply.append(QString::fromStdString(std::to_string(usdtState.buy())));
                reply.append(" RUB\n");
                reply.append("2. Перведи USDT на ");
                reply.append(marketBuy);
                reply.append("\n3. Купи ");
                reply.append(coin);
                reply.append(" на все USDT");
                reply.append("\n4. Переведи ");
                reply.append(coin);
                reply.append(" на ");
                reply.append(marketSell);
                reply.append("\n5. Продай за:\n\t");
                reply.append(QString::fromStdString(std::to_string(priceSell)));
                reply.append(" RUB\n");

                reply.append("И поторопись, эта связка зависит от скорости.\n\n");
                reply.append("Прибыль с 10к:\nот:\n");
                reply.append(getVihlop(10000, spred() - buySellRangePC));
                reply.append(" RUB\n");
                reply.append("до:\n");
                reply.append(getVihlop(10000, spred()));
                reply.append(" RUB\n");

                reply.append("-комиссия за перевод:\n");
                reply.append(getTransferComission(coin, priceBuy));
                reply.append(" RUB\n");
            }
            else {
                reply.append(" Спред: ");
                reply.append(QString::fromStdString(std::to_string(spred())));
                reply.append(" %\n");
            }

            if(marketSell == market::bb)
            {
                reply.append("\n1. Купи ");
                reply.append(coin);
                reply.append(" в tgWallet дешевле:\n\t");
                reply.append(QString::fromStdString(std::to_string(priceBuy)));
                reply.append(" RUB\n");
                reply.append("2. Перведи и продай ");
                reply.append(coin);
                reply.append(" на ");
                reply.append(marketSell);
                reply.append("\n3. Переведи USDT на ");
                reply.append(marketBuy);
                reply.append("\n4. Продай USDT по:\n\t");
                reply.append(QString::fromStdString(std::to_string(usdtState.sell())));
                reply.append(" RUB (или дороже)\n\n");

                reply.append("Прибыль с 10к:\n");
                reply.append(getVihlop(10000, spred()));
                reply.append(" RUB\n");
                reply.append("-комиссия за перевод:\n");
                reply.append(getTransferComission(coin, priceBuy));
                reply.append(" RUB\n");

            }
            if(marketSell == marketBuy)
            {
                reply.append("\n1. Купи ");
                reply.append(coin);
                reply.append(" в tgWallet дешевле:\n\t");
                reply.append(QString::fromStdString(std::to_string(priceBuy)));
                reply.append(" RUB\n");
                reply.append("2. Продай ");
                reply.append(coin);
                reply.append(" на ");
                reply.append(marketSell);
                reply.append(" дороже:\n\t");
                reply.append(QString::fromStdString(std::to_string(priceSell)));
                reply.append(" RUB\n\n");
                reply.append("Прибыль с 10к:\n");
                reply.append(getVihlop(10000, spred()));
                reply.append(" RUB\n");
            }

            reply.append("_________\n");

            return std::move(reply);
#endif

		}

        QString getVihlop(double margin, double spred, double comission = 0) const
        {
            return  QString::fromStdString(std::to_string(  (margin * spred / 100) - comission  ));
        }
private:

        QString getTransferComission(const QString &coin, double priceRUB) const
        {
            auto usdt = comission.at(coin);
            if(usdt <= 0)
            {
                return "0";
            }
            return QString::fromStdString(std::to_string(usdt * priceRUB));
        }

    };//EOF CHAIN STRUCT

class CoinState : public AbstractRequests
{
    Q_OBJECT

	const QString coinName;
    State state;

    std::shared_ptr<QString> authToken;
    std::mutex tokenGenMutex;
    std::shared_ptr<bool> tokenUpdated;

	void parce_tgOrders(QJsonObject &&orders)
	{
        //set tgBuy || tgSell
        if(orders["status"] == "SUCCESS" ){
            auto data = orders["data"].toArray();
            if(data.size() > 3){
                auto type = data.begin()->toObject()["type"].toString();
                double midPrice{0.0};
                int ordersCount{0};
                auto it = data.cbegin();
                it++;it++;
                for(;it != data.cend(); it++)
                {
                    auto obj =  it->toObject();
                    auto price = obj["price"].toObject()["value"].toString().toDouble();
                    auto minLimitRub = obj["orderVolumeLimits"].toObject()["min"].toString().toDouble() * price;
                    auto maxLimitRub = obj["orderVolumeLimits"].toObject()["max"].toString().toDouble() * price;
                    auto paymentMethods = obj["paymentMethods"].toArray();
                    auto paymentMethodGood{true};
                    for(const auto pay : paymentMethods){
                        auto payObj = pay.toObject();
                        auto code = payObj["code"];
                        if(code == "payeer" || code == "yoomoney" || code == "webmoney"){
                            paymentMethodGood = false;
                            break;
                        }
                    }
                    if(minLimitRub < 25'000 && maxLimitRub < 200'000 && paymentMethodGood)
                    {
                        midPrice += price;
                        ordersCount++;
                    }
                }

                midPrice /= ordersCount;

                if(type == "SALE")
                {
                    state.tgSell = midPrice;
                }
                else if(type == "PURCHASE")
                {
                    state.tgBuy = midPrice;
                }
            }
        }
        else{
            std::cout << coinName.toStdString() + " empty reply from tg...\n";
        }
	}

    void parce_bbKline(QJsonObject&& allKlines)
	{
        auto kline = allKlines["result"].toObject()["list"].toArray().begin()->toArray();
        //auto kline = allKlines["data"].toArray().begin()->toArray();

        if(!kline.isEmpty()){
            state.bbUsdt = kline[4].toString().toDouble();
            //state.bbUsdt = kline[2].toString().toDouble();
        }
	}


public:
    CoinState(const QString &coin, std::shared_ptr<QString> auth, std::shared_ptr<bool> tokenUpdTrg, std::shared_ptr<QNetworkAccessManager> manager = nullptr) :
		AbstractRequests(manager),
        coinName{coin},
        authToken{auth},
        tokenUpdated{tokenUpdTrg}
    {
        state.coin = coin;
	}

    void getAuthToken()
    {
        tokenGenMutex.lock();

        if(!(*tokenUpdated)){
                if(system(". venv_dir/bin/activate && python3.12 main.py") == 0)
                    *tokenUpdated = true;
            }

            QFile inputFile("authToken.irken");
            if (inputFile.open(QIODevice::ReadOnly))
            {
                QTextStream in(&inputFile);
                while (!in.atEnd())
                {
                    QString line = in.readLine();
                    *authToken = line;
                }
                inputFile.close();
            }
        tokenGenMutex.unlock();

    }

	void clear()
	{
		state.tgBuy = 0.0;
		state.tgSell = 0.0;
		state.bbUsdt = 0.0;
	}

	bool isUpdated() const
	{
		if(coinName != "USDT")
			return 	state.tgBuy != 0 &&
					state.tgSell != 0 &&
					state.bbUsdt != 0;
		else
			return	state.tgBuy != 0 &&
					state.tgSell !=0;
	}

	State getState() const
	{
		return state;
	}

	QString getCoinName() const
	{
		return coinName;
	}

    std::vector<Chain> generateChain (const State &state, const State &usdtState = State()) const
	{
		std::vector<Chain> reply;

        reply.emplace_back(Chain{	market::tg, state.buy(),
                                 market::tg, state.sell(), getCoinName()});

		if(state.bbUsdt != 0.0)
		{
            reply.emplace_back(Chain{	market::tg, state.buy(),
                                     market::bb, state.bbUsdt * usdtState.sell(), getCoinName()} );

            reply.emplace_back(Chain{	market::bb, state.bbUsdt * usdtState.buy(),
                                     market::tg, state.sell(), getCoinName()});
		}

		return std::move(reply);
	}

    std::vector<Chain> getChain(const State &usdtState) const
    {
        return generateChain(state, usdtState);
    }

public slots:
	void upd()
	{
		this->clear();
		updateTgBuy();
		updateTgSell();
		
		if(coinName != "USDT")
			updateBB();
	}

private:
	void updateTgBuy()
    {
        if(coinName == "USDT"){
            getAvaibleTgOrders("RUB", coinName, "PURCHASE", "20");
        }
        else{
            getAvaibleTgOrders("RUB", coinName, "PURCHASE", "10");
        }
	}
	
	void updateTgSell()
	{
        if(coinName == "USDT"){
            getAvaibleTgOrders("RUB", coinName, "SALE", "20");
        }
        else{
            getAvaibleTgOrders("RUB", coinName, "SALE", "10");
        }
	}

	void updateBB()
	{
        downloadKlines(coinName + "USDT", "5", "1");
        //downloadKlines(coinName + "-USDT", "5", "1");

	}

protected slots:
    virtual void parceNetworkReply(	QNetworkRequest request,
                                    QNetworkReply::NetworkError error,
                                    QByteArray reply) final
	{

        auto url = request.url();
        auto path = url.path();
        auto doc = QJsonDocument::fromJson(std::move(reply));

		if(error == QNetworkReply::NoError)
		{
            if(path == "/p2p/public-api/v2/offer/depth-of-market")
			{
                tokenGenMutex.lock();
                if((*tokenUpdated))
                    *tokenUpdated = false;
                tokenGenMutex.unlock();


                parce_tgOrders(doc.object());
                if(this->isUpdated())
                {
#ifdef DEBUG

                    std::cout << coinName.toStdString() + " has updated...\n";
#endif
                    emit updatingComplete();
                }
			}

            else if(path == "/v5/market/kline")
            //else if(path == "/api/v1/market/candles")
            {
                parce_bbKline(doc.object());
                if(this->isUpdated())
                {
#ifdef DEBUG

                    std::cout << coinName.toStdString() + " has updated...\n";
#endif
                    emit updatingComplete();
                }
			}
		}
        else
        {
#ifdef DEBUG

            QString message("request error - code: ");
            lsendGet("api.telegram.org",
                         config::tgBotToken() + "/sendMessage",
                         "chat_id=-1001964821237&text=" + message);
#endif

            getAuthToken();
        }
	}

    /*
        :param desired_amount:
        :param base_currency_code: TON | BTC
        :param quote_currency_code: RUB | KZT etc.
        :param offer_type: SALE | PURCHASE
        :param limit:
    */
	void getAvaibleTgOrders(	const QString &baseCoin, 
								const QString &quoteCoin, 
								const QString &offerType, 
								const QString &limit) 
	{
		QJsonObject dataObj;
        dataObj.insert("baseCurrencyCode", quoteCoin);
        dataObj.insert("quoteCurrencyCode", baseCoin);
        dataObj.insert("offerType", offerType);
        dataObj.insert("limit", limit.toInt());
        dataObj.insert("offset", 0);
        //dataObj.insert("desiredAmount", 0);



        //QString authToken("Bearer " + config::tgAuthToken());
        QString token("Bearer ");
        token.append(*authToken);

        std::vector<std::pair<QByteArray, QByteArray>> headers;
        headers.emplace_back(std::make_pair<QByteArray, QByteArray>("content-type", "application/json"));
        headers.emplace_back(std::make_pair<QByteArray, QByteArray>("authorization", token.toUtf8()));

        //'https://walletbot.me/p2p/public-api/v2/offer/depth-of-market'
        sendPost(	"walletbot.me",
                    "/p2p/public-api/v2/offer/depth-of-market",
                    QJsonDocument(dataObj).toJson(QJsonDocument::Compact),
					"https",
					headers);
	}

	void downloadKlines(const QString &symbol, const QString &interval, const QString &limit)
	{

        QString query(	"category=spot&symbol=" + symbol+
                        "&interval=" + interval+
                        "&limit=" +limit);

        sendGet("api.bybit.com", "/v5/market/kline", std::move(query));
        // QString query(	"category=spot&symbol=" + symbol+
        //                 "&type=" +"5min");

        //       sendGet("api.kucoin.com", "/api/v1/market/candles", std::move(query));
	}
signals:
	void updatingComplete();
};
#endif
