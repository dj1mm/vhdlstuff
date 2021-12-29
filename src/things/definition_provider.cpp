
#include "definition_provider.h"

void things::vhdl_definition_provider::definition_found(common::location target, common::position selection_begin, common::position selection_end)
{
    found = true;
    json_populated = true;
    w->StartObject();
    w->Key("targetUri"); w->String(std::string(target.filename));
    w->Key("targetRange");
        w->StartObject();
        w->Key("start");
            w->StartObject();
            w->Key("line"); w->Int(target.begin.line-1);
            w->Key("character"); w->Int(target.begin.column-1);
            w->EndObject();
        w->Key("end");
            w->StartObject();
            w->Key("line"); w->Int(target.end.line-1);
            w->Key("character"); w->Int(target.end.column-1);
            w->EndObject();
        w->EndObject();
    w->Key("targetSelectionRange");
        w->StartObject();
        w->Key("start");
            w->StartObject();
            w->Key("line"); w->Int(selection_begin.line-1);
            w->Key("character"); w->Int(selection_begin.column-1);
            w->EndObject();
        w->Key("end");
            w->StartObject();
            w->Key("line"); w->Int(selection_end.line-1);
            w->Key("character"); w->Int(selection_end.column-1);
            w->EndObject();
        w->EndObject();
    w->EndObject();
}

void things::vhdl_definition_provider::identifier_denotes_that_entity(vhdl::syntax::design_unit* u)
{
    auto target = u->v.entity.identifier.location;
    auto selection_begin = u->first__.begin;
    auto selection_end   = u->__last.end;
    definition_found(target, selection_begin, selection_end);
}

void things::vhdl_definition_provider::identifier_denotes_that_architecture(vhdl::syntax::design_unit* u)
{
    auto target = u->v.architecture.identifier.location;
    auto selection_begin = u->first__.begin;
    auto selection_end   = u->__last.end;
    definition_found(target, selection_begin, selection_end);
}

void things::vhdl_definition_provider::identifier_denotes_that_configuration(vhdl::syntax::design_unit* u)
{
    auto target = u->v.configuration.identifier.location;
    auto selection_begin = u->first__.begin;
    auto selection_end   = u->__last.end;
    definition_found(target, selection_begin, selection_end);
}

void things::vhdl_definition_provider::identifier_denotes_that_package(vhdl::syntax::design_unit* u)
{
    auto target = u->v.package.identifier.location;
    auto selection_begin = u->first__.begin;
    auto selection_end   = u->__last.end;
    definition_found(target, selection_begin, selection_end);
}

void things::vhdl_definition_provider::identifier_denotes_that_package_body(vhdl::syntax::design_unit* u)
{
    auto target = u->v.package_body.identifier.location;
    auto selection_begin = u->first__.begin;
    auto selection_end   = u->__last.end;
    definition_found(target, selection_begin, selection_end);
}

void things::vhdl_definition_provider::identifier_denotes_that_typedecl(vhdl::syntax::declarative_item* d)
{
    auto target = d->v.type.identifier.location;
    auto selection_begin = d->first__.begin;
    auto selection_end   = d->__last.end;
    definition_found(target, selection_begin, selection_end);
}

void things::vhdl_definition_provider::identifier_denotes_that_subtype(vhdl::syntax::declarative_item* d)
{
    auto target = d->v.subtype.identifier.location;
    auto selection_begin = d->first__.begin;
    auto selection_end   = d->__last.end;
    definition_found(target, selection_begin, selection_end);
}

void things::vhdl_definition_provider::identifier_denotes_that_constant(vhdl::syntax::declarative_item* d, int i)
{
    common::location target;
    auto selection_begin = d->first__.begin;
    auto selection_end   = d->__last.end;

    if (d->v_kind == vhdl::syntax::declarative_item::v_::object)
    {
        target = d->v.object.v->identifier[i].location;
    }
    else
    {
        target = d->v.interface.v->identifier[i].location;
    }
    definition_found(target, selection_begin, selection_end);
}

void things::vhdl_definition_provider::identifier_denotes_that_signal(vhdl::syntax::declarative_item* d, int i)
{
    common::location target;
    auto selection_begin = d->first__.begin;
    auto selection_end   = d->__last.end;

    if (d->v_kind == vhdl::syntax::declarative_item::v_::object)
    {
        target = d->v.object.v->identifier[i].location;
    }
    else
    {
        target = d->v.interface.v->identifier[i].location;
    }
    definition_found(target, selection_begin, selection_end);
}

void things::vhdl_definition_provider::identifier_denotes_that_variable(vhdl::syntax::declarative_item* d, int i)
{
    common::location target;
    auto selection_begin = d->first__.begin;
    auto selection_end   = d->__last.end;

    if (d->v_kind == vhdl::syntax::declarative_item::v_::object)
    {
        target = d->v.object.v->identifier[i].location;
    }
    else
    {
        target = d->v.interface.v->identifier[i].location;
    }
    definition_found(target, selection_begin, selection_end);
}

void things::vhdl_definition_provider::identifier_denotes_that_file(vhdl::syntax::declarative_item* d, int i)
{
    common::location target;
    auto selection_begin = d->first__.begin;
    auto selection_end   = d->__last.end;

    if (d->v_kind == vhdl::syntax::declarative_item::v_::object)
    {
        target = d->v.object.v->identifier[i].location;
    }
    else
    {
        target = d->v.interface.v->identifier[i].location;
    }
    definition_found(target, selection_begin, selection_end);
}

void things::vhdl_definition_provider::identifier_denotes_that_alias(vhdl::syntax::declarative_item* d)
{
    auto target = d->v.alias.designator.location;
    auto selection_begin = d->first__.begin;
    auto selection_end   = d->__last.end;
    definition_found(target, selection_begin, selection_end);
}

void things::vhdl_definition_provider::identifier_denotes_that_function(vhdl::syntax::declarative_item* d)
{
    common::location target;
    if (d->v_kind == vhdl::syntax::declarative_item::v_::subprogram)
        target = d->v.subprogram.spec->designator.location;
    else if (d->v_kind == vhdl::syntax::declarative_item::v_::subprogram_body)
        target = d->v.subprogram_body.spec->designator.location;

    auto selection_begin = d->first__.begin;
    auto selection_end   = d->__last.end;
    definition_found(target, selection_begin, selection_end);
}

void things::vhdl_definition_provider::identifier_denotes_that_procedure(vhdl::syntax::declarative_item* d)
{
    common::location target;
    if (d->v_kind == vhdl::syntax::declarative_item::v_::subprogram)
        target = d->v.subprogram.spec->designator.location;
    else if (d->v_kind == vhdl::syntax::declarative_item::v_::subprogram_body)
        target = d->v.subprogram_body.spec->designator.location;
    else
        return;

    auto selection_begin = d->first__.begin;
    auto selection_end   = d->__last.end;
    definition_found(target, selection_begin, selection_end);
}

void things::vhdl_definition_provider::identifier_denotes_that_component(vhdl::syntax::declarative_item* d)
{
    auto target = d->v.component.identifier.location;
    auto selection_begin = d->first__.begin;
    auto selection_end   = d->__last.end;
    definition_found(target, selection_begin, selection_end);
}

void things::vhdl_definition_provider::identifier_denotes_that_element(vhdl::syntax::element_declaration* e, int i)
{
    auto target = e->identifier[i].location;
    auto selection_begin = e->identifier[i].location.begin;
    auto selection_end   = e->identifier[i].location.end;
    definition_found(target, selection_begin, selection_end);
}

void things::vhdl_definition_provider::identifier_denotes_that_library(vhdl::syntax::context_item* c, int i)
{
    auto target = c->v.library_clause.names[i].location;
    auto selection_begin = c->v.library_clause.names[i].location.begin;
    auto selection_end   = c->v.library_clause.names[i].location.end;
    definition_found(target, selection_begin, selection_end);
}

void things::vhdl_definition_provider::identifier_denotes_that_literal(vhdl::syntax::type_definition* t, int i)
{
    auto target = t->v.enumeration.literals[i].location;
    auto selection_begin = t->v.enumeration.literals[i].location.begin;
    auto selection_end   = t->v.enumeration.literals[i].location.end;
    definition_found(target, selection_begin, selection_end);
}

void things::vhdl_definition_provider::identifier_denotes_these_entities(std::string_view name, std::vector<vhdl::node::named_entity*> denotes)
{
    if (found)
        return;

    if (denotes.size() == 0)
        return;

    w->StartArray();
    for (auto it: denotes) {
        std::string target_file;

        switch (it->get_kind()) {
        case vhdl::node::kind::entity:
            identifier_denotes_that_entity(it->as_entity()->u);
            break;
        case vhdl::node::kind::architecture:
            identifier_denotes_that_architecture(it->as_architecture()->u);
            break;
        case vhdl::node::kind::configuration:
            identifier_denotes_that_configuration(it->as_configuration()->u);
            break;
        case vhdl::node::kind::package:
            identifier_denotes_that_package(it->as_package()->u);
            break;
        case vhdl::node::kind::package_body:
            identifier_denotes_that_package_body(it->as_package_body()->u);
            break;
        case vhdl::node::kind::typedecl:
            identifier_denotes_that_typedecl(it->as_typedecl()->d);
            break;
        case vhdl::node::kind::subtype:
            identifier_denotes_that_subtype(it->as_subtype()->d);
            break;
        case vhdl::node::kind::constant:
            identifier_denotes_that_constant(it->as_constant()->d, it->as_constant()->index);
            break;
        case vhdl::node::kind::signal:
            identifier_denotes_that_signal(it->as_signal()->d, it->as_signal()->index);
            break;
        case vhdl::node::kind::variable:
            identifier_denotes_that_variable(it->as_variable()->d, it->as_variable()->index);
            break;
        case vhdl::node::kind::file:
            identifier_denotes_that_file(it->as_file()->d, it->as_file()->index);
            break;
        case vhdl::node::kind::alias:
            identifier_denotes_that_alias(it->as_alias()->d);
            break;
        case vhdl::node::kind::function:
            identifier_denotes_that_function(it->as_function()->d);
            break;
        case vhdl::node::kind::procedure:
            identifier_denotes_that_procedure(it->as_procedure()->d);
            break;
        case vhdl::node::kind::component:
            identifier_denotes_that_component(it->as_component()->d);
            break;
        case vhdl::node::kind::element:
            identifier_denotes_that_element(it->as_element()->e, it->as_element()->index);
            break;
        case vhdl::node::kind::library:
            identifier_denotes_that_library(it->as_library()->c, it->as_library()->index);
            break;
        case vhdl::node::kind::literal:
            identifier_denotes_that_literal(it->as_literal()->t, it->as_literal()->index);
            break;
        default:
            break;
        }
    }
    w->EndArray();
}

things::vhdl_definition_provider::vhdl_definition_provider(rapidjson::Writer<rapidjson::StringBuffer>* w, bool& j, common::position p)
: w(w), json_populated(j), position(p), found(false)
{

}

things::vhdl_definition_provider::~vhdl_definition_provider()
{

}

bool things::vhdl_definition_provider::visit(vhdl::syntax::component_specification* c)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::configuration_item* c)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::block_configuration* b)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::component_configuration* c)
{
    if (found)
        return false;

    if (c->for__ > position || c->__end < position)
        return false;

    return true;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::subprogram* s)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::signature* s)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::type_definition* t)
{
    switch (t->v_kind) {
    case vhdl::syntax::type_definition::v_::enumeration: {
        auto i = 0;
        for (auto it: t->v.enumeration.literals) { if (found) break;
            if (it.location == position)
                identifier_denotes_that_literal(t, i);
            i++;
        }
    }   break;
    
    default:
        break;
    }
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::subtype* s)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::index_subtype* i)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::range* r)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::constraint* c)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::unit_declaration* u)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::physical_literal* p)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::element_declaration* e)
{
    auto i = 0;
    for (auto it: e->identifier) {
        if (found)
            break;
        if (it.location == position)
            identifier_denotes_that_element(e, i);
        ++i;
    }
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::declarative_item* d)
{
    if (found)
        return false;

    if (d->first__ > position || d->__last < position)
        return false;

    switch (d->v_kind) {
    case vhdl::syntax::declarative_item::v_::type:
        if (d->v.type.identifier.location == position)
            identifier_denotes_that_typedecl(d);

        break;
    case vhdl::syntax::declarative_item::v_::subtype:
        if (d->v.subtype.identifier.location == position)
            identifier_denotes_that_subtype(d);

        break;
    case vhdl::syntax::declarative_item::v_::object:
        d->v.object.v->traverse(*this);
        break;

    case vhdl::syntax::declarative_item::v_::interface:
        d->v.interface.v->traverse(*this);
        break;

    case vhdl::syntax::declarative_item::v_::alias:
        if (d->v.alias.designator.location == position)
            identifier_denotes_that_alias(d);

        break;
    case vhdl::syntax::declarative_item::v_::attribute:
        break;

    case vhdl::syntax::declarative_item::v_::component: {
        if (d->v.component.identifier.location == position)
            identifier_denotes_that_component(d);

        if (d->v.component.gl__ < position && d->v.component.__gr > position)
            for (auto it: d->v.component.gens) { if (found) break;
                it->traverse(*this);
        }

        if (d->v.component.pl__ < position && d->v.component.__pr > position)
            for (auto it: d->v.component.ports) { if (found) break;
                it->traverse(*this);
        }
        return false;
    }
    case vhdl::syntax::declarative_item::v_::attr_spec:
        break;
    case vhdl::syntax::declarative_item::v_::subprogram:{
        if (d->v.subprogram.spec->v_kind == vhdl::syntax::subprogram::v_::procedure)
        {
            if (d->v.subprogram.spec->designator.location == position)
                identifier_denotes_that_procedure(d);
        }
        else
        {
            if (d->v.subprogram.spec->designator.location == position)
                identifier_denotes_that_function(d);

            if (d->v.subprogram_body.spec->v.function.result)
                d->v.subprogram_body.spec->v.function.result->traverse(*this);
        }

        if (d->v.subprogram.pl__ < position && d->v.subprogram.__pr > position)
            for (auto it: d->v.subprogram.spec->parameters) { if (found) break;
                it->traverse(*this);
        }
        return false;
    }
    case vhdl::syntax::declarative_item::v_::subprogram_body: {
        if (d->v.subprogram_body.spec->v_kind == vhdl::syntax::subprogram::v_::procedure)
        {
            if (d->v.subprogram_body.spec->designator.location == position)
                identifier_denotes_that_procedure(d);
        }
        else
        {
            if (d->v.subprogram_body.spec->designator.location == position)
                identifier_denotes_that_function(d);

            if (d->v.subprogram_body.spec->v.function.result)
                d->v.subprogram_body.spec->v.function.result->traverse(*this);
        }

        if (d->v.subprogram_body.pl__ < position && d->v.subprogram_body.__pr > position)
            for (auto it: d->v.subprogram_body.spec->parameters) { if (found) break;
                it->traverse(*this);
        }

        if (d->v.subprogram_body.is__ < position && d->v.subprogram_body.__begin__ > position)
            for (auto it: d->v.subprogram_body.decls) { if (found) break;
                it->traverse(*this);
        }

        if (d->v.subprogram_body.__begin__ < position && d->v.subprogram_body.__end > position)
            for (auto it: d->v.subprogram_body.stmts) { if (found) break;
                it->traverse(*this);
        }

        return false;
    }
    case vhdl::syntax::declarative_item::v_::config_spec:
        break;
    case vhdl::syntax::declarative_item::v_::use:
        break;

    default:
        break;
    }

    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::object* i)
{
    if (found)
        return false;

    if (i->decl->first__ > position || i->decl->__last < position)
        return false;

    auto index = 0;
    switch (i->v_kind) {
    case vhdl::syntax::object::v_::constant:
        for (auto it: i->identifier) { if (found) break;
            if (it.location == position)
                identifier_denotes_that_constant(i->decl, index);
            ++index;
        }
        break;

    case vhdl::syntax::object::v_::signal:
        for (auto it: i->identifier) { if (found) break;
            if (it.location == position)
                identifier_denotes_that_signal(i->decl, index);
            ++index;
        }
        break;

    case vhdl::syntax::object::v_::variable:
        for (auto it: i->identifier) { if (found) break;
            if (it.location == position)
                identifier_denotes_that_variable(i->decl, index);
            ++index;
        }
        break;

    case vhdl::syntax::object::v_::file:
        for (auto it: i->identifier) { if (found) break;
            if (it.location == position)
                identifier_denotes_that_file(i->decl, index);
            ++index;
        }
        break;

    default:
        break;
    }
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::interface* i)
{
    if (found)
        return false;

    if (i->decl->first__ > position || i->decl->__last < position)
        return false;

    auto index = 0;
    switch (i->v_kind) {
    case vhdl::syntax::interface::v_::constant:
        for (auto it: i->identifier) { if (found) break;
            if (it.location == position)
                identifier_denotes_that_constant(i->decl, index);
            ++index;
        }
        break;

    case vhdl::syntax::interface::v_::signal:
        for (auto it: i->identifier) { if (found) break;
            if (it.location == position)
                identifier_denotes_that_signal(i->decl, index);
            ++index;
        }
        break;

    case vhdl::syntax::interface::v_::variable:
        for (auto it: i->identifier) { if (found) break;
            if (it.location == position)
                identifier_denotes_that_variable(i->decl, index);
            ++index;
        }
        break;

    case vhdl::syntax::interface::v_::file:
        for (auto it: i->identifier) { if (found) break;
            if (it.location == position)
                identifier_denotes_that_file(i->decl, index);
            ++index;
        }
        break;

    default:
        break;
    }
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::association_element* a)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::entity_specification* e)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::configuration_specification* c)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::binding_indication* b)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::name* n)
{
    if (found)
        return false;

    switch (n->v_kind) {
    case vhdl::syntax::name::v_::simple:
        if (n->v.simple.identifier.location == position)
            identifier_denotes_these_entities(n->v.simple.identifier.value, n->denotes);
        break;
    case vhdl::syntax::name::v_::selected:
        if (n->v.selected.identifier.location == position)
            identifier_denotes_these_entities(n->v.selected.identifier.value, n->denotes);
        break;
    case vhdl::syntax::name::v_::slice:
    case vhdl::syntax::name::v_::fcall:
    case vhdl::syntax::name::v_::ambiguous:
    case vhdl::syntax::name::v_::qualified:
    case vhdl::syntax::name::v_::attribute:
    case vhdl::syntax::name::v_::signature:
        break;
    
    default:
        break;
    }
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::expression* e)
{
    if (found)
        return false;

    switch (e->v_kind) {
    case vhdl::syntax::expression::v_::literal:
        if (e->v.literal.kind != vhdl::syntax::literal_kind::enumeration)
            break;

        if (e->v.literal.token.location == position)
            identifier_denotes_these_entities(e->v.literal.token.value, {});
        break;

    case vhdl::syntax::expression::v_::physical:
        if (e->v.physical.token.location == position)
            identifier_denotes_these_entities(e->v.physical.token.value, {});
        break;

    default:
        break;
    }
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::element_association* e)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::choice* c)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::sequential_statement* s)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::target* t)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::waveform_element* w)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::case_statement_alternative* c)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::concurrent_statement* c)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::con_waveform* c)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::sel_waveform* s)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::instantiated_unit* i)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::design_file* d)
{
    return !found;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::design_unit* d)
{
    if (found)
        return false;

    for (auto it: d->contexts)
        it->traverse(*this);

    if (d->first__ > position || d->__last < position)
        return false;

    switch (d->v_kind) {
    case vhdl::syntax::design_unit::v_::entity: {
        auto& entity = d->v.entity;
        if (entity.identifier.location == position)
            identifier_denotes_that_entity(d);

        if (entity.gl__ < position && entity.__gr > position)
            for (auto it: entity.gens) { if (found) break;
                it->traverse(*this);
        }

        if (entity.pl__ < position && entity.__pr > position)
            for (auto it: entity.ports) { if (found) break;
                it->traverse(*this);
        }
        break;
    }
    case vhdl::syntax::design_unit::v_::architecture: {
        auto& architecture = d->v.architecture;
        if (architecture.identifier.location == position)
            identifier_denotes_that_architecture(d);

        if (d->first__ < position && architecture.is__ > position)
            architecture.entity->traverse(*this);

        if (architecture.is__ < position && architecture.__begin__ > position)
            for (auto it: architecture.decls) { if (found) break;
                it->traverse(*this);
        }

        if (architecture.__begin__ < position && architecture.__end > position)
            for (auto it: architecture.stmts) { if (found) break;
                it->traverse(*this);
        }
        break;
    }
    case vhdl::syntax::design_unit::v_::package: {
        auto& package = d->v.package;
        if (package.identifier.location == position)
            identifier_denotes_that_package(d);

        if (package.is__ < position && package.__end > position)
            for (auto it: package.decls) { if (found) break;
                it->traverse(*this);
        }
        break;
    }
    case vhdl::syntax::design_unit::v_::package_body: {
        auto& package = d->v.package_body;
        if (package.identifier.location == position)
            identifier_denotes_that_package_body(d);

        if (package.is__ < position && package.__end > position)
            for (auto it: package.decls) { if (found) break;
                it->traverse(*this);
        }
        break;
    }
    case vhdl::syntax::design_unit::v_::configuration: {
        auto& configuration = d->v.configuration;
        if (configuration.identifier.location == position)
            identifier_denotes_that_configuration(d);

        break;
    }
    default:
        break;
    };

    return false;
}

bool things::vhdl_definition_provider::visit(vhdl::syntax::context_item* c)
{
    switch (c->v_kind) {
    case vhdl::syntax::context_item::v_::library_clause: {
        auto& l = c->v.library_clause;
        auto i = 0;
        for (auto n: l.names)
        {
            if (n.location == position)
                identifier_denotes_that_library(c, i);
            ++i;
        }
    }   break;
    
    default:
        break;
    }
    return !found;
}
