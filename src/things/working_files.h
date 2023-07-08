
#ifndef THINGS_WORKING_FILES_H
#define THINGS_WORKING_FILES_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

#include "vhdl/ast.h"

#include "project.h"

namespace things
{

class client;

class working_file: public std::enable_shared_from_this<working_file>
{
    struct task
    {
        std::string name;
        std::chrono::steady_clock::time_point request_time;
        std::function<void(bool)> action;

        bool is_superseded = false;
    };

    public:
    working_file(std::string, things::client*, things::project*);
    working_file(const working_file&) = delete;
    working_file(working_file&&) = delete;
    working_file& operator=(const working_file&) = delete;
    working_file& operator=(working_file&&) = delete;
    ~working_file();

    void invalidate_potentially_referenced_file(std::string);
    void forever_loop();
    void stop();

    // virtual functions that every derived working files must implement. This
    // is because the way to gather folding ranges, document symbols etc will
    // depend on the file parser / ast and will need its own bespoke
    // implementation. 
    virtual void update() = 0;
    virtual void folding_ranges(std::shared_ptr<lsp::incoming_request>) = 0;
    virtual void symbols       (std::shared_ptr<lsp::incoming_request>) = 0;
    virtual void hover         (std::shared_ptr<lsp::incoming_request>, common::position) = 0;
    virtual void definition    (std::shared_ptr<lsp::incoming_request>, common::position) = 0;

    // we support two run policies.
    // - Run on main thread: This will run tasks on the main thread.
    // - Run on different thread: This will run all tasks on a different thread
    //   In this case, there will be a `thread` running `this->foreveer_loop()`
    //   and tasks will be queued and processed first come first served.
    enum run_policy { run_on_main_thread, run_on_different_thread };
    run_policy policy;
    std::thread thread;

    protected:
    std::string file_;
    things::client* client_;
    things::project* project_;

    std::mutex mutex_;
    std::condition_variable cv_;
    std::deque<task> queue_;

    void add_task(std::string, std::function<void(bool)>);


    // We should keep track of the current loaded project version number as it
    // is possible that the vhdl_config.yaml file has been updated and in this
    // case, whatever ast we have cached is invalid and must be rebuilt.
    int current_project_version_;

    // track whether the libraries have been fully loaded (indexing is done)
    bool library_fully_loaded_;

    // This is used to know whete r a file has been updated by the user and its
    // content should be recompiled
    std::mutex mutex_to_invalidate_files_;
    std::vector<std::string> list_of_potentially_referenced_files_now_invalid;

    std::atomic_bool stopped_ = false;
};

// forward declaration because of cyclic dependence issues
class language;

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

    bool update(std::string);
    void remove(std::string);
    void update_all_files();

    void folding_ranges(std::string, std::shared_ptr<lsp::incoming_request>);
    void symbols       (std::string, std::shared_ptr<lsp::incoming_request>);
    void hover         (std::string, std::shared_ptr<lsp::incoming_request>, common::position);
    void definition    (std::string, std::shared_ptr<lsp::incoming_request>, common::position);

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


class vhdl_working_file : public working_file
{
    public:
    vhdl_working_file(std::string, things::client*, things::project*);

    void update();
    void folding_ranges(std::shared_ptr<lsp::incoming_request>);
    void symbols       (std::shared_ptr<lsp::incoming_request>);
    void hover         (std::shared_ptr<lsp::incoming_request>, common::position);
    void definition    (std::shared_ptr<lsp::incoming_request>, common::position);

    private:
    std::shared_ptr<vhdl::ast> ast;
    std::vector<std::string> work_libraries_;

    void run_with_vhdl_ast(std::function<void(std::shared_ptr<vhdl::ast>)>);
    void make_sure_this_is_latest_project_version();
    void send_diagnostics_back_to_client_if_needed();
};

}

#endif
