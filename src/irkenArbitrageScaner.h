#ifndef IRKEN_ARBITRAGE_SCANER_H
#define IRKEN_ARBITRAGE_SCANER_H

#include "CoinState.h"

class IrkenArbitrageScaner : public QObject
{
	Q_OBJECT

    inline static const std::array<QString, 3> coins
	{
		"NOT",
		"TON",
		"BTC"
	};

    inline static const QString coinBase {"USDT"};
	std::shared_ptr<CoinState> usdtState;
	std::vector<std::shared_ptr<CoinState>> states;
	std::shared_ptr<QNetworkAccessManager> manager;
    std::unique_ptr<QDateTime>       updateTime;
    std::shared_ptr<QString> authToken;
    std::shared_ptr<bool> tokenUpdated;

public:
	explicit IrkenArbitrageScaner(QObject *parent = nullptr):
		QObject(parent),
        manager{std::make_shared<QNetworkAccessManager>(nullptr)},
        updateTime(new QDateTime(QDateTime::currentDateTime())),
        authToken{std::make_shared<QString>("")},
        tokenUpdated{std::make_shared<bool>(false)}
	{


        usdtState = std::make_shared<CoinState>(coinBase, authToken, tokenUpdated, manager);
		QObject::connect(this, &IrkenArbitrageScaner::updateUSDT,
						usdtState.get(), &CoinState::upd);
        usdtState->getAuthToken();
		
		//make states
		for(const auto &it : coins)
		{
            auto tmp = std::make_shared<CoinState>(it, authToken, tokenUpdated, manager);

			QObject::connect(tmp.get(), &CoinState::updatingComplete,
							this, &IrkenArbitrageScaner::sendResult);

			QObject::connect(this, &IrkenArbitrageScaner::updatingBegins,
							tmp.get(), &CoinState::upd);

			states.push_back(tmp);
		}

        upd();

        auto timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &IrkenArbitrageScaner::timerChanged);
        timer->start(1000);

	}

public slots:
	void upd()
	{

#ifdef DEBUG
		std::cout << "Starting update...\n";
#endif


	QEventLoop loop;
	QTimer timer;

    QObject::connect(usdtState.get(), &CoinState::updatingComplete, &loop, &QEventLoop::quit);
	QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
	
	emit updateUSDT();
	timer.start(config::rectWindow());
	loop.exec();

	emit updatingBegins();

	}

private slots:
	void sendResult()
	{
        auto snd = dynamic_cast<CoinState*>(QObject::sender());
        if(snd != nullptr)
        {
            auto chain = snd->getChain(usdtState->getState());

            //found best chain
            Chain actual = *(chain.begin());
            for(const auto &it : chain)
            {
                if(it.spred() > actual.spred())
                    actual = it;
            }

            QString message;
            if(actual.spred() > 2.0 && actual.priceBuy > 0 && actual.priceSell > 0){
                message.append(actual.toUserNative(usdtState->getState()));
                snd->sendGet("api.telegram.org",
                             config::tgBotToken() + "/sendMessage",
                             "chat_id=" + config::tgChatId() + "&text=" + message);
            }
        }
    }

    void timerChanged()
    {
        auto current = QDateTime::currentDateTime();

        if(updateTime->secsTo(current) > 60){
            upd();
            updateTime->setDate(current.date());
            updateTime->setTime(current.time());
        }
    }

signals:
	void updatingBegins();
	void updateUSDT();
};
#endif
