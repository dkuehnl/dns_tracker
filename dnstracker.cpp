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

#include <QHostAddress>
#include <QTimer>
#include <QDebug>

DnsTracker::DnsTracker(const Options& o) : m_options(o) {

}

void DnsTracker::start() {
    if (m_options.continue_measurment) {
        DnsTracker::start_tracking();
    } else {
        DnsTracker::run_lookup();
    }
}

void DnsTracker::run_lookup() {
    QDnsLookup* dns = nullptr;
    if (m_options.dns_type.toUpper() == "SRV") {
        dns = new QDnsLookup(QDnsLookup::Type(QDnsLookup::SRV), m_options.dns_name, this);
    } else if (m_options.dns_type.toUpper() == "A") {
        dns = new QDnsLookup(QDnsLookup::Type(QDnsLookup::A), m_options.dns_name, this);
    } else {
        std::cerr << "DNS-Type " << m_options.dns_type.toStdString() << " is not supported!!" << std::endl;
        QCoreApplication::quit();
        return;
    }

    dns->setNameserver(QHostAddress(m_options.dns_server));
    if (m_options.continue_measurment) {
        QObject::connect(dns, &QDnsLookup::finished, this, &DnsTracker::start_tracking);
    } else {
        QObject::connect(dns, &QDnsLookup::finished, this, &DnsTracker::display_lookup);
    }
    dns->lookup();
}

void DnsTracker::display_lookup() {
    QDnsLookup* dns = qobject_cast<QDnsLookup*>(sender());
    if (!dns) {
        std::cerr << "Unspecified error during dns-request." << std::endl;
        QCoreApplication::exit(1);
        return;
    }

    if (dns->error() != QDnsLookup::NoError) {
        std::cerr << "Error during DNS: " << dns->errorString().toStdString() << std::endl;
        QCoreApplication::exit(1);
        return;
    }

    std::cout << "Requested" << "\t" << "TTL" << "\t" << "Priority" << "\t" << "Target" << std::endl;
    if (m_options.dns_type.toUpper() == "SRV") {
        for (const auto& rec : dns->serviceRecords()) {
            std::cout << rec.name().toStdString() << "\t" << rec.timeToLive() << "\t" << rec.priority() << "\t" << rec.target().toStdString() << std::endl;
        }
    } else if (m_options.dns_type.toUpper() == "A") {
        for (const auto& rec : dns->hostAddressRecords()) {
            std::cout << rec.name().toStdString() << "\t" << rec.timeToLive() << "\t\t" << rec.value().toString().toStdString() << std::endl;
        }
    } else {
        std::cerr << "not supported type" << std::endl;
    }

    QCoreApplication::quit();
    return;
}

void DnsTracker::start_tracking() {
    QDnsLookup* dns = qobject_cast<QDnsLookup*>(sender());
    if (!dns) {
        std::cerr << "Unspecified error during dns-request." << std::endl;
        QCoreApplication::exit(1);
        return;
    }
    if (dns->error() != QDnsLookup::NoError) {
        std::cerr << "Error during DNS: " << dns->errorString().toStdString() << std::endl;
        QCoreApplication::exit(1);
        return;
    }

    bool has_changed = false;
    //Response in member-variable speichern
    if (m_options.dns_type.toUpper() == "SRV") {
        m_cur_srv_record = dns->serviceRecords();
        //Vergleich mit previos wenn vorhanden
        has_changed = DnsTracker::analyze_srv();
    } else if (m_options.dns_type.toUpper() == "A") {
        m_cur_a_record = dns->hostAddressRecords();
        has_changed = DnsTracker::analyze_a();
    }

    dns->deleteLater();


    //bei Verbose response anzeigen


    //Wenn ungleich:
        //Sleep-Timer
    //Wenn gleich:
        //Loop stoppen
        //Zeit von Start bis Änderung berechnen
        //Ergebnis anzeigen
        //Programm beenden

    std::cout << "Läuft bis hierhin fehlerfrei: " << m_cur_a_record.size() << " A-Records bekommen" << std::endl;

}

bool DnsTracker::analyze_srv() {
    if (m_prev_srv_record.isEmpty()) {
        return false;
    }

}

bool DnsTracker::analyze_a() {
    if (m_prev_a_record.isEmpty()) {
        return false;
    }

    for (const QDnsHostAddressRecord& prev_record : m_prev_a_record) {
        for (const QDnsHostAddressRecord& cur_record : m_cur_a_record) {
            if (prev_record.value().toString() != cur_record.value().toString()) {
                return true;
            }
        }
    }
}
