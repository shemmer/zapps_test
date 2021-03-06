#ifndef VERIFYLOG_H
#define VERIFYLOG_H

#include "command.h"
#include "handler.h"

class VerifyLog : public LogScannerCommand
{
public:
    void setupOptions();
    void run();
private:
    bool merge;
    string archdir;
    string logdir;
};

class VerifyHandler : public Handler {
public:
    VerifyHandler(bool merge);
    virtual ~VerifyHandler() {};

    virtual void invoke(logrec_t& r);
    virtual void finalize();
    virtual void newFile(const char* fname);
private:
    lsn_t minLSN;
    lsn_t maxLSN;
    lsn_t lastLSN;
    lpid_t lastPID;
    long count;
    bool merge;
};;

#endif
