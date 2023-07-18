
#include "vhdl/ast.h"

#include "vhdl/parser.h"
#include "vhdl/binder.h"
#include "vhdl_syntax.h"

#include <fstream>

namespace Err
{
    constexpr std::string_view File_Not_Found = "{} not found";
}

std::tuple<vhdl::library_unit_kind, unsigned, unsigned, std::string,
           std::optional<std::string>, std::string, time_t>
convert_to_tuple(vhdl::node::library_unit* ptr)
{
    vhdl::library_unit_kind kind = vhdl::library_unit_kind::invalid;
    std::string identifier;
    std::optional<std::string> identifier2;
    switch (ptr->syntax->v_kind) {
    case vhdl::syntax::design_unit::v_::entity:
        kind = vhdl::library_unit_kind::entity;
        identifier = ptr->syntax->v.entity.identifier.value;
        break;
    case vhdl::syntax::design_unit::v_::architecture:
        kind = vhdl::library_unit_kind::architecture;
        identifier = ptr->syntax->v.architecture.identifier.value;
        break;
    case vhdl::syntax::design_unit::v_::package:
        kind = vhdl::library_unit_kind::package;
        identifier = ptr->syntax->v.package.identifier.value;
        break;
    case vhdl::syntax::design_unit::v_::package_body:
        kind = vhdl::library_unit_kind::package_body;
        identifier = ptr->syntax->v.package_body.identifier.value;
        break;
    case vhdl::syntax::design_unit::v_::configuration:
        kind = vhdl::library_unit_kind::configuration;
        identifier = ptr->syntax->v.configuration.identifier.value;
        break;
    default:
        break;
    }
    return std::make_tuple(kind, 0, 0, identifier, identifier2,
                           ptr->syntax->file->filename, 0);
}

vhdl::ast::ast(std::string f, std::shared_ptr<vhdl::library_manager> m,
               std::string w)
    : filename(f), library_manager(m), worklibrary(w), invalidated_(true)
{

}

void vhdl::ast::invalidate_main_file()
{
    invalidated_ = true;
}

void vhdl::ast::invalidate_reference_file(std::string& file)
{
    for (auto& [lib, unit] : cached_library_units) {
        for (auto& it : unit) {
            if (it->syntax->file->filename == file)
            {
                it->state = vhdl::node::library_unit_state::outdated;
                invalidated_ |= true;
            }
        }
    }
}

bool vhdl::ast::update()
{
    // check if invalidated
    if (!invalidated_)
        return true;

    std::ifstream content(filename);
    if(!content.good())
    {
        parse_errors.clear();
        semantic_errors.clear();
        main_file.reset();
        return false;
    }

    content.seekg(0, std::ios::end);
    auto size = content.tellg();
    content.seekg(0);

    auto file = std::make_shared<vhdl::syntax::design_file>();
    file->filename = filename;
    file->src.resize(size);
    content.read(file->src.data(), size);

    // parse
    vhdl::parser parse_file(&strings, file.get());
    auto [ok, diags] = parse_file();

    parse_errors.swap(diags);
    main_file = file;

    // some cache house keeping
    auto& cache = cached_library_units[worklibrary];
    std::vector<std::shared_ptr<vhdl::node::library_unit>>
        libunits_we_just_parsed;
    for (auto unit : main_file->units)
    {
        auto it = std::remove_if(
            cache.begin(), cache.end(),
            [lhs = unit](auto const& rhs) { return *lhs == *rhs->syntax; });
        cache.erase(it, cache.end());

        auto libunit = std::make_shared<vhdl::node::library_unit>();
        libunit->state = vhdl::node::library_unit_state::parsed;
        libunit->syntax = unit;
        libunit->file = file;
        cache.push_back(libunit);
        libunits_we_just_parsed.push_back(libunit);
    }

    main_file->owns_units = false;

    auto lib = library_manager->get(worklibrary);

    // semantic analysis
    std::vector<common::diagnostic> big_diags;
    for (auto& libunit: libunits_we_just_parsed)
    {
        libunit->state = vhdl::node::library_unit_state::analysing;

        vhdl::semantic::binder bind(this, libunit);
        auto [ok, rdclrgn, diags] = bind();

        big_diags.insert(big_diags.end(), diags.begin(), diags.end());
        libunit->root_declarative_region = rdclrgn;
        libunit->state = vhdl::node::library_unit_state::analysed;
    }

    semantic_errors.swap(big_diags);

    if (lib->is_known())
        for (auto& libunit: libunits_we_just_parsed)
            lib->put(convert_to_tuple(libunit.get()));

    invalidated_ = false;
    return false;
}

vhdl::syntax::design_file* vhdl::ast::get_main_file()
{
    return main_file.get();
}

std::tuple<std::vector<common::diagnostic>, std::vector<common::diagnostic>>
vhdl::ast::get_diagnostics()
{
    return std::make_tuple(parse_errors, semantic_errors);
}

std::vector<std::shared_ptr<vhdl::node::library_unit>>
vhdl::ast::load_primary_unit(std::optional<std::string> library,
                             std::string_view identifier,
                             std::optional<std::string_view> identifier2)
{
    std::vector<std::shared_ptr<vhdl::node::library_unit>> candidates;

    auto& cache = cached_library_units[library.value_or(worklibrary)];
    for (auto& libunit: cache) {
        switch (libunit->state) {
        case vhdl::node::library_unit_state::outdated:
        case vhdl::node::library_unit_state::parsed:
            // do nothing. These will be evicted from the cache below
            continue;

        default:
            break;
        }

        switch (libunit->syntax->v_kind) {
        case vhdl::syntax::design_unit::v_::entity:
            if (identifier2.has_value())
                break;
            if (libunit->syntax->v.entity.identifier.value != identifier)
                break;
            candidates.push_back(libunit);
            break;

        case vhdl::syntax::design_unit::v_::package:
            if (identifier2.has_value())
                break;
            if (libunit->syntax->v.package.identifier.value != identifier)
                break;
            candidates.push_back(libunit);
            break;

        case vhdl::syntax::design_unit::v_::configuration:
            if (!identifier2.has_value())
                break;
            if (libunit->syntax->v.configuration.identifier.value != identifier)
                break;
            candidates.push_back(libunit);
            break;

        default:
            break;
        }
    }

    if (candidates.size())
        return candidates;

    auto be = library_manager->get(library.value_or(worklibrary));
    if (!be->is_known())
    {
        return candidates;
    }

    std::optional<std::string> oid2 = identifier2.has_value() ?
                            std::make_optional<std::string>(*identifier2)
                          : std::nullopt;
    auto [kind, line, column, id1, id2, filename, time] =
        be->get(std::string(identifier), oid2);
    switch (kind) {
    case vhdl::library_unit_kind::entity:
    case vhdl::library_unit_kind::package:
    case vhdl::library_unit_kind::configuration:
        break;

    case vhdl::library_unit_kind::architecture:
    case vhdl::library_unit_kind::package_body:
    default:
        return candidates; // Could not find what we are looking for. Return
                           // empty candidate list
    }

    // if we are here, actually load from library and start parse and analysis
    std::ifstream content(filename);
    if (!content.good())
        return candidates;

    content.seekg(0, std::ios::end);
    auto size = content.tellg();
    content.seekg(0);

    auto file = std::make_shared<vhdl::syntax::design_file>();
    file->filename = filename;
    file->src.resize(size);
    content.read(file->src.data(), size);

    // parse
    vhdl::parser parse_file(&strings, file.get());
    auto [ok, diags] = parse_file();

    // more cache house keeping
    std::vector<std::shared_ptr<vhdl::node::library_unit>>
        libunits_we_just_parsed;
    for (auto unit : file->units)
    {
        auto it = std::remove_if(
            cache.begin(), cache.end(),
            [lhs = unit](auto const& rhs) { return *lhs == *rhs->syntax; });
        cache.erase(it, cache.end());

        auto libunit = std::make_shared<vhdl::node::library_unit>();
        libunit->state = vhdl::node::library_unit_state::parsed;
        libunit->syntax = unit;
        libunit->file = file;
        cache.push_back(libunit);
        libunits_we_just_parsed.push_back(libunit);

        switch (unit->v_kind)
        {
        case vhdl::syntax::design_unit::v_::entity:
            if (unit->v.entity.identifier.value == identifier &&
                kind == vhdl::library_unit_kind::entity)
                candidates.push_back(libunit);
            break;
        case vhdl::syntax::design_unit::v_::package:
            if (unit->v.package.identifier.value == identifier &&
                kind == vhdl::library_unit_kind::package)
                candidates.push_back(libunit);
            break;

        default:
            break;
        }
    }

    file->owns_units = false;

    // semantic analysis
    for (auto& libunit : libunits_we_just_parsed)
    {
        libunit->state = vhdl::node::library_unit_state::analysing;

        vhdl::semantic::binder bind(this, libunit);
        auto [ok, rdclrgn, diags] = bind();

        libunit->root_declarative_region = rdclrgn;
        libunit->state = vhdl::node::library_unit_state::analysed;
    }

    return candidates;
}

std::string vhdl::ast::get_work_library_name()
{
    return worklibrary;
}

bool vhdl::ast::is_uptodate()
{
    return main_file != nullptr;
}

bool vhdl::is_a_vhdl_file(std::string& ext)
{
    return ext == ".vhd" || ext == ".vhdl";
}
