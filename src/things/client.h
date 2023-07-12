
#ifndef THINGS_CLIENT_H
#define THINGS_CLIENT_H

#include "common/diagnostics.h"
#include "lsp/structures.h"

#include "lsp/client.h"

namespace things
{

class client;

class workdone_progress_bar
{
    client* client_;
    std::string token_;
    bool in_progress_;

    public:
    workdone_progress_bar(client*, std::string);
    workdone_progress_bar(const workdone_progress_bar&) = delete;
    workdone_progress_bar(workdone_progress_bar&&);
    workdone_progress_bar& operator=(const workdone_progress_bar&) = delete;
    workdone_progress_bar& operator=(workdone_progress_bar&&);

    virtual ~workdone_progress_bar();
    void report(int, std::string);
};

class client: public lsp::client
{
    public:

    client(lsp::frontend*);
    virtual ~client();

    void show_message(std::string);
    void log_message(std::string);
    void send_diagnostics(std::string, std::vector<common::diagnostic>);
    void send_diagnostics(std::string, std::vector<lsp::diagnostic>);

    std::optional<workdone_progress_bar> create_workdone_progress(std::string);

    private:

    std::atomic_bool workdone_progress_bar_visible;

    friend workdone_progress_bar;
    
};


}

#endif
