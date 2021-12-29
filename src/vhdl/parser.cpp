
#include <fstream>
#include <optional>
#include <sstream>

#include "parser.h"
#include "common/scope_guard.h"

namespace Err
{
    constexpr std::string_view Assert_report_before_severity = "report must be before severity";
    constexpr std::string_view Block_guard = "Right parenthesis expected after guard expression";
    constexpr std::string_view Block_label = "Block statement must have a label";
    constexpr std::string_view Case_missing_alternative = "Missing alternative in case statement";
    constexpr std::string_view Char_entity_designator_vhdl87 = "Character is not a valid entity designator in vhdl87";
    constexpr std::string_view Colon_in_attr_spec = "use keyword of after attribute identifier";
    constexpr std::string_view Empty_assoc_list = "Empty association list";
    constexpr std::string_view Empty_design_file = "Design file is empty";
    constexpr std::string_view End_architecture_vhdl87 = "Keyword not allowed in vhdl87";
    constexpr std::string_view End_config_keyword_vhdl87 = "'configuration' keyword not allowed in vhdl87";
    constexpr std::string_view End_name_misspelling = "Mispelling, expected {}";
    constexpr std::string_view End_name_not_allowed_vhdl87 = "End names are not allowed in vhdl87";
    constexpr std::string_view End_package_keyword_vhdl87 = "'package' keyword not allowed in vhdl87";
    constexpr std::string_view Expected_alias_designator = "Expected alias designator";
    constexpr std::string_view Expected_architecture_name = "Expected architecture name";
    constexpr std::string_view Expected_attribute_identifier = "Expected attribute identifier";
    constexpr std::string_view Expected_component_identifier = "Expected component identifier";
    constexpr std::string_view Expected_component_name = "Expected component name";
    constexpr std::string_view Expected_composite_type_defn = "Expected a composite type definition";
    constexpr std::string_view Expected_configuration_name = "Expected configuration name";
    constexpr std::string_view Expected_entity_aspect = "expected 'entity', 'configuration' or 'open'";
    constexpr std::string_view Expected_entity_designator = "Expected entity designator";
    constexpr std::string_view Expected_entity_name = "Expected entity name";
    constexpr std::string_view Expected_expression = "Expecting an expression";
    constexpr std::string_view Expected_identifier_or_character = "identifier or character expected";
    constexpr std::string_view Expected_instantiation_list = "Expected instantiation list";
    constexpr std::string_view Expected_library_name = "Expected library name";
    constexpr std::string_view Expected_name = "Expected name";
    constexpr std::string_view Expected_object_class = "A class such as a variable, signal or constant is expected";
    constexpr std::string_view Expected_package_name = "Expected package name";
    constexpr std::string_view Expected_primary_unit = "Physical literal expects a primary unit";
    constexpr std::string_view Expected_scalar_type_defn = "Expected a scalar type definition";
    constexpr std::string_view Expected_secondary_unit = "Physical literal expects a secondary unit";
    constexpr std::string_view Expected_sel_name = "Expected selected name";
    constexpr std::string_view Expected_subtype_name = "Expected type name";
    constexpr std::string_view Expected_to_downto = "Expecting keyowrd 'to' or 'downto'";
    constexpr std::string_view Expected_type_name = "Expected type name";
    constexpr std::string_view Expected_use_name = "Expected use name";
    constexpr std::string_view Expecting_assignment = "Expecting a signal assignment or a variable assignment";
    constexpr std::string_view Expecting_seq_stmt = "Expecting a signal assignment, variable assignment or a procedure call";
    constexpr std::string_view Expecting_sprgm_designator = "Expecting subprogram designator";
    constexpr std::string_view Expecting_subprogram_spec = "This is not a subprogram specification";
    constexpr std::string_view Expecting_type_def = "Expecting a type definition";
    constexpr std::string_view File_mode_vhdl87 = "mode is only allowed in vhdl87";
    constexpr std::string_view File_name_vhdl87 = "file name expected in vhdl87";
    constexpr std::string_view Generate_label = "Generate must have a label";
    constexpr std::string_view Index_constraint_not_allowed = "index constraint not allowed";
    constexpr std::string_view Inertial_vhdl87 = "inertial delay mechanism is not allowed in vhdl87";
    constexpr std::string_view Inst_label = "Component instantiation must have a label";
    constexpr std::string_view Is_not_allowed_in_vhdl87 = "Is is not allowed here in vhdl87";
    constexpr std::string_view Malformed_function = "malformed function";
    constexpr std::string_view Malformed_procedure = "malformed procedure";
    constexpr std::string_view Nested_package_vhdl08 = "Nested package is not allowed before vhdl08";
    constexpr std::string_view No_attr_in_cfg = "Attribute not allowed in a configuration";
    constexpr std::string_view No_cfg_spec_in_entity = "Configuration specification not allowed in an entity";
    constexpr std::string_view No_cfg_spec_in_package_body = "Configuration specification not allowed in a package body";
    constexpr std::string_view No_cfg_spec_in_process = "Configuration specification not allowed in a process";
    constexpr std::string_view No_cfg_spec_in_subprogram = "Configuration specification not allowed in a subprogram";
    constexpr std::string_view No_component_in_entity = "Component is not allowed in an entity";
    constexpr std::string_view No_component_in_package_body = "Component is not allowed in a package body";
    constexpr std::string_view No_component_in_process = "Component is not allowed in a process";
    constexpr std::string_view No_component_in_subprogram = "Component is not allowed in a subprogram";
    constexpr std::string_view No_constant_in_port = "constant interface not allowed in port";
    constexpr std::string_view No_file_in_generic = "file interface not allowed in generic";
    constexpr std::string_view No_file_in_port = "file interface not allowed in port";
    constexpr std::string_view No_file_in_vhdl87 = "file interface not allowed in vhdl87";
    constexpr std::string_view No_function_body_in_package = "Function body is not allowed in a package";
    constexpr std::string_view No_nsvar_in_block = "Non-shared variable not allowed in a block";
    constexpr std::string_view No_nsvar_in_entity = "Non-shared variable not allowed in an entity";
    constexpr std::string_view No_nsvar_in_package = "Non-shared variable not allowed in a package";
    constexpr std::string_view No_nsvar_in_packbody = "Non-shared variable not allowed in a package body";
    constexpr std::string_view No_Packbody_in_pack = "package body not allowed in a package";
    constexpr std::string_view No_procedure_body_in_package = "Procedure body is not allowed in a package";
    constexpr std::string_view No_return_in_proc = "Return is not allowed in a procedure";
    constexpr std::string_view No_sig_in_packbody = "Signal not allowed in a package body";
    constexpr std::string_view No_sig_in_process = "Signal not allowed in a process";
    constexpr std::string_view No_sig_in_subprogram = "Signal not allowed in a subprogram";
    constexpr std::string_view No_signal_in_generic = "signal interface not allowed in generic";
    constexpr std::string_view No_svar_in_process = "Shared variable not allowed in a process";
    constexpr std::string_view No_svar_in_subprogram = "Shared variable not allowed in a subprogram";
    constexpr std::string_view No_svar_in_vhdl87 = "Shared variables not allowed in vhdl87";
    constexpr std::string_view No_variable_in_generic = "variable interface not allowed in generic";
    constexpr std::string_view No_variable_in_port = "variable interface not allowed in port";
    constexpr std::string_view Open_file_vhdl87 = "open kind expressions not allowed in vhdl87";
    constexpr std::string_view Package_only_in_generic = "package interface only allowed in generic";
    constexpr std::string_view Package_only_in_vhdl08 = "package interface only allowed in vhdl08";
    constexpr std::string_view Param_spec_expecting_ident = "Expecting identifier here";
    constexpr std::string_view Parser_encountered_a_problem = "Parser encountered a problem";
    constexpr std::string_view Plus_minus_skip = "+ and - cannot be used directly. Skipping this. Use parenthesis";
    constexpr std::string_view Reject_vhdl87 = "reject delay mechanism is not allowed in vhdl87";
    constexpr std::string_view Report_vhdl87 = "report statement is not allowed in vhdl87";
    constexpr std::string_view Return_in_func = "Return is expected in a function";
    constexpr std::string_view Subprogram_only_in_generic = "subprogram interface only allowed in generic";
    constexpr std::string_view Subprogram_only_in_vhdl08 = "subprogram interface only allowed in vhdl08";
    constexpr std::string_view Type_only_in_generic = "type interface only allowed in generic";
    constexpr std::string_view Type_only_in_vhdl08 = "type interface only allowed in vhdl08";
    constexpr std::string_view Unaffected_waveforms_vhdl87 = "Unaffected waveforms is not allowed in vhdl87";
    constexpr std::string_view Unexpected_comma = "Unexpected comma";
    constexpr std::string_view Wait_for_semic = "Expecting for or a semicolon";
    constexpr std::string_view Wait_on_until_for_semic = "Expecting on, until, for or a semicolon";
    constexpr std::string_view Wait_only_name_sensitivity = "Only names allowed in a sensitivity list";
    constexpr std::string_view Wait_only_one_condition = "Only one condition is allowed";
    constexpr std::string_view Wait_only_one_sensitivity = "Only one sensitivity lists is allowed";
    constexpr std::string_view Wait_until_for_semic = "Expecting until, for or a semicolon";
    constexpr std::string_view Wrong_file_mode = "This mode is not allowed in a file declaration";
}

//
// Initialize the parser with content to parse and optional file name
//
vhdl::parser::parser(common::stringtable* st, vhdl::syntax::design_file* file,
                     vhdl::version version)
    : version_(version),
      lexer_(file->src.data(), &file->src.data()[file->src.size()], st,
             &diagnostics, file->filename)
{
    file_ = file;
    lexer_.scan();
    // first token is always invalid. We skip that
}

vhdl::parser::~parser()
{

}

// ----------------------------------------------------------------------------
// parser methods
// ----------------------------------------------------------------------------

//
// Parse the initialized file
//
std::tuple<bool, std::vector<common::diagnostic>>
vhdl::parser::operator()()
{
    try
    {
        auto ok = parse_design_file();
        return std::make_tuple(ok, std::move(diagnostics));
    }
    catch (std::string)
    {
        diag(Err::Parser_encountered_a_problem);
    }

    return std::make_tuple(false, std::move(diagnostics));
}

// ----------------------------------------------------------------------------
// Design entities and configurations
// ----------------------------------------------------------------------------
// Implements these chapters from the VHDL LRM:
// LRM93 1.0
// ----------------------------------------------------------------------------
vhdl::syntax::design_unit* vhdl::parser::parse_entity_declaration()
{
    auto result = std::make_unique<vhdl::syntax::design_unit>();
    result->set_v_kind(vhdl::syntax::design_unit::v_::entity);

    result->first__ = eat(tk::kw_entity);

    auto [t, v] = parse_identifier(Err::Expected_entity_name);
    result->v.entity.identifier = t;

    consume(tk::kw_is);

    if (current_token() == tk::kw_generic)
    {
        auto [generic__, gl__, gens, __gr] = parse_generic_clause();
        result->v.entity.generic__ = generic__;
        result->v.entity.gl__ = gl__;
        result->v.entity.gens = gens;
        result->v.entity.__gr = __gr;
    }

    if (current_token() == tk::kw_port)
    {
        auto [port__, pl__, ports, __pr] = parse_port_clause();
        result->v.entity.port__ = port__;
        result->v.entity.pl__ = pl__;
        result->v.entity.ports = ports;
        result->v.entity.__pr = __pr;
    }

    result->v.entity.decls = parse_entity_declarative_part();

    if (optional(tk::kw_begin))
    {
        result->v.entity.stmts = parse_entity_statement_part();
    }

    consume(tk::kw_end);

    optional(tk::kw_entity);

    parse_optional_closing_label(t);

    result->__last = eat(tk::semicolon);

    if (!v)
        return nullptr;

    return result.release();
}

std::tuple<common::location, common::location,
           std::vector<vhdl::syntax::declarative_item*>, common::location>
vhdl::parser::parse_generic_clause()
{
    auto generic__ = eat(tk::kw_generic);
    auto gl__ = eat(tk::leftpar);

    auto result = parse_interface_list(interface::generic);

    auto __gr = eat(tk::rightpar);
    consume(tk::semicolon);

    return std::make_tuple(generic__, gl__, result, __gr);
}

std::tuple<common::location, common::location,
           std::vector<vhdl::syntax::declarative_item*>, common::location>
vhdl::parser::parse_port_clause()
{
    auto port__ = eat(tk::kw_port);
    auto pl__ = eat(tk::leftpar);

    auto result = parse_interface_list(interface::port);

    auto __pr = eat(tk::rightpar);
    consume(tk::semicolon);

    return std::make_tuple(port__, pl__, result, __pr);
}

std::vector<vhdl::syntax::declarative_item*>
vhdl::parser::parse_entity_declarative_part()
{
    return parse_many<vhdl::syntax::declarative_item*>(
        ps::declarative_part_begin_end,
        [this] { return parse_entity_declarative_item(); });
}

vhdl::syntax::declarative_item* vhdl::parser::parse_entity_declarative_item()
{
    return parse_declarative_item(declarative_part::entity);
}

std::vector<vhdl::syntax::concurrent_statement*>
vhdl::parser::parse_entity_statement_part()
{
    return parse_many<vhdl::syntax::concurrent_statement*>(
        ps::entity_statement_part, [this] { return parse_entity_statement(); });
}

vhdl::syntax::concurrent_statement* vhdl::parser::parse_entity_statement()
{
    const bool has_label = peek(0) == tk::identifier && peek() == tk::colon;

    const auto first = peek(has_label ? 2 : 0);
    const auto secnd = peek(has_label ? 3 : 1);

    auto first__ = lexer_.get_current_location();

    vhdl::syntax::concurrent_statement* result = nullptr;
    switch (first) {
    case tk::identifier:
        result = parse_concurrent_procedure_call_statement();
        break;

    case tk::kw_process:
        result = parse_process_statement();
        break;

    case tk::kw_assert:
        result = parse_concurrent_assertion_statement();
        break;

    case tk::kw_postponed:
        if (secnd == tk::kw_assert)
        {
            result = parse_concurrent_assertion_statement();
            break;
        }

        result = parse_process_statement();
        break;

    default:
        assert(false && "we should never get here");
        break;
    }

    if (!result)
        return nullptr;

    result->first__ = first__;
    result->__last = lexer_.get_previous_location();

    return result;
}

vhdl::syntax::design_unit* vhdl::parser::parse_architecture_body()
{
    auto result = std::make_unique<vhdl::syntax::design_unit>();
    result->set_v_kind(vhdl::syntax::design_unit::v_::architecture);

    result->first__ = eat(tk::kw_architecture);

    auto [t, v] = parse_identifier(Err::Expected_architecture_name);
    result->v.architecture.identifier = t;

    consume(tk::kw_of);

    name_options options;
    options.allow_qifts = false;
    options.allow_complex_names = false;
    result->v.architecture.entity = parse_name(options);

    result->v.architecture.is__ = eat(tk::kw_is);

    result->v.architecture.decls = parse_architecture_declarative_part();

    result->v.architecture.__begin__ = eat(tk::kw_begin);

    result->v.architecture.stmts = parse_architecture_statement_part();

    result->v.architecture.__end = eat(tk::kw_end);

    if (version_ == vhdl87 && current_token() == tk::kw_architecture)
        diag(Err::End_architecture_vhdl87);

    optional(tk::kw_architecture);

    parse_optional_closing_label(t);

    result->__last = eat(tk::semicolon);

    if (!v || !result->v.architecture.entity)
        return nullptr;

    return result.release();
}

std::vector<vhdl::syntax::declarative_item*>
vhdl::parser::parse_architecture_declarative_part()
{
    return parse_many<vhdl::syntax::declarative_item*>(
        ps::declarative_part_begin,
        [this] { return parse_block_declarative_item(); });
}

vhdl::syntax::declarative_item* vhdl::parser::parse_block_declarative_item()
{
    return parse_declarative_item(declarative_part::block);
}

std::vector<vhdl::syntax::concurrent_statement*>
vhdl::parser::parse_architecture_statement_part()
{
    return parse_concurrent_statements();
}

vhdl::syntax::design_unit* vhdl::parser::parse_configuration_declaration()
{
    auto result = std::make_unique<vhdl::syntax::design_unit>();
    result->set_v_kind(vhdl::syntax::design_unit::v_::configuration);

    result->first__ = eat(tk::kw_configuration);

    auto [t, v] = parse_identifier(Err::Expected_configuration_name);
    result->v.package_body.identifier = t;

    consume(tk::kw_of);

    name_options options;
    options.allow_qifts = false;
    options.allow_complex_names = false;
    result->v.configuration.entity = parse_name(options);

    result->v.configuration.is__ = eat(tk::kw_is);

    result->v.configuration.decls = parse_many<vhdl::syntax::declarative_item*>(
        ps::declarative_part_for,
        [this] { return parse_configuration_declarative_item(); });

    result->v.configuration.cfg = parse_block_configuration();

    result->v.configuration.__end = eat(tk::kw_end);

    if (version_ == vhdl87 && current_token() == tk::kw_configuration)
        diag(Err::End_config_keyword_vhdl87);

    optional(tk::kw_configuration);

    parse_optional_closing_label(t);

    result->__last = eat(tk::semicolon);

    if (!v || !result->v.configuration.entity)
        return nullptr;

    return result.release();
}

vhdl::syntax::declarative_item*
vhdl::parser::parse_configuration_declarative_item()
{
    return parse_declarative_item(declarative_part::configuration);
}

vhdl::syntax::block_configuration* vhdl::parser::parse_block_configuration()
{
    auto result = std::make_unique<vhdl::syntax::block_configuration>();

    result->for__ = eat(tk::kw_for);

    parse_block_specification(result);

    while (current_token() == tk::kw_use)
    {
        if (auto use = parse_use_clause())
            result->uses.push_back(use);
    }

    while (current_token() == tk::kw_for)
    {
        if (auto item = parse_configuration_item())
            result->items.push_back(item);
    }

    result->__end = eat(tk::kw_end);

    consume(tk::kw_for);

    consume(tk::semicolon);

    if (!result->label)
        return nullptr;

    return result.release();
}

void vhdl::parser::parse_block_specification(
    std::unique_ptr<vhdl::syntax::block_configuration>& block)
{
    name_options options;
    options.allow_qifts = false;
    options.allow_complex_names = false;
    block->label = parse_name(options);

    if (current_token() == tk::leftpar)
    {
        consume(tk::leftpar);
        parse_index_specification(block);

        consume(tk::rightpar);
    }
}

void vhdl::parser::parse_index_specification(
    std::unique_ptr<vhdl::syntax::block_configuration>& block)
{
    using is = vhdl::syntax::block_configuration::index_specification_;
    const look_params that = {.look = {tk::kw_downto, tk::kw_to, tk::kw_range},
                              .stop = {tk::rightpar, tk::comma, tk::rightarrow},
                              .abort = tk::semicolon,
                              .nest_in = tk::leftpar,
                              .nest_out = tk::rightpar,
                              .depth = 0};

    if (lexer_.look_for(that))
    {
        block->set_index_specification_kind(is::discrete);
        block->index_specification.discrete.range = parse_discrete_range();
        return;
    }

    block->set_index_specification_kind(is::expr);
    block->index_specification.expr.expr = parse_expression();
    return;
}

vhdl::syntax::configuration_item* vhdl::parser::parse_configuration_item()
{
    tk second = peek(2);

    if ((second == tk::colon) || (second == tk::comma))
        return parse_component_configuration();
    else
        return parse_block_configuration();
}

vhdl::syntax::component_configuration*
vhdl::parser::parse_component_configuration()
{
    auto component = std::make_unique<vhdl::syntax::component_configuration>();

    component->for__ = eat(tk::kw_for);

    component->spec = parse_component_specification();

    switch (current_token()) {
    case tk::kw_use:
    case tk::kw_generic:
    case tk::kw_port:
        component->binding = parse_binding_indication();

        consume(tk::semicolon);
        break;
    default:
        break;
    }

    switch (current_token()) {
    case tk::kw_for:
        component->block = parse_block_configuration();
        break;
    default:
        break;
    }

    component->__end = eat(tk::kw_end);
    consume(tk::kw_for);
    consume(tk::semicolon);

    if (!component->spec)
    {
        return nullptr;
    }

    return component.release();
}

// ------------------------------------------------------------------------
// Subprograms and packages
// ------------------------------------------------------------------------
// Implements these chapters from the VHDL LRM:
// LRM93 2.0
// ------------------------------------------------------------------------

std::tuple<std::optional<common::location>, vhdl::syntax::subprogram*,
           std::optional<common::location>>
vhdl::parser::parse_subprogram_specification()
{
    auto result = std::make_unique<vhdl::syntax::subprogram>();
    std::optional<common::location> pl__;
    std::optional<common::location> __pr;

    switch (current_token()) {
    case tk::kw_function:
        consume(tk::kw_function);
        result->set_v_kind(vhdl::syntax::subprogram::v_::function);
        result->v.function.pure = true;
        break;
    case tk::kw_pure:
        consume(tk::kw_pure);
        consume(tk::kw_function, "'pure' must preceed 'function'");
        result->set_v_kind(vhdl::syntax::subprogram::v_::function);
        result->v.function.pure = true;
        break;
    case tk::kw_impure:
        consume(tk::kw_impure);
        consume(tk::kw_function, "'impure' must preceed 'function'");
        result->set_v_kind(vhdl::syntax::subprogram::v_::function);
        result->v.function.pure = false;
        break;
    case tk::kw_procedure:
        consume(tk::kw_procedure);
        result->set_v_kind(vhdl::syntax::subprogram::v_::procedure);
        break;
    default:
        diag(Err::Expecting_subprogram_spec);
        return std::make_tuple(pl__, nullptr, __pr);
    }

    auto [t, v] = parse_designator(Err::Expecting_sprgm_designator);
    result->designator = t;

    using is = vhdl::syntax::subprogram::v_;
    if (current_token() == tk::leftpar && result->v_kind == is::function)
    {
        pl__ = eat(tk::leftpar);

        result->parameters = parse_interface_list(interface::function);

        __pr = eat(tk::rightpar);
    }
    else if (current_token() == tk::leftpar && result->v_kind == is::procedure)
    {
        pl__ = eat(tk::leftpar);

        result->parameters = parse_interface_list(interface::procedure);

        __pr = eat(tk::rightpar);
    }

    if (current_token() == tk::kw_return && result->v_kind == is::function)
    {
        consume(tk::kw_return);
        result->v.function.result = parse_type_mark();
    }
    else if (current_token() == tk::kw_return &&
             result->v_kind == is::procedure)
    {
        consume(tk::kw_return);
        result->v.procedure.result = parse_type_mark();
        diag(Err::No_return_in_proc);
    }
    else if (current_token() != tk::kw_return && result->v_kind == is::function)
    {
        diag(Err::Return_in_func);
        return std::make_tuple(pl__, nullptr, __pr);
    }

    if (!v)
        return std::make_tuple(pl__, nullptr, __pr);

    return std::make_tuple(pl__, result.release(), __pr);
}

vhdl::syntax::declarative_item* vhdl::parser::parse_function_body(
    std::tuple<std::optional<common::location>, vhdl::syntax::subprogram*,
               std::optional<common::location>>
        spec)
{
    auto result = std::make_unique<vhdl::syntax::declarative_item>();
    result->set_v_kind(vhdl::syntax::declarative_item::v_::subprogram_body);

    std::tie(result->v.subprogram_body.pl__,
             result->v.subprogram_body.spec,
             result->v.subprogram_body.__pr) = spec;

    result->v.subprogram_body.is__ = eat(tk::kw_is);

    result->v.subprogram_body.decls =
        parse_many<vhdl::syntax::declarative_item*>(
            ps::declarative_part_begin,
            [this] { return parse_subprogram_declarative_item(); });

    result->v.subprogram_body.__begin__ = eat(tk::kw_begin);

    result->v.subprogram_body.stmts = parse_sequence_of_statements();

    result->v.subprogram_body.__end = eat(tk::kw_end);
    optional(tk::kw_function);

    parse_optional_closing_designator(result->v.subprogram_body.spec);

    consume(tk::semicolon);

    if (result->v.subprogram_body.spec == nullptr ||
        result->v.subprogram_body.spec->v_kind != vhdl::syntax::subprogram::v_::function)
    {
        return nullptr;
    }

    return result.release();
}

vhdl::syntax::declarative_item* vhdl::parser::parse_procedure_body(
    std::tuple<std::optional<common::location>, vhdl::syntax::subprogram*,
               std::optional<common::location>>
        spec)
{
    auto result = std::make_unique<vhdl::syntax::declarative_item>();
    result->set_v_kind(vhdl::syntax::declarative_item::v_::subprogram_body);

    std::tie(result->v.subprogram_body.pl__,
             result->v.subprogram_body.spec,
             result->v.subprogram_body.__pr) = spec;

    result->v.subprogram_body.is__ = eat(tk::kw_is);

    result->v.subprogram_body.decls =
        parse_many<vhdl::syntax::declarative_item*>(
            ps::declarative_part_begin,
            [this] { return parse_subprogram_declarative_item(); });

    result->v.subprogram_body.__begin__ = eat(tk::kw_begin);

    result->v.subprogram_body.stmts = parse_sequence_of_statements();

    result->v.subprogram_body.__end = eat(tk::kw_end);
    optional(tk::kw_procedure);

    parse_optional_closing_designator(result->v.subprogram_body.spec);

    consume(tk::semicolon);

    if (result->v.subprogram_body.spec == nullptr ||
        result->v.subprogram_body.spec->v_kind != vhdl::syntax::subprogram::v_::procedure)
    {
        return nullptr;
    }

    return result.release();
}

vhdl::syntax::declarative_item*
vhdl::parser::parse_subprogram_declarative_item()
{
    return parse_declarative_item(declarative_part::subprogram);
}

// LRM93 2.3.2 Signatures
// signature ::= [ [ type_mark { , type_mark } ] [ RETURN type_mark ] ]
vhdl::syntax::signature* vhdl::parser::parse_signature()
{
    auto result = std::make_unique<vhdl::syntax::signature>();

    consume(tk::leftsquare);

    if (current_token() == tk::identifier)
    {
        do
        {
            if (auto t = parse_type_mark())
                result->parameters.push_back(t);

            if (current_token() == tk::leftpar)
            {
                diag(Err::Index_constraint_not_allowed);
                resync_to_end_of_rightpar();
            }
        } while (optional(tk::comma));
    }

    if (optional(tk::kw_return))
    {
        result->result = parse_type_mark();
    }

    consume(tk::rightsquare);

    return result.release();
}

vhdl::syntax::design_unit* vhdl::parser::parse_package_declaration()
{
    auto package = std::make_unique<vhdl::syntax::design_unit>();
    package->set_v_kind(vhdl::syntax::design_unit::v_::package);

    package->first__ = eat(tk::kw_package);

    auto [t, v] = parse_identifier(Err::Expected_package_name);
    package->v.package.identifier = t;

    package->v.package.is__ = eat(tk::kw_is);

    package->v.package.decls = parse_many<vhdl::syntax::declarative_item*>(
        ps::declarative_part_end,
        [this] { return parse_package_declarative_item(); });

    package->v.package.__end = eat(tk::kw_end);

    if (version_ == vhdl87 && current_token() == tk::kw_package)
        diag(Err::End_package_keyword_vhdl87);

    optional(tk::kw_package);

    parse_optional_closing_label(t);

    if (!v)
        return nullptr;

    package->__last = eat(tk::semicolon);

    return package.release();
}

vhdl::syntax::declarative_item* vhdl::parser::parse_package_declarative_item()
{
    return parse_declarative_item(declarative_part::package);
}

vhdl::syntax::design_unit* vhdl::parser::parse_package_body()
{
    auto result = std::make_unique<vhdl::syntax::design_unit>();
    result->set_v_kind(vhdl::syntax::design_unit::v_::package_body);

    result->first__ = eat(tk::kw_package);

    consume(tk::kw_body);

    auto [t, v] = parse_identifier(Err::Expected_package_name);
    result->v.package_body.identifier = t;

    result->v.package_body.is__ = eat(tk::kw_is);

    result->v.package_body.decls = parse_many<vhdl::syntax::declarative_item*>(
        ps::declarative_part_end,
        [this] { return parse_package_body_declarative_item(); });

    result->v.package_body.__end = eat(tk::kw_end);

    if (version_ == vhdl87 && current_token() == tk::kw_package)
        diag(Err::End_package_keyword_vhdl87);

    if (optional(tk::kw_package))
    {
        consume(tk::kw_body);
    }

    parse_optional_closing_label(t);

    result->__last = eat(tk::semicolon);

    if (!v)
        return nullptr;

    return result.release();
}

vhdl::syntax::declarative_item*
vhdl::parser::parse_package_body_declarative_item()
{
    return parse_declarative_item(declarative_part::package_body);
}

// ----------------------------------------------------------------------------
// Types
// ----------------------------------------------------------------------------
// Implements these chapters from the VHDL LRM:
// LRM93 3.0
// ----------------------------------------------------------------------------

vhdl::syntax::type_definition* vhdl::parser::parse_scalar_type_definition()
{
    switch (current_token()) {
    case tk::kw_range: {

        consume(tk::kw_range);

        auto r = parse_range();

        if (current_token() == tk::kw_units)
        {
            return parse_physical_type_definition(r);
        }

        return parse_anonymous_type_definition(r);
    }
    case tk::leftpar: {
        auto result = std::make_unique<vhdl::syntax::type_definition>();
        result->set_v_kind(vhdl::syntax::type_definition::v_::enumeration);

        // skip the (
        consume(tk::leftpar);

        while (current_token() != tk::eof)
        {
            switch (current_token()) {
            case tk::character:
            case tk::identifier:
                result->v.enumeration.literals.push_back(lexer_.current_token());
                skip();
                break;
            default:
                diag(Err::Expected_identifier_or_character);
                break;
            }

            if (current_token() != tk::comma)
                break;

            consume(tk::comma);

            if (current_token() == tk::rightpar)
            {
                diag(Err::Unexpected_comma, lexer_.get_previous_location());
                break;
            }
        }

        consume(tk::rightpar);

        // return enumeration type
        return result.release();
    }
    default:
        diag(Err::Expected_scalar_type_defn);
        break;
    }

    return nullptr;
}

vhdl::syntax::type_definition*
vhdl::parser::parse_anonymous_type_definition(vhdl::syntax::range* r)
{
    if (!r)
        return nullptr;

    bool is_real = false;
    if (r->v_kind == vhdl::syntax::range::v_::upto &&
        r->v.upto.v1->v_kind == vhdl::syntax::expression::v_::literal &&
        r->v.upto.v1->v.literal.kind == vhdl::syntax::literal_kind::real)
        is_real = true;

    if (r->v_kind == vhdl::syntax::range::v_::upto &&
        r->v.upto.v2->v_kind == vhdl::syntax::expression::v_::literal &&
        r->v.upto.v2->v.literal.kind == vhdl::syntax::literal_kind::real)
        is_real = true;

    if (r->v_kind == vhdl::syntax::range::v_::downto &&
        r->v.downto.v1->v_kind == vhdl::syntax::expression::v_::literal &&
        r->v.downto.v1->v.literal.kind == vhdl::syntax::literal_kind::real)
        is_real = true;

    if (r->v_kind == vhdl::syntax::range::v_::downto &&
        r->v.downto.v2->v_kind == vhdl::syntax::expression::v_::literal &&
        r->v.downto.v2->v.literal.kind == vhdl::syntax::literal_kind::real)
        is_real = true;

    if (is_real)
    {
        auto result = std::make_unique<vhdl::syntax::type_definition>();
        result->set_v_kind(vhdl::syntax::type_definition::v_::floating);
        result->v.floating.dimension = r;
        return result.release();
    }

    auto result = std::make_unique<vhdl::syntax::type_definition>();
    result->set_v_kind(vhdl::syntax::type_definition::v_::integer);
    result->v.integer.dimension = r;
    return result.release();
}

vhdl::syntax::type_definition*
vhdl::parser::parse_physical_type_definition(vhdl::syntax::range* r)
{
    auto result = std::make_unique<vhdl::syntax::type_definition>();
    result->set_v_kind(vhdl::syntax::type_definition::v_::physical);
    result->v.physical.dimension = r;

    consume(tk::kw_units);

    if (current_token() == tk::identifier)
    {
        auto p = std::make_unique<vhdl::syntax::unit_declaration>();
        p->identifier = lexer_.current_token();

        skip();
        result->v.physical.primary = p.release();
    }
    else
    {
        diag(Err::Expected_primary_unit);
    }

    consume(tk::semicolon);

    while (current_token() == tk::identifier)
    {
        auto s = std::make_unique<vhdl::syntax::unit_declaration>();
        s->identifier = lexer_.current_token();
        s->multiplier = new vhdl::syntax::physical_literal;

        consume(tk::identifier);
        consume(tk::eq);

        switch (current_token()) {
        case tk::real:
        case tk::integer:
            s->multiplier->value = lexer_.current_token();
            skip();
            break;
        default:
            break;
        }

        if (current_token() == tk::identifier)
        {
            name_options options;
            options.allow_complex_names = false;
            s->multiplier->unit = parse_name(options);
        }
        else
        {
            diag(Err::Expected_secondary_unit);
        }

        consume(tk::semicolon);

        if (!s->multiplier->unit)
            result->v.physical.secondary.push_back(s.release());
    }

    consume(tk::kw_end);
    consume(tk::kw_units);
    optional(tk::identifier);

    if (!result->v.physical.dimension && !result->v.physical.primary)
    {
        return nullptr;
    }

    return result.release();
}

vhdl::syntax::constraint* vhdl::parser::parse_range_constraint()
{
    auto result = std::make_unique<vhdl::syntax::constraint>();
    result->set_v_kind(vhdl::syntax::constraint::v_::range);

    consume(tk::kw_range);

    auto r = parse_range();
    if (!r)
    {
        return nullptr;
    }

    result->v.range.dimension = r;
    return result.release();
}

vhdl::syntax::range* vhdl::parser::parse_range()
{
    auto result = std::make_unique<vhdl::syntax::range>();

    // add a checkpoint and corresponding RAII to automatically drop the
    // checkpoint without programmer's intervention.
    lexer_.add_checkpoint();
    auto drop_checkpoint = common::make_scope_guard(
        [l = &lexer_]() { l->drop_checkpoint(); });

    // parse the expression.
    // This can be an expression/a value, or a range attribute that will be
    // handled in a special manner ...
    std::unique_ptr<vhdl::syntax::expression> e(parse_expression());

    switch (current_token()) {
    case tk::kw_to:
        consume(tk::kw_to);

        result->set_v_kind(vhdl::syntax::range::v_::upto);
        result->v.upto.v1 = e.release();
        result->v.upto.v2 = parse_expression();

        if (!result->v.upto.v1 || !result->v.upto.v2)
            return nullptr;

        return result.release();

    case tk::kw_downto:
        consume(tk::kw_downto);

        result->set_v_kind(vhdl::syntax::range::v_::downto);
        result->v.downto.v1 = e.release();
        result->v.downto.v2 = parse_expression();

        if (!result->v.downto.v1 || !result->v.downto.v2)
            return nullptr;

        return result.release();

    default:
        // error message was probably already printed. Just return null
        if (!e)
            return nullptr;

        // this is not a discrete range by attribute'range. This is an error
        if (e->v_kind != vhdl::syntax::expression::v_::unresolved ||
            e->v.unresolved.name == nullptr ||
            e->v.unresolved.name->v_kind != vhdl::syntax::name::v_::attribute ||
            e->v.unresolved.name->v.attribute.kind != vhdl::syntax::attr_kind::range)
        {
            diag(Err::Expected_to_downto);
            return nullptr;
        }

        // this is a discrete by attribute'range. So we backtrack the lexer
        // and parse the attribute name again.
        lexer_.backtrack();
        drop_checkpoint.dismiss();

        result->set_v_kind(vhdl::syntax::range::v_::range);
        name_options options;
        options.allow_qifts = false;
        result->v.range.attribute = parse_name(options);

        if (result->v.range.attribute->v_kind != vhdl::syntax::name::v_::attribute ||
            result->v.range.attribute->v.attribute.kind != vhdl::syntax::attr_kind::range)
        {
            assert(false && "this should never be reached");
            return nullptr;
        }
        return result.release();
    }
}

vhdl::syntax::type_definition* vhdl::parser::parse_composite_type_definition()
{
    using is = vhdl::syntax::type_definition::v_;

    switch (current_token()) {

    //
    // record_type_definition ::=
    //              RECORD element_declaration { element_declaration }
    //              END RECORD [ record_type_simple_name ]
    //
    // element_declaration ::=
    //              identifier_list : element_subtype_definition ;
    //
    // identifier_list ::= identifier { , identifier }
    //
    // element_subtype_definition ::= subtype_indication
    //
    case tk::kw_record: {
        auto result = std::make_unique<vhdl::syntax::type_definition>();
        result->set_v_kind(is::record);

        consume(tk::kw_record);

        std::vector<vhdl::syntax::element_declaration*> elements;
        while (current_token() == tk::identifier)
        {
            if (auto e = parse_element_declaration())
                result->v.record.elements.push_back(e);
        }

        consume(tk::kw_end);
        consume(tk::kw_record);

        optional(tk::identifier);

        return result.release();
    }

    //
    // array_type_definition ::=
    //          unconstrained_array_definition | constrained_array_definition
    //
    // unconstrained_array_definition ::=
    //          ARRAY ( index_subtype_definition { , index_subtype_definition } )
    //          OF element_subtype_indication
    //
    // index_subtype_definition ::=
    //          type_mark RANGE <>
    //
    // constrained_array_definition ::=
    //          ARRAY index_constraint OF element_subtype_indication
    //
    // index_constraint ::=
    //          ( discrete_range { , discrete_range } )
    //
    case tk::kw_array: {

        const look_params box = {.look = {tk::box},
                                 .stop = {tk::rightpar},
                                 .abort = tk::semicolon,
                                 .nest_in = tk::leftpar,
                                 .nest_out = tk::rightpar,
                                 .depth = 1};
        const bool is_unconstrained_array = lexer_.look_for(box);

        if (is_unconstrained_array)
        {
            auto result = std::make_unique<vhdl::syntax::type_definition>();
            result->set_v_kind(is::u_array);

            consume(tk::kw_array);
            consume(tk::leftpar);

            do
            {
                if (auto i = parse_index_subtype())
                    result->v.u_array.index_subtypes.push_back(i);
            } while (optional(tk::comma));

            consume(tk::rightpar);
            consume(tk::kw_of);

            result->v.u_array.element = parse_subtype_indication();

            if (!result->v.u_array.element ||
                result->v.u_array.index_subtypes.size() == 0)
            {
                return nullptr;
            }

            return result.release();
        }
        else
        {
            auto result = std::make_unique<vhdl::syntax::type_definition>();
            result->set_v_kind(is::c_array);

            consume(tk::kw_array);
            consume(tk::leftpar);

            do
            {
                if (auto d = parse_discrete_range())
                    result->v.c_array.indices.push_back(d);
            } while (optional(tk::comma));

            consume(tk::rightpar);
            consume(tk::kw_of);

            result->v.c_array.element = parse_subtype_indication();

            if (!result->v.c_array.element ||
                result->v.c_array.indices.size() == 0)
            {
                return nullptr;
            }

            return result.release();
        }
    }
    break;
    default:
        diag(Err::Expected_composite_type_defn);
    }

    return nullptr;
}

vhdl::syntax::element_declaration* vhdl::parser::parse_element_declaration()
{
    auto result = std::make_unique<vhdl::syntax::element_declaration>();
    result->identifier = parse_identifier_list();

    consume(tk::colon);

    result->type = parse_subtype_indication();

    consume(tk::semicolon);

    if (!result->type || result->identifier.size() == 0)
        return nullptr;

    return result.release();
}

std::vector<vhdl::token> vhdl::parser::parse_identifier_list()
{
    std::vector<vhdl::token> identifiers;

    do
    {
        identifiers.push_back(lexer_.current_token());
        consume(tk::identifier);
    } while (optional(tk::comma));

    return identifiers;
}

vhdl::syntax::index_subtype* vhdl::parser::parse_index_subtype()
{
    auto result = std::make_unique<vhdl::syntax::index_subtype>();
    result->mark = parse_type_mark();

    consume(tk::kw_range);
    consume(tk::box);

    if (!result->mark)
        return nullptr;

    return result.release();
}

vhdl::syntax::constraint* vhdl::parser::parse_index_constraint()
{
    auto result = std::make_unique<vhdl::syntax::constraint>();
    result->set_v_kind(vhdl::syntax::constraint::v_::index);

    consume(tk::leftpar);

    do
    {
        if (auto d = parse_discrete_range())
            result->v.index.indices.push_back(d);
    } while (optional(tk::comma));

    consume(tk::rightpar);

    if (result->v.index.indices.size() == 0)
        return nullptr;

    return result.release();
}

vhdl::syntax::range* vhdl::parser::parse_discrete_range()
{
    auto result = std::make_unique<vhdl::syntax::range>();

    // add a checkpoint and corresponding RAII to automatically drop the
    // checkpoint without programmer's intervention.
    lexer_.add_checkpoint();
    auto drop_checkpoint = common::make_scope_guard(
        [l = &lexer_]() { l->drop_checkpoint(); });

    // parse the expression.
    std::unique_ptr<vhdl::syntax::expression> e(parse_expression());

    switch (current_token()) {
    case tk::kw_to:
        consume(tk::kw_to);

        result->set_v_kind(vhdl::syntax::range::v_::upto);
        result->v.upto.v1 = e.release();
        result->v.upto.v2 = parse_expression();

        if (!result->v.upto.v1 || !result->v.upto.v2)
            return nullptr;

        return result.release();

    case tk::kw_downto:
        consume(tk::kw_downto);

        result->set_v_kind(vhdl::syntax::range::v_::downto);
        result->v.downto.v1 = e.release();
        result->v.downto.v2 = parse_expression();

        if (!result->v.downto.v1 || !result->v.downto.v2)
            return nullptr;

        return result.release();

    case tk::kw_range:
        // this is a subtype indication with a range constraint. Handled later
        [[fallthrough]];

    default:
        // the syntax for discrete ranges only allows attribute names or a
        // subtype indication without the 'to' or 'downto' keyword

        // error message was probably already printed. Just return null
        if (!e)
            return nullptr;

        // this is a discrete range by subtype indication
        if (e->v_kind == vhdl::syntax::expression::v_::unresolved &&
            e->v.unresolved.name != nullptr &&
            e->v.unresolved.name->v_kind != vhdl::syntax::name::v_::attribute)
        {
            lexer_.backtrack();
            drop_checkpoint.dismiss();

            result->set_v_kind(vhdl::syntax::range::v_::subtype);
            result->v.subtype.indication = parse_subtype_indication();
            return result.release();
        }

        // this is not a discrete range by attribute'range. This is an error
        if (e->v_kind != vhdl::syntax::expression::v_::unresolved ||
            e->v.unresolved.name == nullptr ||
            e->v.unresolved.name->v_kind != vhdl::syntax::name::v_::attribute ||
            e->v.unresolved.name->v.attribute.kind !=
                vhdl::syntax::attr_kind::range)
        {
            diag(Err::Expected_to_downto);
            return nullptr;
        }

        // this is a discrete range by attribute'range
        lexer_.backtrack();
        drop_checkpoint.dismiss();

        result->set_v_kind(vhdl::syntax::range::v_::range);
        result->v.range.attribute = parse_name();

        if (result->v.range.attribute->v_kind !=
                vhdl::syntax::name::v_::attribute ||
            result->v.range.attribute->v.attribute.kind !=
                vhdl::syntax::attr_kind::range)
        {
            assert(false && "this should never be reached");
            return nullptr;
        }
        return result.release();
    }
}

vhdl::syntax::type_definition* vhdl::parser::parse_access_type_definition()
{
    consume(tk::kw_access);

    auto result = std::make_unique<vhdl::syntax::type_definition>();
    result->set_v_kind(vhdl::syntax::type_definition::v_::access);
    result->v.access.subtype = parse_subtype_indication();

    if (!result->v.access.subtype)
        return nullptr;

    return result.release();
}

vhdl::syntax::type_definition* vhdl::parser::parse_file_type_definition()
{
    consume(tk::kw_file);
    consume(tk::kw_of);

    auto result = std::make_unique<vhdl::syntax::type_definition>();
    result->set_v_kind(vhdl::syntax::type_definition::v_::file);
    result->v.file.mark = parse_type_mark();

    if (current_token() == tk::leftpar)
    {
        diag(Err::Index_constraint_not_allowed);
        resync_to_end_of_rightpar();
    }

    if (!result->v.file.mark)
        return nullptr;

    return result.release();
}

// ----------------------------------------------------------------------------
// Declaration
// ----------------------------------------------------------------------------
// Implements these chapters from the VHDL LRM:
// LRM93 4.0
// ----------------------------------------------------------------------------

vhdl::syntax::declarative_item*
vhdl::parser::parse_declarative_item(declarative_part kind)
{
    vhdl::syntax::declarative_item* result = nullptr;
    std::unique_ptr<vhdl::syntax::declarative_item> throw_away;

    auto first__ = lexer_.get_current_location();

    switch (current_token()) {
    case tk::kw_type:
        result = parse_type_declaration();

        // if (kind == declarative_part::package && result && result->v.type.type && result->v.type.type->v_kind.protected)
        // {
        //     std::cout << "protected type body not allowed in a package\n";
        // }
        break;
    case tk::kw_subtype:
        result = parse_subtype_declaration();
        break;
    case tk::kw_signal:
        if (kind == declarative_part::subprogram)
        {
            diag(Err::No_sig_in_subprogram);
            throw_away.reset(parse_signal_declaration());
        }
        else if (kind == declarative_part::package_body)
        {
            diag(Err::No_sig_in_packbody);
            throw_away.reset(parse_signal_declaration());
        }
        else if (kind == declarative_part::process)
        {
            diag(Err::No_sig_in_process);
            throw_away.reset(parse_signal_declaration());
        }
        else
        {
            result = parse_signal_declaration();
        }
        break;
    case tk::kw_constant:
        result = parse_constant_declaration();
        break;
    case tk::kw_variable:
        if (kind == vhdl::parser::declarative_part::entity)
        {
            diag(Err::No_nsvar_in_entity);
            throw_away.reset(parse_variable_declaration());
        }
        else if (kind == declarative_part::block)
        {
            diag(Err::No_nsvar_in_block);
            throw_away.reset(parse_variable_declaration());
        }
        else if (kind == declarative_part::package)
        {
            diag(Err::No_nsvar_in_package);
            throw_away.reset(parse_variable_declaration());
        }
        else if (kind == declarative_part::package_body)
        {
            diag(Err::No_nsvar_in_packbody);
            throw_away.reset(parse_variable_declaration());
        }
        else
        {
            result = parse_variable_declaration();
        }
        break;
    case tk::kw_shared:
        if (version_ <= vhdl87)
        {
            diag(Err::No_svar_in_vhdl87);
            throw_away.reset(parse_variable_declaration());
        }
        if (kind == declarative_part::subprogram)
        {
            diag(Err::No_svar_in_subprogram);
            throw_away.reset(parse_variable_declaration());
        }
        else if (kind == declarative_part::process)
        {
            diag(Err::No_svar_in_process);
            throw_away.reset(parse_variable_declaration());
        }
        else
        {
            result = parse_variable_declaration();
        }
        break;
    case tk::kw_file:
        result = parse_file_declaration();
        break;
    case tk::kw_pure:
    case tk::kw_impure:
    case tk::kw_function: {
        auto s = parse_subprogram_specification();
        if (current_token() == tk::semicolon)
        {
            skip();

            result = new vhdl::syntax::declarative_item;
            result->set_v_kind(vhdl::syntax::declarative_item::v_::subprogram);
            std::tie(result->v.subprogram.pl__,
                     result->v.subprogram.spec,
                     result->v.subprogram.__pr) = s;
        }
        else if (current_token() == tk::kw_is &&
                 kind == declarative_part::package)
        {
            diag(Err::No_function_body_in_package);
            throw_away.reset(parse_function_body(s));
        }
        else if (current_token() == tk::kw_is)
        {
            result = parse_function_body(s);
        }
        else
        {
            diag(Err::Malformed_function);
        }
        break;
    }
    case tk::kw_procedure: {
        auto s = parse_subprogram_specification();
        if (current_token() == tk::semicolon)
        {
            skip();

            result = new vhdl::syntax::declarative_item;
            result->set_v_kind(vhdl::syntax::declarative_item::v_::subprogram);
            std::tie(result->v.subprogram.pl__,
                     result->v.subprogram.spec,
                     result->v.subprogram.__pr) = s;
        }
        else if (peek(0) == tk::kw_is && kind == declarative_part::package)
        {
            diag(Err::No_procedure_body_in_package);
            throw_away.reset(parse_procedure_body(s));
        }
        else if (peek(0) == tk::kw_is)
        {
            result = parse_procedure_body(s);
        }
        else
        {
            diag(Err::Malformed_procedure);
        }
        break;
    }
    case tk::kw_alias:
        result = parse_alias_declaration();
        break;
    case tk::kw_component:
        if (kind == declarative_part::entity)
        {
            diag(Err::No_component_in_entity);
            throw_away.reset(parse_component_declaration());
        }
        else if (kind == declarative_part::subprogram)
        {
            diag(Err::No_component_in_subprogram);
            throw_away.reset(parse_component_declaration());
        }
        else if (kind == declarative_part::package_body)
        {
            diag(Err::No_component_in_package_body);
            throw_away.reset(parse_component_declaration());
        }
        else if (kind == declarative_part::process)
        {
            diag(Err::No_component_in_process);
            throw_away.reset(parse_component_declaration());
        }
        else
        {
            result = parse_component_declaration();
        }
        break;
    case tk::kw_for:
        if (kind == declarative_part::entity)
        {
            diag(Err::No_cfg_spec_in_entity);
            throw_away.reset(parse_configuration_specification());
        }
        else if (kind == declarative_part::subprogram)
        {
            diag(Err::No_cfg_spec_in_subprogram);
            throw_away.reset(parse_configuration_specification());
        }
        else if (kind == declarative_part::package_body)
        {
            diag(Err::No_cfg_spec_in_package_body);
            throw_away.reset(parse_configuration_specification());
        }
        else if (kind == declarative_part::process)
        {
            diag(Err::No_cfg_spec_in_process);
            throw_away.reset(parse_configuration_specification());
        }
        else
        {
            result = parse_configuration_specification();
        }
        break;
    case tk::kw_attribute:
        if (peek(2) == tk::kw_of)
        {
            result = parse_attribute_specification();
        }
        else if (peek(2) != tk::kw_of &&
                 kind == declarative_part::configuration)
        {
            diag(Err::No_attr_in_cfg);
            throw_away.reset(parse_attribute_declaration());
        }
        else
        {
            result = parse_attribute_declaration();
        }
        break;
    case tk::kw_use:
        if (auto u = parse_use_clause())
        {
            result = new vhdl::syntax::declarative_item;
            result->set_v_kind(vhdl::syntax::declarative_item::v_::use);
            result->v.use.clause = u;
        }
        break;
    case tk::kw_group:
        // TODO:
        // FIXME: write this up
        // if (peek(2) == tk::kw_is)
        // {
        //     result = parse_group_template_declaration();
        // }
        // else
        // {
        //     result = parse_group_declaration();
        // }
        break;
    case tk::kw_package:
        if (version_ < vhdl08)
        {
            diag(Err::Nested_package_vhdl08);
        }

        // TODO for vhdl08: add parse_package();

        if (kind == declarative_part::package ||
            kind == declarative_part::package_body)
        {
            diag(Err::No_Packbody_in_pack);
        }
        break;
    case tk::identifier:
        if (kind == declarative_part::package ||
            kind == declarative_part::package_body ||
            kind == declarative_part::subprogram)
        {
            result = parse_variable_declaration();
        }
        else
        {
            diag(Err::Expected_object_class);
            throw_away.reset(parse_variable_declaration());
        }
        break;
    default:
        assert(false && "we should never get here");
        break;
    }

    auto __last = lexer_.get_previous_location();

    if (!result)
        return nullptr;

    result->first__ = first__;
    result->__last = __last;
    return result;
}

vhdl::syntax::declarative_item* vhdl::parser::parse_type_declaration()
{
    auto decl = std::make_unique<vhdl::syntax::declarative_item>();
    decl->set_v_kind(vhdl::syntax::declarative_item::v_::type);

    // skip the keyword 'type'
    consume(tk::kw_type);

    auto [t, v] = parse_identifier(Err::Expected_type_name);
    decl->v.type.identifier = t;

    if (optional(tk::semicolon))
    {
        return decl.release();
    }

    consume(tk::kw_is);

    vhdl::syntax::type_definition* d = nullptr;
    switch (current_token()) {
    case tk::kw_range:
    case tk::leftpar:
        d = parse_scalar_type_definition();
        break;

    case tk::kw_access:
        d = parse_access_type_definition();
        break;

    case tk::kw_file:
        d = parse_file_type_definition();
        break;

    case tk::kw_record:
    case tk::kw_array:
        d = parse_composite_type_definition();
        break;

    case tk::kw_protected:
        // TODO:
        // FIXME: write this up
        // if (peek(1) == tk::kw_body)
        //     d = parse_protected_type_body(result->get_node("identifier")->get_text("identifier"));
        // else
        //     d = parse_protected_type_declaration(result->get_node("identifier")->get_text("identifier"));
        // break;

    default:
        diag(Err::Expecting_type_def);
        return nullptr;
    }

    consume(tk::semicolon);

    // there was probably a parse error in the type definition. Return
    decl->v.type.type = d;
    if (!decl->v.type.type)
    {
        return nullptr;
    }

    decl->v.type.type->identifier = decl->v.type.identifier;

    return decl.release();
}

vhdl::syntax::declarative_item* vhdl::parser::parse_subtype_declaration()
{
    auto decl = std::make_unique<vhdl::syntax::declarative_item>();
    decl->set_v_kind(vhdl::syntax::declarative_item::v_::subtype);

    // skip the keyword 'subtype'
    consume(tk::kw_subtype);

    auto [t, v] = parse_identifier(Err::Expected_subtype_name);
    decl->v.type.identifier = t;

    consume(tk::kw_is);

    decl->v.subtype.subtype = parse_subtype_indication();

    consume(tk::semicolon);

    // there was probably a parse error in indication
    if (!decl->v.subtype.subtype)
    {
        return nullptr;
    }

    return decl.release();
}

vhdl::syntax::subtype* vhdl::parser::parse_subtype_indication()
{
    auto result = std::make_unique<vhdl::syntax::subtype>();

    if (current_token() != tk::identifier)
        return nullptr;

    if (peek(0) == tk::identifier && peek() == tk::identifier)
    {
        name_options options;
        options.allow_qifts = false;
        options.allow_complex_names = false;
        result->resolution = parse_name(options);
    }

    result->mark = parse_type_mark();

    switch (current_token()) {
    case tk::leftpar:
        result->constraint = parse_index_constraint();
        break;
    case tk::kw_range:
        result->constraint = parse_range_constraint();
    default:
        break;
    }

    if (!result->mark)
        return nullptr;

    return result.release();
}

vhdl::syntax::name* vhdl::parser::parse_type_mark()
{
    vhdl::parser::name_options options;
    options.allow_qifts = false;
    return parse_name(options);
}

vhdl::syntax::declarative_item* vhdl::parser::parse_constant_declaration()
{
    auto object = std::make_unique<vhdl::syntax::object>();
    object->set_v_kind(vhdl::syntax::object::v_::constant);

    consume(tk::kw_constant);

    object->identifier = parse_identifier_list();

    consume(tk::colon);

    object->type = parse_subtype_indication();

    vhdl::syntax::expression* v = nullptr;
    if (optional(tk::coloneq))
        object->v.constant.value = parse_expression();

    consume(tk::semicolon);

    if (object->identifier.size() == 0 || !object->type)
        return nullptr;

    auto result = std::make_unique<vhdl::syntax::declarative_item>();
    result->set_v_kind(vhdl::syntax::declarative_item::v_::object);
    result->v.object.v = object.release();
    result->v.object.v->decl = result.get();
    return result.release();
}

vhdl::syntax::declarative_item* vhdl::parser::parse_signal_declaration()
{
    auto object = std::make_unique<vhdl::syntax::object>();
    object->set_v_kind(vhdl::syntax::object::v_::signal);

    consume(tk::kw_signal);

    object->identifier = parse_identifier_list();

    consume(tk::colon);

    object->type = parse_subtype_indication();

    if (current_token() == tk::kw_bus)
    {
        consume(tk::kw_bus);

        object->v.signal.is_guarded = true;
        object->v.signal.is_bus = true;
    }
    else if (current_token() == tk::kw_register)
    {
        consume(tk::kw_register);

        object->v.signal.is_guarded = true;
        object->v.signal.is_bus = false;
    }
    else
    {
        object->v.signal.is_guarded = false;
        object->v.signal.is_bus = true;
    }

    if (optional(tk::coloneq))
        object->v.signal.value = parse_expression();

    consume(tk::semicolon);

    if (object->identifier.size() == 0 || !object->type)
        return nullptr;

    auto result = std::make_unique<vhdl::syntax::declarative_item>();
    result->set_v_kind(vhdl::syntax::declarative_item::v_::object);
    result->v.object.v = object.release();
    result->v.object.v->decl = result.get();
    return result.release();
}

vhdl::syntax::declarative_item* vhdl::parser::parse_variable_declaration()
{
    auto object = std::make_unique<vhdl::syntax::object>();
    object->set_v_kind(vhdl::syntax::object::v_::variable);

    if (optional(tk::kw_shared))
        object->v.variable.shared = true;

    consume(tk::kw_variable);

    object->identifier = parse_identifier_list();

    consume(tk::colon);

    object->type = parse_subtype_indication();

    if (optional(tk::coloneq))
        object->v.variable.value = parse_expression();

    consume(tk::semicolon);

    if (object->identifier.size() == 0 || !object->type)
        return nullptr;

    auto result = std::make_unique<vhdl::syntax::declarative_item>();
    result->set_v_kind(vhdl::syntax::declarative_item::v_::object);
    result->v.object.v = object.release();
    result->v.object.v->decl = result.get();
    return result.release();
}

vhdl::syntax::declarative_item* vhdl::parser::parse_file_declaration()
{
    auto object = std::make_unique<vhdl::syntax::object>();
    object->set_v_kind(vhdl::syntax::object::v_::file);

    consume(tk::kw_file);

    object->identifier = parse_identifier_list();

    consume(tk::colon);

    object->type = parse_subtype_indication();

    vhdl::syntax::expression* o = nullptr;
    if (optional(tk::kw_open))
    {
        if (version_ == vhdl87)
            diag(Err::Open_file_vhdl87);
        object->v.file.open = parse_expression();
    }

    if (current_token() != tk::kw_is && version_ == vhdl87)
        diag(Err::File_name_vhdl87);

    object->v.file.mode = vhdl::syntax::mode::in;
    if (optional(tk::kw_is))
    {
        switch (current_token()) {
        case tk::kw_in:
        case tk::kw_out:

            if (version_ != vhdl87)
                diag(Err::File_mode_vhdl87);

            object->v.file.mode = parse_mode();
            skip();
            break;
        case tk::kw_inout:
        case tk::kw_linkage:
            diag(Err::Wrong_file_mode);
            skip();
            [[fallthrough]];
        default:
            // defaults to "in" (read mode)
            break;
        }

        object->v.file.name = parse_expression();
    }

    consume(tk::semicolon);

    if (object->identifier.size() == 0 || !object->type ||
        !object->v.file.name)
        return nullptr;

    auto result = std::make_unique<vhdl::syntax::declarative_item>();
    result->set_v_kind(vhdl::syntax::declarative_item::v_::object);
    result->v.object.v = object.release();
    result->v.object.v->decl = result.get();
    return result.release();
}

std::vector<vhdl::syntax::declarative_item*>
vhdl::parser::parse_interface_list(vhdl::parser::interface kind)
{
    return parse_many_sep<vhdl::syntax::declarative_item*>(
        ps::interface_lists, tk::semicolon,
        [this, kind] { return parse_interface_element(kind); });
}

vhdl::syntax::declarative_item* vhdl::parser::parse_interface_element(
    interface kind)
{
    vhdl::syntax::declarative_item* result = nullptr;
    std::unique_ptr<vhdl::syntax::declarative_item> throw_away;

    auto first__ = lexer_.get_current_location();

    switch (current_token()) {
    case tk::kw_constant:
        if (kind == interface::port)
        {
            diag(Err::No_constant_in_port);
            throw_away.reset(parse_interface_constant_declaration());
        }
        else
        {
            result = parse_interface_constant_declaration();
        }
        break;
    case tk::kw_signal:
        if (kind == interface::generic)
        {
            diag(Err::No_signal_in_generic);
            throw_away.reset(parse_interface_signal_declaration());
        }
        else
        {
            result = parse_interface_signal_declaration();
        }
        break;
    case tk::kw_variable:
        if (kind == interface::generic)
        {
            diag(Err::No_variable_in_generic);
            throw_away.reset(parse_interface_variable_declaration());
        }
        else if (kind == interface::port)
        {
            diag(Err::No_variable_in_port);
            throw_away.reset(parse_interface_variable_declaration());
        }
        else
        {
            result = parse_interface_variable_declaration();
        }
        break;
    case tk::kw_file:
        if (version_ == vhdl87)
            diag(Err::No_file_in_vhdl87);

        if (kind == interface::generic)
        {
            diag(Err::No_file_in_generic);
            throw_away.reset(parse_interface_file_declaration());
        }
        else if (kind == interface::port)
        {
            diag(Err::No_file_in_port);
            throw_away.reset(parse_interface_file_declaration());
        }
        else
        {
            result = parse_interface_file_declaration();
        }
        break;
    case tk::identifier:
        // For functions: `If no reserved word is used, it is assumed that the
        // parameter is a constant`
        // http://vhdl.renerta.com/mobile/source/vhd00032.htm
        //
        // For procedures: `When in mode is declared and object class is not
        // defined, then by default it is assumed that the object is a constant
        // In case tk::of inout and out modes, the default class is variable
        // http://vhdl.renerta.com/mobile/source/vhd00052.htm

        if (kind == interface::function || kind == interface::generic)
        {
            result = parse_interface_constant_declaration();
        }
        else if (kind == interface::port)
        {
            result = parse_interface_signal_declaration();
        }
        else if (kind == interface::procedure)
        {
            const look_params keyword_in = {.look = {tk::kw_in},
                                            .stop = {},
                                            .abort = tk::semicolon,
                                            .nest_in = tk::leftpar,
                                            .nest_out = tk::rightpar,
                                            .depth = 0};
            if (lexer_.look_for(keyword_in))
                result = parse_interface_constant_declaration();
            else
                result = parse_interface_variable_declaration();
        }
        else
        {
            assert(false && "unhandled interface element from identifier");
        }

        break;
    case tk::kw_package:
        if (version_ < vhdl08)
            diag(Err::Package_only_in_vhdl08);

        if (kind != interface::generic)
            diag(Err::Package_only_in_generic);

        assert(false && "not yet supported");
        break;
    case tk::kw_type:
        if (version_ < vhdl08)
            diag(Err::Type_only_in_vhdl08);

        if (kind != interface::generic)
            diag(Err::Type_only_in_generic);

        assert(false && "not yet supported");
        break;
    case tk::kw_procedure:
    case tk::kw_pure:
    case tk::kw_impure:
    case tk::kw_function:
        if (version_ < vhdl08)
            diag(Err::Subprogram_only_in_vhdl08);

        if (kind != interface::generic)
            diag(Err::Subprogram_only_in_generic);

        assert(false && "not yet supported");
        break;
    default:
        assert(false && "we should never get here");
        break;
    }

    if (!result)
        return nullptr;

    result->first__ = first__;
    result->__last = lexer_.get_previous_location();
    return result;
}

vhdl::syntax::declarative_item*
vhdl::parser::parse_interface_constant_declaration()
{
    auto interface = std::make_unique<vhdl::syntax::interface>();
    interface->set_v_kind(vhdl::syntax::interface::v_::constant);

    optional(tk::kw_constant);

    interface->identifier = parse_identifier_list();

    consume(tk::colon);

    optional(tk::kw_in);

    interface->type = parse_subtype_indication();

    if (optional(tk::coloneq))
        interface->v.constant.value = parse_expression();

    if (!interface->type)
        return nullptr;

    auto result = std::make_unique<vhdl::syntax::declarative_item>();
    result->set_v_kind(vhdl::syntax::declarative_item::v_::interface);
    result->v.interface.v = interface.release();
    result->v.interface.v->decl = result.get();
    return result.release();
}

vhdl::syntax::declarative_item*
vhdl::parser::parse_interface_signal_declaration()
{
    auto interface = std::make_unique<vhdl::syntax::interface>();
    interface->set_v_kind(vhdl::syntax::interface::v_::signal);

    optional(tk::kw_signal);

    interface->identifier = parse_identifier_list();

    consume(tk::colon);

    switch (current_token()) {
    case tk::kw_in:
    case tk::kw_out:
    case tk::kw_inout:
    case tk::kw_buffer:
    case tk::kw_linkage:
        interface->v.signal.mode = parse_mode();
        skip();
        break;
    default:
        // defaults to "in" (read mode)
        interface->v.signal.mode = vhdl::syntax::mode::in;
        break;
    }

    interface->type = parse_subtype_indication();

    if (current_token() == tk::kw_bus)
        interface->v.signal.bus = true;

    vhdl::syntax::expression* v = nullptr;
    if (optional(tk::coloneq))
        interface->v.signal.value = parse_expression();

    if (interface->identifier.size() == 0 || !interface->type)
        return nullptr;

    auto result = std::make_unique<vhdl::syntax::declarative_item>();
    result->set_v_kind(vhdl::syntax::declarative_item::v_::interface);
    result->v.interface.v = interface.release();
    result->v.interface.v->decl = result.get();
    return result.release();
}

vhdl::syntax::mode vhdl::parser::parse_mode()
{
    using mode = vhdl::syntax::mode;

    switch (current_token()) {
    case tk::kw_in:      return mode::in;
    case tk::kw_out:     return mode::out;
    case tk::kw_inout:   return mode::inout;
    case tk::kw_buffer:  return mode::buffer;
    case tk::kw_linkage: return mode::linkage;
    default:             return mode::invalid;
    }
}

vhdl::syntax::declarative_item*
vhdl::parser::parse_interface_variable_declaration()
{
    auto interface = std::make_unique<vhdl::syntax::interface>();
    interface->set_v_kind(vhdl::syntax::interface::v_::variable);

    optional(tk::kw_variable);

    interface->identifier = parse_identifier_list();

    consume(tk::colon);

    switch (current_token()) {
    case tk::kw_in:
    case tk::kw_out:
    case tk::kw_inout:
    case tk::kw_buffer:
    case tk::kw_linkage:
        interface->v.variable.mode = parse_mode();
        skip();
        break;
    default:
        // defaults to "in" (read mode)
        interface->v.variable.mode = vhdl::syntax::mode::in;
        break;
    }

    interface->type = parse_subtype_indication();

    if (optional(tk::coloneq))
        interface->v.variable.value = parse_expression();

    if (interface->identifier.size() == 0 || !interface->type)
        return nullptr;

    auto result = std::make_unique<vhdl::syntax::declarative_item>();
    result->set_v_kind(vhdl::syntax::declarative_item::v_::interface);
    result->v.interface.v = interface.release();
    result->v.interface.v->decl = result.get();
    return result.release();
}

vhdl::syntax::declarative_item*
vhdl::parser::parse_interface_file_declaration()
{
    auto interface = std::make_unique<vhdl::syntax::interface>();
    interface->set_v_kind(vhdl::syntax::interface::v_::file);

    consume(tk::kw_file);

    interface->identifier = parse_identifier_list();

    consume(tk::colon);

    interface->type = parse_subtype_indication();

    if (!interface->type)
        return nullptr;

    auto result = std::make_unique<vhdl::syntax::declarative_item>();
    result->set_v_kind(vhdl::syntax::declarative_item::v_::interface);
    result->v.interface.v = interface.release();
    result->v.interface.v->decl = result.get();
    return result.release();
}

std::vector<vhdl::syntax::association_element*>
vhdl::parser::parse_association_list()
{
    std::vector<vhdl::syntax::association_element*> result;

    if (optional(tk::rightpar))
    {
        diag(Err::Empty_assoc_list, lexer_.get_previous_location());
        return result;
    }

    while (current_token() != tk::eof)
    {
        auto e = std::make_unique<vhdl::syntax::association_element>();

        const look_params right_arrow = {.look = {tk::rightarrow},
                                         .stop = {tk::rightpar, tk::comma},
                                         .abort = tk::semicolon,
                                         .nest_in = tk::leftpar,
                                         .nest_out = tk::rightpar,
                                         .depth = 0};

        if (lexer_.look_for(right_arrow))
        {
            e->formal = parse_name();
            consume(tk::rightarrow);
        }

        if (!optional(tk::kw_open))
        {
            e->actual = parse_expression();
        }

        result.push_back(e.release());

        if (!optional(tk::comma))
            break;

        if (current_token() == tk::rightpar)
        {
            diag(Err::Unexpected_comma, lexer_.get_previous_location());
            break;
        }
    }

    return result;
}

vhdl::syntax::declarative_item* vhdl::parser::parse_alias_declaration()
{
    auto result = std::make_unique<vhdl::syntax::declarative_item>();
    result->set_v_kind(vhdl::syntax::declarative_item::v_::alias);

    consume(tk::kw_alias);

    auto [t, v] = parse_alias_designator();
    result->v.alias.designator = t;

    if (optional(tk::colon))
    {
        result->v.alias.type = parse_subtype_indication();
    }

    consume(tk::kw_is);

    name_options options;
    options.allow_qifts = true;
    options.allow_complex_names = true;
    options.allow_signature = true;
    result->v.alias.name = parse_name(options);

    consume(tk::semicolon);

    if (!v || !result->v.alias.name)
        return nullptr;

    return result.release();
}

vhdl::syntax::declarative_item* vhdl::parser::parse_attribute_declaration()
{
    auto result = std::make_unique<vhdl::syntax::declarative_item>();
    result->set_v_kind(vhdl::syntax::declarative_item::v_::attribute);

    consume(tk::kw_attribute);

    auto [t, v] = parse_identifier(Err::Expected_attribute_identifier);
    result->v.attribute.identifier = t;

    consume(tk::colon);

    result->v.attribute.type = parse_type_mark();

    consume(tk::semicolon);

    if (!v || !result->v.attribute.type)
        return nullptr;

    return result.release();
}

vhdl::syntax::declarative_item* vhdl::parser::parse_component_declaration()
{
    auto result = std::make_unique<vhdl::syntax::declarative_item>();
    result->set_v_kind(vhdl::syntax::declarative_item::v_::component);

    consume(tk::kw_component);

    auto [t, v] = parse_identifier(Err::Expected_component_identifier);
    result->v.component.identifier = t;

    if (version_ == vhdl87 && current_token() == tk::kw_is)
        diag(Err::Is_not_allowed_in_vhdl87);

    optional(tk::kw_is);

    if (current_token() == tk::kw_generic)
    {
        auto [generic__, gl__, gens, __gr] = parse_generic_clause();
        result->v.component.generic__ = generic__;
        result->v.component.gl__ = gl__;
        result->v.component.gens = gens;
        result->v.component.__gr = __gr;
    }

    if (current_token() == tk::kw_port)
    {
        auto [port__, pl__, ports, __pr] = parse_port_clause();
        result->v.component.port__ = port__;
        result->v.component.pl__ = pl__;
        result->v.component.ports = ports;
        result->v.component.__pr = __pr;
    }

    consume(tk::kw_end);
    consume(tk::kw_component);

    parse_optional_closing_label(t);

    consume(tk::semicolon);

    if (!v)
        return nullptr;

    return result.release();
}

// ----------------------------------------------------------------------------
// Specification
// ----------------------------------------------------------------------------
// Implements these chapters from the VHDL LRM:
// LRM93 5.0
// ----------------------------------------------------------------------------

vhdl::syntax::declarative_item* vhdl::parser::parse_attribute_specification()
{
    auto result = std::make_unique<vhdl::syntax::declarative_item>();
    result->set_v_kind(vhdl::syntax::declarative_item::v_::attr_spec);

    consume(tk::kw_attribute);

    name_options options;
    options.allow_qifts = false;
    options.allow_complex_names = false;
    result->v.attr_spec.designator = parse_name(options);

    if (current_token() == tk::colon)
    {
        diag(Err::Colon_in_attr_spec);
        consume(tk::colon);
    }

    consume(tk::kw_of);

    result->v.attr_spec.spec = parse_entity_specification();

    consume(tk::kw_is);

    result->v.attr_spec.value = parse_expression();

    consume(tk::semicolon);

    if (!result->v.attr_spec.designator || !result->v.attr_spec.spec)
    {
        return nullptr;
    }

    return result.release();
}

vhdl::syntax::entity_specification* vhdl::parser::parse_entity_specification()
{
    auto result = std::make_unique<vhdl::syntax::entity_specification>();

    using is = vhdl::syntax::entity_specification::entity_name_list_;
    switch (current_token()) {
    case tk::identifier:
    case tk::character:
    case tk::stringliteral:
        result->set_entity_name_list_kind(is::name);
        do
        {
            if (auto d = parse_entity_designator())
                result->entity_name_list.name.names.push_back(d);
        } while (optional(tk::comma));
        break;
    case tk::kw_others:
        result->set_entity_name_list_kind(is::others);
        result->entity_name_list.others.token = lexer_.current_token();
        consume(tk::kw_others);
        break;
    case tk::kw_all:
        result->set_entity_name_list_kind(is::all);
        result->entity_name_list.all.token = lexer_.current_token();
        consume(tk::kw_all);
        break;
    default:
        break;
    }

    consume(tk::colon);

    auto v = parse_entity_class(result->entity_class);

    if (result->entity_name_list_kind == is::none || !v)
        return nullptr;

    return result.release();
}

vhdl::syntax::name* vhdl::parser::parse_entity_designator()
{
    switch (current_token()) {
    case tk::character:
        if (version_ == vhdl87)
            diag(Err::Char_entity_designator_vhdl87);
        [[fallthrough]];
    case tk::identifier:
    case tk::stringliteral:
        return parse_name();

    default:
        diag(Err::Expected_entity_designator);
        return nullptr;
    }
}

bool vhdl::parser::parse_entity_class(vhdl::syntax::entity_class& entry)
{
    using is = vhdl::syntax::entity_class;
    switch (current_token()) {
    case tk::kw_entity:        entry = is::entity;        break;
    case tk::kw_procedure:     entry = is::procedure;     break;
    case tk::kw_type:          entry = is::type;          break;
    case tk::kw_signal:        entry = is::signal;        break;
    case tk::kw_label:         entry = is::label;         break;
    case tk::kw_group:         entry = is::group;         break;
    case tk::kw_architecture:  entry = is::architecture;  break;
    case tk::kw_function:      entry = is::function;      break;
    case tk::kw_subtype:       entry = is::subtype;       break;
    case tk::kw_variable:      entry = is::variable;      break;
    case tk::kw_literal:       entry = is::literal;       break;
    case tk::kw_file:          entry = is::file;          break;
    case tk::kw_configuration: entry = is::configuration; break;
    case tk::kw_package:       entry = is::package;       break;
    case tk::kw_constant:      entry = is::constant;      break;
    case tk::kw_component:     entry = is::component;     break;
    case tk::kw_units:         entry = is::units;         break;
    default:                                       return false;
    }

    skip();
    return true;
}

vhdl::syntax::declarative_item*
vhdl::parser::parse_configuration_specification()
{
    auto result = std::make_unique<vhdl::syntax::declarative_item>();
    result->set_v_kind(vhdl::syntax::declarative_item::v_::config_spec);

    consume(tk::kw_for);

    result->v.config_spec.spec = parse_component_specification();
    result->v.config_spec.binding = parse_binding_indication();

    if (!result->v.config_spec.spec || !result->v.config_spec.binding)
    {
        return nullptr;
    }

    consume(tk::semicolon);

    return result.release();
}

vhdl::syntax::component_specification*
vhdl::parser::parse_component_specification()
{
    auto spec = std::make_unique<vhdl::syntax::component_specification>();
    using is = vhdl::syntax::component_specification::instantiation_list_;

    switch (current_token()) {
    case tk::kw_others:
        spec->set_instantiation_list_kind(is::others);
        consume(tk::kw_others);
        break;
    case tk::kw_all:
        spec->set_instantiation_list_kind(is::all);
        consume(tk::kw_all);
        break;
    case tk::identifier:
        spec->set_instantiation_list_kind(is::labels);
        do
        {
            spec->instantiation_list.labels.labels.push_back(
                lexer_.current_token());
            consume(tk::identifier);
        } while (optional(tk::comma));
        break;
    default:
        diag(Err::Expected_instantiation_list);
        spec->set_instantiation_list_kind(is::all);
        break;
    }

    consume(tk::colon);

    if (current_token() != tk::identifier)
    {
        diag(Err::Expected_component_name);
        return nullptr;
    }

    spec->component = parse_name();

    return spec.release();
}

vhdl::syntax::binding_indication* vhdl::parser::parse_binding_indication()
{
    auto binding = std::make_unique<vhdl::syntax::binding_indication>();

    if (current_token() == tk::kw_use)
    {
        consume(tk::kw_use);

        parse_entity_aspect(binding.get());
    }

    if (current_token() == tk::kw_generic)
    {
        auto gm = parse_generic_map_aspect();
        binding->gens = std::get<2>(gm);
    }

    if (current_token() == tk::kw_port)
    {
        auto pm = parse_port_map_aspect();
        binding->ports = std::get<2>(pm);
    }

    return binding.release();
}

void vhdl::parser::parse_entity_aspect(
    vhdl::syntax::binding_indication* binding)
{
    using is = vhdl::syntax::binding_indication::entity_aspect_;
    switch (current_token()) {
    case tk::kw_entity: {
        consume(tk::kw_entity);
        binding->set_entity_aspect_kind(is::entity);

        name_options options;
        options.allow_qifts = false;
        options.allow_complex_names = false;
        binding->entity_aspect.entity.name = parse_name(options);

        if (optional(tk::leftpar))
        {
            binding->entity_aspect.entity.architecture =
                lexer_.current_token();

            consume(tk::identifier);
            consume(tk::rightpar);
        }
        break;
    }

    case tk::kw_configuration: {
        consume(tk::kw_configuration);
        binding->set_entity_aspect_kind(is::configuration);

        name_options options;
        options.allow_qifts = false;
        options.allow_complex_names = false;
        binding->entity_aspect.configuration.name = parse_name(options);
        break;
    }

    case tk::kw_open:
        consume(tk::kw_open);
        binding->set_entity_aspect_kind(is::open);
        break;

    default:
        binding->set_entity_aspect_kind(is::open);
        diag(Err::Expected_entity_aspect);
        break;
    }
}

std::tuple<common::location, common::location,
           std::vector<vhdl::syntax::association_element*>, common::location>
vhdl::parser::parse_generic_map_aspect()
{
    auto generic__ = eat(tk::kw_generic);
    consume(tk::kw_map);

    auto gl__ = eat(tk::leftpar);

    auto result = parse_association_list();

    auto __gr = eat(tk::rightpar);

    return std::make_tuple(generic__, gl__, result, __gr);
}

std::tuple<common::location, common::location,
           std::vector<vhdl::syntax::association_element*>, common::location>
vhdl::parser::parse_port_map_aspect()
{
    auto port__ = eat(tk::kw_port);
    consume(tk::kw_map);

    auto pl__ = eat(tk::leftpar);

    auto result = parse_association_list();

    auto __pr = eat(tk::rightpar);

    return std::make_tuple(port__, pl__, result, __pr);
}

// ----------------------------------------------------------------------------
// Names
// ----------------------------------------------------------------------------
// Implements these chapters from the VHDL LRM:
// LRM93 6.0
// ----------------------------------------------------------------------------

vhdl::syntax::name* vhdl::parser::parse_name(vhdl::parser::name_options options)
{
    vhdl::syntax::name* result = nullptr;
    switch (current_token()) {
    case tk::identifier:
    case tk::character:
    case tk::stringliteral:
        result = parse_simple_name(options);
        break;
    default:
        diag(Err::Expected_name);
        return nullptr;
    }

    while (current_token() != tk::eof)
    {
        switch (current_token()) {
        case tk::tick:
            // this can either be an attribute name, or a qualified expression.
            // If it is a qualified expression, check whether allow_qifts

            // this is definitely an attribute name
            if (peek() != tk::leftpar)
            {
                result = parse_attribute_name(options, result);
            }
            else if (options.allow_qifts)
            {
                return parse_qualified_expression(options, result);
            }
            else
            {
                return result;
            }
            break;
        case tk::leftsquare: {
            // this is either an attribute name or a signatured name

            const look_params right_square = {.look = {tk::rightsquare},
                                              .stop = {tk::rightpar, tk::comma},
                                              .abort = tk::semicolon,
                                              .nest_in = tk::leftpar,
                                              .nest_out = tk::rightpar,
                                              .depth = 1};
            if (/*FIXME lexer_.after(right_square, 1) == tk::tick*/ true)
            {
                result = parse_signed_name(options, result);
            }
            else if (options.allow_signature)
            {
                return parse_signed_name(options, result);
            }
            else
            {
                return result;
            }
            break;
        }
        case tk::leftpar:
            if (!options.allow_qifts)
            {
                return result;
            }

            // this is either a function call, a type conversion, an indexed or
            // a slice name.

            result = parse_parenthesis_name(options, result);
            break;
        case tk::dot:
            // this is either an explicit dereference (access.all), an expanded
            // name or a record reference

            result = parse_selected_name(options, result);
            break;
        default:

            return result;
        }
    }

    return result;
}

vhdl::syntax::name* vhdl::parser::parse_simple_name(
    vhdl::parser::name_options& options)
{
    auto result = std::make_unique<vhdl::syntax::name>();
    result->set_v_kind(vhdl::syntax::name::v_::simple);

    result->v.simple.identifier = lexer_.current_token();
    skip();

    return result.release();
}

vhdl::syntax::name* vhdl::parser::parse_selected_name(
    vhdl::parser::name_options& options, vhdl::syntax::name* p)
{
    auto result = std::make_unique<vhdl::syntax::name>();
    consume(tk::dot);

    vhdl::token s;
    switch (current_token()) {
    case tk::identifier:
    case tk::character:
    case tk::stringliteral:
    case tk::kw_all:
        result->set_v_kind(vhdl::syntax::name::v_::selected);
        result->v.selected.prefix = p;
        result->v.selected.identifier = lexer_.current_token();
        skip();
        break;
    default:
        // this is a parse error
        diag(Err::Expected_sel_name);
        result->set_v_kind(vhdl::syntax::name::v_::error);
        result->v.error.prefix = p;
        return nullptr;
    }

    return result.release();
}

vhdl::syntax::name* vhdl::parser::parse_parenthesis_name(
    vhdl::parser::name_options& options, vhdl::syntax::name* p)
{
    const look_params downto_upto_or_range_keyword = {
        .look = {tk::kw_downto, tk::kw_to, tk::kw_range},
        .stop = {tk::rightpar, tk::comma},
        .abort = tk::semicolon,
        .nest_in = tk::leftpar,
        .nest_out = tk::rightpar,
        .depth = 1};
    if (lexer_.look_for(downto_upto_or_range_keyword))
    {
        auto result = std::make_unique<vhdl::syntax::name>();
        result->set_v_kind(vhdl::syntax::name::v_::slice);
        result->v.slice.prefix = p;

        consume(tk::leftpar);
        result->v.slice.range = parse_discrete_range();
        consume(tk::rightpar);

        if (!p || !result->v.slice.range)
            return nullptr;

        return result.release();
    }

    const look_params rightarrow = {.look = {tk::rightarrow},
                                    .stop = {tk::rightpar},
                                    .abort = tk::semicolon,
                                    .nest_in = tk::leftpar,
                                    .nest_out = tk::rightpar,
                                    .depth = 1};
    if (lexer_.look_for(rightarrow))
    {
        return parse_function_call(options, p);
    }

    // Do note that the vhdl syntax for indexed names and function calls can
    // sometimes be ambiguous.
    // For example, depending on context, wawa(1) can either mean:
    // - indexed name, to get index 1 of array wawa
    // - function call of wawa with params 1
    // - function call of wawa no params, returns array, get index 1

    consume(tk::leftpar);

    auto result = std::make_unique<vhdl::syntax::name>();
    result->set_v_kind(vhdl::syntax::name::v_::ambiguous);
    result->v.ambiguous.prefix = p;

    do
    {
        if (auto e = parse_expression())
            result->v.ambiguous.expression.push_back(e);
    } while (optional(tk::comma));

    consume(tk::rightpar);

    if (!p)
        return nullptr;

    return result.release();
}

vhdl::syntax::name* vhdl::parser::parse_signed_name(
    vhdl::parser::name_options& options, vhdl::syntax::name* p)
{
    auto result = std::make_unique<vhdl::syntax::name>();
    result->set_v_kind(vhdl::syntax::name::v_::signature);
    result->v.signature.prefix = p;
    result->v.signature.signature = parse_signature();

    if (!result->v.signature.prefix || !result->v.signature.signature)
        return nullptr;

    return result.release();
}

vhdl::syntax::attr_kind get_attr_kind(vhdl::token& tok)
{
    using attr = vhdl::syntax::attr_kind;

    auto value = tok.value;
    if (value == "length")        return attr::length;
    if (value == "left")          return attr::left;
    if (value == "right")         return attr::right;
    if (value == "low")           return attr::low;
    if (value == "high")          return attr::high;
    if (value == "event")         return attr::event;
    if (value == "active")        return attr::active;
    if (value == "image")         return attr::image;
    if (value == "ascending")     return attr::ascending;
    if (value == "last_value")    return attr::last_value;
    if (value == "last_event")    return attr::last_event;
    if (value == "path_name")     return attr::path_name;
    if (value == "instance_name") return attr::instance_name;
    if (value == "delayed")       return attr::delayed;
    if (value == "stable")        return attr::stable;
    if (value == "quiet")         return attr::quiet;
    if (value == "transaction")   return attr::transaction;
    if (value == "driving_value") return attr::driving_value;
    if (value == "last_active")   return attr::last_active;
    if (value == "driving")       return attr::driving;
    if (value == "value")         return attr::value;
    if (value == "succ")          return attr::succ;
    if (value == "pred")          return attr::pred;
    if (value == "leftof")        return attr::leftof;
    if (value == "rightof")       return attr::rightof;
    if (value == "pos")           return attr::pos;
    if (value == "val")           return attr::val;
    if (value == "range")         return attr::range;

    return attr::user_defined;
}

vhdl::syntax::name* vhdl::parser::parse_attribute_name(
    vhdl::parser::name_options& options, vhdl::syntax::name* p)
{
    auto result = std::make_unique<vhdl::syntax::name>();
    result->set_v_kind(vhdl::syntax::name::v_::attribute);
    result->v.attribute.prefix = p;

    consume(tk::tick);

    auto attr = lexer_.current_token();
    result->v.attribute.kind = get_attr_kind(attr);

    // 'range is a valid attribute. However, range is a vhdl keyword. So we
    // treat it differently
    if (attr.kind == tk::kw_range)
    {
        attr.kind = tk::identifier;
        attr.is_delimiter = false;
        attr.is_identifier = true;
        attr.is_keyword = false;
        attr.is_literal = false;
    }

    result->v.attribute.attr = attr;

    skip();

    if (optional(tk::leftpar))
    {
        if (auto e = parse_expression())
            result->v.attribute.expression = e;

        consume(tk::rightpar);
    }

    if (!result->v.attribute.prefix)
        return nullptr;

    return result.release();
}

// ----------------------------------------------------------------------------
// Expressions
// ----------------------------------------------------------------------------
// Implements these chapters from the VHDL LRM:
// LRM93 7.0
// ----------------------------------------------------------------------------

vhdl::syntax::op vhdl::parser::parse_operation()
{
    using op = vhdl::syntax::op;

    switch (current_token()) {
    case tk::kw_and:   skip();  return op::op_and;
    case tk::kw_or:    skip();  return op::op_or;
    case tk::kw_nand:  skip();  return op::op_nand;
    case tk::kw_nor:   skip();  return op::op_nor;
    case tk::kw_xor:   skip();  return op::op_xor;
    case tk::kw_xnor:  skip();  return op::op_xnor;
    case tk::eq:       skip();  return op::op_eq;
    case tk::ne:       skip();  return op::op_neq;
    case tk::lt:       skip();  return op::op_lt;
    case tk::lte:      skip();  return op::op_lte;
    case tk::gt:       skip();  return op::op_gt;
    case tk::gte:      skip();  return op::op_gte;
    case tk::kw_sll:   skip();  return op::op_sll;
    case tk::kw_srl:   skip();  return op::op_srl;
    case tk::kw_sla:   skip();  return op::op_sla;
    case tk::kw_sra:   skip();  return op::op_sra;
    case tk::kw_rol:   skip();  return op::op_rol;
    case tk::kw_ror:   skip();  return op::op_ror;
    case tk::plus:     skip();  return op::op_add;
    case tk::minus:    skip();  return op::op_sub;
    case tk::concat:   skip();  return op::op_concat;
    case tk::times:    skip();  return op::op_mul;
    case tk::div:      skip();  return op::op_div;
    case tk::kw_mod:   skip();  return op::op_mod;
    case tk::kw_rem:   skip();  return op::op_rem;
    case tk::pow:      skip();  return op::op_pow;
    case tk::kw_abs:   skip();  return op::op_abs;
    case tk::kw_not:   skip();  return op::op_not;
    default:                    return op::error;
    }
}

vhdl::syntax::expression* vhdl::parser::parse_expression()
{
    return parse_boolean_expression();
}

vhdl::syntax::expression* vhdl::parser::parse_primary()
{
    switch (current_token()) {
    case tk::integer: {
        auto literal = lexer_.current_token();
        skip();

        if (current_token() == tk::identifier)
        {
            auto result = std::make_unique<vhdl::syntax::expression>();
            result->set_v_kind(vhdl::syntax::expression::v_::physical);
            result->v.physical.token = literal;
            result->v.physical.name = parse_name();

            if (!result->v.physical.name)
                return nullptr;

            return result.release();
        }
        else
        {
            auto result = std::make_unique<vhdl::syntax::expression>();
            result->set_v_kind(vhdl::syntax::expression::v_::literal);
            result->v.literal.kind = vhdl::syntax::literal_kind::integer;
            result->v.literal.token = literal;
            return result.release();
        }
    }
    case tk::real: {
        auto literal = lexer_.current_token();
        skip();

        if (current_token() == tk::identifier)
        {
            auto result = std::make_unique<vhdl::syntax::expression>();
            result->set_v_kind(vhdl::syntax::expression::v_::physical);
            result->v.physical.token = literal;
            result->v.physical.name = parse_name();

            if (!result->v.physical.name)
                return nullptr;

            return result.release();
        }
        else
        {
            auto result = std::make_unique<vhdl::syntax::expression>();
            result->set_v_kind(vhdl::syntax::expression::v_::literal);
            result->v.literal.kind = vhdl::syntax::literal_kind::real;
            result->v.literal.token = literal;
            return result.release();
        }
    }
    case tk::identifier: {
        auto result = std::make_unique<vhdl::syntax::expression>();
        result->set_v_kind(vhdl::syntax::expression::v_::unresolved);
        result->v.unresolved.name = parse_name();

        if (!result->v.unresolved.name)
            return nullptr;

        return result.release();
    }
    case tk::character: {
        auto result = std::make_unique<vhdl::syntax::expression>();
        result->set_v_kind(vhdl::syntax::expression::v_::unresolved);

        name_options options;
        options.allow_complex_names = false;
        result->v.unresolved.name = parse_name(options);

        if (!result->v.unresolved.name)
            return nullptr;

        return result.release();
    }
    case tk::leftpar: {
        const look_params comma_or_arrow = {.look = {tk::comma, tk::rightarrow},
                                            .stop = {tk::rightpar},
                                            .abort = tk::semicolon,
                                            .nest_in = tk::leftpar,
                                            .nest_out = tk::rightpar,
                                            .depth = 1};
        if (lexer_.look_for(comma_or_arrow))
            return parse_aggregate();

        // this is probably a nested expression
        auto result = std::make_unique<vhdl::syntax::expression>();
        result->set_v_kind(vhdl::syntax::expression::v_::nested);

        consume(tk::leftpar);
        result->v.nested.expr = parse_boolean_expression();
        consume(tk::rightpar);

        if (!result->v.nested.expr)
            return nullptr;

        return result.release();
    }
    case tk::stringliteral: {

        if (peek() == tk::leftpar)
        {
            auto n = parse_name();

            auto result = std::make_unique<vhdl::syntax::expression>();
            result->set_v_kind(vhdl::syntax::expression::v_::unresolved);
            result->v.unresolved.name = n;

            if (!result->v.unresolved.name)
                return nullptr;

            return result.release();
        }

        auto result = std::make_unique<vhdl::syntax::expression>();
        result->set_v_kind(vhdl::syntax::expression::v_::literal);
        result->v.literal.kind = vhdl::syntax::literal_kind::string;
        result->v.literal.token = lexer_.current_token();

        skip();

        return result.release();
    }
    case tk::kw_null:
        // skip the null keyword
        skip();
        break;

    case tk::kw_new:
        return parse_allocator();

    case tk::bitstring: {
        auto result = std::make_unique<vhdl::syntax::expression>();
        result->set_v_kind(vhdl::syntax::expression::v_::literal);
        result->v.literal.kind = vhdl::syntax::literal_kind::bitstring;
        result->v.literal.token = lexer_.current_token();

        skip();

        return result.release();
    }
    case tk::plus:
    case tk::minus: {
        diag(Err::Plus_minus_skip);

        // skip this expression
        auto result = std::make_unique<vhdl::syntax::expression>();
        result->set_v_kind(vhdl::syntax::expression::v_::nested);
        result->v.nested.expr = parse_boolean_expression();

        return result.release();
    }
    case tk::comma:
    case tk::semicolon:
    case tk::rightpar:
    case tk::eof:
    case tk::kw_end:
        diag(Err::Expected_expression);
        break;
    default:
        diag(Err::Expected_expression);
        break;
    }

    auto result = std::make_unique<vhdl::syntax::expression>();
    result->set_v_kind(vhdl::syntax::expression::v_::null);

    return result.release();
}

vhdl::syntax::expression* vhdl::parser::parse_boolean_expression()
{
    auto result = parse_relation();

    parse_boolean_expression:

    switch (current_token()) {
    case tk::kw_nand:
    case tk::kw_nor: {
        auto e = std::make_unique<vhdl::syntax::expression>();
        e->set_v_kind(vhdl::syntax::expression::v_::binary);
        e->v.binary.op = parse_operation();
        e->v.binary.lhs = result;
        e->v.binary.rhs = parse_relation();
        return e.release();
    }
    case tk::kw_and:
    case tk::kw_or:
    case tk::kw_xor:
    case tk::kw_xnor: {
        auto e = std::make_unique<vhdl::syntax::expression>();
        e->set_v_kind(vhdl::syntax::expression::v_::binary);
        e->v.binary.op = parse_operation();
        e->v.binary.lhs = result;
        e->v.binary.rhs = parse_relation();

        result = e.release();

        goto parse_boolean_expression;
    }
    default:
        return result;
    }
}

vhdl::syntax::expression* vhdl::parser::parse_relation()
{
    auto result = parse_shift_expression();

    switch (current_token()) {
    case tk::eq:
    case tk::ne:
    case tk::lt:
    case tk::gt:
    case tk::lte:
    case tk::gte: {
        auto e = std::make_unique<vhdl::syntax::expression>();
        e->set_v_kind(vhdl::syntax::expression::v_::binary);
        e->v.binary.op = parse_operation();
        e->v.binary.lhs = result;
        e->v.binary.rhs = parse_shift_expression();
        return e.release();
    }
    default:
        return result;
    }
}

vhdl::syntax::expression* vhdl::parser::parse_shift_expression()
{
    auto result = parse_simple_expression();

    parse_shift_expression:

    switch (current_token()) {
    case tk::kw_sll:
    case tk::kw_srl:
    case tk::kw_sla:
    case tk::kw_sra:
    case tk::kw_rol:
    case tk::kw_ror: {
        auto e = std::make_unique<vhdl::syntax::expression>();
        e->set_v_kind(vhdl::syntax::expression::v_::binary);
        e->v.binary.op = parse_operation();
        e->v.binary.lhs = result;
        e->v.binary.rhs = parse_simple_expression();
        result = e.release();

        goto parse_shift_expression;
    }
    default:
        return result;
    }
}

vhdl::syntax::expression* vhdl::parser::parse_simple_expression()
{
    vhdl::syntax::op op = vhdl::syntax::op::error;
    switch (current_token()) {
    case tk::plus:
        op = vhdl::syntax::op::u_add;
        skip();
        break;
    case tk::minus:
        op = vhdl::syntax::op::u_sub;
        skip();
        break;
    default:
        break;
    }

    auto result = parse_term();

    if (op != vhdl::syntax::op::error)
    {
        auto e = std::make_unique<vhdl::syntax::expression>();
        e->set_v_kind(vhdl::syntax::expression::v_::unary);
        e->v.unary.lhs = result;
        e->v.unary.op = op;
        result = e.release();
    }

    parse_simple_expression:

    switch (current_token()) {
    case tk::concat:
    case tk::plus:
    case tk::minus: {
        auto e = std::make_unique<vhdl::syntax::expression>();
        e->set_v_kind(vhdl::syntax::expression::v_::binary);
        e->v.binary.op = parse_operation();
        e->v.binary.lhs = result;
        e->v.binary.rhs = parse_term();
        result = e.release();

        goto parse_simple_expression;
    }
    default:
        return result;
    }
}

vhdl::syntax::expression* vhdl::parser::parse_term()
{
    vhdl::syntax::expression* result = parse_factor();

    parse_term:

    switch (current_token()) {
    case tk::times:
    case tk::div:
    case tk::kw_mod:
    case tk::kw_rem: {
        auto e = std::make_unique<vhdl::syntax::expression>();
        e->set_v_kind(vhdl::syntax::expression::v_::binary);
        e->v.binary.op = parse_operation();
        e->v.binary.lhs = result;
        e->v.binary.rhs = parse_factor();
        result = e.release();

        goto parse_term;
    }
    default:
        return result;
    }
}

vhdl::syntax::expression* vhdl::parser::parse_factor()
{
    switch (current_token()) {
    case tk::kw_abs:
    case tk::kw_not: {
        auto e = std::make_unique<vhdl::syntax::expression>();
        e->set_v_kind(vhdl::syntax::expression::v_::unary);
        e->v.unary.op = parse_operation();
        e->v.unary.lhs = parse_primary();
        return e.release();
    }
    default:
        break;
    }

    auto result = parse_primary();

    if (current_token() == tk::pow)
    {
        auto e = std::make_unique<vhdl::syntax::expression>();
        e->set_v_kind(vhdl::syntax::expression::v_::binary);
        e->v.binary.op = parse_operation();
        e->v.binary.lhs = result;
        e->v.binary.rhs = parse_factor();
        result = e.release();
    }

    return result;
}

vhdl::syntax::expression* vhdl::parser::parse_aggregate()
{
    consume(tk::leftpar);

    auto result = std::make_unique<vhdl::syntax::expression>();
    result->set_v_kind(vhdl::syntax::expression::v_::aggregate);

    do
    {
        if (auto association = parse_element_association())
            result->v.aggregate.literal.push_back(association);
    } while (optional(tk::comma));

    consume(tk::rightpar);

    return result.release();
}

vhdl::syntax::element_association* vhdl::parser::parse_element_association()
{
    auto result = std::make_unique<vhdl::syntax::element_association>();

    const look_params rightarrow = {.look = {tk::rightarrow},
                                    .stop = {tk::comma, tk::rightpar},
                                    .abort = tk::semicolon,
                                    .nest_in = tk::leftpar,
                                    .nest_out = tk::rightpar,
                                    .depth = 0};

    if (current_token() == tk::kw_others || lexer_.look_for(rightarrow))
    {
        result->choices = parse_choices();
        consume(tk::rightarrow);
    }

    result->expression = parse_expression();

    if (!result->expression)
        return nullptr;

    return result.release();
}

std::vector<vhdl::syntax::choice*> vhdl::parser::parse_choices()
{
    std::vector<vhdl::syntax::choice*> choices;
    while (current_token() != tk::eof)
    {
        // do parse choice
        if (auto choice = parse_choice())
            choices.push_back(choice);

        if (current_token() != tk::bar)
            break;

        // skip the current token |
        consume(tk::bar);
    }

    return choices;
}

vhdl::syntax::choice* vhdl::parser::parse_choice()
{
    if (optional(tk::kw_others))
    {
        auto choice = std::make_unique<vhdl::syntax::choice>();
        choice->set_v_kind(vhdl::syntax::choice::v_::others);
        return choice.release();
    }

    const look_params that = {
        .look = {tk::kw_downto, tk::kw_to, tk::kw_range},
        .stop = {tk::comma, tk::rightpar, tk::rightarrow, tk::bar},
        .abort = tk::semicolon,
        .nest_in = tk::leftpar,
        .nest_out = tk::rightpar,
        .depth = 0};

    if (lexer_.look_for(that))
    {
        auto choice = std::make_unique<vhdl::syntax::choice>();
        choice->set_v_kind(vhdl::syntax::choice::v_::range);
        choice->v.range.range = parse_discrete_range();

        if (!choice->v.range.range)
            return nullptr;

        return choice.release();
    }

    auto choice = std::make_unique<vhdl::syntax::choice>();
    choice->set_v_kind(vhdl::syntax::choice::v_::name);
    choice->v.name.expression = parse_expression();

    if (!choice->v.name.expression)
        return nullptr;

    return choice.release();
}

vhdl::syntax::name* vhdl::parser::parse_function_call(name_options& options,
                                                      vhdl::syntax::name* name)
{
    consume(tk::leftpar);
    auto p = parse_actual_parameter_part();
    consume(tk::rightpar);

    auto result = std::make_unique<vhdl::syntax::name>();
    result->set_v_kind(vhdl::syntax::name::v_::fcall);
    result->v.fcall.prefix = name;
    result->v.fcall.params = p;

    if (!result->v.fcall.prefix)
        return nullptr;

    return result.release();
}

std::vector<vhdl::syntax::association_element*>
vhdl::parser::parse_actual_parameter_part()
{
    return parse_association_list();
}

vhdl::syntax::name* vhdl::parser::parse_qualified_expression(
    vhdl::parser::name_options& options, vhdl::syntax::name* name)
{
    auto result = std::make_unique<vhdl::syntax::name>();
    result->set_v_kind(vhdl::syntax::name::v_::qualified);
    result->v.qualified.prefix = name;

    consume(tk::tick);

    const look_params comma_or_rightarrow = {
        .look = {tk::comma, tk::rightarrow},
        .stop = {tk::rightpar},
        .abort = tk::semicolon,
        .nest_in = tk::leftpar,
        .nest_out = tk::rightpar,
        .depth = 1};

    if (lexer_.look_for(comma_or_rightarrow))
    {
        // aggregate
        result->v.qualified.expression = parse_expression();
    }
    else
    {
        // just an expression
        consume(tk::leftpar);
        result->v.qualified.expression = parse_expression();
        consume(tk::rightpar);
    }

    if (!result->v.fcall.prefix || !result->v.qualified.expression)
        return nullptr;

    return result.release();
}

vhdl::syntax::expression* vhdl::parser::parse_allocator()
{
    consume(tk::kw_new);

    auto result = std::make_unique<vhdl::syntax::expression>();
    result->set_v_kind(vhdl::syntax::expression::v_::allocator);
    result->v.allocator.expr = parse_boolean_expression();

    if (!result->v.allocator.expr)
        return nullptr;

    return result.release();
}

// ----------------------------------------------------------------------------
// Sequential statements
// ----------------------------------------------------------------------------
// Implements these chapters from the VHDL LRM:
// LRM93 8.0
// ----------------------------------------------------------------------------

std::vector<vhdl::syntax::sequential_statement*>
vhdl::parser::parse_sequence_of_statements()
{
    return parse_many<vhdl::syntax::sequential_statement*>(
        ps::sequential_statements,
        [this] { return parse_sequential_statement(); });
}

vhdl::syntax::sequential_statement* vhdl::parser::parse_sequential_statement()
{
    std::optional<vhdl::token> label = parse_optional_label();

    vhdl::syntax::sequential_statement* result = nullptr;
    switch (current_token()) {
    case tk::kw_wait:
        result = parse_wait_statement(label);
        break;

    case tk::kw_assert:
        result = parse_assertion_statement(label);
        break;

    case tk::kw_report:
        result = parse_report_statement(label);
        break;

    case tk::identifier:
    case tk::stringliteral: {
        auto name = parse_name();

        if (current_token() == tk::semicolon)
        {
            result = parse_procedure_call_statement(label, name);
            break;
        }

        auto t = std::make_unique<vhdl::syntax::target>();
        t->set_v_kind(vhdl::syntax::target::v_::name);
        t->v.name.name = name;

        if (!t->v.name.name)
            t.reset(nullptr);

        if (current_token() == tk::lte)
        {
            result = parse_signal_assignment_statement(label, t.release());
            break;
        }
        else if (current_token() == tk::coloneq)
        {
            result = parse_variable_assignment_statement(label, t.release());
            break;
        }
        else
        {
            diag(Err::Expecting_seq_stmt);
            skip();
        }

        break;
    }
    case tk::leftpar: {
        auto t = std::make_unique<vhdl::syntax::target>();
        t->set_v_kind(vhdl::syntax::target::v_::expression);
        t->v.expression.aggregate = parse_expression();

        if (!t->v.expression.aggregate)
            t.reset(nullptr);

        if (current_token() == tk::lte)
        {
            result = parse_signal_assignment_statement(label, t.release());
            break;
        }
        else if (current_token() == tk::coloneq)
        {
            result = parse_variable_assignment_statement(label, t.release());
            break;
        }
        else
        {
            diag(Err::Expecting_assignment);
            skip();
        }
        break;
    }
    case tk::kw_if:
        result = parse_if_statement(label);
        break;

    case tk::kw_for:
    case tk::kw_while:
    case tk::kw_loop:
        result = parse_loop_statement(label);
        break;

    case tk::kw_case:
        result = parse_case_statement(label);
        break;

    case tk::kw_null:
        result = parse_null_statement(label);
        break;

    case tk::kw_next:
        result = parse_next_statement(label);
        break;

    case tk::kw_exit:
        result = parse_exit_statement(label);
        break;

    case tk::kw_return:
        result = parse_return_statement(label);
        break;

    default:
        assert(false && "we should never get here");
        break;
    }

    if (!result)
        return nullptr;

    // result->first__ = first__;
    // result->__last = lexer_.get_previous_location();

    return result;
}

vhdl::syntax::sequential_statement* vhdl::parser::parse_wait_statement(
    std::optional<vhdl::token> label)
{
    auto result = std::make_unique<vhdl::syntax::sequential_statement>();
    result->set_v_kind(vhdl::syntax::sequential_statement::v_::wait);

    consume(tk::kw_wait);

    switch (current_token()) {
    case tk::kw_on:
        consume(tk::kw_on);
        result->v.wait.sensitivity = parse_sensitivity_list();
        break;
    case tk::kw_until:
    case tk::kw_for:
        break;
    case tk::semicolon:
        consume(tk::semicolon);
        return result.release();
    default:
        diag(Err::Wait_on_until_for_semic);
        return result.release();
    }

    switch (current_token()) {
    case tk::kw_on:
        diag(Err::Wait_only_one_sensitivity);
        return result.release();
    case tk::kw_until:
        consume(tk::kw_until);
        result->v.wait.condition = parse_expression();
        break;
    case tk::kw_for:
        break;
    case tk::semicolon:
        consume(tk::semicolon);
        return result.release();
    default:
        diag(Err::Wait_until_for_semic);
        return result.release();
    }

    switch (current_token()) {
    case tk::kw_on:
        diag(Err::Wait_only_one_sensitivity);
        break;
    case tk::kw_until:
        diag(Err::Wait_only_one_condition);
        break;
    case tk::kw_for:
        consume(tk::kw_for);
        result->v.wait.timeout = parse_expression();
        [[fallthrough]];
    case tk::semicolon:
        consume(tk::semicolon);
        break;
    default:
        diag(Err::Wait_for_semic);
        break;
    }

    return result.release();
}

std::vector<vhdl::syntax::name*> vhdl::parser::parse_sensitivity_list()
{
    std::vector<vhdl::syntax::name*> list;
    do
    {
        if (auto n = parse_name())
        {
            list.push_back(std::move(n));
        }
        else
        {
            // Dont need to report any error because error was probably already
            // flagged out in parse name. Or should we?
            diag(Err::Wait_only_name_sensitivity);
        }
    } while (optional(tk::comma));

    return list;
}

vhdl::syntax::sequential_statement* vhdl::parser::parse_assertion_statement(
    std::optional<vhdl::token> label)
{
    auto result = std::make_unique<vhdl::syntax::sequential_statement>();
    result->set_v_kind(vhdl::syntax::sequential_statement::v_::assertion);

    result->label = label;
    std::tie(result->v.assertion.condition,
             result->v.assertion.report,
             result->v.assertion.severity) = parse_assertion();

    consume(tk::semicolon);

    if (!result->v.assertion.condition)
        return nullptr;

    return result.release();
}

std::tuple<vhdl::syntax::expression*, vhdl::syntax::expression*,
           vhdl::syntax::expression*>
vhdl::parser::parse_assertion()
{
    consume(tk::kw_assert);

    vhdl::syntax::expression* c = parse_expression();
    vhdl::syntax::expression* r = nullptr;
    vhdl::syntax::expression* s = nullptr;

    if (optional(tk::kw_report))
    {
        r = parse_expression();
    }

    if (optional(tk::kw_severity))
    {
        s = parse_expression();

        if (optional(tk::kw_report))
        {
            diag(Err::Assert_report_before_severity);
            r = parse_expression();
        }
    }

    return std::make_tuple(c, r, s);
}

vhdl::syntax::sequential_statement* vhdl::parser::parse_report_statement(
    std::optional<vhdl::token> label)
{
    auto result = std::make_unique<vhdl::syntax::sequential_statement>();
    result->set_v_kind(vhdl::syntax::sequential_statement::v_::report);
    result->label = label;

    if (version_ == vhdl87)
        diag(Err::Report_vhdl87);

    consume(tk::kw_report);

    result->v.report.report = parse_expression();

    if (optional(tk::kw_severity))
        result->v.report.severity = parse_expression();

    consume(tk::semicolon);

    if (!result->v.report.report)
        return nullptr;

    return result.release();
}

vhdl::syntax::sequential_statement*
vhdl::parser::parse_signal_assignment_statement(
    std::optional<vhdl::token> label,
    vhdl::syntax::target* t)
{
    auto result = std::make_unique<vhdl::syntax::sequential_statement>();
    result->set_v_kind(vhdl::syntax::sequential_statement::v_::sig_assign);
    result->label = label;
    result->v.sig_assign.target = t;

    consume(tk::lte);

    std::tie(result->v.sig_assign.is_transport,
             result->v.sig_assign.is_reject,
             result->v.sig_assign.time,
             result->v.sig_assign.is_inertial) = parse_delay_mechanism();

    result->v.sig_assign.waveforms = parse_waveform();

    consume(tk::semicolon);

    if (!result->v.sig_assign.target || result->v.sig_assign.waveforms.size() == 0)
        return nullptr;

    return result.release();
}

std::tuple<bool, bool, vhdl::syntax::expression*, bool>
vhdl::parser::parse_delay_mechanism()
{
    vhdl::syntax::expression* time = nullptr;
    auto is_reject = false;

    switch (current_token()) {
    case tk::kw_transport:
        consume(tk::kw_transport);
        return std::make_tuple(true, false, nullptr, false);

    case tk::kw_reject:
        if (version_ == vhdl87)
            diag(Err::Reject_vhdl87);

        consume(tk::kw_reject);
        time = parse_expression();
        is_reject = true;
        [[fallthrough]];

    case tk::kw_inertial:
        if (version_ == vhdl87 && current_token() == tk::kw_inertial)
            diag(Err::Inertial_vhdl87);

        consume(tk::kw_inertial);
        return std::make_tuple(false, is_reject, time, true);

    default:
        return std::make_tuple(false, false, nullptr, false);
    }
}

vhdl::syntax::target* vhdl::parser::parse_target(vhdl::syntax::name* name)
{
    if (name != nullptr)
    {
        auto result = std::make_unique<vhdl::syntax::target>();
        result->set_v_kind(vhdl::syntax::target::v_::name);
        result->v.name.name = name;
        return result.release();
    }

    if (current_token() == tk::leftpar)
    {
        auto result = std::make_unique<vhdl::syntax::target>();
        result->set_v_kind(vhdl::syntax::target::v_::expression);
        result->v.expression.aggregate = parse_expression();

        if (!result->v.expression.aggregate)
            return nullptr;

        return result.release();
    }

    auto result = std::make_unique<vhdl::syntax::target>();
    result->set_v_kind(vhdl::syntax::target::v_::name);
    result->v.name.name = parse_name();

    if (!result->v.name.name)
        return nullptr;

    return result.release();
}

std::vector<vhdl::syntax::waveform_element*> vhdl::parser::parse_waveform()
{
    if (current_token() == tk::kw_unaffected)
    {
        if (version_ == vhdl87)
            diag(Err::Unaffected_waveforms_vhdl87);
        skip();

        return {};
    }

    std::vector<vhdl::syntax::waveform_element*> waveforms;
    do
    {
        if (auto waveform = parse_waveform_element())
            waveforms.push_back(waveform);
    } while (optional(tk::comma));

    return waveforms;
}

vhdl::syntax::waveform_element* vhdl::parser::parse_waveform_element()
{
    auto element = std::make_unique<vhdl::syntax::waveform_element>();

    if (current_token() == tk::kw_null)
    {
        skip();
        element->is_null = true;
    }
    else
    {
        element->value = parse_expression();
    }

    if (current_token() == tk::kw_after)
    {
        skip();
        element->time = parse_expression();
    }

    return element.release();
}

vhdl::syntax::sequential_statement*
vhdl::parser::parse_variable_assignment_statement(
    std::optional<vhdl::token> label,
    vhdl::syntax::target* target)
{
    auto result = std::make_unique<vhdl::syntax::sequential_statement>();
    result->set_v_kind(vhdl::syntax::sequential_statement::v_::var_assign);
    result->label = label;
    result->v.var_assign.target = target;

    consume(tk::coloneq);

    result->v.var_assign.value = parse_expression();

    consume(tk::semicolon);

    if (!result->v.var_assign.target || !result->v.var_assign.value)
        return nullptr;

    return result.release();
}

vhdl::syntax::sequential_statement*
vhdl::parser::parse_procedure_call_statement(std::optional<vhdl::token> label,
                                   vhdl::syntax::name* name)
{
    auto result = std::make_unique<vhdl::syntax::sequential_statement>();
    result->set_v_kind(vhdl::syntax::sequential_statement::v_::pcall);
    result->label = label;
    result->v.pcall.procedure = name;

    consume(tk::semicolon);

    if (!result->v.pcall.procedure)
        return nullptr;

    return result.release();
}

vhdl::syntax::sequential_statement* vhdl::parser::parse_if_statement(
    std::optional<vhdl::token> label)
{
    auto result = std::make_unique<vhdl::syntax::sequential_statement>();
    result->set_v_kind(vhdl::syntax::sequential_statement::v_::if_stmt);
    auto tail = result.get();

    result->label = label;

    consume(tk::kw_if);

    result->v.if_stmt.condition = parse_expression();

    consume(tk::kw_then);

    result->v.if_stmt.then__ = lexer_.get_previous_location();
    result->v.if_stmt.then = parse_sequence_of_statements();

    while (current_token() == tk::kw_elsif)
    {
        tail->v.if_stmt.__end = lexer_.get_current_location();

        auto temp = std::make_unique<vhdl::syntax::sequential_statement>();
        temp->set_v_kind(vhdl::syntax::sequential_statement::v_::if_stmt);
        consume(tk::kw_elsif);

        temp->v.if_stmt.condition = parse_expression();

        consume(tk::kw_then);

        temp->v.if_stmt.then__ = lexer_.get_previous_location();
        temp->v.if_stmt.then = parse_sequence_of_statements();

        tail->v.if_stmt.otherwise = temp.release();
        tail = tail->v.if_stmt.otherwise;
    }

    if (current_token() == tk::kw_else)
    {
        tail->v.if_stmt.__end = lexer_.get_current_location();

        auto temp = std::make_unique<vhdl::syntax::sequential_statement>();
        temp->set_v_kind(vhdl::syntax::sequential_statement::v_::if_stmt);

        consume(tk::kw_else);

        temp->v.if_stmt.then__ = lexer_.get_previous_location();
        temp->v.if_stmt.then = parse_sequence_of_statements();
        tail->v.if_stmt.otherwise = temp.release();
    }

    tail->v.if_stmt.__end = eat(tk::kw_end);
    consume(tk::kw_if);

    if (label)
        parse_optional_closing_label(label.value());

    consume(tk::semicolon);

    return result.release();
}

vhdl::syntax::sequential_statement* vhdl::parser::parse_case_statement(
    std::optional<vhdl::token> label)
{
    auto result = std::make_unique<vhdl::syntax::sequential_statement>();
    result->set_v_kind(vhdl::syntax::sequential_statement::v_::case_stmt);

    consume(tk::kw_case);

    result->v.case_stmt.expression = parse_expression();

    consume(tk::kw_is);

    if (current_token() == tk::kw_end)
        diag(Err::Case_missing_alternative);

    while (current_token() == tk::kw_when)
    {
        auto alt = std::make_unique<vhdl::syntax::case_statement_alternative>();

        consume(tk::kw_when);

        alt->choices = parse_choices();

        consume(tk::rightarrow);

        alt->stmts = parse_sequence_of_statements();

        result->v.case_stmt.alternatives.push_back(alt.release());
    }

    consume(tk::kw_end);
    consume(tk::kw_case);

    if (label)
        parse_optional_closing_label(label.value());

    consume(tk::semicolon);

    if (!result->v.case_stmt.expression)
        return nullptr;

    return result.release();
}

vhdl::syntax::sequential_statement* vhdl::parser::parse_loop_statement(
    std::optional<vhdl::token> label)
{
    switch (current_token()) {
    case tk::kw_for:
        return parse_for_loop_statement(std::move(label));
    case tk::kw_while:
    case tk::kw_loop:
        return parse_while_loop_statement(std::move(label));
    default:
        return nullptr;
    }
}

std::tuple<vhdl::syntax::param_spec*, bool>
vhdl::parser::parse_parameter_specification()
{
    auto result = std::make_unique<vhdl::syntax::param_spec>();
    auto [t, v] = parse_identifier(Err::Param_spec_expecting_ident);
    result->identifier = t;

    consume(tk::kw_in);

    result->in = parse_discrete_range();
    v &= result->in != nullptr;

    return std::make_tuple(result.release(), v);
}

vhdl::syntax::sequential_statement*
vhdl::parser::parse_for_loop_statement(std::optional<vhdl::token> label)
{
    auto result = std::make_unique<vhdl::syntax::sequential_statement>();
    result->set_v_kind(vhdl::syntax::sequential_statement::v_::for_loop);
    result->label = label;

    consume(tk::kw_for);

    auto [p, v] = parse_parameter_specification();
    result->v.for_loop.param = p;

    result->v.for_loop.loop__ = eat(tk::kw_loop);

    result->v.for_loop.loop = parse_sequence_of_statements();

    result->v.for_loop.__end = eat(tk::kw_end);
    consume(tk::kw_loop);

    if (label)
        parse_optional_closing_label(label.value());

    consume(tk::semicolon);

    if (!v)
        return nullptr;

    return result.release();
}

vhdl::syntax::sequential_statement*
vhdl::parser::parse_while_loop_statement(std::optional<vhdl::token> label)
{
    auto result = std::make_unique<vhdl::syntax::sequential_statement>();
    result->set_v_kind(vhdl::syntax::sequential_statement::v_::while_loop);
    result->label = label;

    if (optional(tk::kw_while))
    {
        result->v.while_loop.condition = parse_expression();
    }

    result->v.for_loop.loop__ = eat(tk::kw_loop);

    result->v.while_loop.loop = parse_sequence_of_statements();

    result->v.for_loop.__end = eat(tk::kw_end);
    consume(tk::kw_loop);

    if (label)
        parse_optional_closing_label(label.value());

    consume(tk::semicolon);

    return result.release();
}

vhdl::syntax::sequential_statement*
vhdl::parser::parse_next_statement(std::optional<vhdl::token> label)
{
    auto result = std::make_unique<vhdl::syntax::sequential_statement>();
    result->set_v_kind(vhdl::syntax::sequential_statement::v_::next);
    result->label = label;

    result->v.next.next = lexer_.current_token();
    consume(tk::kw_next);

    if (optional(tk::kw_when))
        result->v.next.when = parse_expression();

    consume(tk::semicolon);

    return result.release();
}

vhdl::syntax::sequential_statement*
vhdl::parser::parse_exit_statement(std::optional<vhdl::token> label)
{
    auto result = std::make_unique<vhdl::syntax::sequential_statement>();
    result->set_v_kind(vhdl::syntax::sequential_statement::v_::exit);
    result->label = label;

    result->v.exit.loop = lexer_.current_token();
    consume(tk::kw_exit);

    optional(tk::identifier);

    if (optional(tk::kw_when))
        result->v.exit.when = parse_expression();

    consume(tk::semicolon);

    return result.release();
}

vhdl::syntax::sequential_statement*
vhdl::parser::parse_return_statement(std::optional<vhdl::token> label)
{
    auto result = std::make_unique<vhdl::syntax::sequential_statement>();
    result->set_v_kind(vhdl::syntax::sequential_statement::v_::return_stmt);
    result->label = label;

    consume(tk::kw_return);

    if (current_token() != tk::semicolon)
        result->v.return_stmt.expression = parse_expression();

    consume(tk::semicolon);

    return result.release();
}

vhdl::syntax::sequential_statement*
vhdl::parser::parse_null_statement(std::optional<vhdl::token> label)
{
    auto result = std::make_unique<vhdl::syntax::sequential_statement>();
    result->set_v_kind(vhdl::syntax::sequential_statement::v_::null);
    result->label = label;

    result->v.null.null = lexer_.current_token();

    consume(tk::kw_null);
    consume(tk::semicolon);

    return result.release();
}

// ----------------------------------------------------------------------------
//  Concurrent statements
// ----------------------------------------------------------------------------
// Implements these chapters from the VHDL LRM:
// LRM93 9.0
// ----------------------------------------------------------------------------

std::vector<vhdl::syntax::concurrent_statement*>
vhdl::parser::parse_concurrent_statements()
{
    return parse_many<vhdl::syntax::concurrent_statement*>(
        ps::concurrent_statements,
        [this] { return parse_concurrent_statement(); });
}

vhdl::syntax::concurrent_statement* vhdl::parser::parse_concurrent_statement()
{
    const bool has_label = peek(0) == tk::identifier && peek(1) == tk::colon;

    const tk first = peek(has_label ? 2 : 0);
    const tk secnd = peek(has_label ? 3 : 1);

    auto first__ = lexer_.get_current_location();

    const look_params that = {.look = {tk::lte},
                              .stop = {tk::semicolon},
                              .abort = {},
                              .nest_in = tk::leftpar,
                              .nest_out = tk::rightpar,
                              .depth = 0};
    const bool there_is_a_signal_assignment = lexer_.look_for(that);

    vhdl::syntax::concurrent_statement* result = nullptr;
    switch (first) {
    case tk::identifier:
        if (has_label && (secnd == tk::semicolon || secnd == tk::kw_generic ||
                          secnd == tk::kw_port))
        {
            result = parse_component_instantiation_statement();
            break;
        }

        if (there_is_a_signal_assignment)
        {
            result = parse_concurrent_signal_assignment_statement();
            break;
        }

        result = parse_concurrent_procedure_call_statement();
        break;

    case tk::kw_process:
        result = parse_process_statement();
        break;

    case tk::kw_component:
    case tk::kw_entity:
    case tk::kw_configuration:
        result = parse_component_instantiation_statement();
        break;

    case tk::kw_with:
        result = parse_concurrent_signal_assignment_statement();
        break;

    case tk::kw_assert:
        result = parse_concurrent_assertion_statement();
        break;

    case tk::kw_postponed:
        if (secnd == tk::kw_assert)
        {
            result = parse_concurrent_assertion_statement();
            break;
        }
        result = parse_process_statement();
        break;

    case tk::kw_block:
        result = parse_block_statement();
        break;

    case tk::kw_if:
    case tk::kw_for:
        result = parse_generate_statement();
        break;

    case tk::leftpar:
        result = parse_concurrent_signal_assignment_statement();
        break;

    default:
        assert(false && "we should never get here");
        break;
    }

    if (!result)
        return nullptr;

    result->first__ = first__;
    result->__last = lexer_.get_previous_location();

    return result;
}

vhdl::syntax::concurrent_statement* vhdl::parser::parse_block_statement()
{
    auto result = std::make_unique<vhdl::syntax::concurrent_statement>();
    result->set_v_kind(vhdl::syntax::concurrent_statement::v_::block);

    auto [t, v] = parse_non_optional_label(Err::Block_label);
    result->label = t;

    consume(tk::kw_block);

    if (optional(tk::leftpar))
    {
        result->v.block.guard = parse_expression();

        consume(tk::rightpar, Err::Block_guard);
    }

    if (optional(tk::kw_is))
    {
        if (version_ == vhdl87)
            diag(Err::Is_not_allowed_in_vhdl87);
    }

    if (current_token() == tk::kw_generic)
    {
        auto [generic__, gl__, gens, __gr] = parse_generic_clause();
        // result->v.block.generic__ = generic__;
        // result->v.block.gl__ = gl__;
        result->v.block.gens = gens;
        // result->v.block.__gr = __gr;

        if (current_token() == tk::kw_generic)
        {
            auto gm = parse_generic_map_aspect();
            result->v.block.gen_map = std::get<2>(gm);
            consume(tk::semicolon);
        }
    }

    if (current_token() == tk::kw_port)
    {
        auto [port__, pl__, ports, __pr] = parse_port_clause();
        // result->v.block.port__ = port__;
        // result->v.block.pl__ = pl__;
        result->v.block.ports = ports;
        // result->v.block.__pr = __pr;

        if (current_token() == tk::kw_port)
        {
            auto pm = parse_port_map_aspect();
            result->v.block.port_map = std::get<2>(pm);
            consume(tk::semicolon);
        }
    }

    result->v.block.decls = parse_block_declarative_part();

    consume(tk::kw_begin);

    result->v.block.stmts = parse_block_statement_part();

    consume(tk::kw_end);
    consume(tk::kw_block);

    if (result->label)
        parse_optional_closing_label(result->label.value());

    consume(tk::semicolon);

    if (!v)
        return nullptr;

    return result.release();
}

std::vector<vhdl::syntax::declarative_item*>
vhdl::parser::parse_block_declarative_part()
{
    return parse_many<vhdl::syntax::declarative_item*>(
        ps::declarative_part_begin,
        [this] { return parse_block_declarative_item(); });
}

std::vector<vhdl::syntax::concurrent_statement*>
vhdl::parser::parse_block_statement_part()
{
    return parse_concurrent_statements();
}

vhdl::syntax::concurrent_statement* vhdl::parser::parse_process_statement()
{
    auto result = std::make_unique<vhdl::syntax::concurrent_statement>();
    result->set_v_kind(vhdl::syntax::concurrent_statement::v_::process);

    result->label = parse_optional_label();

    result->v.process.postponed = optional(tk::kw_postponed);

    result->v.process.process__ = eat(tk::kw_process);

    if (optional(tk::leftpar))
    {
        result->v.process.sensitivity = parse_sensitivity_list();
        consume(tk::rightpar);
    }

    if (version_ == vhdl87 && current_token() == tk::kw_is)
        diag(Err::Is_not_allowed_in_vhdl87);

    optional(tk::kw_is);

    result->v.process.decls = parse_many<vhdl::syntax::declarative_item*>(
        ps::declarative_part_begin,
        [this] { return parse_process_declarative_item(); });

    result->v.process.__begin__ = eat(tk::kw_begin);

    result->v.process.stmts = parse_sequence_of_statements();

    result->v.process.__end = eat(tk::kw_end);

    if (result->v.process.postponed)
        optional(tk::kw_postponed);

    consume(tk::kw_process);

    if (result->label)
        parse_optional_closing_label(result->label.value());

    consume(tk::semicolon);

    return result.release();
}

vhdl::syntax::declarative_item* vhdl::parser::parse_process_declarative_item()
{
    return parse_declarative_item(declarative_part::process);
}

vhdl::syntax::concurrent_statement*
vhdl::parser::parse_concurrent_procedure_call_statement()
{
    auto result = std::make_unique<vhdl::syntax::concurrent_statement>();
    result->set_v_kind(vhdl::syntax::concurrent_statement::v_::pcall);

    result->label = parse_optional_label();

    result->v.pcall.postponed = optional(tk::kw_postponed);

    result->v.pcall.procedure = parse_name();

    consume(tk::semicolon);

    if (!result->v.pcall.procedure)
        return nullptr;

    return result.release();
}

vhdl::syntax::concurrent_statement*
vhdl::parser::parse_concurrent_assertion_statement()
{
    auto result = std::make_unique<vhdl::syntax::concurrent_statement>();
    result->set_v_kind(vhdl::syntax::concurrent_statement::v_::assertion);

    result->label = parse_optional_label();

    result->v.assertion.postponed = optional(tk::kw_postponed);

    std::tie(result->v.assertion.condition,
             result->v.assertion.report,
             result->v.assertion.severity) = parse_assertion();

    consume(tk::semicolon);

    if (!result->v.assertion.condition)
        return nullptr;

    return result.release();
}

vhdl::syntax::concurrent_statement*
vhdl::parser::parse_concurrent_signal_assignment_statement()
{
    auto label = parse_optional_label();

    bool postponed = optional(tk::kw_postponed);

    if (current_token() == tk::kw_with)
        return parse_concurrent_selected_signal_assignment_statement(label,
                                                                     postponed);
    else
        return parse_concurrent_conditional_signal_assignment_statement(
            label, postponed);
}

std::tuple<bool, bool, bool, vhdl::syntax::expression*, bool>
vhdl::parser::parse_option()
{
    auto guarded = optional(tk::kw_guarded);

    auto [transport, reject, time, inertial] = parse_delay_mechanism();

    return std::make_tuple(guarded, transport, reject, time, inertial);
}

vhdl::syntax::concurrent_statement*
vhdl::parser::parse_concurrent_conditional_signal_assignment_statement(
        std::optional<vhdl::token> label, bool postponed)
{
    auto result = std::make_unique<vhdl::syntax::concurrent_statement>();
    result->set_v_kind(vhdl::syntax::concurrent_statement::v_::con_assign);

    result->v.con_assign.target = parse_target();

    consume(tk::lte);

    std::tie(result->v.con_assign.is_guarded,
             result->v.con_assign.is_transport,
             result->v.con_assign.is_reject,
             result->v.con_assign.time,
             result->v.con_assign.is_inertial) = parse_option();

    parse_conditional_waveforms(result->v.con_assign.waveforms);

    consume(tk::semicolon);

    if (!result->v.con_assign.target ||
        result->v.con_assign.waveforms.size() == 0)
        return nullptr;

    return result.release();
}

void vhdl::parser::parse_conditional_waveforms(
    std::vector<vhdl::syntax::con_waveform*>& waveforms)
{
    do
    {
        if (auto waveform = parse_conditional_waveform())
            waveforms.push_back(waveform);
    } while (optional(tk::kw_else));
}

vhdl::syntax::con_waveform* vhdl::parser::parse_conditional_waveform()
{
    auto conditional = std::make_unique<vhdl::syntax::con_waveform>();
    conditional->waveforms = parse_waveform();

    if (optional(tk::kw_when))
    {
        conditional->when = parse_expression();
    }

    return conditional.release();
}

vhdl::syntax::concurrent_statement*
vhdl::parser::parse_concurrent_selected_signal_assignment_statement(
        std::optional<vhdl::token> label, bool postponed)
{
    auto result = std::make_unique<vhdl::syntax::concurrent_statement>();
    result->set_v_kind(vhdl::syntax::concurrent_statement::v_::sel_assign);
    result->label = label;
    result->v.sel_assign.postponed = postponed;

    consume(tk::kw_with);

    result->v.sel_assign.value = parse_expression();

    consume(tk::kw_select);

    result->v.sel_assign.target = parse_target();

    consume(tk::lte);

    std::tie(result->v.sel_assign.is_guarded,
             result->v.sel_assign.is_transport,
             result->v.sel_assign.is_reject,
             result->v.sel_assign.time,
             result->v.sel_assign.is_inertial) = parse_option();

    parse_selected_waveforms(result->v.sel_assign.waveforms);

    consume(tk::semicolon);

    if (!result->v.sel_assign.target || !result->v.sel_assign.value ||
        result->v.sel_assign.waveforms.size() == 0)
        return nullptr;

    return result.release();
}

void vhdl::parser::parse_selected_waveforms(
    std::vector<vhdl::syntax::sel_waveform*>& waveforms)
{
    do
    {
        auto waveform = parse_selected_waveform();

        if (waveform)
            waveforms.push_back(waveform);
    } while (optional(tk::comma));
}

vhdl::syntax::sel_waveform* vhdl::parser::parse_selected_waveform()
{
    auto waveform = std::make_unique<vhdl::syntax::sel_waveform>();

    waveform->waveforms = parse_waveform();
    consume(tk::kw_when);
    waveform->choices = parse_choices();

    if (waveform->waveforms.size() == 0 || waveform->choices.size() == 0)
        return nullptr;

    return waveform.release();
}

vhdl::syntax::concurrent_statement*
vhdl::parser::parse_component_instantiation_statement()
{
    auto result = std::make_unique<vhdl::syntax::concurrent_statement>();
    result->set_v_kind(vhdl::syntax::concurrent_statement::v_::inst);

    auto [t, v] = parse_non_optional_label(Err::Inst_label);
    result->label = t;

    result->v.inst.inst = parse_instantiated_unit();

    if (current_token() == tk::kw_generic)
    {
        auto [generic__, gl__, gens, __gr] = parse_generic_map_aspect();
        result->v.inst.generic__ = generic__;
        result->v.inst.gl__ = gl__;
        result->v.inst.gens = gens;
        result->v.inst.__gr = __gr;
    }

    if (current_token() == tk::kw_port)
    {
        auto [port__, pl__, ports, __pr] = parse_port_map_aspect();
        result->v.inst.port__ = port__;
        result->v.inst.pl__ = pl__;
        result->v.inst.ports = ports;
        result->v.inst.__pr = __pr;
    }

    consume(tk::semicolon);

    if (!result->v.inst.inst)
        return nullptr;

    return result.release();
}

vhdl::syntax::instantiated_unit* vhdl::parser::parse_instantiated_unit()
{
    using is = vhdl::syntax::instantiated_unit::v_;

    auto k = is::none;
    switch (current_token()) {
    case tk::kw_entity:
        consume(tk::kw_entity);
        k = is::entity;
        break;
    case tk::kw_configuration:
        consume(tk::kw_configuration);
        k = is::config;
        break;
    case tk::kw_component:
        consume(tk::kw_component);
        [[fallthrough]];
    default:
        k = is::component;
        break;
    }

    // only allow selected identifiers
    vhdl::parser::name_options options;
    options.allow_qifts = false;
    options.allow_complex_names = true;
    options.allow_signature = false;
    auto n = parse_name(options);

    auto result = std::make_unique<vhdl::syntax::instantiated_unit>();
    result->set_v_kind(k);
    switch (k) {
    case is::entity:
        result->v.entity.unit = n;
        if (optional(tk::leftpar))
        {
            result->v.entity.architecture = lexer_.current_token();
            consume(tk::identifier);

            consume(tk::rightpar);
        }
        break;
    case is::config:
        result->v.config.unit = n;
        break;
    case is::component:
        result->v.component.unit = n;
        break;
    default:
        break;
    }

    if (!n)
        return nullptr;

    return result.release();
}

vhdl::syntax::concurrent_statement* vhdl::parser::parse_generate_statement()
{
    if (peek(2) == tk::kw_if)
        return parse_generate_if_statement();
    else if (peek(2) == tk::kw_for)
        return parse_generate_for_statement();
    else
        return nullptr;
}

vhdl::syntax::concurrent_statement* vhdl::parser::parse_generate_if_statement()
{
    auto result = std::make_unique<vhdl::syntax::concurrent_statement>();
    result->set_v_kind(vhdl::syntax::concurrent_statement::v_::if_gen);

    auto [t, v] = parse_non_optional_label(Err::Generate_label);
    result->label = t;

    consume(tk::kw_if);
    result->v.if_gen.condition = parse_expression();

    result->v.if_gen.generate__ = eat(tk::kw_generate);

    switch (current_token()) {
    case tk::kw_function:
    case tk::kw_procedural:
    case tk::kw_type:
    case tk::kw_subtype:
    case tk::kw_constant:
    case tk::kw_signal:
    case tk::kw_variable:
    case tk::kw_file:
    case tk::kw_alias:
    case tk::kw_component:
    case tk::kw_attribute:
    case tk::kw_configuration:
    case tk::kw_disconnect:
    case tk::kw_use:
    case tk::kw_group:
        result->v.if_gen.decls = parse_many<vhdl::syntax::declarative_item*>(
            ps::declarative_part_begin,
            [this] { return parse_block_declarative_item(); });
        [[fallthrough]];
    case tk::kw_begin:
        consume(tk::kw_begin);
        break;
    default:
        break;
    }

    if (current_token() != tk::kw_end)
        result->v.if_gen.stmts = parse_concurrent_statements();

    result->v.if_gen.__end = eat(tk::kw_end);
    consume(tk::kw_generate);

    if (result->label)
        parse_optional_closing_label(result->label.value());

    consume(tk::semicolon);

    if (!v || !result->v.if_gen.condition)
        return nullptr;

    return result.release();
}

vhdl::syntax::concurrent_statement* vhdl::parser::parse_generate_for_statement()
{
    auto result = std::make_unique<vhdl::syntax::concurrent_statement>();
    result->set_v_kind(vhdl::syntax::concurrent_statement::v_::for_gen);

    auto [t, v] = parse_non_optional_label(Err::Generate_label);
    result->label = t;

    consume(tk::kw_for);
    auto [p, v2] = parse_parameter_specification();
    result->v.for_gen.param = p;

    result->v.for_gen.generate__ = eat(tk::kw_generate);

    switch (current_token()) {
    case tk::kw_function:
    case tk::kw_procedural:
    case tk::kw_type:
    case tk::kw_subtype:
    case tk::kw_constant:
    case tk::kw_signal:
    case tk::kw_variable:
    case tk::kw_file:
    case tk::kw_alias:
    case tk::kw_component:
    case tk::kw_attribute:
    case tk::kw_configuration:
    case tk::kw_disconnect:
    case tk::kw_use:
    case tk::kw_group:
        result->v.for_gen.decls = parse_many<vhdl::syntax::declarative_item*>(
            ps::declarative_part_begin,
            [this] { return parse_block_declarative_item(); });
        [[fallthrough]];
    case tk::kw_begin:
        consume(tk::kw_begin);
        break;
    default:
        break;
    }

    if (current_token() != tk::kw_end)
        result->v.for_gen.stmts = parse_concurrent_statements();

    result->v.for_gen.__end = eat(tk::kw_end);
    consume(tk::kw_generate);

    if (result->label)
        parse_optional_closing_label(result->label.value());

    consume(tk::semicolon);

    if (!v || !v2)
        return nullptr;

    return result.release();
}

// ----------------------------------------------------------------------------
//  Scope and visibility
// ----------------------------------------------------------------------------
// Implements these chapters from the VHDL LRM:
// LRM93 10.0
// ----------------------------------------------------------------------------

vhdl::syntax::context_item* vhdl::parser::parse_use_clause()
{
    auto result = std::make_unique<vhdl::syntax::context_item>();
    result->set_v_kind(vhdl::syntax::context_item::v_::use_clause);

    // skip the keyword 'use'
    consume(tk::kw_use);

    while (current_token() != tk::eof)
    {
        if (current_token() != tk::identifier)
        {
            diag(Err::Expected_use_name);
            break;
        }

        vhdl::parser::name_options options;
        options.allow_qifts = false;
        if (auto name = parse_name(options))
            result->v.use_clause.names.push_back(name);

        if (current_token() == tk::semicolon)
            break;

        if (current_token() != tk::comma)
            break;

        consume(tk::comma);

        if (current_token() == tk::semicolon)
        {
            diag(Err::Unexpected_comma, lexer_.get_previous_location());
            break;
        }
    }

    if (current_token() != tk::semicolon)
        resync_to_next_semicolon(false);

    consume(tk::semicolon);

    if (result->v.use_clause.names.size() == 0)
    {
        return nullptr;
    }

    return result.release();
}

// ----------------------------------------------------------------------------
//  Design units and their analysis
// ----------------------------------------------------------------------------
// Implements these chapters from the VHDL LRM:
// LRM93 11.0
// ----------------------------------------------------------------------------

bool vhdl::parser::parse_design_file()
{
    file_->owns_units = true;

    if (current_token() == tk::eof)
    {
        diag(Err::Empty_design_file);
        return true;
    }

    file_->units = parse_many<vhdl::syntax::design_unit*>(
        ps::design_unit_in_design_file, [this] { return parse_design_unit(); });

    for (auto it : file_->units)
        it->file = file_;

    return true;
}

vhdl::syntax::design_unit* vhdl::parser::parse_design_unit()
{
    auto contexts = parse_context_clause();

    vhdl::syntax::design_unit* unit = nullptr;
    switch (current_token()) {
    case tk::kw_entity:
        unit = parse_entity_declaration();
        break;
    case tk::kw_architecture:
        unit = parse_architecture_body();
        break;
    case tk::kw_package:
        if (peek() == tk::kw_body)
            unit = parse_package_body();
        else
            unit = parse_package_declaration();
        break;
    case tk::kw_configuration:
        unit = parse_configuration_declaration();
        break;
    default:
        assert(false && "we should never get here");
        break;
    }

    if (!unit)
        return nullptr;

    unit->contexts = contexts;
    return unit;
}

vhdl::syntax::context_item* vhdl::parser::parse_library_clause()
{
    auto result = std::make_unique<vhdl::syntax::context_item>();
    result->set_v_kind(vhdl::syntax::context_item::v_::library_clause);

    // skip the library keyword
    consume(tk::kw_library);

    while (current_token() != tk::eof)
    {
        auto [t, v] = parse_identifier(Err::Expected_library_name);
        result->v.library_clause.names.push_back(t);

        if (current_token() == tk::semicolon)
            break;

        if (current_token() != tk::comma)
            break;

        consume(tk::comma);

        if (current_token() == tk::semicolon)
        {
            diag(Err::Unexpected_comma, lexer_.get_previous_location());
            break;
        }
    }

    if (current_token() != tk::semicolon)
        resync_to_next_semicolon(false);

    consume(tk::semicolon);

    if (result->v.library_clause.names.size() == 0)
    {
        return nullptr;
    }

    return result.release();
}

std::vector<vhdl::syntax::context_item*> vhdl::parser::parse_context_clause()
{
    std::vector<vhdl::syntax::context_item*> result;

    vhdl::syntax::context_item* c = nullptr;
    while (current_token() != tk::eof)
    {
        switch (current_token()) {
        case tk::kw_library:
            c = parse_library_clause();
            break;
        case tk::kw_use:
            c = parse_use_clause();
            break;

        case tk::kw_entity:
        case tk::kw_architecture:
        case tk::kw_package:
        case tk::kw_configuration:
            return result;

        default:
            skip();
            c = nullptr;
            break;
        }

        if (c != nullptr)
            result.push_back(c);
    }

    return result;
}

// ----------------------------------------------------------------------------
//  Lexical elements
// ----------------------------------------------------------------------------
// LRM93 13.0

std::tuple<vhdl::token, bool> vhdl::parser::parse_identifier(
    std::string_view message)
{
    auto token = lexer_.current_token();
    if (token != tk::identifier)
    {
        diag(message);
        return std::make_tuple(token, false);
    }

    skip();
    return std::make_tuple(token, true);
}

std::optional<vhdl::token> vhdl::parser::parse_optional_label()
{
    if (peek(0) != tk::identifier || peek() != tk::colon)
        return std::nullopt;

    auto result = lexer_.current_token();
    skip();
    consume(tk::colon);

    return result;
}

std::tuple<vhdl::token, bool> vhdl::parser::parse_non_optional_label(
    std::string_view message)
{
    auto token = lexer_.current_token();
    if (token != tk::identifier)
    {
        diag(message);
        optional(tk::colon);
        return std::make_tuple(token, false);
    }

    skip();
    consume(tk::colon);
    return std::make_tuple(token, true);
}

std::tuple<vhdl::token, bool> vhdl::parser::parse_designator(
    std::string_view message)
{
    auto token = lexer_.current_token();
    if (token != tk::identifier && token != tk::stringliteral)
    {
        diag(message);
        return std::make_tuple(token, false);
    }

    skip();
    return std::make_tuple(token, true);
}

std::tuple<vhdl::token, bool> vhdl::parser::parse_alias_designator()
{
    auto token = lexer_.current_token();
    switch (token.kind) {
    case tk::identifier:
    case tk::character:
    case tk::stringliteral:
        break;
    default:
        diag(Err::Expected_alias_designator);
        return std::make_tuple(token, false);
    }

    skip();
    return std::make_tuple(token, true);
}

void vhdl::parser::parse_optional_closing_label(vhdl::token name)
{
    if (current_token() == tk::identifier)
    {
        if (version_ < vhdl87)
            diag(Err::End_name_not_allowed_vhdl87);

        if (lexer_.current_token().value != name.value &&
            name.kind != tk::identifier)
            diag(Err::End_name_misspelling) << name.value;

        skip();
    }
}

void vhdl::parser::parse_optional_closing_designator(
    vhdl::syntax::subprogram* s)
{
    if (current_token() == tk::identifier ||
        current_token() == tk::stringliteral)
    {
        if (s != nullptr && lexer_.current_token().value != s->designator.value &&
            s->designator.kind != current_token())
            diag(Err::End_name_misspelling) << s->designator.value;

        skip();
    }
}

void vhdl::parser::skip()
{
    temporary_count_that_prevents_forever_loops = 0;
    lexer_.scan();
}

vhdl::token::kind_t vhdl::parser::peek(unsigned nth)
{
    return lexer_.peek(nth).kind;
}

vhdl::token::kind_t vhdl::parser::current_token()
{
    if (temporary_count_that_prevents_forever_loops++ >= 100)
        throw std::string{""};

    return lexer_.current_token().kind;
}

bool vhdl::parser::expecting(token::kind_t token)
{
    static constexpr std::string_view message = "Expected '{}'";
    return expecting(token, message);
}

bool vhdl::parser::expecting(token::kind_t token, std::string_view message)
{
    if (lexer_.current_token() == token)
        return true;

    diag(message) << vhdl::get_token_string_view(token);

    return false;
}

common::location vhdl::parser::eat(token::kind_t token)
{
    temporary_count_that_prevents_forever_loops = 0;
    bool found = expecting(token);
    if (found)
    {
        lexer_.scan();
        return lexer_.get_previous_location();
    }

    return lexer_.get_current_location();
}

void vhdl::parser::consume(token::kind_t token)
{
    temporary_count_that_prevents_forever_loops = 0;
    bool found = expecting(token);
    if (found)
        lexer_.scan();
}

void vhdl::parser::consume(token::kind_t token, std::string_view message)
{
    temporary_count_that_prevents_forever_loops = 0;
    bool found = expecting(token, message);
    if (found)
        lexer_.scan();
}

bool vhdl::parser::optional(token::kind_t token)
{
    if (lexer_.current_token() == token)
    {
        temporary_count_that_prevents_forever_loops = 0;
        lexer_.scan();
        return true;
    }

    return false;
}

common::diagnostic& vhdl::parser::diag(const std::string_view msg)
{
    auto loc = lexer_.get_current_location();
    return diag(msg, loc);
}

common::diagnostic& vhdl::parser::diag(const std::string_view msg,
                                       common::location loc)
{
    diagnostics.emplace_back(msg, loc);
    return diagnostics.back();
}

void vhdl::parser::resync_to_next_semicolon(bool consume_the_semicolon)
{
    scan:

    lexer_.scan();
    switch (lexer_.current_token().kind) {
    case tk::semicolon:
        if (consume_the_semicolon)
            consume(tk::semicolon);
        [[fallthrough]];
    case tk::eof:
        return;
    default:
        goto scan;
    }
}

void vhdl::parser::resync_to_end_of_rightpar(unsigned nests)
{
    if (nests == 0 && (current_token() != tk::leftpar))
        return; // expected (

    check_this_token:

    switch (lexer_.current_token().kind) {
    case tk::eof:
        return;
    case tk::leftpar:
        nests++;
        break;
    case tk::rightpar:
        nests--;
        [[fallthrough]];
    case tk::kw_end:
    case tk::kw_port:
    case tk::kw_is:
    case tk::kw_begin:
    case tk::kw_signal:
    case tk::kw_variable:
    case tk::kw_constant:
    case tk::kw_file:
    case tk::kw_function:
    case tk::kw_type:
    case tk::kw_package:
    case tk::semicolon:
        if (nests == 0)
        {
            skip();
            return;
        }
        break;
    default:
        break;
    }

    skip();

    goto check_this_token;
}

bool vhdl::parser::is_begin_of_element_in_state(tk token,
                                                vhdl::parser::state state)
{
    if (token == tk::eof)
        return false;

    switch (state) {
    case vhdl::parser::state::design_unit_in_design_file:
        return is_begin_element_of_design_unit_in_design_file(token);
    case vhdl::parser::state::interface_lists:
        return is_begin_element_of_interface_lists(token);
    case vhdl::parser::state::declarative_part_begin_end:
        return is_begin_element_of_declarative_part_begin_end(token);
    case vhdl::parser::state::declarative_part_begin:
        return is_begin_element_of_declarative_part_begin(token);
    case vhdl::parser::state::declarative_part_end:
        return is_begin_element_of_declarative_part_end(token);
    case vhdl::parser::state::declarative_part_for:
        return is_begin_element_of_declarative_part_for(token);
    case vhdl::parser::state::entity_statement_part:
        return is_begin_element_of_entity_statement_part(token);
    case vhdl::parser::state::concurrent_statements:
        return is_begin_element_of_concurrent_statements(token);
    case vhdl::parser::state::sequential_statements:
        return is_begin_element_of_sequential_statements(token);
    default:
        break;
    }
    return false;
}

bool vhdl::parser::is_end_of_state(tk token, vhdl::parser::state state)
{
    if (token == tk::eof)
        return true;

    switch (state) {
    case vhdl::parser::state::design_unit_in_design_file:
        return is_end_of_design_unit_in_design_file(token);
    case vhdl::parser::state::interface_lists:
        return is_end_of_interface_lists(token);
    case vhdl::parser::state::declarative_part_begin_end:
        return is_end_of_declarative_part_begin_end(token);
    case vhdl::parser::state::declarative_part_begin:
        return is_end_of_declarative_part_begin(token);
    case vhdl::parser::state::declarative_part_end:
        return is_end_of_declarative_part_end(token);
    case vhdl::parser::state::declarative_part_for:
        return is_end_of_declarative_part_for(token);
    case vhdl::parser::state::entity_statement_part:
        return is_end_of_entity_statement_part(token);
    case vhdl::parser::state::concurrent_statements:
        return is_end_of_concurrent_statements(token);
    case vhdl::parser::state::sequential_statements:
        return is_end_of_sequential_statements(token);
    default:
        break;
    }
    return false;
}

bool vhdl::parser::is_valid_in_parsing_states(tk token)
{
    for (auto& it : array_of_states)
        if (parsing_states.test(static_cast<size_t>(it)))
            if (is_begin_of_element_in_state(token, it) ||
                is_end_of_state(token, it))
                return true;

    return false;
}

bool vhdl::parser::is_begin_element_of_design_unit_in_design_file(tk token)
{
    switch (token) {
    case tk::kw_entity:
    case tk::kw_architecture:
    case tk::kw_package:
    case tk::kw_configuration:
    case tk::kw_library:
    case tk::kw_use:
        return true;
    default:
        break;
    }
    return false;
}

bool vhdl::parser::is_begin_element_of_interface_lists(tk token)
{
    switch (token) {
    case tk::kw_constant:
    case tk::kw_signal:
    case tk::kw_variable:
    case tk::kw_file:
    case tk::identifier:
    case tk::kw_package:
    case tk::kw_type:
    case tk::kw_procedure:
    case tk::kw_pure:
    case tk::kw_impure:
    case tk::kw_function:
        return true;
    default:
        break;
    }
    return false;
}

bool vhdl::parser::is_begin_element_of_declarative_part_begin_end(tk token)
{
    switch (token) {
    case tk::kw_type:
    case tk::kw_subtype:
    case tk::kw_signal:
    case tk::kw_constant:
    case tk::kw_variable:
    case tk::kw_shared:
    case tk::kw_file:
    case tk::kw_pure:
    case tk::kw_impure:
    case tk::kw_function:
    case tk::kw_procedure:
    case tk::kw_alias:
    case tk::kw_component:
    case tk::kw_for:
    case tk::kw_attribute:
    case tk::kw_use:
    case tk::kw_group:
    case tk::kw_package:
    case tk::identifier:
        return true;
    default:
        break;
    }
    return false;
}

bool vhdl::parser::is_begin_element_of_declarative_part_begin(tk token)
{
    return is_begin_element_of_declarative_part_begin_end(token);
}

bool vhdl::parser::is_begin_element_of_declarative_part_end(tk token)
{
    return is_begin_element_of_declarative_part_begin_end(token);
}

bool vhdl::parser::is_begin_element_of_declarative_part_for(tk token)
{
    return is_begin_element_of_declarative_part_begin_end(token);
}

bool vhdl::parser::is_begin_element_of_entity_statement_part(tk token)
{
    switch (token) {
    case tk::identifier:
    case tk::kw_process:
    case tk::kw_assert:
    case tk::kw_postponed:
        return true;
    default:
        break;
    }
    return false;
}

bool vhdl::parser::is_begin_element_of_concurrent_statements(tk token)
{
    switch (token) {
    case tk::identifier:
    case tk::kw_process:
    case tk::kw_component:
    case tk::kw_entity:
    case tk::kw_configuration:
    case tk::kw_with:
    case tk::kw_assert:
    case tk::kw_postponed:
    case tk::kw_block:
    case tk::kw_if:
    case tk::kw_for:
    case tk::leftpar:
        return true;
    default:
        break;
    }
    return false;
}

bool vhdl::parser::is_begin_element_of_sequential_statements(tk token)
{
    switch (token) {
    case tk::kw_wait:
    case tk::kw_assert:
    case tk::kw_report:
    case tk::identifier:
    case tk::stringliteral:
    case tk::leftpar:
    case tk::kw_if:
    case tk::kw_for:
    case tk::kw_while:
    case tk::kw_loop:
    case tk::kw_case:
    case tk::kw_null:
    case tk::kw_next:
    case tk::kw_exit:
    case tk::kw_return:
        return true;
    default:
        break;
    }
    return false;
}

bool vhdl::parser::is_end_of_design_unit_in_design_file(tk token)
{
    return false;
}

bool vhdl::parser::is_end_of_interface_lists(tk token)
{
    switch (token) {
    case tk::rightpar:
        return true;
    default:
        break;
    }
    return false;
}

bool vhdl::parser::is_end_of_declarative_part_begin_end(tk token)
{
    switch (token) {
    case tk::kw_begin:
    case tk::kw_end:
        return true;
    default:
        break;
    }
    return false;
}

bool vhdl::parser::is_end_of_declarative_part_begin(tk token)
{
    switch (token) {
    case tk::kw_begin:
        return true;
    default:
        break;
    }
    return false;
}

bool vhdl::parser::is_end_of_declarative_part_end(tk token)
{
    switch (token) {
    case tk::kw_end:
        return true;
    default:
        break;
    }
    return false;
}

bool vhdl::parser::is_end_of_declarative_part_for(tk token)
{
    switch (token) {
    case tk::kw_for:
        return true;
    default:
        break;
    }
    return false;
}

bool vhdl::parser::is_end_of_entity_statement_part(tk token)
{
    return is_end_of_concurrent_statements(token);
}

bool vhdl::parser::is_end_of_concurrent_statements(tk token)
{
    switch (token) {
    case tk::kw_end:
        return true;
    default:
        break;
    }
    return false;
}

bool vhdl::parser::is_end_of_sequential_statements(tk token)
{
    switch (current_token()) {
    case tk::kw_end:
    case tk::kw_else:
    case tk::kw_elsif:
    case tk::kw_when:
        return true;
    default:
        break;
    }
    return false;
}
