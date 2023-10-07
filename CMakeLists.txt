cmake_minimum_required(VERSION 3.14)

project(Irken LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

include_directories(core klines areas areas/core orders tg manager
    orderControl orderControl/client orderControl/account orderControl/logger)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network Test)
find_package(Threads REQUIRED)

add_executable(Irken
  main.cpp
  core/requests.h core/requests.cpp
  klines/klines.h klines/klines.cpp
  klines/candlestick.h
  klines/KlineEnums.h
  areas/core/areas.h areas/core/areas.cpp
  areas/core/areapair.h
  areas/core/AeasEnums.h
  areas/selltobuy.h areas/selltobuy.cpp
  areas/buytosell.h areas/buytosell.cpp
  areas/imbalance.h areas/imbalance.cpp
  areas/swicksk.h areas/swicksk.cpp
  areas/liquidity.h areas/liquidity.cpp
  orders/orders.h orders/orders.cpp
  orders/orderdata.h
  tg/telegrambot.h tg/telegrambot.cpp
  manager/manager.h manager/manager.cpp
  manager/managerEnums.h
  orderControl/binancebot.h orderControl/binancebot.cpp
  orderControl/client/binanceapiclient.h orderControl/client/binanceapiclient.cpp
  orderControl/client/source.h
  orderControl/client/key.h
  orderControl/account/binanceaccount.h orderControl/account/binanceaccount.cpp
  orderControl/logger/logger.h orderControl/logger/logger.cpp
  orderControl/account/positionsbuffer.h orderControl/account/positionsbuffer.cpp
  settings.h
)
target_link_libraries(Irken
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Test
    Threads::Threads
)

install(TARGETS Irken
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
