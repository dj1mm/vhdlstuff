
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
#include "slang/text/SourceManager.h"
#include "sv/library_manager.h"
#include "vhdl/fast_parser.h"
#include "vhdl/library_manager.h"
#include "yaml-cpp/yaml.h"


namespace things
{

// This namespace holds the c++ classes to model the content and to parse the
// contents of the vjhdl_config.yaml file.
namespace config
{

struct library_spec;

struct file_filter
{
    std::string directory;
    std::string search;
    std::optional<int> depth;
};

struct folder_spec
{
    std::string path;
    things::config::library_spec* library;

    int line = -1;   // the line in the vhdl_config.yaml this spec comes from
    int column = -1; // the column in the vhdl_config.yaml this spec comes from
};

using folder_specs = std::vector<things::config::folder_spec>;

struct file_spec
{
    bool is_path() const;
    bool is_file_query() const;
    const std::string& as_path() const;
    const file_filter& as_file_query() const;

    things::config::library_spec* library;
    std::variant<std::string, file_filter> content;

    int line = -1;   // the line in the vhdl_config.yaml this spec comes from
    int column = -1; // the column in the vhdl_config.yaml this spec comes from

    std::string to_string();
};

using file_specs = std::vector<things::config::file_spec>;
using file_specs_ptr = std::vector<things::config::file_spec*>;

struct library_spec
{
    std::string name;
    std::vector<file_spec> files;
    std::vector<folder_spec> incdirs;
};

using library_specs = std::vector<things::config::library_spec>;

struct root
{
    std::vector<library_spec> vhdl;
    library_spec sv;
};



} // namespace config

// forward declaration is needed because of reasons...
class filelist;
class explorer;
class language; // ikr this is truly horrible

// a project is defined by a vhdl_config.yaml file located in the root of the
// workspace. This vhdl_config.yaml should contain the list of libraries and
// the filelist.
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

    // this is the workspace folder which is also the folder from which the
    // language server will look for a vhdl_config.yaml file
    void set_project_folder(std::string);

    // returns true if this project instance is currently loaded with valid
    // content from a yaml file
    bool is_loaded();

    int get_loaded_version();

    // returns true if this project instance is currently loaded with valid
    // content from a yaml file AND background exploration is done.
    bool libraries_have_been_populated();

    // get the current library manager
    std::shared_ptr<vhdl::library_manager> get_current_library_manager();
    std::shared_ptr<sv::library_manager> get_current_sv_library_manager();

    // search for a yaml file in the project folder and read it.
    // If no error, reset project, kick background indexing and clear libraries
    // If error, complain about it, but dont reset project and definitely dont
    //           clear libraries.
    bool reload_yaml_reset_project_kick_background_index_destroy_libraries();

    // get the list of libraries a file is currently part of
    std::vector<std::string> get_libraries_this_file_is_part_of(std::string&);

    private:
    std::filesystem::path project_folder_;

    std::optional<std::filesystem::path> path_to_loaded_yaml_;

    // I dont think the filelist need to be protected by a mutex. Only the main
    // thread ever uses this. Ditto for the explorer.
    std::unique_ptr<things::explorer> current_background_explorer_;
    std::shared_ptr<filelist> current_filelist_;

    std::mutex clm_mtx_;
    std::shared_ptr<vhdl::library_manager> current_library_manager_;
    std::shared_ptr<sv::library_manager> current_sv_library_manager_;

    things::language* server_;
    things::client* client_;

    std::atomic_int loaded_version_;

    friend explorer;
};

// The filelist object keeps track of the list of files (or file entries) the
// language server is currently aware of and which library(ies) each of them
// belongs to.
class filelist
{
    public:

    // this structure represents a file (or an entry) in the filelist.
    //
    // Each file (or entry) can be mapped exactly to a file specification in
    // the vhdl_config.yaml. This is the *spec pointer.
    //
    // It is possible that the vhdl_config.yaml written by the programmer
    // assigns a file to multiple libraries - which is wrong. We still want to
    // represent this in the filelist so errors can be generated. We use the
    // *next pointer for this.
    struct entry
    {
        things::config::file_spec* spec;
        entry* next;
    };

    void add_entry(std::string, things::config::file_spec*);
    entry* get_entry(std::string);

    std::mutex mtx_;
    std::vector<std::unique_ptr<entry>> entries;
    std::unordered_map<std::string, entry*> path_to_entries;
    int total_number_of_files = 0;

    std::unique_ptr<things::config::root> vhdl_config;

};

// this is the background project explorer. Give it a list of files (or specs
// or tasks - unfortunately the naming is not fixed yet) to browse and it will
// look for design units in them in the background
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

        worker(int, int, things::config::file_specs_ptr,
               std::shared_ptr<things::filelist>,
               std::shared_ptr<vhdl::library_manager>,
               std::shared_ptr<sv::library_manager>,
               progress*, std::function<void()>,
               std::string, client*, std::string);

        // worker is not movable not copyable
        worker(const worker&) = delete;
        worker(const worker&&) = delete;
        worker& operator=(const worker&) = delete;
        worker& operator=(worker&&) = delete;
        ~worker() = default;

        int explore_spec(things::config::file_spec*);
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

        std::string header;
        int version;
        int id;
        std::vector<things::config::file_spec*> specs;
        common::stringtable str;

        slang::SourceManager sm;

        std::shared_ptr<things::filelist> filelist;
        std::shared_ptr<vhdl::library_manager> manager;
        std::shared_ptr<sv::library_manager> sv_manager;

        progress* progress_;
        std::function<void()> send_progress_update;
        std::string path_to_yaml;
        client* client_;
        std::string workspace_folder;

    };

    public:

    explorer(int, std::shared_ptr<things::filelist>,
             std::shared_ptr<vhdl::library_manager>,
             std::shared_ptr<sv::library_manager>,
             std::string, things::language*,
             things::client*, std::string);
    explorer(const explorer&) = delete;
    explorer(explorer&&) = delete;
    explorer& operator=(const explorer&) = delete;
    explorer& operator=(explorer&&) = delete;
    ~explorer();

    // create workers
    void start(things::config::file_specs_ptr&);

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
    std::shared_ptr<sv::library_manager> sv_manager;
    std::string path_to_yaml;
    progress progress_;
    things::language* server_;
    things::client* client_;
    std::string workspace_folder;

    std::optional<things::workdone_progress_bar> progress_bar;
    int version_;
    std::string header;

};

} // namespace things

namespace YAML
{

// these are the parsers to automatically convert a vhdl_config.yaml to its
// corresponding hierarchy of c++ classes.

template <>
struct convert<things::config::root>
{
    static Node encode(const things::config::root& rhs)
    {
        Node node;

        node["vhdl"] = rhs.vhdl;
        node["sv"] = rhs.sv;

        return node;
    }

    static bool decode(const YAML::Node& node, things::config::root& rhs)
    {
        if (!node.IsMap())
            return false;

        auto has_vhdl = node["vhdl"].IsDefined();
        auto has_sv = node["sv"].IsDefined();

        if ((has_vhdl || has_sv) == false)
            throw RepresentationException(node.Mark(), "Missing a vhdl or "
                                                       "sv section");
        if (has_vhdl)
            rhs.vhdl = node["vhdl"].as<things::config::library_specs>();
        if (has_sv)
            rhs.sv = node["sv"].as<things::config::library_spec>();
        return true;
    }
};

template <>
struct convert<things::config::library_spec>
{
    static Node encode(const things::config::library_spec& rhs)
    {
        Node node;

        node["name"] = rhs.name;
        for (auto file : rhs.files)
            node["files"].push_back(file);
        for (auto folder : rhs.incdirs)
            node["incdirs"].push_back(folder);

        return node;
    }

    static bool decode(const YAML::Node& node, things::config::library_spec& rhs)
    {
        if (!node.IsMap())
            return false;

        auto has_name = node["name"].IsDefined();
        auto has_files = node["files"].IsDefined();
        auto has_incdirs = node["incdirs"].IsDefined();

        if ((has_name && has_files) == false)
            throw RepresentationException(node.Mark(), "a library must contain "
                                                       "a name and a files "
                                                       "section");
        if (has_name)
            rhs.name = node["name"].as<std::string>();
        if (has_files)
            rhs.files = node["files"].as<things::config::file_specs>();
        if (has_incdirs)
            rhs.incdirs = node["incdirs"].as<things::config::folder_specs>();
        return true;
    }
};

template<>
struct convert<things::config::file_filter>
{
    static Node encode(const things::config::file_filter& rhs)
    {
        Node node;

        node["directory"] = rhs.directory;
        node["search"] = rhs.search;
        if (rhs.depth.has_value())
            node["depth"] = rhs.depth.value();

        return node;
    }

    static bool decode(const YAML::Node& node, things::config::file_filter& rhs)
    {
        if (!node.IsMap())
            return false;

        auto has_directory = node["directory"].IsDefined();
        auto has_search = node["search"].IsDefined();
        auto has_depth = node["depth"].IsDefined();

        if ((has_directory && has_search) == false)
            throw RepresentationException(node.Mark(),
                                          "a file specification must contain a "
                                          "directory and a search field");
        if (has_directory)
            rhs.directory = node["directory"].as<std::string>();
        if (has_search)
            rhs.search = node["search"].as<std::string>();
        if (has_depth)
            rhs.depth = node["depth"].as<int>();

        return true;
    }
};

template <>
struct convert<things::config::file_spec>
{
    static Node encode(const things::config::file_spec& rhs)
    {
        Node node;

        if (rhs.is_path())
            node = rhs.as_path();
        if (rhs.is_file_query())
            node = rhs.as_file_query();

        return node;
    }

    static bool decode(const YAML::Node& node,
                       things::config::file_spec& rhs)
    {
        if ((node.IsScalar() || node.IsMap()) == false)
            throw RepresentationException(node.Mark(), "a file is either a "
                                                       "string or a file "
                                                       "specification");

        if (node.IsScalar())
            rhs.content = node.as<std::string>();
        if (node.IsMap())
        {
            things::config::file_filter query;
            convert<things::config::file_filter>::decode(node, query);
            rhs.content = query;
        }

        rhs.line = node.Mark().line + 1;
        rhs.column = node.Mark().column + 1;
        return true;
    }
};

template <>
struct convert<things::config::folder_spec>
{
    static Node encode(const things::config::folder_spec& rhs)
    {
        Node node(rhs.path);

        return node;
    }

    static bool decode(const YAML::Node& node,
                       things::config::folder_spec& rhs)
    {
        if (!node.IsScalar())
            return false;

        rhs.path = node.as<std::string>();

        rhs.line = node.Mark().line + 1;
        rhs.column = node.Mark().column + 1;
        return true;
    }
};

} // namespace YAML

#endif