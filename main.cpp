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

#include "dnstracker.h"

void print_help() {
    std::cout << "Usage: dns_tracker -t [TYPE] -s [IP] -n [NAME] [OPTION]" << std::endl;
    std::cout << "In standard-mode an dns-request is issued and the answer displayed." << std::endl;
    std::cout << "If -c for continues measurment is activated the same request will be send every 30 seconds until quit with STRG+C or the response changed." << std::endl;
    std::cout << std::endl;
    std::cout << "Mandatory arguments are labled with *" << std::endl;
    std::cout << "\t*-t DNS-TYPE (NAPTR, SRV, A)" << std::endl;
    std::cout << "\t*-s DNS-SERVER (IP-address)" << std::endl;
    std::cout << "\t*-n DNS-NAME" << std::endl;
    std::cout << "\t[-c continues-measurment, pulls request every 30 seconds]" << std::endl;
    std::cout << "\t[-v verbose-mode]" << std::endl;
    std::cout << "\t[-h show help]" << std::endl;
}

int main(int argc, char *argv[])
{
    Options opts;
    const char* short_opts = "t:s:n:vch";
    static struct option long_opts[] = {
        {"dns_type", required_argument, nullptr, 't'},
        {"dns_server", required_argument, nullptr, 's'},
        {"dns_name", required_argument, nullptr, 'n'},
        {"verbose", no_argument, nullptr, 'v'},
        {"continue", no_argument, nullptr, 'c'},
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
            opts.dns_server = optarg;
            break;
        case 'n':
            opts.dns_name = optarg;
            break;
        case 'v':
            opts.verbose = true;
            break;
        case 'c':
            opts.continue_measurment = true;
            break;
        case 'h':
            opts.show_help = true;
            break;
        case '?':
        default:
            std::cout << "help gesetzt" << std::endl;
            return 1;
        }
    }

    if (opts.show_help || opts.dns_type.isEmpty() || opts.dns_name.isEmpty() || opts.dns_server.isEmpty()) {
        print_help();
        return !opts.show_help;
    }

    //Using Qt-Event-Loop only because of the conviniend QLookUp-Class
    QCoreApplication app(argc, argv);
    DnsTracker tracker(opts);

    return app.exec();
}
