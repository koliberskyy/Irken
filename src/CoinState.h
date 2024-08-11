#ifndef COIN_STATE_H
#define COIN_STATE_H

#define DEBUG

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

#include <memory>

#include "abstractrequests.h"
#include "config.h"

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
	};

	struct Chain
	{
		QString marketBuy;
		double priceBuy;
		QString marketSell;
		double priceSell;

		double spred() const
		{
			if(marketSell == market::tg)
				return 0.991 * 100 * (priceSell - priceBuy) / priceBuy;

			return 100 * (priceSell - priceBuy) / priceBuy;
		}
		QString toUserNative() const
		{
			QString reply;
			reply.append("SPRED - ");
			reply.append(QString::fromStdString(std::to_string(spred())));
			reply.append(".\n");

			reply.append("Buy on  ");
			reply.append(marketBuy);
			reply.append(".\nWith price - ");
			reply.append(QString::fromStdString(std::to_string(priceBuy)));
			reply.append("\n");

			reply.append("Sell on  ");
			reply.append(marketSell);
			reply.append(".\nWith price - ");
			reply.append(QString::fromStdString((std::to_string(priceSell))));
			reply.append("\n");

			return std::move(reply);
		}
	};

class CoinState : public AbstractRequests
{
    Q_OBJECT

	const QString coinName;
	State state; 	
	
	void parce_tgOrders(QJsonObject &&orders)
	{
		//set tgBuy || tgSell
		state.tgBuy = 1.0;
		state.tgSell = 10.0;
	}

	void parce_bbKline(QJsonArray&& allKlines)
	{
		// set bbUsdt
		state.bbUsdt = 0.001345;
	}


public:
	CoinState(const QString &coin, std::shared_ptr<QNetworkAccessManager> manager = nullptr) : 
		AbstractRequests(manager),
		coinName{coin}
	{
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

	std::vector<Chain> getChain(const State &usdtState) const 
	{
		return generateChain(this->state, usdtState);
	}

	static std::vector<Chain> generateChain (const State &state, const State &usdtState = State())
	{
		std::vector<Chain> reply;

		reply.emplace_back(Chain{	market::tg, state.tgBuy, 
									market::tg, state.tgSell});

		if(state.bbUsdt != 0.0)
		{
			reply.emplace_back(Chain{	market::tg, state.tgBuy, 
										market::bb, state.bbUsdt * usdtState.tgSell});
		
			reply.emplace_back(Chain{	market::bb, state.bbUsdt * usdtState.tgBuy, 
										market::tg, state.tgSell});
		}

		return std::move(reply);
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
		getAvaibleTgOrders(QString("RUB"), coinName, QString("PRUSHARE"), "10");
	}
	
	void updateTgSell()
	{
		getAvaibleTgOrders(QString("RUB"), coinName, QString("SELL"), "10");
	}

	void updateBB()
	{
		downloadKlines(coinName + "USDT", "5", "1");
	}

protected slots:
    virtual void parceNetworkReply(	QNetworkRequest request,
                                    QNetworkReply::NetworkError error,
                                    QByteArray reply) final
	{

        auto url = request.url();
        auto path = url.path();
        auto doc = QJsonDocument::fromJson(std::move(reply));
        auto headers = request.rawHeaderList();
        auto header1 = request.rawHeader(headers.at(0));
        auto header2 = request.rawHeader(headers.at(1));

		if(error == QNetworkReply::NoError)
		{
			if(path == "/p2p/public-api/v2/offer/depth-of-market?")
			{
				parce_tgOrders(doc.object()["data"].toObject());
			}

			else if(path == "/v5/market/kline?")
			{
				parce_bbKline(doc.array());
			}
		}

		if(this->isUpdated())
		{
#ifdef DEBUG

	std::cout << coinName.toStdString() + " has updated...\n";
#endif
			emit updatingComplete();
		}
	}
	
	void getAvaibleTgOrders(	const QString &baseCoin, 
								const QString &quoteCoin, 
								const QString &offerType, 
								const QString &limit) 
	{
		QJsonObject dataObj;
   		dataObj.insert("baseCurrencyCode", quoteCoin);
   		dataObj.insert("quoteCurrencyCode", baseCoin);
		dataObj.insert("offerType", offerType);
		dataObj.insert("limit", limit);

        QString authToken(config::tgAuthToken());

        std::vector<std::pair<QByteArray, QByteArray>> headers;
			headers.push_back(std::make_pair<QByteArray, QByteArray>("content-type", "application/json"));
            headers.push_back(std::make_pair<QByteArray, QByteArray>("authorization", (authToken.toUtf8())));
		sendPost(	"walletbot.me", 
					"/p2p/public-api/v2/offer/depth-of-market?", 
					QJsonDocument(dataObj).toJson(),
					"https",
					headers);
	}

	void downloadKlines(const QString &symbol, const QString &interval, const QString &limit)
	{
		QString query(	"category=spot&symbol=" + symbol+ 
						"&interval=" + interval+ 
						"&limit=" +limit);

		sendGet("api.bybit.com", "/v5/market/kline?", std::move(query));
	}
signals:
	void updatingComplete();
};
#endif
