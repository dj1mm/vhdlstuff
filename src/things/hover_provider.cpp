
#include "hover_provider.h"

things::vhdl_hover_provider::vhdl_hover_provider(rapidjson::Writer<rapidjson::StringBuffer>* w, bool& f, common::position p)
: w(w), found(f), position(p)
{

}

things::vhdl_hover_provider::~vhdl_hover_provider()
{

}

void things::vhdl_hover_provider::hover(vhdl::node::named_entity* n)
{
    switch (n->get_kind()) {
    case vhdl::node::kind::record:
    case vhdl::node::kind::named_entity:
    case vhdl::node::kind::typedecl:
        hover(n->as_typedecl()->d);
        break;

    case vhdl::node::kind::subtype:
        hover(n->as_subtype()->d);
        break;

    case vhdl::node::kind::constant:
        hover(n->as_constant()->d->v.object.v, n->as_constant()->index);
        break;

    case vhdl::node::kind::signal:
        hover(n->as_signal()->d->v.object.v, n->as_signal()->index);
        break;

    case vhdl::node::kind::variable:
        hover(n->as_variable()->d->v.object.v, n->as_variable()->index);
        break;

    case vhdl::node::kind::file:
        hover(n->as_file()->d->v.object.v, n->as_file()->index);
        break;

    case vhdl::node::kind::alias:
        hover(n->as_alias()->d);
        break;

    case vhdl::node::kind::function:
        hover(n->as_function()->d);
        break;

    case vhdl::node::kind::procedure:
        hover(n->as_procedure()->d);
        break;

    case vhdl::node::kind::component:
        hover(n->as_component()->d);
        break;

    case vhdl::node::kind::entity:
        hover(n->as_entity()->u);
        break;

    case vhdl::node::kind::architecture:
        hover(n->as_architecture()->u);
        break;

    case vhdl::node::kind::configuration:
        hover(n->as_configuration()->u);
        break;

    case vhdl::node::kind::package:
        hover(n->as_package()->u);
        break;

    case vhdl::node::kind::package_body:
        hover(n->as_package_body()->u);
        break;

    case vhdl::node::kind::library:
        hover(n->as_library()->c, n->as_library()->index);
        break;

    case vhdl::node::kind::literal:
        hover(n->as_literal()->t, n->as_literal()->index);
        break;

    case vhdl::node::kind::element:
        hover(n->as_element()->e, n->as_element()->index);
        break;

    case vhdl::node::kind::library_unit:
    case vhdl::node::kind::direct_visibility:
    case vhdl::node::kind::declarative_region:
    default:
        break;
    }
}

void things::vhdl_hover_provider::hover(vhdl::syntax::type_definition* t, int i)
{
    switch (t->v_kind) {
    case vhdl::syntax::type_definition::v_::enumeration:
        hover("enumeration literal {}.{};", t->identifier.value, t->v.enumeration.literals[i].value);
        break;
    
    default:
        break;
    }
}

void things::vhdl_hover_provider::hover(vhdl::syntax::element_declaration* e, int i)
{
    hover("record element {};", e->identifier[i].value);
}

void things::vhdl_hover_provider::hover(vhdl::syntax::declarative_item* d)
{
    switch (d->v_kind) {
    case vhdl::syntax::declarative_item::v_::type:
        hover("type {};", d->v.type.identifier.value);
        break;
    case vhdl::syntax::declarative_item::v_::subtype:
        hover("subtype {};", d->v.subtype.identifier.value);
        break;
    case vhdl::syntax::declarative_item::v_::object:
        break;

    case vhdl::syntax::declarative_item::v_::interface:
        break;

    case vhdl::syntax::declarative_item::v_::alias:
        hover("subtype {};", d->v.alias.designator.value);
        break;
    case vhdl::syntax::declarative_item::v_::attribute:
        hover("attribute {};", d->v.attribute.identifier.value);
        break;
    case vhdl::syntax::declarative_item::v_::component: {
        hover("component {};", d->v.component.identifier.value);
        break;
    }
    case vhdl::syntax::declarative_item::v_::attr_spec:
        break;
    case vhdl::syntax::declarative_item::v_::subprogram:{
        if (d->v.subprogram.spec->v_kind == vhdl::syntax::subprogram::v_::procedure)
        {
            hover("procedure {};", d->v.subprogram.spec->designator.value);
        }
        else
        {
            hover("function {};", d->v.subprogram.spec->designator.value);
        }
        break;
    }
    case vhdl::syntax::declarative_item::v_::subprogram_body: {
        if (d->v.subprogram_body.spec->v_kind == vhdl::syntax::subprogram::v_::procedure)
        {
            hover("procedure {};", d->v.subprogram_body.spec->designator.value);
        }
        else
        {
            hover("function {};", d->v.subprogram_body.spec->designator.value);
        }
        break;
    }
    case vhdl::syntax::declarative_item::v_::config_spec:
        break;
    case vhdl::syntax::declarative_item::v_::use:
        break;

    default:
        break;
    }
}

void things::vhdl_hover_provider::hover(vhdl::syntax::object* i, int index)
{
    switch (i->v_kind) {
    case vhdl::syntax::object::v_::constant:
        hover("constant {};", i->identifier[index].value);
        break;

    case vhdl::syntax::object::v_::signal:
        hover("signal {};", i->identifier[index].value);
        break;

    case vhdl::syntax::object::v_::variable:
        hover("variable {};", i->identifier[index].value);
        break;

    case vhdl::syntax::object::v_::file:
        hover("file {};", i->identifier[index].value);
        break;

    default:
        break;
    }
}

void things::vhdl_hover_provider::hover(vhdl::syntax::interface* i, int index)
{
    switch (i->v_kind) {
    case vhdl::syntax::interface::v_::constant:
        hover("constant {};", i->identifier[index].value);
        break;

    case vhdl::syntax::interface::v_::signal:
        hover("signal {};", i->identifier[index].value);
        break;

    case vhdl::syntax::interface::v_::variable:
        hover("variable {};", i->identifier[index].value);
        break;

    case vhdl::syntax::interface::v_::file:
        hover("file {};", i->identifier[index].value);
        break;

    default:
        break;
    }
}

void things::vhdl_hover_provider::hover(vhdl::syntax::design_unit* d)
{
    switch (d->v_kind) {
    case vhdl::syntax::design_unit::v_::entity: {
        auto& entity = d->v.entity;
        hover("entity {}", entity.identifier.value);
        break;
    }
    case vhdl::syntax::design_unit::v_::architecture: {
        auto& architecture = d->v.architecture;
        hover("architecture {}", architecture.identifier.value);
        break;
    }
    case vhdl::syntax::design_unit::v_::package: {
        auto& package = d->v.package;
        hover("package {}", package.identifier.value);
        break;
    }
    case vhdl::syntax::design_unit::v_::package_body: {
        auto& package = d->v.package_body;
        hover("package body {}", package.identifier.value);
        break;
    }
    case vhdl::syntax::design_unit::v_::configuration: {
        auto& configuration = d->v.configuration;
        hover("configuration {}", configuration.identifier.value);
        break;
    }
    default:
        break;
    };
}

void things::vhdl_hover_provider::hover(vhdl::syntax::context_item* c, int i)
{
    switch (c->v_kind) {
    case vhdl::syntax::context_item::v_::library_clause: {
        hover("library {}", c->v.library_clause.names[i].value);
    }   break;
    
    default:
        break;
    }
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::component_specification* c)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::configuration_item* c)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::block_configuration* b)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::component_configuration* c)
{
    if (found)
        return false;

    if (c->for__ > position || c->__end < position)
        return false;

    return true;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::subprogram* s)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::signature* s)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::type_definition* t)
{
    switch (t->v_kind) {
    case vhdl::syntax::type_definition::v_::enumeration: {
        auto i = 0;
        for (auto it: t->v.enumeration.literals) { if (found) break;
            if (it.location == position)
                hover(t, i);
            ++i;
        }
    }   break;
    
    default:
        break;
    }
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::subtype* s)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::index_subtype* i)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::range* r)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::constraint* c)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::unit_declaration* u)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::physical_literal* p)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::element_declaration* e)
{
    auto i = 0;
    for (auto it: e->identifier) {
        if (found)
            break;
        if (it.location == position)
            hover(e, i);
        ++i;
    }
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::declarative_item* d)
{
    if (found)
        return false;

    if (d->first__ > position || d->__last < position)
        return false;

    switch (d->v_kind) {
    case vhdl::syntax::declarative_item::v_::type:
        if (d->v.type.identifier.location == position)
            hover(d);

        break;
    case vhdl::syntax::declarative_item::v_::subtype:
        if (d->v.subtype.identifier.location == position)
            hover(d);

        break;
    case vhdl::syntax::declarative_item::v_::object:
        d->v.object.v->traverse(*this);
        break;

    case vhdl::syntax::declarative_item::v_::interface:
        d->v.interface.v->traverse(*this);
        break;

    case vhdl::syntax::declarative_item::v_::alias:
        if (d->v.alias.designator.location == position)
            hover(d);

        break;
    case vhdl::syntax::declarative_item::v_::attribute:
        if (d->v.attribute.identifier.location == position)
            hover(d);

        break;
    case vhdl::syntax::declarative_item::v_::component: {
        if (d->v.component.identifier.location == position)
            hover(d);

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
                hover(d);
        }
        else
        {
            if (d->v.subprogram.spec->designator.location == position)
                hover(d);

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
                hover(d);
        }
        else
        {
            if (d->v.subprogram_body.spec->designator.location == position)
                hover(d);

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

bool things::vhdl_hover_provider::visit(vhdl::syntax::object* i)
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
                hover(i, index);
            ++index;
        }
        break;

    case vhdl::syntax::object::v_::signal:
        for (auto it: i->identifier) { if (found) break;
            if (it.location == position)
                hover(i, index);
            ++index;
        }
        break;

    case vhdl::syntax::object::v_::variable:
        for (auto it: i->identifier) { if (found) break;
            if (it.location == position)
                hover(i, index);
            ++index;
        }
        break;

    case vhdl::syntax::object::v_::file:
        for (auto it: i->identifier) { if (found) break;
            if (it.location == position)
                hover(i, index);
            ++index;
        }
        break;

    default:
        break;
    }
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::interface* i)
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
                hover(i, index);
            ++index;
        }
        break;

    case vhdl::syntax::interface::v_::signal:
        for (auto it: i->identifier) { if (found) break;
            if (it.location == position)
                hover(i, index);
            ++index;
        }
        break;

    case vhdl::syntax::interface::v_::variable:
        for (auto it: i->identifier) { if (found) break;
            if (it.location == position)
                hover(i, index);
            ++index;
        }
        break;

    case vhdl::syntax::interface::v_::file:
        for (auto it: i->identifier) { if (found) break;
            if (it.location == position)
                hover(i, index);
            ++index;
        }
        break;

    default:
        break;
    }
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::association_element* a)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::entity_specification* e)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::configuration_specification* c)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::binding_indication* b)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::name* n)
{
    if (found)
        return false;

    switch (n->v_kind) {
    case vhdl::syntax::name::v_::simple:
        if (n->v.simple.identifier.location == position)
            hover_identifier_denotes_entity(n->v.simple.identifier.value, n->denotes);
        break;
    case vhdl::syntax::name::v_::selected:
        if (n->v.selected.identifier.location == position)
            hover_identifier_denotes_entity(n->v.selected.identifier.value, n->denotes);
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

bool things::vhdl_hover_provider::visit(vhdl::syntax::expression* e)
{
    if (found)
        return false;

    switch (e->v_kind) {
    case vhdl::syntax::expression::v_::literal:
        if (e->v.literal.kind != vhdl::syntax::literal_kind::enumeration)
            break;

        if (e->v.literal.token.location == position)
            hover_identifier_denotes_entity(e->v.literal.token.value, {});
        break;

    case vhdl::syntax::expression::v_::physical:
        if (e->v.physical.token.location == position)
            hover_identifier_denotes_entity(e->v.physical.token.value, {});
        break;

    default:
        break;
    }
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::element_association* e)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::choice* c)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::sequential_statement* s)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::target* t)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::waveform_element* w)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::case_statement_alternative* c)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::concurrent_statement* c)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::con_waveform* c)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::sel_waveform* s)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::instantiated_unit* i)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::design_file* d)
{
    return !found;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::design_unit* d)
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
            hover(d);

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
            hover(d);

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
            hover(d);

        if (package.is__ < position && package.__end > position)
            for (auto it: package.decls) { if (found) break;
                it->traverse(*this);
        }
        break;
    }
    case vhdl::syntax::design_unit::v_::package_body: {
        auto& package = d->v.package_body;
        if (package.identifier.location == position)
            hover(d);

        if (package.is__ < position && package.__end > position)
            for (auto it: package.decls) { if (found) break;
                it->traverse(*this);
        }
        break;
    }
    case vhdl::syntax::design_unit::v_::configuration: {
        auto& configuration = d->v.configuration;
        if (configuration.identifier.location == position)
            hover(d);

        break;
    }
    default:
        break;
    };

    return false;
}

bool things::vhdl_hover_provider::visit(vhdl::syntax::context_item* c)
{
    switch (c->v_kind) {
    case vhdl::syntax::context_item::v_::library_clause: {
        auto& l = c->v.library_clause;
        auto i = 0;
        for (auto n: l.names)
        {
            if (n.location == position)
                hover(c, i);
            i++;
        }
    }   break;
    
    default:
        break;
    }
    return !found;
}

bool things::sv_hover_provider(rapidjson::Writer<rapidjson::StringBuffer>* w,
                               slang::ast::Compilation& compilation,
                               slang::SourceManager& sm, std::string file,
                               lsp::position position)
{
    for (auto candidate : compilation.getSyntaxTrees())
    {
        find_declaration_under_position visitor(sm, file, position);
        candidate->root().visit(visitor);
        if (visitor.node != nullptr)
        {
            slang::syntax::SyntaxPrinter printer(sm);
            printer.print(*visitor.node);

            w->StartObject();
            w->Key("kind");
            w->String("markdown");
            w->Key("contents");
            w->String("```sv\n" + printer.str() + "\n```");
            w->EndObject();
            return true;
        }
    }
    return false;
}

things::find_declaration_under_position::find_declaration_under_position(
    slang::SourceManager& sm, std::string file, lsp::position position)
    : sm(sm), position(position), node(nullptr), file(file)
{
}

bool things::find_declaration_under_position::position_is_within_source_range(
    const slang::SourceRange range)
{
    if (std::filesystem::canonical(sm.getFileName(range.start())) != file)
        false;

    auto this_line = position.line + 1;
    auto this_coln = position.character + 1;
    auto lhs_line = sm.getLineNumber(range.start());
    auto lhs_coln = sm.getColumnNumber(range.start());
    auto rhs_line   = sm.getLineNumber(range.end());
    auto rhs_coln   = sm.getColumnNumber(range.end());

    if (this_line < lhs_line || (this_line == lhs_line && this_coln < lhs_coln))
        return false;
    if (this_line > rhs_line || (this_line == rhs_line && this_coln > rhs_coln))
        return false;
    return true;
}

void things::find_declaration_under_position::handle(const slang::syntax::DataDeclarationSyntax& syntax)
{
    for (auto declarator: syntax.declarators)
        if (position_is_within_source_range(declarator->name.range()))
            node = &syntax;
}

void things::find_declaration_under_position::handle(const slang::syntax::TypedefDeclarationSyntax& syntax)
{
    if (position_is_within_source_range(syntax.name.range()))
        node = &syntax;
}

void things::find_declaration_under_position::handle(const slang::syntax::ForwardTypedefDeclarationSyntax& syntax)
{
    if (position_is_within_source_range(syntax.name.range()))
        node = &syntax;
}

void things::find_declaration_under_position::handle(const slang::syntax::ForwardInterfaceClassTypedefDeclarationSyntax& syntax)
{
    if (position_is_within_source_range(syntax.name.range()))
        node = &syntax;
}

void things::find_declaration_under_position::handle(const slang::syntax::NetDeclarationSyntax& syntax)
{
    for (auto declarator: syntax.declarators)
        if (position_is_within_source_range(declarator->name.range()))
            node = &syntax;
}

void things::find_declaration_under_position::handle(const slang::syntax::UserDefinedNetDeclarationSyntax& syntax)
{
    for (auto declarator: syntax.declarators)
        if (position_is_within_source_range(declarator->name.range()))
            node = &syntax;
}

void things::find_declaration_under_position::handle(const slang::syntax::NetTypeDeclarationSyntax& syntax)
{
    if (position_is_within_source_range(syntax.name.range()))
        node = &syntax;
}

void things::find_declaration_under_position::handle(const slang::syntax::PackageImportDeclarationSyntax& syntax)
{

}

void things::find_declaration_under_position::handle(const slang::syntax::ParameterDeclarationSyntax& syntax)
{
    for (auto declarator: syntax.declarators)
        if (position_is_within_source_range(declarator->name.range()))
            node = &syntax;
}

void things::find_declaration_under_position::handle(const slang::syntax::TypeParameterDeclarationSyntax& syntax)
{
    for (auto declarator: syntax.declarators)
        if (position_is_within_source_range(declarator->name.range()))
            node = &syntax;
}

void things::find_declaration_under_position::handle(const slang::syntax::PortDeclarationSyntax& syntax)
{
    for (auto declarator: syntax.declarators)
        if (position_is_within_source_range(declarator->name.range()))
            node = &syntax;
}

void things::find_declaration_under_position::handle(const slang::syntax::ImplicitAnsiPortSyntax& syntax)
{
    if (position_is_within_source_range(syntax.declarator->name.range()))
        node = &syntax;
}

void things::find_declaration_under_position::handle(const slang::syntax::ExplicitAnsiPortSyntax& syntax)
{
    if (position_is_within_source_range(syntax.name.range()))
        node = &syntax;
}

void things::find_declaration_under_position::handle(const slang::syntax::GenvarDeclarationSyntax& syntax)
{
    for (auto identifier: syntax.identifiers)
        if (position_is_within_source_range(identifier->identifier.range()))
            node = &syntax;
}

void things::find_declaration_under_position::handle(const slang::syntax::ForVariableDeclarationSyntax& syntax)
{

}

void things::find_declaration_under_position::handle(const slang::syntax::ModuleDeclarationSyntax& syntax)
{
    if (!position_is_within_source_range(syntax.sourceRange()))
        return;
    if (position_is_within_source_range(syntax.header->name.range()))
        node = &syntax;
    visitDefault(syntax);
}

void things::find_declaration_under_position::handle(const slang::syntax::TimeUnitsDeclarationSyntax& syntax)
{
    if (position_is_within_source_range(syntax.time.range()))
        node = &syntax;
}

void things::find_declaration_under_position::handle(const slang::syntax::FunctionDeclarationSyntax& syntax)
{
    if (position_is_within_source_range(syntax.prototype->name->getLastToken().range()))
        node = &syntax;
}

void things::find_declaration_under_position::handle(const slang::syntax::LetDeclarationSyntax& syntax)
{
    if (position_is_within_source_range(syntax.identifier.range()))
        node = &syntax;
}

void things::find_declaration_under_position::handle(const slang::syntax::PropertyDeclarationSyntax& syntax)
{

}

void things::find_declaration_under_position::handle(const slang::syntax::SequenceDeclarationSyntax& syntax)
{

}

void things::find_declaration_under_position::handle(const slang::syntax::ClassDeclarationSyntax& syntax)
{
    if (position_is_within_source_range(syntax.name.range()))
        node = &syntax;
}

void things::find_declaration_under_position::handle(const slang::syntax::ClassPropertyDeclarationSyntax& syntax)
{

}

void things::find_declaration_under_position::handle(const slang::syntax::ClassMethodPrototypeSyntax& syntax)
{

}

void things::find_declaration_under_position::handle(const slang::syntax::ModportDeclarationSyntax& syntax)
{

}

void things::find_declaration_under_position::handle(const slang::syntax::ClockingDeclarationSyntax& syntax)
{

}

void things::find_declaration_under_position::handle(const slang::syntax::UdpOutputPortDeclSyntax& syntax)
{

}

void things::find_declaration_under_position::handle(const slang::syntax::UdpInputPortDeclSyntax& syntax)
{

}

void things::find_declaration_under_position::handle(const slang::syntax::UdpDeclarationSyntax& syntax)
{

}

void things::find_declaration_under_position::handle(const slang::syntax::SpecparamDeclarationSyntax& syntax)
{

}

void things::find_declaration_under_position::handle(const slang::syntax::PathDeclarationSyntax& syntax)
{

}

void things::find_declaration_under_position::handle(const slang::syntax::ConditionalPathDeclarationSyntax& syntax)
{

}

void things::find_declaration_under_position::handle(const slang::syntax::IfNonePathDeclarationSyntax& syntax)
{

}

void things::find_declaration_under_position::handle(const slang::syntax::PulseStyleDeclarationSyntax& syntax)
{

}

void things::find_declaration_under_position::handle(const slang::syntax::ConstraintDeclarationSyntax& syntax)
{

}

void things::find_declaration_under_position::handle(const slang::syntax::CovergroupDeclarationSyntax& syntax)
{

}

void things::find_declaration_under_position::handle(const slang::syntax::ProceduralBlockSyntax& syntax)
{

}
