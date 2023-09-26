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
    const std::array<std::pair<int, QString>, 27> minQty{
    std::pair{3, "BTCUSDT"},
    std::pair{2, "BNBUSDT"},
    std::pair{1, "XRPUSDT"},
    std::pair{1, "EOSUSDT"},
    std::pair{3, "ETHUSDT"},
    std::pair{3, "LTCUSDT"},
    std::pair{2, "LINKUSDT"},
    std::pair{7, "TRXUSDT"},
    std::pair{0, "XLMUSDT"},
    std::pair{0, "ADAUSDT"},
    std::pair{3, "XMRUSDT"},
    std::pair{3, "DASHUSDT"},
    std::pair{1, "ZECUSDT"},
    std::pair{0, "XTZUSDT"},
    std::pair{2, "ATOMUSDT"},
    std::pair{2, "ETCUSDT"},
    std::pair{3, "BCHUSDT"},
    std::pair{0, "SXPUSDT"},
    std::pair{0, "IOTAUSDT"},
    std::pair{0, "MATICUSDT"},
    std::pair{1, "FILUSDT"},
    std::pair{0, "NEARUSDT"},
    std::pair{1, "EGLDUSDT"},
    std::pair{0, "THETAUSDT"},
    std::pair{1, "DOTUSDT"},
    std::pair{0, "SOLUSDT"},
    std::pair{2, "NEOUSDT"}
};
    const std::array<QString, 1> timefraemes{
        //"1h",
        "4h"
        //"6h"
    };
    inline QString filter(QString value, const QString &symbol){

        int filter_count = 0;
        for(int i = 0; i < pairs.size(); i++){
            if(minQty[i].second == symbol)
                filter_count = minQty[i].first;
        }

        QString result;
        int i = 0;
        do{
            result.append(value.at(i));
            i++;
        }while(value.at(i) != '.');
        result.append(value.at(i));

        int null_counter{0};
        for(int j = i+1; j < value.size(); j++ ){
            if(null_counter < filter_count){
                result.append(value.at(j));
                null_counter++;
            }
            else{
                result.append('0');
            }
        }

        return result;
    }
}

#endif // MANAGERENUMS_H
