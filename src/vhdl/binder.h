
#ifndef VHDL_SEMANTIC_H
#define VHDL_SEMANTIC_H

#include <memory>
#include <tuple>

#include "ast.h"
#include "vhdl_syntax.h"
#include "vhdl_nodes.h"

#include "common/diagnostics.h"
#include "common/scope_guard.h"

namespace vhdl
{

namespace semantic
{

class binder
{
    public:
    binder(vhdl::ast*, std::shared_ptr<vhdl::node::library_unit>);
    ~binder();

    std::tuple<bool, vhdl::node::declarative_region*,
               std::vector<common::diagnostic>>
    operator()();

    // ------------------------------------------------------------------------
    // Entity
    // ------------------------------------------------------------------------
    // Implements semantic binder for these chapters from the VHDL LRM:
    // LRM93 1.0
    // ------------------------------------------------------------------------

    bool bind_entity_declaration(vhdl::syntax::design_unit*);
    bool bind_architecture_body(vhdl::syntax::design_unit*);
    bool bind_configuration_declaration(vhdl::syntax::design_unit*);


    // ------------------------------------------------------------------------
    // Subprograms and packages
    // ------------------------------------------------------------------------
    // Implements semantic binder for these chapters from the VHDL LRM:
    // LRM93 2.0
    // ------------------------------------------------------------------------

    bool bind_package_declaration(vhdl::syntax::design_unit*);
    bool bind_package_body(vhdl::syntax::design_unit*);

    // ------------------------------------------------------------------------
    // Types
    // ------------------------------------------------------------------------
    // Implements semantic binder for these chapters from the VHDL LRM:
    // LRM93 3.0
    // ------------------------------------------------------------------------

    std::tuple<bool, vhdl::node::type*> bind_type_definition(vhdl::syntax::type_definition*);

    std::tuple<bool, vhdl::node::type*> bind_enumeration_type_definition(vhdl::syntax::type_definition*);
    std::tuple<bool, vhdl::node::type*> bind_integer_type_definition(vhdl::syntax::type_definition*);
    std::tuple<bool, vhdl::node::type*> bind_floating_type_definition(vhdl::syntax::type_definition*);
    std::tuple<bool, vhdl::node::type*> bind_physical_type_definition(vhdl::syntax::type_definition*);
    std::tuple<bool, vhdl::node::type*> bind_c_array_type_definition(vhdl::syntax::type_definition*);
    std::tuple<bool, vhdl::node::type*> bind_u_array_type_definition(vhdl::syntax::type_definition*);
    std::tuple<bool, vhdl::node::type*> bind_record_type_definition(vhdl::syntax::type_definition*);
    std::tuple<bool, vhdl::node::type*> bind_access_type_definition(vhdl::syntax::type_definition*);
    std::tuple<bool, vhdl::node::type*> bind_file_type_definition(vhdl::syntax::type_definition*);

    bool bind_range(vhdl::syntax::range*);
    bool bind_discrete_range(vhdl::syntax::range*);

    // ------------------------------------------------------------------------
    // Declarations
    // ------------------------------------------------------------------------
    // Implements semantic binder for these chapters from the VHDL LRM:
    // LRM93 4.0
    // ------------------------------------------------------------------------

    bool bind_declarative_item(vhdl::syntax::declarative_item*);

    bool bind_type_declaration(vhdl::syntax::declarative_item*);
    bool bind_subtype_declaration(vhdl::syntax::declarative_item*);

    std::tuple<bool, vhdl::node::type*> bind_subtype(vhdl::syntax::subtype*);
    bool bind_index_subtype(vhdl::syntax::index_subtype*);
    bool bind_constraint(vhdl::syntax::constraint*);

    bool bind_constant_declaration(vhdl::syntax::declarative_item*);
    bool bind_signal_declaration(vhdl::syntax::declarative_item*);
    bool bind_variable_declaration(vhdl::syntax::declarative_item*);
    bool bind_file_declaration(vhdl::syntax::declarative_item*);
    bool bind_alias_declaration(vhdl::syntax::declarative_item*);
    bool bind_subprogram_declaration(vhdl::syntax::declarative_item*);
    bool bind_subprogram_body(vhdl::syntax::declarative_item*);
    bool bind_component_declaration(vhdl::syntax::declarative_item*);

    bool bind_interface_declaration(vhdl::syntax::declarative_item*);
    bool bind_interface_constant_declaration(vhdl::syntax::declarative_item*);
    bool bind_interface_signal_declaration(vhdl::syntax::declarative_item*);
    bool bind_interface_variable_declaration(vhdl::syntax::declarative_item*);
    bool bind_interface_file_declaration(vhdl::syntax::declarative_item*);

    bool bind_association_list(std::vector<vhdl::syntax::association_element*>&);

    // ------------------------------------------------------------------------
    // Names
    // ------------------------------------------------------------------------
    // Implements vhdl nodes for these chapters from the VHDL LRM:
    // LRM93 6.0
    // ------------------------------------------------------------------------

    void lookup(vhdl::syntax::name*);
    void lookup_simple_name(vhdl::syntax::name*);
    void lookup_selected_name(vhdl::syntax::name*);
    void lookup_expanded_name(vhdl::syntax::name*, vhdl::node::named_entity*);
    void lookup_selected_element(vhdl::syntax::name*, vhdl::node::named_entity*);
    void lookup_slice_name(vhdl::syntax::name*);
    void lookup_ambiguous_name(vhdl::syntax::name*);
    void lookup_fcall(vhdl::syntax::name*);

    bool resolve(vhdl::syntax::name*);

    std::tuple<bool, vhdl::node::direct_visibility*> resolve_use_name(vhdl::syntax::name*);
    std::tuple<bool, vhdl::node::declarative_region*> resolve_denotes_entity(vhdl::syntax::name*);

    // ------------------------------------------------------------------------
    // Expressions
    // ------------------------------------------------------------------------
    // Implements vhdl nodes for these chapters from the VHDL LRM:
    // LRM93 7.0
    // ------------------------------------------------------------------------

    bool resolve_binary_expression(vhdl::syntax::expression*);
    bool resolve_unary_expression(vhdl::syntax::expression*);
    bool resolve_literal_expression(vhdl::syntax::expression*);
    bool resolve_physical_expression(vhdl::syntax::expression*);
    bool resolve_aggregate_expression(vhdl::syntax::expression*);
    bool resolve_unresolved_expression(vhdl::syntax::expression*);
    bool resolve_allocator_expression(vhdl::syntax::expression*);
    bool resolve_nested_expression(vhdl::syntax::expression*);
    bool resolve_null_expression(vhdl::syntax::expression*);

    bool resolve_expression(vhdl::syntax::expression*);

    bool bind_choices(std::vector<vhdl::syntax::choice*>&);

    // ------------------------------------------------------------------------
    // Sequential statements
    // ------------------------------------------------------------------------
    // Implements vhdl nodes for these chapters from the VHDL LRM:
    // LRM93 8.0
    // ------------------------------------------------------------------------

    bool bind_sequential_statement(vhdl::syntax::sequential_statement*);
    bool bind_wait_statement(vhdl::syntax::sequential_statement*);
    bool bind_assertion_statement(vhdl::syntax::sequential_statement*);
    bool bind_report_statement(vhdl::syntax::sequential_statement*);
    bool bind_sig_assign_statement(vhdl::syntax::sequential_statement*);
    bool bind_var_assign_statement(vhdl::syntax::sequential_statement*);
    bool bind_pcall_statement(vhdl::syntax::sequential_statement*);
    bool bind_if_stmt_statement(vhdl::syntax::sequential_statement*);
    bool bind_for_loop_statement(vhdl::syntax::sequential_statement*);
    bool bind_while_loop_statement(vhdl::syntax::sequential_statement*);
    bool bind_case_stmt_statement(vhdl::syntax::sequential_statement*);
    bool bind_next_statement(vhdl::syntax::sequential_statement*);
    bool bind_exit_statement(vhdl::syntax::sequential_statement*);
    bool bind_return_stmt_statement(vhdl::syntax::sequential_statement*);
    bool bind_null_statement(vhdl::syntax::sequential_statement*);

    bool bind_target(vhdl::syntax::target*);
    bool bind_waveform(std::vector<vhdl::syntax::waveform_element*>&);

    // ------------------------------------------------------------------------
    // Concurrent statements
    // ------------------------------------------------------------------------
    // Implements vhdl nodes for these chapters from the VHDL LRM:
    // LRM93 9.0
    // ------------------------------------------------------------------------

    bool bind_concurrent_statement(vhdl::syntax::concurrent_statement*);
    bool bind_process_statement(vhdl::syntax::concurrent_statement*);
    bool bind_concurrent_assertion_statement(vhdl::syntax::concurrent_statement*);
    bool bind_concurrent_conditional_signal_assignment_statement(vhdl::syntax::concurrent_statement*);
    bool bind_concurrent_selected_signal_assignment_statement(vhdl::syntax::concurrent_statement*);
    bool bind_component_instantiation(vhdl::syntax::concurrent_statement*);
    bool bind_for_generate_statement(vhdl::syntax::concurrent_statement*);
    bool bind_if_generate_statement(vhdl::syntax::concurrent_statement*);

    // ------------------------------------------------------------------------
    // Scope and Visibility
    // ------------------------------------------------------------------------
    // Implements vhdl nodes for these chapters from the VHDL LRM:
    // LRM93 10.0
    // ------------------------------------------------------------------------

    void open_declarative_region(vhdl::node::declarative_region::v_, vhdl::syntax::design_unit*);
    void open_declarative_region(vhdl::node::declarative_region::v_, vhdl::syntax::declarative_item*);
    void open_declarative_region(vhdl::node::declarative_region::v_, vhdl::syntax::concurrent_statement*);
    void open_declarative_region(vhdl::node::declarative_region::v_, vhdl::syntax::type_definition*);
    void open_declarative_region();
    void close_declarative_region();

    bool add_named_entity(vhdl::node::named_entity*);

    std::vector<vhdl::node::named_entity*> lookup(std::string_view, vhdl::node::declarative_region*);

    // ------------------------------------------------------------------------
    // Design units and their binder
    // ------------------------------------------------------------------------
    // Implements vhdl nodes for these chapters from the VHDL LRM:
    // LRM93 11.0
    // ------------------------------------------------------------------------
    bool bind_context_clause(std::vector<vhdl::syntax::context_item*>);
    bool bind_library_clause(vhdl::syntax::context_item*);
    bool bind_use_clause(vhdl::syntax::context_item*);

    // ------------------------------------------------------------------------
    // Misc
    // ------------------------------------------------------------------------

    common::diagnostic& diag(const std::string_view msg, vhdl::token& loc);
    common::diagnostic& diag(const std::string_view msg, common::location loc);

    private:

    vhdl::node::declarative_region* current_region = nullptr;

    std::shared_ptr<vhdl::node::library_unit> unit;
    std::vector<common::diagnostic> diagnostics;
    vhdl::ast* ast;
};

}

}

#endif
