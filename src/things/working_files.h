
#ifndef THINGS_WORKING_FILES_H
#define THINGS_WORKING_FILES_H

#include <chrono>
#include <deque>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

#include "common/diagnostics.h"
#include "vhdl/ast.h"

namespace things
{

class project;

class working_file
{
    struct task
    {
        std::string name;
        std::chrono::steady_clock::time_point request_time;
        std::function<void(bool)> action;

        bool is_superseded = false;
    };

    public:
    working_file(
        std::string,
        std::function<void(std::string, std::vector<common::diagnostic>)>,
        things::project*);
    working_file(const working_file&) = delete;
    working_file(working_file&&) = delete;
    working_file& operator=(const working_file&) = delete;
    working_file& operator=(working_file&&) = delete;
    ~working_file();

    void add_task(std::string, std::function<void(bool)>);
    void forever_loop();
    void stop();

    void run_with_ast(std::function<void(std::shared_ptr<vhdl::ast>)>);
    void update();

    enum run_policy { run_on_main_thread, run_on_different_thread };
    run_policy policy;
    std::thread thread;

    std::shared_ptr<vhdl::ast> ast;

    std::vector<std::string> list_of_potentially_invalidated_reference_files;

    private:
    std::string file_;
    std::function<void(std::string, std::vector<common::diagnostic>)>
        send_diagnostics_;
    things::project* project_;
    std::vector<std::string> work_libraries_;
    int current_project_version_;
    bool library_fully_loaded_;

    std::mutex mutex_;
    std::condition_variable cv_;
    std::deque<task> queue_;

    std::atomic_bool stopped_ = false;

    void make_sure_this_is_latest_project_version_();
    void send_diagnostics_back_to_client_if_needed_();
};

class language;
class client;

// this is only used by the main thread. So there should be no need for mutexes
// and other thread synchronisation primitives. Instances of working_files are
// not meant to be moved around and definitely not meant to be copied.
//
// The synchronisation primitives here are because working_files create threads
// on the go. It needs to keep track of how many threads are running and also
// needs to ensure that no threads are running when it is destroyed. These
// primitives are here to ensure this is done properly.
class working_files
{
    public:
    working_files(things::language*, things::client*, bool);
    working_files(const working_files&) = delete;
    working_files(working_files&&) = delete;
    working_files& operator=(const working_files&) = delete;
    working_files& operator=(working_files&&) = delete;
    ~working_files();

    void run_with_ast(std::string,
                      std::function<void(std::shared_ptr<vhdl::ast>)>);
    bool update(std::string);
    void remove(std::string);
    void update_all_files();

    private:
    std::unordered_map<std::string, std::shared_ptr<working_file>>
        working_files_;

    mutable std::mutex mutex_;
    mutable std::condition_variable no_running_threads_;
    int number_of_running_threads_;

    things::language* server_;
    things::client* client_;
    bool everything_on_main_thread;
};


}

#endif
