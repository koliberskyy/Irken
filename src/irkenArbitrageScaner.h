
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
	std::vector<std::shared_ptr<CoinState>> states;
	std::shared_ptr<QNetworkAccessManager> manager;
	std::shared_ptr<TelegramCli> tg;

public:
	explicit IrkenArbitrageScaner(QObject *parent = nullptr):
		QObject(parent),
		manager{std::make_shared<QNetworkAccessManager>(new QNetworkAccessManager())}
	{
		//make states
		for(const auto &it : coins)
		{
			auto tmp = std::make_shared<CoinState>(it, tg,  manager);
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

		emit updatingBegins();
	}	
signals:
	void updatingBegins();
};
