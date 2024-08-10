#ifndef IRKEN_ARBITRAGE_SCANER_H
#define IRKEN_ARBITRAGE_SCANER_H

#include "CoinState.h"

class IrkenArbitrageScaner : public QObject
{
	Q_OBJECT

	static const std::array<QString, 3> coins
	{
		"NOT",
		"TON",
		"BTC"
	};
	static const QString coinBase {"USDT"};
	std::shared_ptr<CoinState> usdtState;
	std::vector<std::shared_ptr<CoinState>> states;
	std::shared_ptr<QNetworkAccessManager> manager;

public:
	explicit IrkenArbitrageScaner(QObject *parent = nullptr):
		QObject(parent),
		manager{std::make_shared<QNetworkAccessManager>()}
	{

		usdtState = std::make_shared<CoinState>(coinBase, manager);
		QObject::connect(this, &IrkenArbitrageScaner::updateUSDT,
						usdtState.get(), &CoinState::upd);
		
		//make states
		for(const auto &it : coins)
		{
			auto tmp = std::make_shared<CoinState>(it, manager);

			QObject::connect(tmp.get(), &CoinState::updatingComplete,
							this, &IrkenArbitrageScaner::sendResult);

			QObject::connect(this, &IrkenArbitrageScaner::updatingBegins,
							tmp.get(), &CoinState::upd);

			states.push_back(tmp);
		}
	}

public slots:
	void upd()
	{

#ifdef DEBUG
		std::cout << "Starting update...\n";
#endif


	QEventLoop loop;
	QTimer timer;

    QObject::connect(usdtState, &CoinState::updatingComplete, &loop, &QEventLoop::quit);
	QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
	
	emit updateUSDT();
	timer.start(config::rectWindow());
	loop.exec();

	emit updatingBegins();

	}

private slots:
	void sendResult()
	{
		auto snd = std::dynamic_cast<CoinState>(sender());
		auto chain = snd->getChain(usdtState->getState());

		auto message = snd->getCoinName();
		message.append("\n");
		for(const auto &it : chain)
		{
			message.append(it.toUserNative());
			message.append("_________\n");
		}
		message.append("usdt \nBuy - ");
		message.append(std::to_string(usdtState.getState.tgBuy);
		message.append("\nSell - ");
		message.append(std::to_string(usdtState.getState.tgSell);
		message.append("_________\n");

		QString tgChatId{"-1001964821237"};
		snd.sendGet("https://api.telegram.org", 
					config::tgBotToken() + "/sendMessage?",
					"chat_id=" + config::tgChatId() + "&text=" + message);
	}	

signals:
	void updatingBegins();
	void updateUSDT();
};
#endif
