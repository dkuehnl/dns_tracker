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
