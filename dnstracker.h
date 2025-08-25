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
#include <QFile>

struct Options {
    QString dns_type;
    QString dns_name;
    QString dns_server;
    QList<QString> multi_dns_server;
    QString filepath;
    size_t sleep_intervall = 60000;
    bool verbose = false;
    bool continue_measurment = false;
    bool file_export = false;
    bool multi_requests = false;
    bool show_help = false;
};

struct DnsADisplayData {
    QString server;
    bool hash_changed = false;
    QString prev_timestamp;
    QList<QDnsHostAddressRecord> prev_response;
    QString cur_timestamp;
    QByteArray cur_hash;
    QList<QDnsHostAddressRecord> cur_response;
    QString start_timestamp;
    QString end_timestamp;
    QString duration;
};

struct DnsSrvDisplayData {
    QString server;
    bool hash_changed = false;
    QString prev_timestamp;
    QList<QDnsServiceRecord> prev_response;
    QString cur_timestamp;
    QByteArray cur_hash;
    QList<QDnsServiceRecord> cur_response;
    QString start_timestamp;
    QString end_timestamp;
    QString duration;
};

class DnsTracker : public QObject {
    Q_OBJECT

public:
    DnsTracker(const Options& options, QObject *parent = nullptr);

public slots:
    void start();

signals:
    void send_srv_update(DnsSrvDisplayData cur_data);
    void send_a_update(DnsADisplayData cur_data);
    void finished();

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
    void display_single_lookup();
    void display_summary(qint64 end_time);
    void change_member_values();

    bool analyze_srv();
    bool analyze_a();
    bool compare_hash(const QByteArray& prev_hash, const QByteArray& cur_hash);
    QTime calculate_delay(qint64 end_time);

};



#endif // DNSTRACKER_H
