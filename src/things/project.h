
#ifndef THINGS_PROJECT_H
#define THINGS_PROJECT_H

#include <cassert>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "client.h"

#include "common/diagnostics.h"
#include "common/loguru.h"
#include "vhdl/fast_parser.h"
#include "vhdl/library_manager.h"
#include "yaml-cpp/yaml.h"


namespace things
{

// forward declaration is needed because of reasons...
class filelist;
class explorer;
class language; // ikr this is truly horrible

// a project is defined by a vhdl_config.yaml file located in the root of the
// workspace. This vhdl_config.yaml should contain the list of libraries and
// the filelist.
//
// It is not an error if vhdl_config.yaml cannot be found. Vhdlstuff will just
// do as if there was no libraries defined. Ditto if vhdlstuff finds a typo in
// the yaml. With that said, when vhdlstuff is instructed to parse a file, it
// will very well ignore that request if it cannot find the specified library.
// If vhdlstuff finds the library but cannot find the design unit it wants, it
// will flag this as an error in the diagnostic.
//
class project
{
    public:

    project(language*, client*);
    project(const project&) = delete;
    project(project&&) = delete;
    project& operator=(const project&) = delete;
    project& operator=(project&&) = delete;
    ~project() = default;

    //
    void initialise(std::optional<std::string>);

    // returns true if this project instance is currently loaded with valid
    // content from a yaml file
    bool is_loaded();

    int get_loaded_version();

    // returns true if this project instance is currently loaded with valid
    // content from a yaml file AND background exploration is done.
    bool libraries_have_been_populated();

    void set_messages_and_send_to_client(std::vector<common::diagnostic>&);
    void add_message_and_send_to_client(common::diagnostic);

    // get the current library manager
    std::shared_ptr<vhdl::library_manager> get_current_library_manager();

    // search for a yaml file in the project folder and read it.
    // If no error, reset project, kick background indexing and clear libraries
    // If error, complain about it, but dont reset project and definitely dont
    //           clear libraries.
    bool load_yaml_reset_project_kick_background_index_destroy_libraries();

    // get the list of libraries a file is currently part of
    std::vector<std::string> get_libraries_this_file_is_part_of(std::string&);

    private:
    std::filesystem::path project_folder_;

    std::optional<std::string> path_to_last_yaml_;
    std::optional<std::filesystem::path> path_to_loaded_yaml_;

    // I dont think the filelist need to be protected by a mutex. Only the main
    // thread ever uses this. Ditto for the explorer.
    std::unique_ptr<things::explorer> current_background_explorer_;
    std::shared_ptr<filelist> current_filelist_;

    std::mutex cb_mtx_;
    std::vector<common::diagnostic> diagnostics_;

    std::mutex clm_mtx_;
    std::shared_ptr<vhdl::library_manager> current_library_manager_;

    things::language* server_;
    things::client* client_;

    std::atomic_int loaded_version_;

    friend explorer;
};

// filelist keeps track of the list of files (or entries) vhdlstuff currently
// knows about and which library(ies) each of them belongs to.
class filelist
{
    public:

    // this structure represents an entry in the filelist
    struct entry
    {
        std::string filename;
        std::vector<std::string> libs;
        int index;
    };

    void add_entry(std::string, std::string);
    std::optional<entry> get_entry(std::string);

    std::mutex mtx_;
    std::vector<entry> entries;
    std::unordered_map<std::string, int> path_to_entries;
    int total_number_of_files = 0;

};

// A yaml entry is a entry from the vhdl_config.yaml file, not to be confused
// with the filelist::entry which is an internal data structure
struct yaml_entry
{
    std::string search;
    std::string library;

    std::optional<std::string> directory;
    std::optional<int> depth;
    int line_number = 0;
};

//
// this is the background project explorer. Give the project explorer a list of
// files to browse and it will look for design units in them in the background
class explorer
{
    public:
    struct progress
    {
        std::atomic_int indexed;
        std::atomic_int completed;
        std::atomic_int total;
    };

    class worker
    {
        public:

        worker(int, std::vector<things::yaml_entry>,
               std::shared_ptr<things::filelist>,
               std::shared_ptr<vhdl::library_manager>,
               progress*, std::function<void()>,
               std::function<void(common::diagnostic)>, std::string);

        // worker is not movable not copyable
        worker(const worker&) = delete;
        worker(const worker&&) = delete;
        worker& operator=(const worker&) = delete;
        worker& operator=(worker&&) = delete;
        ~worker() = default;

        int explore_entry(things::yaml_entry&);
        void work();

        // request to stop worker
        void stop();

        // is worker busy doing something? True then yes. False then nop
        bool busy();

        // is worker's job complete? True then yes. False then nop
        bool completed();

        private:

        std::atomic_bool busy_;
        std::atomic_bool quit_;
        std::atomic_bool done_;

        int id;
        std::vector<things::yaml_entry> entries;
        common::stringtable str;

        std::shared_ptr<things::filelist> filelist;
        std::shared_ptr<vhdl::library_manager> manager;

        progress* progress_;
        std::function<void()> send_progress_update;
        std::function<void(common::diagnostic)> add_message_and_send_to_client;
        std::string workspace_folder;

    };

    public:

    explorer(std::shared_ptr<things::filelist>,
             std::shared_ptr<vhdl::library_manager>,
             std::function<void(common::diagnostic)>, things::language*,
             things::client*, std::string);
    explorer(const explorer&) = delete;
    explorer(explorer&&) = delete;
    explorer& operator=(const explorer&) = delete;
    explorer& operator=(explorer&&) = delete;
    ~explorer();

    // create workers
    void start(std::vector<things::yaml_entry>&);

    // stop all workers asap
    void stop();

    // wait until all workers are done
    void join();

    // return true if background indexing is done.
    bool done();

    void send_progress_update();

    private:

    std::vector<std::unique_ptr<worker>> workers;
    std::vector<std::thread> threads;

    std::shared_ptr<things::filelist> filelist;
    std::shared_ptr<vhdl::library_manager> manager;
    std::function<void(common::diagnostic)> add_message_and_send_to_client;
    progress progress_;
    things::language* server_;
    things::client* client_;
    std::string workspace_folder;

    std::optional<things::workdone_progress_bar> progress_bar;

};

}

#endif