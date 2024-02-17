#ifndef SETTINGS_H
#define SETTINGS_H
#include <QByteArray>

namespace settings {
static const QByteArray recv_window{"10000"};
static const double actualityFilter{8.0};
static const double one_order_qty_pc_from_balance{5};
}

#endif // SETTINGS_H
