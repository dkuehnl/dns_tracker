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

Display::Display(QString start_time, const Options& opt) :
    m_start_time(start_time), m_opt(opt) {}

void Display::render_a_display() {
    std::cout << "\033[2J\033[3J\033[H";
    std::cout << "Measurement started at: " << m_start_time.toStdString() << std::endl;
    if (m_opt.verbose) {
        std::cout << "Time"
                  << "\t\t"
                  << "Requested"
                  << "\t"
                  << "Target"
                  << "\t"
                  << "Priority"
                  << std::endl;
    }
    const auto a_responses = m_a_responses;
    for (const auto& response : a_responses) {
        std::cout << "@" << response.server.toStdString();
        if (response.hash_changed) {
            std::cout << " (finished)";
        }
        std::cout << std::endl;
        if (m_opt.verbose) {
            if (!response.prev_response.isEmpty()) {
                std::cout << response.prev_timestamp.toStdString() << "\t";
                const auto prev_a_record = response.prev_response;
                for (const auto& prev_a : prev_a_record) {
                    std::cout << prev_a.name().toStdString() << "\t"
                              << prev_a.value().toString().toStdString() << std::endl;
                }
            }
            std::cout << response.cur_timestamp.toStdString() << "\t";
            const auto cur_a_record = response.cur_response;
            for (const auto& cur_a : cur_a_record) {
                std::cout << cur_a.name().toStdString() << "\t"
                          << cur_a.value().toString().toStdString() << std::endl;
            }
        } else if (!(m_opt.verbose & response.hash_changed)) {
            if (!response.prev_response.isEmpty()) {
                std::cout << response.prev_timestamp.toStdString() << "\t"
                          << "No Change detected" << std::endl;
            }
            std::cout << response.cur_timestamp.toStdString() << "\t"
                      << "No Change detected" << std::endl;
        }

        if (response.hash_changed) {
            std::cout << "----------------------------------------------------------------------" << std::endl;
            std::cout << "DNS-response for "
                      << m_opt.dns_name.toStdString()
                      << " has changed at: "
                      << response.end_timestamp.toStdString()
                      << std::endl;
            std::cout << "For DNS-server: " << response.server.toStdString() << std::endl;

            if (m_opt.verbose) {
                std::cout << "From: ";
                const auto prev_a_record = response.prev_response;
                for (const auto& prev_a : prev_a_record) {
                    std::cout << prev_a.value().toString().toStdString() << std::endl;
                }

                std::cout << "To: ";
                const auto cur_a_record = response.cur_response;
                for (const auto& cur_a : cur_a_record) {
                    std::cout << cur_a.value().toString().toStdString() << std::endl;
                }
            }

            std::cout << "Duration of change since start of measurement: "
                      << response.duration.toStdString()
                      << std::endl << std::endl;
        }
    }
}

void Display::render_srv_display() {
    std::cout << "\033[2J\033[3J\033[H";
    std::cout << "Measurement started at: " << m_start_time.toStdString() << std::endl;
    if (m_opt.verbose) {
        std::cout << "Time"
                  << "\t\t"
                  << "Requested"
                  << "\t"
                  << "Target"
                  << "\t"
                  << "Priority"
                  << std::endl;
    }
    const auto srv_responses = m_srv_responses;
    for (const auto& response : srv_responses) {
        std::cout << "@" << response.server.toStdString() << std::endl;
        if (m_opt.verbose) {
            if (!response.prev_response.isEmpty()) {
                std::cout << response.prev_timestamp.toStdString() << std::endl;
                const auto prev_srv_record = response.prev_response;
                for (const auto& prev_srv : prev_srv_record) {
                    std::cout << "\t" << prev_srv.name().toStdString() << "\t"
                              << prev_srv.target().toStdString() << "\t"
                              << prev_srv.priority() << std::endl;
                }
            }
            std::cout << response.cur_timestamp.toStdString() << std::endl;
            const auto cur_srv_record = response.cur_response;
            for (const auto& cur_srv : cur_srv_record) {
                std::cout << "\t" << cur_srv.name().toStdString() << "\t"
                          << cur_srv.target().toStdString() << "\t"
                          << cur_srv.priority() << std::endl;
            }
        } else if (!(m_opt.verbose & response.hash_changed)) {
            if (!response.prev_response.isEmpty()) {
                std::cout << response.prev_timestamp.toStdString() << "\t"
                          << "No Change detected" << std::endl;
            }
            std::cout << response.cur_timestamp.toStdString() << "\t"
                      << "No Change detected" << std::endl;
        }

        if (response.hash_changed) {
            std::cout << "----------------------------------------------------------------------" << std::endl;
            std::cout << "DNS-response for "
                      << m_opt.dns_name.toStdString()
                      << " has changed at: "
                      << response.end_timestamp.toStdString()
                      << std::endl;
            std::cout << "For DNS-server: " << response.server.toStdString() << std::endl;

            if (m_opt.verbose) {
                std::cout << "From: ";
                const auto prev_srv_record = response.prev_response;
                for (const auto& prev_srv : prev_srv_record) {
                    std::cout << prev_srv.target().toStdString()
                              << " (Prio: " << prev_srv.priority() << ")"
                              << std::endl;
                }

                std::cout << "To: ";
                const auto cur_srv_record = response.cur_response;
                for (const auto& cur_srv : cur_srv_record) {
                    std::cout << cur_srv.target().toStdString()
                    << " (Prio: " << cur_srv.priority() << ")"
                    << std::endl;
                }
            }

            std::cout << "Duration of change since start of measurement: "
                      << response.duration.toStdString()
                      << std::endl << std::endl;
        }
    }
}

void Display::update_a_display(DnsADisplayData cur_data) {
    if (m_a_responses.contains(cur_data.server)) {
        QString temp_timestamp = m_a_responses[cur_data.server].cur_timestamp;
        m_a_responses[cur_data.server] = cur_data;
        m_a_responses[cur_data.server].prev_timestamp = temp_timestamp;
    } else {
        m_a_responses[cur_data.server] = cur_data;
    }

    Display::render_a_display();
}

void Display::update_srv_display(DnsSrvDisplayData cur_data) {
    if (m_srv_responses.contains(cur_data.server)) {
        QString temp_timestamp = m_srv_responses[cur_data.server].cur_timestamp;
        m_srv_responses[cur_data.server] = cur_data;
        m_srv_responses[cur_data.server].prev_timestamp = temp_timestamp;
    } else {
        m_srv_responses[cur_data.server] = cur_data;
    }

    Display::render_srv_display();
}
