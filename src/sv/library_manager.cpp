
#include "sv/library_manager.h"

#include "sqlite3/sqlite3.h"
#include <sstream>

std::size_t id(sv::library_cell_kind kind, std::string identifier,
               std::optional<std::string> identifier2)
{
    std::size_t h0 = 0;
    switch (kind) {
    case sv::library_cell_kind::module:    h0 = 2;  break;
    case sv::library_cell_kind::interface: h0 = 4;  break;
    case sv::library_cell_kind::package:   h0 = 8;  break;
    case sv::library_cell_kind::program:   h0 = 16; break;
    case sv::library_cell_kind::clazz:     h0 = 32; break;
    default:                                        break;
    }   
    auto h1 = std::hash<std::string>{}(identifier);
    auto h2 = h0 ^ (h1 << 1);

    h0 = std::hash<std::string>{}(identifier2.value_or(""));
    return h0 ^ (h2 << 1);
}

sv::library_backend::library_backend(std::optional<std::string> location,
                                     std::string name, bool known)
    : location_(":memory:"), name_(name), is_valid_(true), is_known_(known),
      has_internal_problem_(false), db_(nullptr)
{
    if (location.has_value())
    {
        location_ = location.value() + "/" + name + ".sv.db";
    }
}

sv::library_backend::~library_backend()
{
    if (db_ != nullptr)
    {
        // destroy it
        auto rc = sqlite3_close(db_);
        if (rc == SQLITE_OK)
        {
            db_ = nullptr;
        }
    }
}

std::string sv::library_backend::get_location()
{
    return location_;
}

std::string sv::library_backend::get_name()
{
    return name_;
}

bool sv::library_backend::is_valid()
{
    return is_valid_;
}

bool sv::library_backend::is_known()
{
    return is_known_;
}

bool sv::library_backend::has_internal_problem()
{
    return has_internal_problem_;
}

// get architecture body or configuration
std::tuple<sv::library_cell_kind, unsigned, unsigned, std::string,
           std::optional<std::string>, std::string, time_t>
sv::library_backend::get(std::string identifier,
                           std::optional<std::string> identifier2)
{
    sv::library_cell_kind kind = sv::library_cell_kind::invalid;
    unsigned line = 0;
    unsigned column = 0;
    std::string filename;
    time_t timestamp = 0;
    if (!connected_and_tables_exists())
    {
        return std::make_tuple(kind, line, column, identifier, identifier2,
                               filename, timestamp);
    }

    // build the sql statement
    std::stringstream ss;
    ss << "SELECT * from LIBRARY_UNITS where IDENTIFIER='" << identifier << "' and IDENTIFIER2";
    if (identifier2) ss << "='" << identifier2.value() << "'";
    else             ss << " is NULL";
    ss << " LIMIT 1;";
    std::string sql(ss.str());

    // compile sql statement to binary
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        return std::make_tuple(kind, line, column, identifier, identifier2,
                               filename, timestamp);
    }

    // execute sql statement
    bool found = false;
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        line      = sqlite3_column_int(stmt, 1);
        timestamp = sqlite3_column_int(stmt, 2);
        filename  = std::string((const char*) sqlite3_column_text(stmt, 3));
        switch (sqlite3_column_int(stmt, 4)) {
        case 1:  kind = sv::library_cell_kind::module;      break;
        case 2:  kind = sv::library_cell_kind::interface;   break;
        case 3:  kind = sv::library_cell_kind::package;     break;
        case 4:  kind = sv::library_cell_kind::program;     break;
        case 5:  kind = sv::library_cell_kind::clazz;       break;
        default: kind = sv::library_cell_kind::invalid;     break;
        }
        column = 0;
        found = true;
    }
    if(rc != SQLITE_DONE || !found)
    {
        kind = sv::library_cell_kind::invalid;
    }

    //release resources
    sqlite3_finalize(stmt);

    return std::make_tuple(kind, line, column, identifier, identifier2, filename, timestamp);
}

bool sv::library_backend::put(library_cell_kind kind, unsigned line,
                              unsigned column, std::string identifier,
                              std::optional<std::string> identifier2,
                              std::string filename, time_t timestamp)
{
    if (!connected_and_tables_exists())
    {
        return false;
    }

    std::stringstream ss;
    ss << "INSERT OR REPLACE INTO LIBRARY_UNITS (ID,LINENUMBER,TIMESTAMP,FILENAME,DESIGNUNIT,IDENTIFIER,IDENTIFIER2) VALUES (";
    ss << id(kind, identifier, identifier2) << "," << line << "," << timestamp << ",'" << filename << "',";
    switch (kind) {
    case sv::library_cell_kind::module:    ss << "1,'" << identifier << "',"  << "NULL"; break;
    case sv::library_cell_kind::interface: ss << "2,'" << identifier << "',"  << "NULL"; break;
    case sv::library_cell_kind::package:   ss << "3,'" << identifier << "',"  << "NULL"; break;
    case sv::library_cell_kind::program:   ss << "4,'" << identifier << "',"  << "NULL"; break;
    case sv::library_cell_kind::clazz:     ss << "5,'" << identifier << "',"  << "NULL"; break;
    default:
        return false;
    }
    ss << ");";
    std::string sql(ss.str());

    auto rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, nullptr);
    
    if (rc != SQLITE_OK)
    {
        return false;
    }

    return true;
}

void sv::library_backend::clear()
{
    if (!connected_and_tables_exists())
    {
        return;
    }

    // Create SQL statement
    auto sql = "DELETE from LIBRARY_UNITS;";

    // Execute SQL statement
    auto rc = sqlite3_exec(db_, sql, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK)
    {
        // this is an error, but there's nothing we can do about it :/
    }
}

std::vector<std::tuple<sv::library_cell_kind, unsigned, unsigned, std::string,
                       std::optional<std::string>, std::string, time_t>>
sv::library_backend::all(int limit, std::optional<std::string> filter)
{
    std::vector<std::tuple<library_cell_kind, unsigned, unsigned, std::string,
                           std::optional<std::string>, std::string, time_t>>
        result;
    if (!connected_and_tables_exists())
    {
        return result;
    }

    // build the sql statement
    std::stringstream ss;
    ss << "SELECT * from LIBRARY_UNITS";
    if (filter) ss << " WHERE IDENTIFIER='" << filter.value() << "' OR IDENTIFIER2='" << filter.value() << "'";
    if (limit != 0) ss << " LIMIT " << limit;
    ss << " ;";
    std::string sql(ss.str());

    // compile sql statement to binary
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        sqlite3_finalize(stmt);
        return result;
    }

    // execute sql statement
    bool found = false;
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        unsigned    line      = sqlite3_column_int(stmt, 1);
        time_t      timestamp = sqlite3_column_int(stmt, 2);
        std::string filename  = std::string((const char*) sqlite3_column_text(stmt, 3));
        sv::library_cell_kind kind;
        switch (sqlite3_column_int(stmt, 4)) {
        case 1:  kind = sv::library_cell_kind::module;    break;
        case 2:  kind = sv::library_cell_kind::interface; break;
        case 3:  kind = sv::library_cell_kind::package;   break;
        case 4:  kind = sv::library_cell_kind::program;   break;
        case 5:  kind = sv::library_cell_kind::clazz;     break;
        default: kind = sv::library_cell_kind::invalid;   break;
        }
        std::string identifier = std::string((const char*) sqlite3_column_text(stmt, 5));
        std::optional<std::string> identifier2;
        switch (kind) {
        default:
            break;
        }
        unsigned column = 0;
        result.push_back(std::make_tuple(kind, line, column, identifier,
                                         identifier2, filename, timestamp));
        found = true;
    }
    if(rc != SQLITE_DONE || !found)
    {
        // there's nothing we can do
    }

    //release resources
    sqlite3_finalize(stmt);

    return result;
}


bool sv::library_backend::connected_and_tables_exists()
{
    auto rc = SQLITE_OK;
    if (db_ == nullptr) // create database
    {
        rc = sqlite3_open(location_.c_str(), &db_);
    }

    if (rc != SQLITE_OK || has_internal_problem_ || !is_valid_ || !db_)
    {
        return false;
    }

    // Create SQL statement
    auto sql = "CREATE TABLE IF NOT EXISTS LIBRARY_UNITS (" \
               "ID INT PRIMARY KEY  NOT NULL," \
               "LINENUMBER     INT  NOT NULL," \
               "TIMESTAMP      INT  NOT NULL," \
               "FILENAME       TEXT NOT NULL," \
               "DESIGNUNIT     INT  NOT NULL," \
               "IDENTIFIER     TEXT NOT NULL," \
               "IDENTIFIER2    TEXT);";

    // Execute SQL statement
    rc = sqlite3_exec(db_, sql, nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK)
    {
        has_internal_problem_ = true;
        return false;
    }

    return true;
}

sv::library_manager::library_manager(std::optional<std::string> l, bool p)
: is_initialised_(false), location_(l), fully_populated_(p)
{

}

void sv::library_manager::initialise(std::vector<std::string> names)
{
    std::unique_lock g(mtx_);
    for (auto& it: lbe_)
    {
        it.second->is_valid_ = false;
    }
    lbe_.clear();
    location_.reset();
    is_initialised_ = true;

    for (auto name: names)
    {
        auto it = lbe_.find(name);
        if(it == lbe_.end())
        {
            auto lbe = std::make_shared<sv::library_backend>(std::nullopt, name, true);
            lbe_.insert(std::make_pair(name, lbe));
        }
    }
}

void sv::library_manager::destroy()
{
    std::unique_lock g(mtx_);
    for (auto& it: lbe_)
    {
        it.second->is_valid_ = false;
    }
    lbe_.clear();
}

std::vector<std::string> sv::library_manager::list()
{
    std::shared_lock g(mtx_);
    std::vector<std::string> names;
    for (auto be: lbe_)
    {
        if (be.second->is_valid() && be.second->is_known())
            names.push_back(be.second->get_name());
    }

    return names;

}

std::shared_ptr<sv::library_backend> sv::library_manager::get(std::string name)
{
    {
    // new scope to check whether we have the interesting library in our deck
    // of libraries

    std::shared_lock g(mtx_);
    auto it = lbe_.find(name);
    if(it != lbe_.end())
    {
        return it->second;
    }

    }

    // At this point, turns out we dont have the library we are interested in
    // in our deck. Create a new backend
    std::unique_lock g(mtx_);
    auto result = std::make_shared<sv::library_backend>(location_, name, !is_initialised_);
    lbe_.insert(std::make_pair(name, result));
    return result;
}

void sv::library_manager::set_fully_populated(bool value)
{
    fully_populated_.store(value);
}

bool sv::library_manager::is_fully_populated()
{
    return fully_populated_.load(std::memory_order_relaxed);
}
