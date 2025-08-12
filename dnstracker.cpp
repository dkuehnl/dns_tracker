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
        std::cout << "Measurement started at "
                  << QDateTime::currentDateTime().toString(Qt::ISODate).toStdString()
                  << std::endl;
        m_start_time = QDateTime::currentMSecsSinceEpoch();
        if (m_options.verbose) {
            std::cout << "Time"
                      << "\t\t"
                      << "Requested"
                      << "\t"
                      << "Target"
                      << "\t"
                      << "Priority"
                      << std::endl;
        }
    }
    DnsTracker::run_lookup();

}

void DnsTracker::run_lookup() {
    if (m_dns) {
        m_dns->deleteLater();
    }

    if (m_options.dns_type.toUpper() == "SRV") {
        m_dns = new QDnsLookup(QDnsLookup::Type(QDnsLookup::SRV), m_options.dns_name, this);
    } else if (m_options.dns_type.toUpper() == "A") {
        m_dns = new QDnsLookup(QDnsLookup::Type(QDnsLookup::A), m_options.dns_name, this);
    } else {
        std::cerr << "DNS-Type "
                  << m_options.dns_type.toStdString()
                  << " is not supported!"
                  << std::endl;
        QCoreApplication::exit(1);
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

    std::cout << "Requested"
              << "\t"
              << "TTL"
              << "\t"
              << "Priority"
              << "\t"
              << "Target"
              << std::endl;
    if (m_options.dns_type.toUpper() == "SRV") {
        const auto records = m_dns->serviceRecords();
        for (const auto& rec : records) {
            std::cout << rec.name().toStdString()
                      << "\t" << rec.timeToLive()
                      << "\t" << rec.priority()
                      << "\t" << rec.target().toStdString()
                      << std::endl;
        }
    } else if (m_options.dns_type.toUpper() == "A") {
        const auto records = m_dns->hostAddressRecords();
        for (const QDnsHostAddressRecord& rec : records) {
            std::cout << rec.name().toStdString()
                      << "\t" << rec.timeToLive()
                      << "\t\t" << rec.value().toString().toStdString()
                      << std::endl;
        }
    } else {
        std::cerr << "DNS-Type "
                  << m_options.dns_type.toStdString()
                  << " is not supported!"
                  << std::endl;
        m_dns->deleteLater();
        QCoreApplication::exit(1);
        return;
    }

    m_dns->deleteLater();
    QCoreApplication::exit(0);
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

    bool hash_changed = false;
    if (m_options.dns_type.toUpper() == "SRV") {
        hash_changed = DnsTracker::analyze_srv();
    } else if (m_options.dns_type.toUpper() == "A") {
        hash_changed = DnsTracker::analyze_a();
    }

    if (hash_changed) {
        DnsTracker::display_summary(QDateTime::currentMSecsSinceEpoch());
        QCoreApplication::exit(0);
        return;
    }

    DnsTracker::change_member_values();
    QTimer::singleShot(SLEEP_INTERVALL, this, &DnsTracker::run_lookup);
}

void DnsTracker::display_summary(qint64 end_time) {
    std::cout << "----------------------------------------------------------------------" << std::endl;
    std::cout << "DNS-response for "
              << m_options.dns_name.toStdString()
              << " has changed at: "
              << QDateTime::fromMSecsSinceEpoch(end_time).toString(Qt::ISODate).toStdString()
              << std::endl;
    std::cout << "For DNS-server: " << m_options.dns_server.toStdString() << std::endl;

    if (m_options.verbose) {
        std::cout << "From: ";
        if (m_options.dns_type.toUpper() == "SRV") {
            const auto& prev_srv_response = m_prev_srv_response;
            for (const auto& response : prev_srv_response) {
                std::cout << response.target().toStdString()
                          << " (Prio: " << response.priority() << ")"
                          << std::endl;
            }
            std::cout << "To: ";
            const auto& cur_srv_response = m_cur_srv_response;
            for (const auto& response : cur_srv_response) {
                std::cout << response.target().toStdString()
                          << " (Prio: " << response.priority() << ")"
                          << std::endl;
            }
        } else if (m_options.dns_type.toUpper() == "A") {
            const auto& prev_a_response = m_prev_a_response;
            for (const auto& response : prev_a_response) {
                std::cout << "\t" << response.value().toString().toStdString() << std::endl;
            }
            std::cout << "To: ";
            const auto& cur_a_response = m_cur_a_response;
            for (const auto& response : cur_a_response) {
                std::cout << "\t" << response.value().toString().toStdString() << std::endl;
            }
        }
    }

    std::cout << "Duration of change since start of measurement: "
              << DnsTracker::calculate_delay(end_time).toString("hh:mm:ss").toStdString()
              << std::endl;
}

QTime DnsTracker::calculate_delay(qint64 end_time) {
    qint64 duration = end_time - m_start_time;
    QTime duration_time(0,0);
    duration_time = duration_time.addMSecs(duration);
    return duration_time;
}

bool DnsTracker::analyze_srv() {
    m_cur_srv_response = m_dns->serviceRecords();
    m_cur_srv_hash = Hashing::hash_srv_record(m_dns->serviceRecords());
    bool hash_changed = DnsTracker::compare_hash(m_prev_srv_hash, m_cur_srv_hash);

    if (m_options.verbose) {
        const auto records = m_dns->serviceRecords();
        for (const auto& record : records) {
            std::cout << QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() << "\t"
                      << record.name().toStdString() << "\t"
                      << record.target().toStdString() << "\t"
                      << record.priority() << std::endl;
        }
        std::cout << std::endl;
    } else if (!m_options.verbose & !hash_changed) {
        std::cout << QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() << "\t"
                  << "No Change detected"
                  << std::endl;
    }
    return hash_changed;
}

bool DnsTracker::analyze_a() {
    m_cur_a_response = m_dns->hostAddressRecords();
    m_cur_a_hash = Hashing::hash_a_record(m_dns->hostAddressRecords());
    bool hash_changed = DnsTracker::compare_hash(m_prev_a_hash, m_cur_a_hash);

    if (m_options.verbose) {
        const auto records = m_dns->hostAddressRecords();
        for (const auto& record : records) {
            std::cout << QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() << "\t"
                      << record.name().toStdString() << "\t"
                      << record.value().toString().toStdString() << std::endl;
        }
    } else if (!m_options.verbose & !hash_changed) {
        std::cout << QDateTime::currentDateTime().toString(Qt::ISODate).toStdString() << "\t"
                  << "No Change detected"
                  << std::endl;
    }
    return hash_changed;
}

bool DnsTracker::compare_hash(const QByteArray& prev_hash, const QByteArray& cur_hash) {
    if (prev_hash.isEmpty()) {
        return false;
    }
    if (prev_hash == cur_hash) {
        return false;
    } else {
        return true;
    }
}

void DnsTracker::change_member_values() {
    m_prev_a_hash = m_cur_a_hash;
    m_prev_a_response = m_cur_a_response;
    m_prev_srv_hash = m_cur_srv_hash;
    m_prev_srv_response = m_cur_srv_response;
}
