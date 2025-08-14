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
 * The display-class is responsible for a proper display of continues-
 * measurements. It gets its data from the appropriate a- or srv-signal
 * emited by the dns-tracker.
 * It is build to display the results of more then one dns-tracker-
 * objects.
 * If only one dns-tracker is used, this class won't be started.
 *
 * Author: Dennis Kuehnlein (2025)
********************************************************************/


#ifndef DISPLAY_H
#define DISPLAY_H

#include <QCoreApplication>
#include <QDnsLookup>
#include <QMap>

#include "dnstracker.h"

class Display : public QObject {
    Q_OBJECT

public:
    Display(const QString& start_time, const Options& opt, QObject *parent = nullptr);

public slots:
    void update_a_display(DnsADisplayData cur_data);
    void update_srv_display(DnsSrvDisplayData cur_data);

private:
    QString m_start_time;
    Options m_opt;
    QMap<QString, DnsADisplayData> m_a_responses;
    QMap<QString, DnsSrvDisplayData> m_srv_responses;

    void render_a_display();
    void render_srv_display();
    void render_single_a();
    void render_single_srv();
};

#endif // DISPLAY_H
