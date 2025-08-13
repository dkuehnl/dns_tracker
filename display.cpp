#include "display.h"

#include <iostream>
#include <QHostAddress>

Display::Display(QString start_time, Options opt) :
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
        std::cout << "@" << response.server.toStdString() << std::endl;
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
                std::cout << response.prev_timestamp.toStdString() << "\t";
                const auto prev_srv_record = response.prev_response;
                for (const auto& prev_srv : prev_srv_record) {
                    std::cout << prev_srv.name().toStdString() << "\t"
                              << prev_srv.target().toStdString() << "\t"
                              << prev_srv.priority() << std::endl;
                }
            }
            std::cout << response.cur_timestamp.toStdString() << "\t";
            const auto cur_srv_record = response.cur_response;
            for (const auto& cur_srv : cur_srv_record) {
                std::cout << cur_srv.name().toStdString() << "\t"
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
    }
}
