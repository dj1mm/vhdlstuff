
#ifndef VHDL_LIBRARY_MANAGER_H
#define VHDL_LIBRARY_MANAGER_H

#include <atomic>
#include <unordered_map>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <vector>

#include "common/serialize.h"

// forward declaration
struct sqlite3;

namespace vhdl
{

enum class library_unit_kind
{
    invalid,
    entity,
    architecture,
    package,
    package_body,
    configuration
};

class library_manager;

// sqlite library backend that interfaces with the sqlite database. Frontends
// use this to get/put units out/into the sqlite library.
//
// Note the library backend can be used by multiple frontends at the same time
// and each frontends can be in different threads. So library backend must be
// threadsafe. Seems sqlite itself is thread safe...
// https://www.sqlite.org/threadsafe.html
// https://vadosware.io/post/sqlite-is-threadsafe-and-concurrent-access-safe-but/
//
// Interface is via
//  library_backend::get()
//  library_backend::put()
//  library_backend::clear()
//  library_backend::all()
//
//
// A library backend has validity. Validity depends on whether the library
// manager has disowned the library manager.
//
// A library backend also stores whether it is a known library or an unknown
// library. An unknown library is one that has not been defined in the project
// configuration file .vhdlstuff, and the backend will not save the library to
// the filesystem. A known library will be loaded/stored to the filesystem.
class library_backend
{
    friend library_manager;

    public:

    library_backend(std::optional<std::string>, std::string, bool);
    library_backend(library_backend&&) = default;
    library_backend(const library_backend&) = default;
    library_backend& operator=(library_backend&&) = default;
    library_backend& operator=(const library_backend&) = default;
    ~library_backend();

    std::string get_location();
    std::string get_name();
    bool is_valid();
    bool is_known();
    bool has_internal_problem();

    std::tuple<library_unit_kind, unsigned, unsigned, std::string,
               std::optional<std::string>, std::string, time_t>
        get(std::string, std::optional<std::string> = std::nullopt);

    bool put(std::tuple<library_unit_kind, unsigned, unsigned, std::string,
                        std::optional<std::string>, std::string, time_t>);

    void clear();

    std::vector<std::tuple<library_unit_kind, unsigned, unsigned, std::string,
                           std::optional<std::string>, std::string, time_t>>
    all(int = 0, std::optional<std::string> = std::nullopt);

    private:
    bool connected_and_tables_exists();

    std::string location_;
    std::string name_;
    bool has_internal_problem_;
    bool is_known_;
    bool is_valid_;
    sqlite3* db_;
};

// library manager manages vhdl libraries and library units stored in them. The
// library manager is the way to access vhdl libraries. Every vhdl library can
// store multiple library units.
//
// Before fetching libraries, the library manager must either:
// - be told where libraries are stored on the filesystem, or
//   This is easy, pass the location when creating the manager
// - be told which libraries it should be aware of
//   To do this, pass std::nullopt when creating the manager, and then call the
//   initialise() function
//
// Take note that there should only be ONE library manager in the entire
// program. Enforcement of the singleton is delegated to the programmer.
class library_manager
{
    public:

    library_manager(std::optional<std::string>, bool=true);
    library_manager(library_manager&&) = delete;
    library_manager(const library_manager&) = delete;
    library_manager& operator=(library_manager&&) = delete;
    library_manager& operator=(const library_manager&) = delete;
    ~library_manager() = default;

    void initialise(std::vector<std::string>);
    void destroy();
    std::vector<std::string> list();

    std::shared_ptr<vhdl::library_backend> get(std::string);

    void set_fully_populated(bool);
    bool is_fully_populated();

    private:

    bool is_initialised_;
    std::optional<std::string> location_;

    std::shared_mutex mtx_;
    std::unordered_map<std::string, std::shared_ptr<library_backend>> lbe_;

    std::atomic_bool fully_populated_;
};

}

#endif
