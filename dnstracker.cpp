#include "dnstracker.h"

#include <iostream>

DnsTracker::DnsTracker(const Options& o) : m_options(o) {
    if (m_options.continue_measurment) {
        DnsTracker::start_tracking();
    }

    std::cout << "Single DNS-query not implemented yet" << std::endl;
}
