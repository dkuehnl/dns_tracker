#include "dnstracker.h"

#include <iostream>

#include <QDnsLookup>
#include <QHostAddress>

DnsTracker::DnsTracker(const Options& o) : m_options(o) {
    if (m_options.continue_measurment) {
        //DnsTracker::start_tracking();
        std::cout << "Measuring not implemented yet" << std::endl;
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
        std::cerr << "DNS-Type " << m_options.dns_type.toStdString() << " is not supported!" << std::endl;
        return;
    }

    dns->setNameserver(QHostAddress(m_options.dns_server));
    QObject::connect(dns, &QDnsLookup::finished, this, &DnsTracker::analyze_lookup);
    dns->lookup();
    QCoreApplication::quit();
}

void DnsTracker::analyze_lookup() {
    QDnsLookup* dns = qobject_cast<QDnsLookup*>(sender());
    if (!dns) return;

    if (dns->error() != QDnsLookup::NoError) {
        std::cerr << "Error during DNS: " << dns->errorString().toStdString() << std::endl;
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
        return;
    }

    QCoreApplication::quit();
}
