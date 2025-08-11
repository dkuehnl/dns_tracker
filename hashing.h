#ifndef HASHING_H
#define HASHING_H

#include <QCoreApplication>
#include <QDnsLookup>

namespace Hashing {


static QString normalize_name(const QString &name);
QByteArray hash_a_record(const QList<QDnsHostAddressRecord>& record);
QByteArray hash_srv_record(const QList<QDnsServiceRecord>& record);

}

#endif // HASHING_H
