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
#include "hashing.h"

#include <iostream>

#include <QHostAddress>
#include <QTimer>
#include <QDebug>
#include <QDateTime>

constexpr size_t SLEEP_INTERVALL = 60000;

DnsTracker::DnsTracker(const Options& o) : m_options(o) {

}

void DnsTracker::start() {
    if (m_options.continue_measurment) {
        std::cout << "Measurement started at " << QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() << std::endl;
        m_start_time = QDateTime::currentMSecsSinceEpoch();
        if (m_options.verbose) {
            std::cout << "Time" << "\t\t" << "Requested" << "\t" << "Target" << "\t" << "Priority" << std::endl;
        }
    }
    DnsTracker::run_lookup();

}

void DnsTracker::run_lookup() {
    std::cout << "run lookup erreicht" << std::endl;
    if (m_dns) {
        m_dns->deleteLater();
    }

    if (m_options.dns_type.toUpper() == "SRV") {
        m_dns = new QDnsLookup(QDnsLookup::Type(QDnsLookup::SRV), m_options.dns_name, this);
    } else if (m_options.dns_type.toUpper() == "A") {
        m_dns = new QDnsLookup(QDnsLookup::Type(QDnsLookup::A), m_options.dns_name, this);
    } else {
        std::cerr << "DNS-Type " << m_options.dns_type.toStdString() << " is not supported!!" << std::endl;
        QCoreApplication::quit();
        return;
    }

    m_dns->setNameserver(QHostAddress(m_options.dns_server));
    if (m_options.continue_measurment) {
        QObject::connect(m_dns, &QDnsLookup::finished, this, &DnsTracker::start_tracking);
    } else {
        QObject::connect(m_dns, &QDnsLookup::finished, this, &DnsTracker::display_lookup);
    }
    m_dns->lookup();
}

void DnsTracker::display_lookup() {
    if (!m_dns) {
        std::cerr << "Unspecified error during dns-request." << std::endl;
        QCoreApplication::exit(1);
        return;
    }

    if (m_dns->error() != QDnsLookup::NoError) {
        std::cerr << "Error during DNS: " << m_dns->errorString().toStdString() << std::endl;
        QCoreApplication::exit(1);
        return;
    }

    std::cout << "Requested" << "\t" << "TTL" << "\t" << "Priority" << "\t" << "Target" << std::endl;
    if (m_options.dns_type.toUpper() == "SRV") {
        for (const auto& rec : m_dns->serviceRecords()) {
            std::cout << rec.name().toStdString() << "\t" << rec.timeToLive() << "\t" << rec.priority() << "\t" << rec.target().toStdString() << std::endl;
        }
    } else if (m_options.dns_type.toUpper() == "A") {
        for (const auto& rec : m_dns->hostAddressRecords()) {
            std::cout << rec.name().toStdString() << "\t" << rec.timeToLive() << "\t\t" << rec.value().toString().toStdString() << std::endl;
        }
    } else {
        std::cerr << "not supported type" << std::endl;
    }

    m_dns->deleteLater();

    QCoreApplication::quit();
    return;
}

void DnsTracker::start_tracking() {
    if (!m_dns) {
        std::cerr << "Unspecified error during dns-request." << std::endl;
        QCoreApplication::exit(1);
        return;
    }
    if (m_dns->error() != QDnsLookup::NoError) {
        std::cerr << "Error during DNS: " << m_dns->errorString().toStdString() << std::endl;
        QCoreApplication::exit(1);
        return;
    }

    bool has_changed = false;
    if (m_options.dns_type.toUpper() == "SRV") {

        m_cur_srv_record = Hashing::hash_srv_record(m_dns->serviceRecords());
        has_changed = DnsTracker::compare_srv();

        if (m_options.verbose) {
            for (const auto& record : m_dns->serviceRecords()) {
                std::cout << QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() << "\t"
                          << record.name().toStdString() << "\t"
                          << record.target().toStdString() << "\t"
                          << record.priority() << std::endl;
            }
            std::cout << std::endl;
        }
    } else if (m_options.dns_type.toUpper() == "A") {

        m_cur_a_record = Hashing::hash_a_record(m_dns->hostAddressRecords());
        has_changed = DnsTracker::compare_a();

        if (m_options.verbose) {
            for (const auto& record : m_dns->hostAddressRecords()) {
                std::cout << QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() << "\t"
                          << record.name().toStdString() << "\t"
                          << record.value().toString().toStdString() << std::endl;
            }
        }
    }

    if (has_changed) {
        std::cout << "has changed" << std::endl;
        //Wenn gleich:
        //Loop stoppen
        //Zeit von Start bis Änderung berechnen
        //Ergebnis anzeigen
        //Programm beenden
        QCoreApplication::quit();
        return;
    }

    m_prev_a_record = m_cur_a_record;
    m_prev_srv_record = m_cur_srv_record;
    QTimer::singleShot(SLEEP_INTERVALL, this, &DnsTracker::run_lookup);
}

bool DnsTracker::compare_srv() {
    if (m_prev_srv_record.isEmpty()) {
        return false;
    }

    if (m_prev_srv_record == m_cur_srv_record) {
        return false;
    } else {
        return true;
    }
}

bool DnsTracker::compare_a() {
    if (m_prev_a_record.isEmpty()) {
        return false;
    }
    if (m_prev_a_record == m_cur_a_record) {
        return false;
    } else {
        return true;
    }
}
