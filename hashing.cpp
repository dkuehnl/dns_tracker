/********************************************************************
 * DNS-Tracker
 *
 * This tool is build for use at DTAG and Deutsche Telekom Technik.
 * The purpose of this program is to trigger the DTAG-BPA-DNS-resolver
 * to monitor changes on external DNS-side.
 * The goal is to verify the delay of changing the DNS-response at
 * DTAG-internal systems and made the change available for the customers
 * on DTAG-external-site
 *
 * Purpose of this file:
 * In the Hasing-namespace are a bunch of helper-function for easy
 * and reliable comparison between the responses
 *
 * Author: Dennis Kuehnlein (2025)
********************************************************************/

#include "hashing.h"

#include <QCoreApplication>
#include <QDnsLookup>
#include <QHostAddress>
#include <QCryptographicHash>

static QString Hashing::normalize_name(const QString &name) {
    QString s = name.trimmed().toLower();
    if (s.endsWith('.')) s.chop(1);
    return s;
}

QByteArray Hashing::hash_a_record(const QList<QDnsHostAddressRecord>& record) {
    QVector<QString> parts;
    parts.reserve(record.size());
    for (const auto& rec : record) {
        QString name = normalize_name(rec.name());
        QString target = rec.value().toString();
        parts << QString("%1|%2").arg(name, target);
    }

    std::sort(parts.begin(), parts.end());
    QByteArray joined_parts;
    for (const auto& p : parts) {
        joined_parts.append(p.toUtf8());
        joined_parts.append('\n');
    }
    return QCryptographicHash::hash(joined_parts, QCryptographicHash::Md5);
}

QByteArray Hashing::hash_srv_record(const QList<QDnsServiceRecord>& record) {

}

