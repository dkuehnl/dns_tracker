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

#include "dnstracker.h"

#include <iostream>

#include <QDnsLookup>
#include <QHostAddress>
#include <QTimer>
#include <QDebug>

DnsTracker::DnsTracker(const Options& o) : m_options(o) {

}

void DnsTracker::start() {
    if (m_options.continue_measurment) {
        std::cerr << "-c: continue-function currently not implemented." << std::endl;
        QCoreApplication::quit();
        return;
        //DnsTracker::start_tracking();
    } else {
        DnsTracker::run_lookup();
    }
}

void DnsTracker::run_lookup() {
    QDnsLookup* dns = nullptr;
    if (m_options.dns_type.toUpper() == "SRV") {
        std::cout << m_options.dns_name.toStdString() << std::endl;
        dns = new QDnsLookup(QDnsLookup::Type(QDnsLookup::SRV), m_options.dns_name, this);
    } else if (m_options.dns_type.toUpper() == "A") {
        dns = new QDnsLookup(QDnsLookup::Type(QDnsLookup::A), m_options.dns_name, this);
    } else {
        std::cerr << "DNS-Type " << m_options.dns_type.toStdString() << " is not supported!!" << std::endl;
        QCoreApplication::quit();
        return;
    }

    dns->setNameserver(QHostAddress(m_options.dns_server));
    QObject::connect(dns, &QDnsLookup::finished, this, &DnsTracker::analyze_lookup);
    dns->lookup();
}

void DnsTracker::analyze_lookup() {
    QDnsLookup* dns = qobject_cast<QDnsLookup*>(sender());
    if (!dns) {
        std::cerr << "Unspecified error during dns-request." << std::endl;
        QCoreApplication::quit();
        return;
    }

    if (dns->error() != QDnsLookup::NoError) {
        std::cerr << "Error during DNS: " << dns->errorString().toStdString() << std::endl;
        QCoreApplication::quit();
        return;
    }

    if (m_options.dns_type.toUpper() == "SRV") {
        for (const auto& rec : dns->serviceRecords()) {
            std::cout << rec.name().toStdString() << "\t" << rec.target().toStdString() << "\t" << rec.timeToLive() << "\t" << rec.priority() << std::endl;
        }
    } else if (m_options.dns_type.toUpper() == "A") {
        for (const auto& rec : dns->hostAddressRecords()) {
            std::cout << rec.value().toString().toStdString() << "\t" << rec.name().toStdString() << "\t" << rec.timeToLive() << std::endl;
        }
    } else {
        std::cerr << "not supported type" << std::endl;
    }

    QCoreApplication::quit();
    return;
}
