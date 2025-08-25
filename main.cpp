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
 * The main.cpp contains the main-business-logic for the program and also the
 * program-mode-control.
 *
 * Author: Dennis Kuehnlein (2025)
********************************************************************/

#include <iostream>
#include <getopt.h>

#include <QCoreApplication>
#include <QTimer>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QDir>

#include "dnstracker.h"
#include "display.h"

void print_help() {
    std::cout << "DNS-Tracker v1.4" << std::endl;
    std::cout << "Usage: dns_tracker -t [TYPE] -s [IP] -n [NAME] [OPTION]" << std::endl;
    std::cout << "In standard-mode an dns-request is issued and the answer displayed." << std::endl;
    std::cout << "If -c for continues measurment is activated the same request will be send every 60 seconds until quit with STRG+C" << std::endl;
    std::cout << std::endl;
    std::cout << "Mandatory arguments are labled with *" << std::endl;
    std::cout << "\t*-t DNS-TYPE (SRV, A)" << std::endl;
    std::cout << "\t*-s DNS-SERVER (IP-address)" << std::endl;
    std::cout << "\t*-n DNS-NAME" << std::endl;
    std::cout << "\t--export=FILEPATH (for file-export)" << std::endl;
    std::cout << "\t[-c SEC (continues-measurment, pulls request every 60 seconds if no value defined)]" << std::endl;
    std::cout << "\t[-v verbose-mode]" << std::endl;
    std::cout << "\t[-h show help]" << std::endl;
}

int main(int argc, char *argv[])
{
    Options opts;
    const char* short_opts = "t:s:n:m:c:vh";
    static struct option long_opts[] = {
        {"dns_type", required_argument, nullptr, 't'},
        {"dns_server", required_argument, nullptr, 's'},
        {"dns_name", required_argument, nullptr, 'n'},
        {"continue", optional_argument, nullptr, 'c'},
        {"export", optional_argument, nullptr, 'e'},
        {"verbose", no_argument, nullptr, 'v'},
        {"help", no_argument, nullptr, 'h'},
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, short_opts, long_opts, nullptr)) != -1) {
        switch (opt) {
        case 't':
            opts.dns_type = optarg;
            break;
        case 's':
            opts.multi_dns_server.push_back(optarg);
            while (optind < argc && opts.multi_dns_server.size() <=5 && argv[optind][0] != '-') {
                opts.multi_dns_server.push_back(argv[optind]);
                ++optind;
            }
            break;
        case 'n':
            opts.dns_name = optarg;
            break;
        case 'v':
            opts.verbose = true;
            break;
        case 'c':
            if (optarg != nullptr) {
                try {
                    int sec = std::stoi(optarg);
                    if (sec < 0) throw std::invalid_argument("negative value");
                    opts.sleep_intervall = static_cast<size_t>(sec) * 1000;
                } catch (const std::exception& e) {
                    std::cerr << "Unsupported time-value for continues-mode: " << optarg << std::endl;
                    return 1;
                }
            }
            opts.continue_measurment = true;
            break;
        case 'e':
            if (optarg != nullptr) {
                QString path = QString::fromUtf8(optarg);
                QFileInfo file(path);

                if (file.fileName().isEmpty()) {
                    std::cerr << "Invalid export-path: No filename specified - " << optarg << std::endl;
                    return 1;
                }

                QDir directory = file.dir();
                if (!directory.exists()) {
                    std::cerr << "Invalid export-path: directory does not exist - " << directory.absolutePath().toStdString() << std::endl;
                    return 1;
                }

                if (file.exists()) {
                    std::cerr << "Invalid export-path: file does already exist - " << file.absoluteFilePath().toStdString() << std::endl;
                    return 1;
                }
                opts.filepath = file.absoluteFilePath();

            } else {
                const char* home = getenv("HOME");
                if (home != nullptr) {
                    opts.filepath = QString::fromUtf8(home) + "/dns_tracker_output.csv";

                    QFileInfo file(opts.filepath);
                    if (file.exists()) {
                        std::cerr << "Invalid export-path: file does already exists - " << file.absoluteFilePath().toStdString() << std::endl;
                        return 1;
                    }
                } else {
                    std::cerr << "No valid home-path found, please check the env-variables" << std::endl;
                    return 1;
                }
            }
            opts.file_export = true;
            break;
        case 'h':
            opts.show_help = true;
            break;
        case '?':
        default:
            print_help();
            return 1;
        }
    }

    //Input-Validierung
    if (opts.show_help || opts.dns_type.isEmpty() || opts.dns_name.isEmpty() || opts.multi_dns_server.empty()) {
        print_help();
        return !opts.show_help;
    }
    if (opts.multi_dns_server.size() > 1) {
        opts.multi_requests = true;
    }
    if (opts.dns_type.toUpper() != "SRV" && opts.dns_type.toUpper() != "A") {
        std::cerr << "Unsupported DNS-type: " << opts.dns_type.toUpper().toStdString() << std::endl;
        print_help();
        return 1;
    }

    //Using Qt-Event-Loop only because of the conviniend QLookUp-Class
    QCoreApplication app(argc, argv);

    QList<QString> dns_server = opts.multi_dns_server;
    auto active_trackers = std::make_shared<size_t>(dns_server.size());
    auto app_ptr = &app;
    QString start_time = QDateTime::currentDateTime().toString(Qt::ISODate);
    auto display = new Display(start_time, opts);
    display->setParent(&app);

    for (const auto& server : dns_server) {
        Options server_opts = opts;
        server_opts.dns_server = server;

        auto tracker = new DnsTracker(server_opts, &app);

        QObject::connect(tracker, &DnsTracker::finished, tracker, [active_trackers, app_ptr]() mutable {
            (*active_trackers)--;
            if (*active_trackers == 0) {
                app_ptr->quit();
            }
        });
        if (server_opts.dns_type.toUpper() == "SRV") {
            QObject::connect(tracker, &DnsTracker::send_srv_update, display, &Display::update_srv_display);
        } else if (server_opts.dns_type.toUpper() == "A") {
            QObject::connect(tracker, &DnsTracker::send_a_update, display, &Display::update_a_display);
        }

        QTimer::singleShot(0, tracker, [tracker]() {
            tracker->start();
        });
    }

    return app.exec();
}
