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

#include "display.h"

#include <iostream>

#include <QHostAddress>
#include <QTextStream>
#include <QDebug>

Display::Display(const QString &start_time, const Options& opt, QObject *parent) :
    QObject(parent), m_start_time(start_time), m_opt(opt) {}

void Display::render_a_display() {
    std::cout << "\033[2J\033[3J\033[H";
    std::cout << "Measurement started at: " << m_start_time.toStdString() << std::endl;

    for (auto outer_it = m_a_occurance.cbegin(); outer_it != m_a_occurance.cend(); ++outer_it) {
        const QString& server = outer_it.key();
        const auto& by_hash = outer_it.value();

        std::cout << "@" << server.toStdString()
                  << std::endl;

        for (auto inner_it = by_hash.cbegin(); inner_it != by_hash.cend(); ++inner_it) {
            const TimestampsARecord& occurance = inner_it.value();

            std::cout << "\tFirst: " << occurance.first_occur.toStdString()
                      << "\tLast: " << occurance.last_occur.toStdString()
                      << std::endl;

            for (const auto& entry : occurance.record) {
                if (m_opt.verbose) {
                    std::cout << "Requested"
                              << "\t"
                              << "Target"
                              << std::endl;
                    std::cout << entry.name().toStdString() << "\t"
                              << entry.value().toString().toStdString() << std::endl;
                } else {
                    std::cout << entry.value().toString().toStdString() << std::endl;
                }
            }
            std::cout << std::endl;
        }
    }
}

void Display::render_single_a() {
    std::cout << "\033[2J\033[3J\033[H";
    std::cout << "Measurement started at: " << m_start_time.toStdString() << std::endl;
    std::cout << "Time"
              << "\t\t"
              << "Requested"
              << "\t"
              << "Target"
              << std::endl;

    const auto& a_responses = m_a_responses;
    for (const auto& response : a_responses) {
        std::cout << "@" << response.server.toStdString();
        if (response.hash_changed) {
            std::cout << " (finished)";
        }
        std::cout << std::endl;

        std::cout << response.cur_timestamp.toStdString() << "\t";
        const auto cur_a_record = response.cur_response;
        for (const auto& cur_a : cur_a_record) {
            std::cout << cur_a.name().toStdString() << "\t"
                      << cur_a.value().toString().toStdString() << std::endl;
        }
    }
}

void Display::render_srv_display() {
    std::cout << "\033[2J\033[3J\033[H";
    std::cout << "Measurement started at: " << m_start_time.toStdString() << std::endl;

    for (auto outer_it = m_srv_occurance.cbegin(); outer_it != m_srv_occurance.cend(); ++outer_it) {
        const QString &server = outer_it.key();
        const auto   &by_hash  = outer_it.value();

        std::cout << "@" << server.toStdString()
                  << std::endl;

        for (auto inner_it = by_hash.cbegin(); inner_it != by_hash.cend(); ++inner_it) {
            const TimestampsSrvRecord& occurance = inner_it.value();

            std::cout << "\tFirst: " << occurance.first_occur.toStdString()
                      << "\tLast: " << occurance.last_occur.toStdString()
                      << std::endl;

            if (m_opt.verbose) {
                std::cout << "Requested"
                          << "\t"
                          << "Target"
                          << "\t"
                          << "Priority"
                          << "\t"
                          << "TTL"
                          << std::endl;
            }
            for (const auto& entry : occurance.record) {
                if (m_opt.verbose) {
                    std::cout << entry.name().toStdString() << "\t"
                              << entry.target().toStdString() << "\t"
                              << entry.priority() << "\t"
                              << entry.timeToLive() << std::endl;
                } else {
                    std::cout << entry.target().toStdString() << "\t"
                              << entry.priority() << std::endl;
                }
            }
        }
        std::cout << std::endl;
    }
}

void Display::render_single_srv() {
    std::cout << "\033[2J\033[3J\033[H";
    std::cout << "Measurement started at: " << m_start_time.toStdString() << std::endl;
    std::cout << "Time"
              << "\t\t"
              << "Requested"
              << "\t"
              << "Target"
              << "\t"
              << "Priority"
              << std::endl;

    const auto& srv_responses = m_srv_responses;
    for (const auto& response : srv_responses) {
        std::cout << "@" << response.server.toStdString() << std::endl;

        std::cout << response.cur_timestamp.toStdString() << std::endl;
        const auto cur_srv_record = response.cur_response;
        for (const auto& cur_srv : cur_srv_record) {
            std::cout << "\t" << cur_srv.name().toStdString() << "\t"
                      << cur_srv.target().toStdString() << "\t"
                      << cur_srv.priority() << std::endl;
        }
    }
}

void Display::update_a_display(DnsADisplayData cur_data) {
    auto& inner_map = m_a_occurance[cur_data.server];

    if (inner_map.contains(cur_data.cur_hash)) {
        inner_map[cur_data.cur_hash].record = cur_data.cur_response;
        inner_map[cur_data.cur_hash].last_occur = cur_data.cur_timestamp;
    } else {
        inner_map[cur_data.cur_hash].first_occur = cur_data.cur_timestamp;
        inner_map[cur_data.cur_hash].last_occur  = cur_data.cur_timestamp;
        inner_map[cur_data.cur_hash].record     = cur_data.cur_response;
        inner_map[cur_data.cur_hash].server     = cur_data.server;
    }

    if (m_opt.file_export) {
        Display::write_a_to_csv(cur_data);
    }
    Display::render_a_display();
}

void Display::write_a_to_csv(DnsADisplayData cur_data) {
    QFile file(m_opt.filepath);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        std::cerr << "File could not be opended: " << m_opt.filepath.toStdString() << std::endl;
        return;
    }

    QStringList record_entry;
    const auto data = cur_data.cur_response;
    for (const auto& rec : data) {
        record_entry << QString("\"%1(%2)\"").arg(rec.value().toString()).arg(rec.timeToLive());
    }

    QStringList row = {
        cur_data.cur_timestamp,
        cur_data.server,
        m_opt.dns_name,
        record_entry.join(';')
    };

    QTextStream out(&file);
    out << row.join(';') << '\n';
    file.close();
}


void Display::update_srv_display(DnsSrvDisplayData cur_data) {
    auto& inner_map = m_srv_occurance[cur_data.server];

    if (inner_map.contains(cur_data.cur_hash)) {
        inner_map[cur_data.cur_hash].record = cur_data.cur_response;
        inner_map[cur_data.cur_hash].last_occur = cur_data.cur_timestamp;
    } else {
        inner_map[cur_data.cur_hash].first_occur = cur_data.cur_timestamp;
        inner_map[cur_data.cur_hash].last_occur  = cur_data.cur_timestamp;
        inner_map[cur_data.cur_hash].record     = cur_data.cur_response;
        inner_map[cur_data.cur_hash].server     = cur_data.server;
    }

    if (m_opt.file_export) {
        Display::write_srv_to_csv(cur_data);
    }
    Display::render_srv_display();
}

void Display::write_srv_to_csv(DnsSrvDisplayData cur_data) {
    QFile file(m_opt.filepath);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        std::cerr << "File could not be opended: " << m_opt.filepath.toStdString() << std::endl;
        return;
    }

    QStringList record_entry;
    const auto& data = cur_data.cur_response;
    for (const auto& rec : data) {
        record_entry << QString("\"%1(%2, %3)\"").arg(rec.target()).arg(rec.priority()).arg(rec.timeToLive());
    }

    QStringList row = {
        cur_data.cur_timestamp,
        cur_data.server,
        m_opt.dns_name,
        record_entry.join(';')
    };

    QTextStream out (&file);
    out << row.join(';') << "\n";
    file.close();
}
