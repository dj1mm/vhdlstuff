
#ifndef VHDL_AST_H
#define VHDL_AST_H

#include <memory>
#include <vector>

#include "vhdl/library_manager.h"
#include "common/diagnostics.h"
#include "common/stringtable.h"

namespace vhdl
{

// forward declaration
namespace syntax
{
class design_file;
};

// forward declaration
namespace node
{
class library_unit;
};

// Vhdl ast is a container for all the things needed around a vhdl design file.
// These include things such as the string table, the vhdl resource libraries
// and very important: a cache of all library units dependencies.
//
//
class ast: public std::enable_shared_from_this<vhdl::ast>
{
    public:
    ast(std::string, std::shared_ptr<vhdl::library_manager>, std::string);
    ast(const ast&) = delete;
    ast(ast&&) = default;
    ast& operator=(const ast&) = delete;
    ast& operator=(ast&&) = default;
    ~ast() = default;

    // this function can take a while to return because all the parsing and
    // semantic analysis will be done there.
    //
    // Returns true if ast was already up to date and nothing was done.
    // Returns false otherwise.
    // In this case, it means that the ast was rebuilt/updated. I think this
    // can be used by the caller to determine whether there are new diagnostics
    // to send to the client
    bool update();

    // this function will return quickly
    void invalidate_main_file();

    // this function will return quickly
    void invalidate_reference_file(std::string&);

    // this function will return quickly
    // Note that the main file might be out of date if invalidate_x() functions
    // were called between the last update() and this one. However, the main
    // file is still useful.
    // This function can return nullptr. In which case, it means that file not
    // found
    vhdl::syntax::design_file* get_main_file();

    // this function will return quickly
    // return the current parse errors and semantic errors.
    //
    // Note that the returned errors might be out of date if the invalidate_x()
    // functions were called between the last update() and this one. However,
    // the diags are still useful.
    std::tuple<std::vector<common::diagnostic>, std::vector<common::diagnostic>>
    get_diagnostics();

    // Load primary unit from a library.
    std::vector<std::shared_ptr<vhdl::node::library_unit>> load_primary_unit(
        std::optional<std::string>, std::string_view,
        std::optional<std::string_view>);

    // Get work library where the main design units in this ast shall be stored
    std::string get_work_library_name();

    // Return true if no invalidate_x() functions were called between the last
    // update() and this one.
    bool is_uptodate();

    private:

    std::string filename;
    std::string worklibrary;
    common::stringtable strings;

    std::shared_ptr<vhdl::library_manager> library_manager;
    std::shared_ptr<vhdl::syntax::design_file> main_file;

    std::vector<common::diagnostic> parse_errors;
    std::vector<common::diagnostic> semantic_errors;

    std::unordered_map<std::string,
                       std::vector<std::shared_ptr<vhdl::node::library_unit>>>
        cached_library_units;

    bool invalidated_;
};

bool is_a_vhdl_file(std::string&);

}

#endif
