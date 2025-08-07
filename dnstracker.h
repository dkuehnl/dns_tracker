#ifndef DNSTRACKER_H
#define DNSTRACKER_H

#include <qobject.h>
#include <QCoreApplication>

struct Options {
    QString dns_type;
    QString dns_server;
    QString dns_request;
    bool verbose = false;
    bool continue_measurment = false;
    bool show_help = false;
};

class DnsTracker : public QObject {
    Q_OBJECT
    Options opts;

public:
    DnsTracker(const Options& o);

private:
    Options m_options;
};



#endif // DNSTRACKER_H
