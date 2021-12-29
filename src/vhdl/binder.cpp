#include "binder.h"

vhdl::semantic::binder::binder(vhdl::ast* ast,
                               std::shared_ptr<vhdl::node::library_unit> unit)
    : ast(ast), unit(unit)
{
    //
}

vhdl::semantic::binder::~binder()
{
    //
}

std::tuple<bool, vhdl::node::declarative_region*,
           std::vector<common::diagnostic>>
vhdl::semantic::binder::operator()()
{
    open_declarative_region();

    auto ok = true;
    switch (unit->syntax->v_kind) {
    case vhdl::syntax::design_unit::v_::entity:
        ok &= bind_entity_declaration(unit->syntax);
        break;
    case vhdl::syntax::design_unit::v_::architecture:
        ok &= bind_architecture_body(unit->syntax);
        break;
    case vhdl::syntax::design_unit::v_::configuration:
        ok &= bind_configuration_declaration(unit->syntax);
        break;
    case vhdl::syntax::design_unit::v_::package:
        ok &= bind_package_declaration(unit->syntax);
        break;
    case vhdl::syntax::design_unit::v_::package_body:
        ok &= bind_package_body(unit->syntax);
        break;
    default:
        break;
    }

    // close_declarative_region();

    return std::make_tuple(ok, current_region, diagnostics);
}

bool vhdl::semantic::binder::bind_entity_declaration(
    vhdl::syntax::design_unit* u)
{
    auto& entity = u->v.entity;

    auto ok = add_named_entity(new vhdl::node::entity(u));

    ok &= bind_context_clause(u->contexts);

    open_declarative_region(vhdl::node::declarative_region::v_::entity, u);

    for (auto decl : entity.gens)
    {
        ok &= bind_declarative_item(decl);
    }

    for (auto decl : entity.ports)
    {
        ok &= bind_declarative_item(decl);
    }

    close_declarative_region();

    return ok;
}

bool vhdl::semantic::binder::bind_architecture_body(
    vhdl::syntax::design_unit* u)
{
    auto& architecture = u->v.architecture;

    auto ok = add_named_entity(new vhdl::node::architecture(u));

    ok &= bind_context_clause(u->contexts);

    open_declarative_region(vhdl::node::declarative_region::v_::architecture,
                            u);

    auto [ok2, ptr2] = resolve_denotes_entity(architecture.entity);
    if (ok2)
        current_region->extends = ptr2;

    for (auto decl : architecture.decls)
    {
        ok &= bind_declarative_item(decl);
    }

    for (auto stmt : architecture.stmts)
    {
        ok &= bind_concurrent_statement(stmt);
    }

    close_declarative_region();

    return ok;
}

bool vhdl::semantic::binder::bind_configuration_declaration(
    vhdl::syntax::design_unit* u)
{
    auto& configuration = u->v.configuration;

    auto ok = add_named_entity(new vhdl::node::configuration(u));

    ok &= bind_context_clause(u->contexts);

    open_declarative_region(vhdl::node::declarative_region::v_::configuration,
                            u);

    close_declarative_region();

    return ok;
}

bool vhdl::semantic::binder::bind_package_declaration(
    vhdl::syntax::design_unit* u)
{
    auto& package = u->v.package;

    auto ok = add_named_entity(new vhdl::node::package(u));

    ok &= bind_context_clause(u->contexts);

    open_declarative_region(vhdl::node::declarative_region::v_::package, u);

    for (auto decl : package.decls)
    {
        ok &= bind_declarative_item(decl);
    }

    close_declarative_region();

    return ok;
}

bool vhdl::semantic::binder::bind_package_body(vhdl::syntax::design_unit* u)
{
    auto& package = u->v.package;

    auto ok = add_named_entity(new vhdl::node::package_body(u));

    ok &= bind_context_clause(u->contexts);

    open_declarative_region(vhdl::node::declarative_region::v_::package_body,
                            u);

    for (auto decl : package.decls)
    {
        ok &= bind_declarative_item(decl);
    }

    close_declarative_region();

    return ok;
}

std::tuple<bool, vhdl::node::type*> vhdl::semantic::binder::
    bind_type_definition(vhdl::syntax::type_definition* d)
{
    switch (d->v_kind) {
    case vhdl::syntax::type_definition::v_::enumeration:
        return bind_enumeration_type_definition(d);

    case vhdl::syntax::type_definition::v_::integer:
        return bind_integer_type_definition(d);

    case vhdl::syntax::type_definition::v_::floating:
        return bind_floating_type_definition(d);

    case vhdl::syntax::type_definition::v_::physical:
        return bind_physical_type_definition(d);

    case vhdl::syntax::type_definition::v_::c_array:
        return bind_c_array_type_definition(d);

    case vhdl::syntax::type_definition::v_::u_array:
        return bind_u_array_type_definition(d);

    case vhdl::syntax::type_definition::v_::record:
        return bind_record_type_definition(d);

    case vhdl::syntax::type_definition::v_::access:
        return bind_access_type_definition(d);

    case vhdl::syntax::type_definition::v_::file:
        return bind_file_type_definition(d);

    default:
        break;
    }
    return std::make_tuple(false, nullptr);
}

std::tuple<bool, vhdl::node::type*> vhdl::semantic::binder::
    bind_enumeration_type_definition(vhdl::syntax::type_definition* d)
{
    auto ok = true;
    auto& type = d->v.enumeration;

    auto i = 0;
    for (auto it : type.literals)
    {
        ok &= add_named_entity(new vhdl::node::literal(d, i++));
    }
    return std::make_tuple(true, nullptr);
}

std::tuple<bool, vhdl::node::type*> vhdl::semantic::binder::
    bind_integer_type_definition(vhdl::syntax::type_definition* d)
{
    return std::make_tuple(true, nullptr);
}

std::tuple<bool, vhdl::node::type*> vhdl::semantic::binder::
    bind_floating_type_definition(vhdl::syntax::type_definition* d)
{
    return std::make_tuple(true, nullptr);
}

std::tuple<bool, vhdl::node::type*> vhdl::semantic::binder::
    bind_physical_type_definition(vhdl::syntax::type_definition* d)
{
    return std::make_tuple(true, nullptr);
}

std::tuple<bool, vhdl::node::type*> vhdl::semantic::binder::
    bind_c_array_type_definition(vhdl::syntax::type_definition* d)
{
    auto& type = d->v.c_array;
    auto [ok, t] = bind_subtype(type.element);

    for (auto it : type.indices)
    {
        ok &= bind_range(it);
    }

    return std::make_tuple(ok, nullptr);
}

std::tuple<bool, vhdl::node::type*> vhdl::semantic::binder::
    bind_u_array_type_definition(vhdl::syntax::type_definition* d)
{
    auto& type = d->v.u_array;
    auto [ok, t] = bind_subtype(type.element);

    for (auto it : type.index_subtypes)
    {
        ok &= bind_index_subtype(it);
    }

    return std::make_tuple(ok, nullptr);
}

std::tuple<bool, vhdl::node::type*> vhdl::semantic::binder::
    bind_record_type_definition(vhdl::syntax::type_definition* d)
{
    auto t = std::make_unique<vhdl::node::record>();
    t->syntax = d;

    open_declarative_region(vhdl::node::declarative_region::v_::record, d);

    auto ok = true;
    auto& type = d->v.record;
    for (auto it : type.elements)
    {
        for (auto i = 0; i < it->identifier.size(); i++)
        {
            auto [kk, ta] = bind_subtype(it->type);
            if (!kk)
                break;
            auto el = std::make_unique<vhdl::node::element>(it, i);
            el->type = ta;
            ok &= add_named_entity(el.release());
        }
    }

    close_declarative_region();

    return std::make_tuple(ok, t.release());
}

std::tuple<bool, vhdl::node::type*> vhdl::semantic::binder::
    bind_access_type_definition(vhdl::syntax::type_definition* d)
{
    return std::make_tuple(true, nullptr);
}

std::tuple<bool, vhdl::node::type*> vhdl::semantic::binder::
    bind_file_type_definition(vhdl::syntax::type_definition* d)
{
    return std::make_tuple(true, nullptr);
}

bool vhdl::semantic::binder::bind_range(vhdl::syntax::range* r)
{
    auto ok = true;
    switch (r->v_kind) {
    case vhdl::syntax::range::v_::downto:
        ok &= resolve_expression(r->v.downto.v1);
        ok &= resolve_expression(r->v.downto.v2);
        break;
    case vhdl::syntax::range::v_::upto:
        ok &= resolve_expression(r->v.upto.v1);
        ok &= resolve_expression(r->v.upto.v2);
        break;
    case vhdl::syntax::range::v_::range:
        break;

    default:
        return false;
    }
    return true;
}

bool vhdl::semantic::binder::bind_discrete_range(vhdl::syntax::range* r)
{
    switch (r->v_kind) {
    case vhdl::syntax::range::v_::subtype:
        return resolve(r->v.range.attribute);

    case vhdl::syntax::range::v_::downto:
    case vhdl::syntax::range::v_::upto:
    case vhdl::syntax::range::v_::range:
        return bind_range(r);

    default:
        return false;
    }
    return true;
}

bool vhdl::semantic::binder::bind_declarative_item(
    vhdl::syntax::declarative_item* d)
{
    switch (d->v_kind) {
    case vhdl::syntax::declarative_item::v_::type:
        return bind_type_declaration(d);

    case vhdl::syntax::declarative_item::v_::subtype:
        return bind_subtype_declaration(d);

    case vhdl::syntax::declarative_item::v_::object:
        switch (d->v.object.v->v_kind) {
        case vhdl::syntax::object::v_::constant:
            return bind_constant_declaration(d);
        case vhdl::syntax::object::v_::signal:
            return bind_signal_declaration(d);
        case vhdl::syntax::object::v_::variable:
            return bind_variable_declaration(d);
        case vhdl::syntax::object::v_::file:
            return bind_file_declaration(d);
        default:
            break;
        }
        return false;

    case vhdl::syntax::declarative_item::v_::interface:
        switch (d->v.interface.v->v_kind) {
        case vhdl::syntax::interface::v_::constant:
            return bind_interface_constant_declaration(d);
        case vhdl::syntax::interface::v_::signal:
            return bind_interface_signal_declaration(d);
        case vhdl::syntax::interface::v_::variable:
            return bind_interface_variable_declaration(d);
        case vhdl::syntax::interface::v_::file:
            return bind_interface_file_declaration(d);
        default:
            break;
        }
        return false;

    case vhdl::syntax::declarative_item::v_::alias:
        return bind_alias_declaration(d);

    case vhdl::syntax::declarative_item::v_::subprogram:
        return bind_subprogram_declaration(d);

    case vhdl::syntax::declarative_item::v_::subprogram_body:
        return bind_subprogram_body(d);

    case vhdl::syntax::declarative_item::v_::component:
        return bind_component_declaration(d);

    default:
        break;
    }
    return false;
}

bool vhdl::semantic::binder::bind_type_declaration(
    vhdl::syntax::declarative_item* d)
{
    auto& type = d->v.type;
    auto td = new vhdl::node::typedecl(d);
    auto ok = add_named_entity(td);

    // this is an incomplete type
    if (!type.type)
        return ok;

    auto [kk, t] = bind_type_definition(type.type);
    td->type = t;

    return ok;
}

bool vhdl::semantic::binder::bind_subtype_declaration(
    vhdl::syntax::declarative_item* d)
{
    auto& subtype = d->v.subtype;
    auto ok = add_named_entity(new vhdl::node::subtype(d));

    return ok;
}

std::tuple<bool, vhdl::node::type*> vhdl::semantic::binder::bind_subtype(
    vhdl::syntax::subtype* s)
{
    auto ok = resolve(s->mark); // as type mark

    vhdl::node::type* t = nullptr;
    if (s->mark->denotes.size() == 1)
    {
        if (s->mark->denotes[0]->is_typedecl())
        {
            t = s->mark->denotes[0]->as_typedecl()->type;
        }
    }

    if (s->constraint)
        ok &= bind_constraint(s->constraint);

    // in the future, if subtype constraint is defined, subtype will refer to
    // an implicit type and so subtype should own that implicit type.

    return std::make_tuple(ok, t);
}

bool vhdl::semantic::binder::bind_index_subtype(vhdl::syntax::index_subtype* s)
{
    return resolve(s->mark); // as type mark
}

bool vhdl::semantic::binder::bind_constraint(vhdl::syntax::constraint* c)
{
    auto ok = true;
    switch (c->v_kind) {
    case vhdl::syntax::constraint::v_::index:
        for (auto it : c->v.index.indices)
            ok &= bind_range(it);
        break;

    case vhdl::syntax::constraint::v_::range:
        ok &= bind_range(c->v.range.dimension);
        break;

    default:
        break;
    }
    return ok;
}

bool vhdl::semantic::binder::bind_constant_declaration(
    vhdl::syntax::declarative_item* d)
{
    auto ok = true;

    std::vector<vhdl::node::constant*> nntts;
    for (auto i = 0; i < d->v.object.v->identifier.size(); i++)
    {
        auto c = new vhdl::node::constant(d, i);
        nntts.push_back(c);
        ok &= add_named_entity(c);
    }

    auto [kk, t] = bind_subtype(d->v.object.v->type);
    ok &= kk;
    if (kk && t)
        for (auto it : nntts)
            it->type = t;

    if (d->v.object.v->v.variable.value)
        ok &= resolve_expression(d->v.object.v->v.constant.value);

    return ok;
}

bool vhdl::semantic::binder::bind_signal_declaration(
    vhdl::syntax::declarative_item* d)
{
    auto ok = true;

    std::vector<vhdl::node::signal*> nntts;
    for (auto i = 0; i < d->v.object.v->identifier.size(); i++)
    {
        auto s = new vhdl::node::signal(d, i);
        nntts.push_back(s);
        ok &= add_named_entity(s);
    }

    auto [kk, t] = bind_subtype(d->v.object.v->type);
    ok &= kk;
    if (kk && t)
        for (auto it : nntts)
        {
            it->owns_type = false;
            it->type = t;
        }

    if (d->v.object.v->v.signal.value)
        ok &= resolve_expression(d->v.object.v->v.signal.value);

    return ok;
}

bool vhdl::semantic::binder::bind_variable_declaration(
    vhdl::syntax::declarative_item* d)
{
    auto ok = true;

    std::vector<vhdl::node::variable*> nntts;
    for (auto i = 0; i < d->v.object.v->identifier.size(); i++)
    {
        auto v = new vhdl::node::variable(d, i);
        nntts.push_back(v);
        ok &= add_named_entity(v);
    }

    auto [kk, t] = bind_subtype(d->v.object.v->type);
    ok &= kk;
    if (kk && t)
        for (auto it : nntts)
            it->type = t;

    if (d->v.object.v->v.variable.value)
        ok &= resolve_expression(d->v.object.v->v.variable.value);

    return ok;
}

bool vhdl::semantic::binder::bind_file_declaration(
    vhdl::syntax::declarative_item* d)
{
    auto ok = true;

    std::vector<vhdl::node::file*> nntts;
    for (auto i = 0; i < d->v.object.v->identifier.size(); i++)
    {
        auto v = new vhdl::node::file(d, i);
        nntts.push_back(v);
        ok &= add_named_entity(v);
    }

    auto [kk, t] = bind_subtype(d->v.object.v->type);
    ok &= kk;
    if (kk && t)
        for (auto it : nntts)
            it->type = t;

    return ok;
}

bool vhdl::semantic::binder::bind_alias_declaration(
    vhdl::syntax::declarative_item* d)
{
    auto& alias = d->v.alias;
    auto ok = add_named_entity(new vhdl::node::alias(d));

    return ok;
}

bool vhdl::semantic::binder::bind_subprogram_declaration(
    vhdl::syntax::declarative_item* d)
{
    auto& subprogram = d->v.subprogram;

    vhdl::node::named_entity* ptr;
    switch (subprogram.spec->v_kind) {
    case vhdl::syntax::subprogram::v_::function:
        ptr = new vhdl::node::function(d);
        break;

    case vhdl::syntax::subprogram::v_::procedure:
        ptr = new vhdl::node::procedure(d);
        break;

    default:
        return false;
    }

    auto ok = add_named_entity(ptr);
    if (!ok)
        return false;

    open_declarative_region(vhdl::node::declarative_region::v_::subprogram, d);

    for (auto param : subprogram.spec->parameters)
    {
        ok &= bind_declarative_item(param);
    }

    close_declarative_region();

    return ok;
}

bool vhdl::semantic::binder::bind_subprogram_body(
    vhdl::syntax::declarative_item* d)
{
    auto& subprogram = d->v.subprogram_body;

    vhdl::node::named_entity* ptr;
    switch (subprogram.spec->v_kind) {
    case vhdl::syntax::subprogram::v_::function:
        ptr = new vhdl::node::function(d);
        break;

    case vhdl::syntax::subprogram::v_::procedure:
        ptr = new vhdl::node::procedure(d);
        break;

    default:
        return false;
    }

    auto ok = add_named_entity(ptr);
    if (!ok)
        return false;

    open_declarative_region(vhdl::node::declarative_region::v_::subprogram_body,
                            d);

    for (auto param : subprogram.spec->parameters)
    {
        ok &= bind_declarative_item(param);
    }

    for (auto decl : subprogram.decls)
    {
        ok &= bind_declarative_item(decl);
    }

    for (auto stmt : subprogram.stmts)
    {
        ok &= bind_sequential_statement(stmt);
    }

    close_declarative_region();

    return ok;
}

bool vhdl::semantic::binder::bind_component_declaration(
    vhdl::syntax::declarative_item* d)
{
    auto& component = d->v.component;
    auto ok = add_named_entity(new vhdl::node::component(d));

    open_declarative_region(vhdl::node::declarative_region::v_::component, d);

    for (auto decl : component.gens)
    {
        ok &= bind_declarative_item(decl);
    }

    for (auto decl : component.ports)
    {
        ok &= bind_declarative_item(decl);
    }

    close_declarative_region();

    return ok;
}

bool vhdl::semantic::binder::bind_interface_constant_declaration(
    vhdl::syntax::declarative_item* d)
{
    auto ok = true;

    for (auto i = 0; i < d->v.interface.v->identifier.size(); i++)
    {
        ok &= add_named_entity(new vhdl::node::constant(d, i));
    }

    auto [kk, t] = bind_subtype(d->v.interface.v->type);

    return ok;
}

bool vhdl::semantic::binder::bind_interface_signal_declaration(
    vhdl::syntax::declarative_item* d)
{
    auto ok = true;

    for (auto i = 0; i < d->v.interface.v->identifier.size(); i++)
    {
        ok &= add_named_entity(new vhdl::node::signal(d, i));
    }

    auto [kk, t] = bind_subtype(d->v.interface.v->type);

    return ok;
}

bool vhdl::semantic::binder::bind_interface_variable_declaration(
    vhdl::syntax::declarative_item* d)
{
    auto ok = true;

    for (auto i = 0; i < d->v.interface.v->identifier.size(); i++)
    {
        ok &= add_named_entity(new vhdl::node::variable(d, i));
    }

    auto [kk, t] = bind_subtype(d->v.interface.v->type);

    return ok;
}

bool vhdl::semantic::binder::bind_interface_file_declaration(
    vhdl::syntax::declarative_item* d)
{
    auto ok = true;

    for (auto i = 0; i < d->v.interface.v->identifier.size(); i++)
    {
        ok &= add_named_entity(new vhdl::node::file(d, i));
    }

    return ok;
}

bool vhdl::semantic::binder::bind_association_list(
    std::vector<vhdl::syntax::association_element*>& a)
{
    auto ok = true;

    for (auto it : a)
    {
        if (it->actual)
            resolve_expression(it->actual);
    }

    return ok;
}

void vhdl::semantic::binder::lookup_simple_name(vhdl::syntax::name* n)
{
    for (auto region = current_region; region;)
    {
        auto e = lookup(n->v.simple.identifier.value, region);
        n->denotes.insert(n->denotes.end(), e.begin(), e.end());

        if (region->extends)
            region = region->extends;
        else
            region = region->outer;
    }
}

void vhdl::semantic::binder::lookup_selected_name(vhdl::syntax::name* n)
{
    auto& p = n->v.selected.prefix;
    if (p == nullptr)
        return;

    auto ok = resolve(p);
    if (!ok)
        return;

    if (p->denotes.size() == 0)
        return;

    if (p->denotes.size() > 1)
    {
        for (auto it : p->denotes)
            lookup_expanded_name(n, it);
        return;
    }

    auto& it = p->denotes[0];
    switch (it->get_kind()) {
    case vhdl::node::kind::library:
    case vhdl::node::kind::entity:
    case vhdl::node::kind::architecture:
    case vhdl::node::kind::configuration:
    case vhdl::node::kind::package:
    case vhdl::node::kind::package_body:
    case vhdl::node::kind::function:
    case vhdl::node::kind::procedure:
        lookup_expanded_name(n, it);
        break;

    case vhdl::node::kind::type:
    case vhdl::node::kind::subtype:
    case vhdl::node::kind::alias:
    case vhdl::node::kind::component:
    case vhdl::node::kind::file:
        // error saying type cannot be selected by name
        break;

    case vhdl::node::kind::element:
    case vhdl::node::kind::constant:
    case vhdl::node::kind::signal:
    case vhdl::node::kind::variable:
        // if type is record
        lookup_selected_element(n, it);
        break;

    default:
        break;
    }
}

void vhdl::semantic::binder::lookup_expanded_name(vhdl::syntax::name* n,
                                                  vhdl::node::named_entity* p)
{
    vhdl::node::declarative_region* rgn = nullptr;
    switch (p->get_kind()) {
    case vhdl::node::kind::library: {
        auto candidates = ast->load_primary_unit(
            std::string(p->as_library()->identifier),
            n->v.selected.identifier.value, std::nullopt);

        for (auto candidate : candidates)
        {
            if (candidate->state != vhdl::node::library_unit_state::analysed)
                continue;
            if (!candidate->syntax->named_entity)
                continue;
            candidate->references.push_back(unit);
            unit->dependencies.push_back(candidate);
            n->denotes.push_back(candidate->syntax->named_entity);
        }
    }
    break;

    case vhdl::node::kind::entity:
        rgn = p->as_entity()->u->v.entity.region;
        break;
    case vhdl::node::kind::architecture:
        rgn = p->as_architecture()->u->v.architecture.region;
        break;
    case vhdl::node::kind::configuration:
        rgn = p->as_configuration()->u->v.configuration.region;
        break;
    case vhdl::node::kind::package:
        rgn = p->as_package()->u->v.package.region;
        break;
    case vhdl::node::kind::package_body:
        rgn = p->as_package_body()->u->v.package_body.region;
        break;
    case vhdl::node::kind::function:
        rgn = p->as_function()->d->v.subprogram.region;
        break;
    case vhdl::node::kind::procedure:
        rgn = p->as_procedure()->d->v.subprogram.region;
        break;
    default:
        assert(false && "how did we get in lookup expanded name");
        break;
    }

    if (!rgn)
        return;

    auto result = lookup(n->v.selected.identifier.value, rgn);
    n->denotes.insert(n->denotes.end(), result.begin(), result.end());
}

void vhdl::semantic::binder::lookup_selected_element(
    vhdl::syntax::name* n, vhdl::node::named_entity* p)
{
    vhdl::node::type* type = nullptr;
    switch (p->get_kind()) {
    case vhdl::node::kind::element:
        type = p->as_element()->type;
        break;
    case vhdl::node::kind::constant:
        type = p->as_constant()->type;
        break;
    case vhdl::node::kind::signal:
        type = p->as_signal()->type;
        break;
    case vhdl::node::kind::variable:
        type = p->as_variable()->type;
        break;
    default:
        assert(false && "how did we get in lookup selected element");
        break;
    }

    if (!type)
        return;

    if (!type->is_record())
        // we can probably output a useful error message here
        return;

    auto rgn = type->as_record()->syntax->v.record.region;

    if (!rgn)
        return;

    auto result = lookup(n->v.selected.identifier.value, rgn);
    n->denotes.insert(n->denotes.end(), result.begin(), result.end());
}

void vhdl::semantic::binder::lookup_slice_name(vhdl::syntax::name* n)
{
    resolve(n->v.slice.prefix);
    bind_range(n->v.slice.range);
}

void vhdl::semantic::binder::lookup_ambiguous_name(vhdl::syntax::name* n)
{
    resolve(n->v.ambiguous.prefix);
    for (auto it : n->v.ambiguous.expression)
    {
        resolve_expression(it);
    }
}

void vhdl::semantic::binder::lookup_fcall(vhdl::syntax::name* n)
{
    resolve(n->v.fcall.prefix);
}

bool vhdl::semantic::binder::resolve(vhdl::syntax::name* n)
{
    if (!n)
        return false;

    switch (n->v_kind) {
    case vhdl::syntax::name::v_::simple:
        lookup_simple_name(n);
        break;

    case vhdl::syntax::name::v_::selected:
        lookup_selected_name(n);
        break;

    case vhdl::syntax::name::v_::slice:
        lookup_slice_name(n);
        break;

    case vhdl::syntax::name::v_::fcall:
        lookup_fcall(n);
        break;

    case vhdl::syntax::name::v_::ambiguous:
        lookup_ambiguous_name(n);
        break;

    case vhdl::syntax::name::v_::qualified:
        return resolve(n->v.qualified.prefix);

    case vhdl::syntax::name::v_::attribute:
        return resolve(n->v.attribute.prefix);

    case vhdl::syntax::name::v_::signature:
        return resolve(n->v.signature.prefix);

    default:
        return false;
    }

    return true;
}

common::location get_name_range(vhdl::syntax::name* n)
{
    return common::location("", 0, 0);
}

std::tuple<bool, vhdl::node::direct_visibility*> vhdl::semantic::binder::
    resolve_use_name(vhdl::syntax::name* n)
{
    if (!n)
        return std::make_tuple(false, nullptr);

    switch (n->v_kind) {
    case vhdl::syntax::name::v_::none:
    case vhdl::syntax::name::v_::error:
        return std::make_tuple(false, nullptr);

    case vhdl::syntax::name::v_::simple:
    case vhdl::syntax::name::v_::slice:
    case vhdl::syntax::name::v_::fcall:
    case vhdl::syntax::name::v_::ambiguous:
    case vhdl::syntax::name::v_::qualified:
    case vhdl::syntax::name::v_::attribute:
    case vhdl::syntax::name::v_::signature:
        // not ok. Complain about it
        break;

    default:
        break;
    }

    if (n->v.selected.identifier.value != "all")
    {
        auto k = resolve(n);
        auto v = new vhdl::node::direct_visibility;
        v->entities = n->denotes;
        return std::make_tuple(k, v);
    }

    auto k = resolve(n->v.selected.prefix);
    auto v = std::make_unique<vhdl::node::direct_visibility>();

    switch (n->v.selected.prefix->denotes.size()) {
    case 0:
        // diag("prefix of all denotes nothing", n->v.selected.identifier);
        return std::make_tuple(false, nullptr);

    case 1:
        break;

    default:
        diag("Multiple entities were found and could not be resolved",
             n->v.selected.identifier);
        return std::make_tuple(false, nullptr);
    }

    auto d = n->v.selected.prefix->denotes[0];
    switch (d->get_kind()) {
    case vhdl::node::kind::library:
        break;

    case vhdl::node::kind::package:
        v->entities = d->as_package()->u->v.package.region->named_entities;
        n->denotes = v->entities;
        break;

    default:
        break;
    }

    return std::make_tuple(true, v.release());
}

std::tuple<bool, vhdl::node::declarative_region*> vhdl::semantic::binder::
    resolve_denotes_entity(vhdl::syntax::name* n)
{
    if (!n)
        return std::make_tuple(false, nullptr);

    std::string name = "unnamed";
    std::vector<std::shared_ptr<vhdl::node::library_unit>> candidates;

    switch (n->v_kind) {
    case vhdl::syntax::name::v_::simple:
        name = n->v.simple.identifier.value;
        candidates = ast->load_primary_unit(
            std::nullopt, n->v.simple.identifier.value, std::nullopt);

        for (auto candidate : candidates)
        {
            if (candidate->state != vhdl::node::library_unit_state::analysed)
                continue;
            if (candidate->syntax->v_kind !=
                vhdl::syntax::design_unit::v_::entity)
                continue;
            if (!candidate->syntax->named_entity)
                continue;
            candidate->references.push_back(unit);
            unit->dependencies.push_back(candidate);
            n->denotes.push_back(candidate->syntax->named_entity);
        }
        break;

    default:
        break;
    }

    switch (n->denotes.size()) {
    case 0:
        diag("Entity {} was not found in library {}", n->v.simple.identifier)
            << n->v.simple.identifier.value << ast->get_work_library_name();
        break;
    case 1:
        return std::make_tuple(true,
                               n->denotes[0]->as_entity()->u->v.entity.region);

    default:
        diag("Multiple named entities {} were found and could not be resolved",
             n->v.simple.identifier)
            << name;
        break;
    }

    return std::make_tuple(false, nullptr);
}

bool vhdl::semantic::binder::resolve_binary_expression(
    vhdl::syntax::expression* e)
{
    auto kk = true;
    kk &= resolve_expression(e->v.binary.lhs);
    kk &= resolve_expression(e->v.binary.rhs);
    return kk;
}

bool vhdl::semantic::binder::resolve_unary_expression(
    vhdl::syntax::expression* e)
{
    auto kk = resolve_expression(e->v.binary.lhs);
    return kk;
}

bool vhdl::semantic::binder::resolve_literal_expression(
    vhdl::syntax::expression* e)
{
    return true;
}

bool vhdl::semantic::binder::resolve_physical_expression(
    vhdl::syntax::expression* e)
{
    auto kk = resolve(e->v.physical.name);
    return kk;
}

bool vhdl::semantic::binder::resolve_aggregate_expression(
    vhdl::syntax::expression* e)
{
    auto kk = true;
    for (auto it : e->v.aggregate.literal)
    {
        kk &= resolve_expression(it->expression);
    }
    return true;
}

bool vhdl::semantic::binder::resolve_unresolved_expression(
    vhdl::syntax::expression* e)
{
    auto kk = resolve(e->v.unresolved.name);
    return kk;
}

bool vhdl::semantic::binder::resolve_allocator_expression(
    vhdl::syntax::expression* e)
{
    return true;
}

bool vhdl::semantic::binder::resolve_nested_expression(
    vhdl::syntax::expression* e)
{
    return resolve_expression(e->v.nested.expr);
}

bool vhdl::semantic::binder::resolve_null_expression(
    vhdl::syntax::expression* e)
{
    return true;
}

bool vhdl::semantic::binder::resolve_expression(vhdl::syntax::expression* e)
{
    switch (e->v_kind) {
    case vhdl::syntax::expression::v_::binary:
        return resolve_binary_expression(e);

    case vhdl::syntax::expression::v_::unary:
        return resolve_unary_expression(e);

    case vhdl::syntax::expression::v_::literal:
        return resolve_literal_expression(e);

    case vhdl::syntax::expression::v_::physical:
        return resolve_physical_expression(e);

    case vhdl::syntax::expression::v_::aggregate:
        return resolve_aggregate_expression(e);

    case vhdl::syntax::expression::v_::unresolved:
        return resolve_unresolved_expression(e);

    case vhdl::syntax::expression::v_::allocator:
        return resolve_allocator_expression(e);

    case vhdl::syntax::expression::v_::nested:
        return resolve_nested_expression(e);

    case vhdl::syntax::expression::v_::null:
        return resolve_null_expression(e);

    default:
        return false;
    }
    return true;
}

bool vhdl::semantic::binder::bind_choices(std::vector<vhdl::syntax::choice*>& c)
{
    auto kk = true;
    for (auto it : c)
    {
        switch (it->v_kind) {
        case vhdl::syntax::choice::v_::name:
            kk &= resolve_expression(it->v.name.expression);
            break;
        case vhdl::syntax::choice::v_::range:
            kk &= bind_discrete_range(it->v.range.range);
            break;

        default:
            break;
        }
    }

    return kk;
}

bool vhdl::semantic::binder::bind_sequential_statement(
    vhdl::syntax::sequential_statement* s)
{
    switch (s->v_kind) {
    case vhdl::syntax::sequential_statement::v_::wait:
        bind_wait_statement(s);
        break;
    case vhdl::syntax::sequential_statement::v_::assertion:
        bind_assertion_statement(s);
        break;
    case vhdl::syntax::sequential_statement::v_::report:
        bind_report_statement(s);
        break;
    case vhdl::syntax::sequential_statement::v_::sig_assign:
        bind_sig_assign_statement(s);
        break;
    case vhdl::syntax::sequential_statement::v_::var_assign:
        bind_var_assign_statement(s);
        break;
    case vhdl::syntax::sequential_statement::v_::pcall:
        bind_pcall_statement(s);
        break;
    case vhdl::syntax::sequential_statement::v_::if_stmt:
        bind_if_stmt_statement(s);
        break;
    case vhdl::syntax::sequential_statement::v_::for_loop:
        bind_for_loop_statement(s);
        break;
    case vhdl::syntax::sequential_statement::v_::while_loop:
        bind_while_loop_statement(s);
        break;
    case vhdl::syntax::sequential_statement::v_::case_stmt:
        bind_case_stmt_statement(s);
        break;
    case vhdl::syntax::sequential_statement::v_::next:
        bind_next_statement(s);
        break;
    case vhdl::syntax::sequential_statement::v_::exit:
        bind_exit_statement(s);
        break;
    case vhdl::syntax::sequential_statement::v_::return_stmt:
        bind_return_stmt_statement(s);
        break;
    case vhdl::syntax::sequential_statement::v_::null:
        bind_null_statement(s);
        break;
    default:
        break;
    }

    return true;
}

bool vhdl::semantic::binder::bind_wait_statement(
    vhdl::syntax::sequential_statement* s)
{
    auto kk = true;
    for (auto it : s->v.wait.sensitivity)
        kk &= resolve(it);

    if (s->v.wait.condition)
        kk &= resolve_expression(s->v.wait.condition);

    if (s->v.wait.timeout)
        kk &= resolve_expression(s->v.wait.timeout);

    return kk;
}

bool vhdl::semantic::binder::bind_assertion_statement(
    vhdl::syntax::sequential_statement* s)
{
    auto kk = true;
    kk &= resolve_expression(s->v.assertion.condition);

    if (s->v.assertion.report)
        kk &= resolve_expression(s->v.assertion.report);

    if (s->v.assertion.severity)
        kk &= resolve_expression(s->v.assertion.severity);

    return kk;
}

bool vhdl::semantic::binder::bind_report_statement(
    vhdl::syntax::sequential_statement* s)
{
    auto kk = true;
    kk &= resolve_expression(s->v.report.report);

    if (s->v.report.severity)
        kk &= resolve_expression(s->v.report.severity);

    return kk;
}

bool vhdl::semantic::binder::bind_sig_assign_statement(
    vhdl::syntax::sequential_statement* s)
{
    auto kk = true;

    kk &= bind_target(s->v.sig_assign.target);
    kk &= bind_waveform(s->v.sig_assign.waveforms);
    return kk;
}

bool vhdl::semantic::binder::bind_var_assign_statement(
    vhdl::syntax::sequential_statement* s)
{
    auto kk = true;

    kk &= bind_target(s->v.var_assign.target);
    kk &= resolve_expression(s->v.var_assign.value);
    return kk;
}

bool vhdl::semantic::binder::bind_pcall_statement(
    vhdl::syntax::sequential_statement* s)
{
    auto kk = true;

    kk &= resolve(s->v.pcall.procedure);
    return kk;
}

bool vhdl::semantic::binder::bind_if_stmt_statement(
    vhdl::syntax::sequential_statement* s)
{
    auto kk = true;
    if (s->v.if_stmt.condition)
        kk &= resolve_expression(s->v.if_stmt.condition);

    for (auto it : s->v.if_stmt.then)
        kk &= bind_sequential_statement(it);

    if (s->v.if_stmt.otherwise)
        kk &= bind_sequential_statement(s->v.if_stmt.otherwise);

    return kk;
}

bool vhdl::semantic::binder::bind_for_loop_statement(
    vhdl::syntax::sequential_statement* s)
{
    auto kk = true;

    kk &= bind_range(s->v.for_loop.param->in);

    for (auto it : s->v.for_loop.loop)
        kk &= bind_sequential_statement(it);

    return kk;
}

bool vhdl::semantic::binder::bind_while_loop_statement(
    vhdl::syntax::sequential_statement* s)
{
    auto kk = true;

    for (auto it : s->v.while_loop.loop)
        kk &= bind_sequential_statement(it);

    return kk;
}

bool vhdl::semantic::binder::bind_case_stmt_statement(
    vhdl::syntax::sequential_statement* s)
{
    auto kk = true;
    kk = resolve_expression(s->v.case_stmt.expression);
    for (auto it : s->v.case_stmt.alternatives)
    {
        kk &= bind_choices(it->choices);
        for (auto it2 : it->stmts)
            kk &= bind_sequential_statement(it2);
    }
    return kk;
}

bool vhdl::semantic::binder::bind_next_statement(
    vhdl::syntax::sequential_statement* s)
{
    auto kk = true;

    if (s->v.next.when)
        kk &= resolve_expression(s->v.next.when);
    return kk;
}

bool vhdl::semantic::binder::bind_exit_statement(
    vhdl::syntax::sequential_statement* s)
{
    auto kk = true;

    if (s->v.exit.when)
        kk &= resolve_expression(s->v.exit.when);
    return kk;
}

bool vhdl::semantic::binder::bind_return_stmt_statement(
    vhdl::syntax::sequential_statement* s)
{
    auto kk = true;

    if (s->v.return_stmt.expression)
        kk &= resolve_expression(s->v.return_stmt.expression);
    return kk;
}

bool vhdl::semantic::binder::bind_null_statement(
    vhdl::syntax::sequential_statement* s)
{
    return true;
}

bool vhdl::semantic::binder::bind_target(vhdl::syntax::target* t)
{
    switch (t->v_kind) {
    case vhdl::syntax::target::v_::name:
        return resolve(t->v.name.name);

    case vhdl::syntax::target::v_::expression:
        return resolve_expression(t->v.expression.aggregate);

    default:
        return false;
    }

    return true;
}

bool vhdl::semantic::binder::bind_waveform(
    std::vector<vhdl::syntax::waveform_element*>& w)
{
    auto kk = true;
    for (auto it : w)
    {
        if (it->value)
            kk &= resolve_expression(it->value);

        if (it->time)
            kk &= resolve_expression(it->time);
    }
    return kk;
}

bool vhdl::semantic::binder::bind_concurrent_statement(
    vhdl::syntax::concurrent_statement* s)
{
    switch (s->v_kind) {
    case vhdl::syntax::concurrent_statement::v_::process:
        bind_process_statement(s);
        break;
    case vhdl::syntax::concurrent_statement::v_::assertion:
        bind_concurrent_assertion_statement(s);
        break;
    case vhdl::syntax::concurrent_statement::v_::con_assign:
        bind_concurrent_conditional_signal_assignment_statement(s);
        break;
    case vhdl::syntax::concurrent_statement::v_::sel_assign:
        bind_concurrent_selected_signal_assignment_statement(s);
        break;
    case vhdl::syntax::concurrent_statement::v_::inst:
        bind_component_instantiation(s);
        break;
    case vhdl::syntax::concurrent_statement::v_::for_gen:
        bind_for_generate_statement(s);
        break;
    case vhdl::syntax::concurrent_statement::v_::if_gen:
        bind_if_generate_statement(s);
        break;

    default:
        break;
    }
    return true;
}

bool vhdl::semantic::binder::bind_concurrent_assertion_statement(
    vhdl::syntax::concurrent_statement* s)
{
    auto kk = true;
    kk &= resolve_expression(s->v.assertion.condition);

    if (s->v.assertion.report)
        kk &= resolve_expression(s->v.assertion.report);

    if (s->v.assertion.severity)
        kk &= resolve_expression(s->v.assertion.severity);

    return kk;
}

bool vhdl::semantic::binder::bind_process_statement(
    vhdl::syntax::concurrent_statement* s)
{
    auto& process = s->v.process;

    open_declarative_region(vhdl::node::declarative_region::v_::process, s);

    auto kk = true;
    for (auto it : s->v.process.sensitivity)
    {
        kk &= resolve(it);
    }

    for (auto it : s->v.process.decls)
    {
        kk &= bind_declarative_item(it);
    }

    for (auto it : s->v.process.stmts)
    {
        kk &= bind_sequential_statement(it);
    }

    close_declarative_region();

    return kk;
}

bool vhdl::semantic::binder::
    bind_concurrent_conditional_signal_assignment_statement(
        vhdl::syntax::concurrent_statement* s)
{
    auto kk = true;
    kk &= bind_target(s->v.con_assign.target);

    for (auto it : s->v.con_assign.waveforms)
    {
        kk &= bind_waveform(it->waveforms);

        if (it->when)
            kk &= resolve_expression(it->when);
    }
    return kk;
}

bool vhdl::semantic::binder::
    bind_concurrent_selected_signal_assignment_statement(
        vhdl::syntax::concurrent_statement* s)
{
    auto kk = true;
    kk &= resolve_expression(s->v.sel_assign.value);
    kk &= bind_target(s->v.sel_assign.target);

    for (auto it : s->v.sel_assign.waveforms)
    {
        kk &= bind_waveform(it->waveforms);

        kk &= bind_choices(it->choices);
    }

    return kk;
}

bool vhdl::semantic::binder::bind_component_instantiation(
    vhdl::syntax::concurrent_statement* s)
{
    auto& inst = s->v.inst;

    auto ok = true;
    if (s->v.inst.inst->v_kind ==
        vhdl::syntax::instantiated_unit::v_::component)
        ok &= resolve(s->v.inst.inst->v.component.unit);

    open_declarative_region(vhdl::node::declarative_region::v_::instantiation,
                            s);

    ok &= bind_association_list(s->v.inst.gens);
    ok &= bind_association_list(s->v.inst.ports);

    close_declarative_region();

    return ok;
}

bool vhdl::semantic::binder::bind_for_generate_statement(
    vhdl::syntax::concurrent_statement* s)
{
    auto& for_gen = s->v.for_gen;

    open_declarative_region(vhdl::node::declarative_region::v_::for_gen, s);

    auto ok = true;

    ok &= bind_range(s->v.for_gen.param->in);

    for (auto decl : for_gen.decls)
    {
        ok &= bind_declarative_item(decl);
    }

    for (auto stmt : for_gen.stmts)
    {
        ok &= bind_concurrent_statement(stmt);
    }

    close_declarative_region();

    return ok;
}

bool vhdl::semantic::binder::bind_if_generate_statement(
    vhdl::syntax::concurrent_statement* s)
{
    auto& if_gen = s->v.if_gen;

    open_declarative_region(vhdl::node::declarative_region::v_::if_gen, s);

    auto ok = true;

    ok &= resolve_expression(if_gen.condition);

    for (auto decl : if_gen.decls)
    {
        ok &= bind_declarative_item(decl);
    }

    for (auto stmt : if_gen.stmts)
    {
        ok &= bind_concurrent_statement(stmt);
    }

    close_declarative_region();

    return ok;
}

void vhdl::semantic::binder::open_declarative_region()
{
    auto n = new vhdl::node::declarative_region();

    // this is probably a root declarative region. Otherwise, the current
    // declarative region owns this new region
    if (current_region != nullptr)
        current_region->inner.push_back(n);

    n->outer = current_region;
    current_region = n;
}

void vhdl::semantic::binder::open_declarative_region(
    vhdl::node::declarative_region::v_ kind, vhdl::syntax::design_unit* u)
{
    open_declarative_region();
    current_region->v_kind = kind;
    switch (kind) {
    case vhdl::node::declarative_region::v_::entity:
        current_region->v.entity = u;
        u->v.entity.region = current_region;
        break;
    case vhdl::node::declarative_region::v_::architecture:
        current_region->v.architecture = u;
        u->v.architecture.region = current_region;
        break;
    case vhdl::node::declarative_region::v_::package:
        current_region->v.package = u;
        u->v.package.region = current_region;
        break;
    case vhdl::node::declarative_region::v_::package_body:
        current_region->v.package_body = u;
        u->v.package_body.region = current_region;
        break;
    case vhdl::node::declarative_region::v_::configuration:
        current_region->v.configuration = u;
        u->v.configuration.region = current_region;
        break;
    default:
        assert(false && "we should never be here");
        break;
    }
}

void vhdl::semantic::binder::open_declarative_region(
    vhdl::node::declarative_region::v_ kind, vhdl::syntax::declarative_item* d)
{
    open_declarative_region();
    current_region->v_kind = kind;
    switch (kind) {
    case vhdl::node::declarative_region::v_::subprogram:
        current_region->v.subprogram = d;
        d->v.subprogram.region = current_region;
        break;
    case vhdl::node::declarative_region::v_::subprogram_body:
        current_region->v.subprogram_body = d;
        d->v.subprogram_body.region = current_region;
        break;
    case vhdl::node::declarative_region::v_::component:
        current_region->v.component = d;
        d->v.component.region = current_region;
        break;
    default:
        assert(false && "we should never be here");
        break;
    }
}

void vhdl::semantic::binder::open_declarative_region(
    vhdl::node::declarative_region::v_ kind,
    vhdl::syntax::concurrent_statement* s)
{
    open_declarative_region();
    current_region->v_kind = kind;
    switch (kind) {
    case vhdl::node::declarative_region::v_::instantiation:
        current_region->v.instantiation = s;
        s->v.inst.region = current_region;
        break;
    case vhdl::node::declarative_region::v_::process:
        current_region->v.process = s;
        s->v.process.region = current_region;
        break;
    case vhdl::node::declarative_region::v_::for_gen:
        current_region->v.for_gen = s;
        s->v.for_gen.region = current_region;
        break;
    case vhdl::node::declarative_region::v_::if_gen:
        current_region->v.if_gen = s;
        s->v.if_gen.region = current_region;
        break;
    default:
        assert(false && "we should never be here");
        break;
    }
}

void vhdl::semantic::binder::open_declarative_region(
    vhdl::node::declarative_region::v_ kind, vhdl::syntax::type_definition* t)
{
    open_declarative_region();
    current_region->v_kind = kind;
    switch (kind) {
    case vhdl::node::declarative_region::v_::record:
        current_region->v.record = t;
        t->v.record.region = current_region;
        break;
    default:
        assert(false && "we should never be here");
        break;
    }
}

void vhdl::semantic::binder::close_declarative_region()
{
    assert(current_region && "unbalanced declarative regions");
    current_region = current_region->outer;
}

bool vhdl::semantic::binder::add_named_entity(vhdl::node::named_entity* ntt)
{
    assert(current_region && "unknown declarative regions");
    // todo check identifier is not in current declarative region because of
    // vhdl scope, visibility and overloading rules
    // return {false, nullptr};

    current_region->named_entities.push_back(ntt);

    return true;
}

std::vector<vhdl::node::named_entity*> vhdl::semantic::binder::lookup(
    std::string_view identifier, vhdl::node::declarative_region* region)
{
    std::vector<vhdl::node::named_entity*> results;
    for (auto ne : region->named_entities)
    {
        if (ne->get_identifier() != identifier)
            continue;

        results.push_back(ne);
    }

    if (results.size() == 0)
    {
        for (auto c : region->potentially_visible)
        {
            for (auto ne : c->entities)
            {
                if (ne->get_identifier() != identifier)
                    continue;

                results.push_back(ne);
            }
        }
    }
    return results;
}

bool vhdl::semantic::binder::bind_context_clause(
    std::vector<vhdl::syntax::context_item*> clauses)
{
    auto ok = true;
    for (auto it : clauses)
        switch (it->v_kind) {
        case vhdl::syntax::context_item::v_::library_clause:
            ok &= bind_library_clause(it);
            break;

        case vhdl::syntax::context_item::v_::use_clause:
            ok &= bind_use_clause(it);
            break;

        default:
            assert(false && "we should never be here");
            break;
        }

    return ok;
}

bool vhdl::semantic::binder::bind_library_clause(vhdl::syntax::context_item* l)
{
    auto ok = true;
    auto i = 0;
    for (auto logical_name : l->v.library_clause.names)
    {
        if (logical_name.value == "work")
            diag("Library work is not allowed", logical_name);

        ok &= add_named_entity(new vhdl::node::library(l, i));

        // TODO check library is known
        ++i;
    }
    return ok;
}

bool vhdl::semantic::binder::bind_use_clause(vhdl::syntax::context_item* u)
{
    auto ok = true;
    for (auto name : u->v.use_clause.names)
    {

        auto [kk, v] = resolve_use_name(name);
        ok &= kk;
        if (kk)
            current_region->potentially_visible.push_back(v);
    }
    return ok;
}

common::diagnostic& vhdl::semantic::binder::diag(const std::string_view msg,
                                                 vhdl::token& tok)
{
    diagnostics.emplace_back(msg, tok.location);
    return diagnostics.back();
}

common::diagnostic& vhdl::semantic::binder::diag(const std::string_view msg,
                                                 common::location loc)
{
    diagnostics.emplace_back(msg, loc);
    return diagnostics.back();
}
