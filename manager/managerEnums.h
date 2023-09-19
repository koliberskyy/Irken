#ifndef MANAGERENUMS_H
#define MANAGERENUMS_H

#include <array>
#include <QString>

namespace manager{
    const std::array<std::pair<int, QString>, 27> pairs{
        std::pair{0, "BTCUSDT"},
        std::pair{1, "BNBUSDT"},
        std::pair{2, "XRPUSDT"},
        std::pair{3, "EOSUSDT"},
        std::pair{4, "ETHUSDT"},
        std::pair{5, "LTCUSDT"},
        std::pair{6, "LINKUSDT"},
        std::pair{7, "TRXUSDT"},
        std::pair{8, "XLMUSDT"},
        std::pair{9, "ADAUSDT"},
        std::pair{10, "XMRUSDT"},
        std::pair{11, "DASHUSDT"},
        std::pair{12, "ZECUSDT"},
        std::pair{13, "XTZUSDT"},
        std::pair{14, "ATOMUSDT"},
        std::pair{15, "ETCUSDT"},
        std::pair{16, "BCHUSDT"},
        std::pair{17, "SXPUSDT"},
        std::pair{18, "IOTAUSDT"},
        std::pair{19, "MATICUSDT"},
        std::pair{20, "FILUSDT"},
        std::pair{21, "NEARUSDT"},
        std::pair{22, "EGLDUSDT"},
        std::pair{23, "THETAUSDT"},
        std::pair{24, "DOTUSDT"},
        std::pair{25, "SOLUSDT"},
        std::pair{26, "NEOUSDT"}
    };
    const std::array<QString, 3> timefraemes{
        "1h",
        "4h",
        "6h"
    };
}

#endif // MANAGERENUMS_H
