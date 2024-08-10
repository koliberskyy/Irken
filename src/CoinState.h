

class CoinState : public QObject, public AbstractRequests
{
	Q_OBJECT
	
	const QString coinName;
	struct State{
		double tgBuy{0.0};	// best buy price rub from tg
		double tgSell{0.0}; // best sell price rub from tg
		double bbUsdt{0.0};	// market COINUSDT pair price from bybit[another market]
	};
	State state; 
public:
	CoinState(const QString &coin, std::shared_ptr<QNetworkAccessManager> manager = nullptr) : 
		AbstractRequests(manager),
		coinName{coin}
	{
	}

	void clear(bool saveName = true)
	{
		if(!saveName)
			coinName.clear();

		state.tgBuy = 0.0;
		state.tgSell = 0.0;
		state.bbUsdt = 0.0;
	}

	bool isUpdated() const
	{
		return 	state.tgBuy != 0 &&
				state.tgSell != 0 &&
				state.bbUsdt != 0;
	}

	State getState() const
	{
		return state;
	}

public slots:
	void upd()
	{
		this->clear();
		updateTgBuy();
		updateTgSell();
		updateBB();
	}

private:
	void updateTgBuy()
	{ 
		getAvaibleTgOrders("RUB", coinName, "PRUSHARE");
	}
	
	void updateTgSell()
	{
		getAvaibleTgOrders("RUB", coinName, "SELL");
	}

	void updateBB()
	{
		downloadKlines(coinName + "USDT", "5", "1");
	}

	virtual void parceNetworkReply(	QNetworkRequests &&request, 
									QNetworkReply::NetworkError &&error, 
									QByteArray &&reply) final
	{
		if(this->isUpdated())
		{
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

		std::vector<std::pair<QByteArray, QByteArray>> headers
		{
			{"content-type", "application/json"},
			{"authorization", "Bearer " + config::tgAuthToken.toUtf8()}
		};

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
