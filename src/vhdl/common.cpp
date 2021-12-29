
#include "vhdl_nodes.h"

vhdl::node::declarative_region::declarative_region(
    vhdl::node::declarative_region* parent)
    : extends(nullptr)
{
    outer = parent;
}

vhdl::node::entity::entity(vhdl::syntax::design_unit* u)
    : identifier(u->v.entity.identifier.value), u(u)
{
    u->named_entity = this;
}

vhdl::node::architecture::architecture(vhdl::syntax::design_unit* u)
    : identifier(u->v.architecture.identifier.value), u(u)
{
    u->named_entity = this;
}

vhdl::node::configuration::configuration(vhdl::syntax::design_unit* u)
    : identifier(u->v.configuration.identifier.value), u(u)
{
    u->named_entity = this;
}

vhdl::node::package::package(vhdl::syntax::design_unit* u)
    : identifier(u->v.package.identifier.value), u(u)
{
    u->named_entity = this;
}

vhdl::node::package_body::package_body(vhdl::syntax::design_unit* u)
    : identifier(u->v.package_body.identifier.value), u(u)
{
    u->named_entity = this;
}

vhdl::node::typedecl::typedecl(vhdl::syntax::declarative_item* d)
    : identifier(d->v.type.identifier.value), d(d), type(nullptr)
{
}

vhdl::node::subtype::subtype(vhdl::syntax::declarative_item* d)
    : identifier(d->v.subtype.identifier.value), d(d)
{
}

vhdl::node::constant::constant(vhdl::syntax::declarative_item* d, int i)
    : d(d), type(nullptr), index(i)
{
    if (d->v_kind == vhdl::syntax::declarative_item::v_::object)
        identifier = d->v.object.v->identifier[i].value;
    else
        identifier = d->v.interface.v->identifier[i].value;
}

vhdl::node::signal::signal(vhdl::syntax::declarative_item* d, int i)
    : d(d), type(nullptr), index(i)
{
    if (d->v_kind == vhdl::syntax::declarative_item::v_::object)
        identifier = d->v.object.v->identifier[i].value;
    else
        identifier = d->v.interface.v->identifier[i].value;
}

vhdl::node::variable::variable(vhdl::syntax::declarative_item* d, int i)
    : d(d), type(nullptr), index(i)
{
    if (d->v_kind == vhdl::syntax::declarative_item::v_::object)
        identifier = d->v.object.v->identifier[i].value;
    else
        identifier = d->v.interface.v->identifier[i].value;
}

vhdl::node::file::file(vhdl::syntax::declarative_item* d, int i)
    : d(d), type(nullptr), index(i)
{
    if (d->v_kind == vhdl::syntax::declarative_item::v_::object)
        identifier = d->v.object.v->identifier[i].value;
    else
        identifier = d->v.interface.v->identifier[i].value;
}

bool vhdl::node::constant::is_interface()
{
    return d->v_kind == vhdl::syntax::declarative_item::v_::interface;
}

bool vhdl::node::signal::is_interface()
{
    return d->v_kind == vhdl::syntax::declarative_item::v_::interface;
}

bool vhdl::node::variable::is_interface()
{
    return d->v_kind == vhdl::syntax::declarative_item::v_::interface;
}

bool vhdl::node::file::is_interface()
{
    return d->v_kind == vhdl::syntax::declarative_item::v_::interface;
}

vhdl::node::alias::alias(vhdl::syntax::declarative_item* d)
    : identifier(d->v.alias.designator.value), d(d)
{
}

vhdl::node::function::function(vhdl::syntax::declarative_item* d)
    : identifier(d->v.subprogram.spec->designator.value), d(d)
{
}

vhdl::node::procedure::procedure(vhdl::syntax::declarative_item* d)
    : identifier(d->v.subprogram.spec->designator.value), d(d)
{
}

bool vhdl::node::procedure::is_body()
{
    return d->v_kind == vhdl::syntax::declarative_item::v_::subprogram_body;
}

vhdl::node::component::component(vhdl::syntax::declarative_item* d)
    : identifier(d->v.component.identifier.value), d(d)
{
}

vhdl::node::literal::literal(vhdl::syntax::type_definition* t, int i)
    : identifier(t->v.enumeration.literals[i].value), index(i), t(t)
{
}

vhdl::node::element::element(vhdl::syntax::element_declaration* e, int i)
    : identifier(e->identifier[i].value), index(i), e(e)
{
}

vhdl::node::library::library(vhdl::syntax::context_item* c, int i)
    : identifier(c->v.library_clause.names[i].value), index(i), c(c)
{
}

std::string_view vhdl::node::typedecl::get_identifier()
{
    return identifier;
}

std::string_view vhdl::node::subtype::get_identifier()
{
    return identifier;
}

std::string_view vhdl::node::constant::get_identifier()
{
    return identifier;
}

std::string_view vhdl::node::signal::get_identifier()
{
    return identifier;
}

std::string_view vhdl::node::variable::get_identifier()
{
    return identifier;
}

std::string_view vhdl::node::file::get_identifier()
{
    return identifier;
}

std::string_view vhdl::node::alias::get_identifier()
{
    return identifier;
}

std::string_view vhdl::node::entity::get_identifier()
{
    return identifier;
}

std::string_view vhdl::node::architecture::get_identifier()
{
    return identifier;
}

std::string_view vhdl::node::configuration::get_identifier()
{
    return identifier;
}

std::string_view vhdl::node::package::get_identifier()
{
    return identifier;
}

std::string_view vhdl::node::package_body::get_identifier()
{
    return identifier;
}

std::string_view vhdl::node::function::get_identifier()
{
    return identifier;
}

std::string_view vhdl::node::procedure::get_identifier()
{
    return identifier;
}

std::string_view vhdl::node::component::get_identifier()
{
    return identifier;
}

std::string_view vhdl::node::literal::get_identifier()
{
    return identifier;
}

std::string_view vhdl::node::element::get_identifier()
{
    return identifier;
}

std::string_view vhdl::node::library::get_identifier()
{
    return identifier;
}

bool vhdl::syntax::design_unit::operator==(const vhdl::syntax::design_unit& rhs)
{
    if (v_kind != rhs.v_kind && v_kind != vhdl::syntax::design_unit::v_::none)
        return false;

    switch (v_kind)
    {
    case vhdl::syntax::design_unit::v_::architecture:
        if (v.architecture.identifier.value !=
            rhs.v.architecture.identifier.value)
            return false;
        if (v.architecture.entity->v.simple.identifier.value !=
            rhs.v.architecture.entity->v.simple.identifier.value)
            return false;
        break;

    case vhdl::syntax::design_unit::v_::configuration:
        if (v.configuration.identifier.value !=
            rhs.v.configuration.identifier.value)
            return false;
        if (v.configuration.entity->v.simple.identifier.value !=
            rhs.v.configuration.entity->v.simple.identifier.value)
            return false;
        break;

    case vhdl::syntax::design_unit::v_::entity:
        if (v.entity.identifier.value != rhs.v.entity.identifier.value)
            return false;
        break;

    case vhdl::syntax::design_unit::v_::package:
        if (v.package.identifier.value != rhs.v.package.identifier.value)
            return false;
        break;

    case vhdl::syntax::design_unit::v_::package_body:
        if (v.package_body.identifier.value !=
            rhs.v.package_body.identifier.value)
            return false;
        break;

    default:
        return false;
    }

    return true;
}
