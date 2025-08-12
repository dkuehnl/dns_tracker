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
 * The dns-tracker-class defines an the tracking-object. It is resposible
 * for running the lookup regarding to the settings it is instantiiated with.
 * Only for continue-mode it will analyze the responses, compare them and
 * stop the lookup-loop when it detects a change. Then it will calculate the
 * time since program-start until the dns-change happened.
 *
 * Author: Dennis Kuehnlein (2025)
********************************************************************/


#ifndef DNSTRACKER_H
#define DNSTRACKER_H

#include <qobject.h>
#include <QCoreApplication>
#include <QDnsLookup>

struct Options {
    QString dns_type;
    QString dns_server;
    QString dns_name;
    bool verbose = false;
    bool continue_measurment = false;
    bool show_help = false;
};

class DnsTracker : public QObject {
    Q_OBJECT

public:
    DnsTracker(const Options& o);

public slots:
    void start();

private:
    QDnsLookup* m_dns = nullptr;
    Options m_options;
    qint64 m_start_time;

    QByteArray m_prev_a_hash;
    QList<QDnsHostAddressRecord> m_prev_a_response;
    QByteArray m_cur_a_hash;
    QList<QDnsHostAddressRecord> m_cur_a_response;

    QByteArray m_prev_srv_hash;
    QList<QDnsServiceRecord> m_prev_srv_response;
    QByteArray m_cur_srv_hash;
    QList<QDnsServiceRecord> m_cur_srv_response;

    void run_lookup();
    void start_tracking();
    void display_lookup();
    bool compare_srv();
    bool compare_a();

};



#endif // DNSTRACKER_H
