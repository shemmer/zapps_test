#ifndef COMMAND_H
#define COMMAND_H


#include "sm_base.h"

#include "basethread.h"
#include "handler.h"
#include "scanner.h"
#include <boost/program_options.hpp>

namespace po = boost::program_options;

class Command : public basethread_t {
public:
    virtual void run() = 0;
    virtual void setupOptions() = 0;

    static Command* parse(int argc, char** argv);
    static void init();

    po::options_description& getOptions() { return options; }
    void setCommandString(string s) { commandString = s; }
    void setOptionValues(po::variables_map& vm) { optionValues = vm; }

protected:
    po::options_description options;
    po::variables_map optionValues;

    string commandString;

    //Setup Options that can be used in every command instance i.e. are independent of executed benchmark or test -> Mostly shore config options
    void setupSMOptions();

    void helpOption();

private:
    typedef map<string, Command*(*)()> ConstructorMap;
    static ConstructorMap constructorMap;

    static void showCommands();
};

class LogScannerCommand : public Command {
public:
    static size_t BLOCK_SIZE;

    virtual void setupOptions();
protected:
    BaseScanner* getScanner( bitset<logrec_t::t_max_logrec>* filter = NULL);
    BaseScanner* getMergeScanner();
    BaseScanner* getLogArchiveScanner();

    string logdir;

private:
    BaseScanner* scanner;
    size_t limit;
};

#endif
