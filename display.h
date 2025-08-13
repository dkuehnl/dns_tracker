#ifndef DISPLAY_H
#define DISPLAY_H

#include "dnstracker.h"

#include <QCoreApplication>
#include <QDnsLookup>
#include <QMap>


struct DnsADisplayData {
    QString server;
    bool hash_changed;
    QString prev_timestamp;
    QList<QDnsHostAddressRecord> prev_response;
    QString cur_timestamp;
    QList<QDnsHostAddressRecord> cur_response;
};

struct DnsSrvDisplayData {
    QString server;
    bool hash_changed;
    QString prev_timestamp;
    QList<QDnsServiceRecord> prev_response;
    QString cur_timestamp;
    QList<QDnsServiceRecord> cur_response;
};


class Display {
public:
    Display(QString start_time, Options opt);
    void update_a_display(DnsADisplayData cur_data);
    void update_srv_display(DnsSrvDisplayData cur_data);
    void render_a_display();
    void render_srv_display();

private:
    QString m_start_time;
    Options m_opt;
    QMap<QString, DnsADisplayData> m_a_responses;
    QMap<QString, DnsSrvDisplayData> m_srv_responses;
};

#endif // DISPLAY_H
