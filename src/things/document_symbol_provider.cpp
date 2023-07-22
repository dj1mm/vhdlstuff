
#include "document_symbol_provider.h"


void things::vhdl_document_symbol_provider::symbol(vhdl::token identifier,
                                                   common::location begin, 
                                                   common::location end,
                                                   symbol_kind kind,
                                                   std::optional<std::string> detail)
{
    w->StartObject();
    w->Key("name");     w->String(identifier.to_string());
    w->Key("kind");     w->Int(static_cast<int>(kind));
    w->Key("range");
        w->StartObject();
        w->Key("start");
            w->StartObject();
            w->Key("line");      w->Int(begin.begin.line-1);
            w->Key("character"); w->Int(begin.begin.column-1);
            w->EndObject();
        w->Key("end");
            w->StartObject();
            w->Key("line");      w->Int(end.end.line-1);
            w->Key("character"); w->Int(end.end.column-1);
            w->EndObject();
        w->EndObject();
    w->Key("selectionRange");
        w->StartObject();
        w->Key("start");
            w->StartObject();
            w->Key("line");      w->Int(identifier.location.begin.line-1);
            w->Key("character"); w->Int(identifier.location.begin.column-1);
            w->EndObject();
        w->Key("end");
            w->StartObject();
            w->Key("line");      w->Int(identifier.location.end.line-1);
            w->Key("character"); w->Int(identifier.location.end.column-1);
            w->EndObject();
        w->EndObject();
    if (detail) { w->Key("detail");   w->String(*detail); }
    w->Key("children");
        w->StartArray();
        // fill in the blanks
}


void things::vhdl_document_symbol_provider::symbol(std::string identifier,
                                                   common::location range, 
                                                   common::location begin, 
                                                   common::location end,
                                                   symbol_kind kind,
                                                   std::optional<std::string> detail)
{
    w->StartObject();
    w->Key("name");     w->String(identifier);
    w->Key("kind");     w->Int(static_cast<int>(kind));
    w->Key("range");
        w->StartObject();
        w->Key("start");
            w->StartObject();
            w->Key("line");      w->Int(begin.begin.line-1);
            w->Key("character"); w->Int(begin.begin.column-1);
            w->EndObject();
        w->Key("end");
            w->StartObject();
            w->Key("line");      w->Int(end.end.line-1);
            w->Key("character"); w->Int(end.end.column-1);
            w->EndObject();
        w->EndObject();
    w->Key("selectionRange");
        w->StartObject();
        w->Key("start");
            w->StartObject();
            w->Key("line");      w->Int(range.begin.line-1);
            w->Key("character"); w->Int(range.begin.column-1);
            w->EndObject();
        w->Key("end");
            w->StartObject();
            w->Key("line");      w->Int(range.end.line-1);
            w->Key("character"); w->Int(range.end.column-1);
            w->EndObject();
        w->EndObject();
    w->Key("children");
        w->StartArray();
        // fill in the blanks
}

void things::vhdl_document_symbol_provider::close_symbol()
{
        w->EndArray();
    w->EndObject();
}

things::vhdl_document_symbol_provider::vhdl_document_symbol_provider(rapidjson::Writer<rapidjson::StringBuffer>* w)
: w(w)
{

}

things::vhdl_document_symbol_provider::~vhdl_document_symbol_provider()
{

};

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::component_specification* c)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::configuration_item* c)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::block_configuration* b)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::component_configuration* c)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::subprogram* s)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::signature* s)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::type_definition* t)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::subtype* s)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::index_subtype* i)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::range* r)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::constraint* c)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::unit_declaration* u)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::physical_literal* p)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::element_declaration* e)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::declarative_item* d)
{
    switch (d->v_kind) {
    case vhdl::syntax::declarative_item::v_::type: {
        auto& t = d->v.type;
        symbol(t.identifier, d->first__, d->__last, symbol_kind::typeparameter, "type");
        close_symbol();
    }   break;

    case vhdl::syntax::declarative_item::v_::subtype: {
        auto& t = d->v.subtype;
        symbol(t.identifier, d->first__, d->__last, symbol_kind::typeparameter, "subtype");
        close_symbol();
    }   break;

    case vhdl::syntax::declarative_item::v_::object:
        d->v.object.v->traverse(*this);
        break;

    case vhdl::syntax::declarative_item::v_::interface:
        d->v.interface.v->traverse(*this);
        break;

    case vhdl::syntax::declarative_item::v_::alias: {
        auto& a = d->v.alias;
        symbol(a.designator, d->first__, d->__last, symbol_kind::key, "alias");
        close_symbol();
    }   break;

    case vhdl::syntax::declarative_item::v_::attribute:
        break;

    case vhdl::syntax::declarative_item::v_::component: {
        auto& c = d->v.component;
        symbol(c.identifier, d->first__, d->__last, symbol_kind::constructor, "component");
        if (c.generic__ && c.gl__ && c.__gr)
        {
            symbol("generic", *c.generic__, *c.generic__, *c.__gr, symbol_kind::ns);
            for (auto it: c.gens)
                it->traverse(*this);
            close_symbol();
        }
        
        if (c.port__ && c.pl__ && c.__pr)
        {
            symbol("port", *c.port__, *c.port__, *c.__pr, symbol_kind::ns);
            for (auto it: c.ports)
                it->traverse(*this);
            close_symbol();
        }

        close_symbol();
    }   break;
 
    case vhdl::syntax::declarative_item::v_::attr_spec:
        break;

    case vhdl::syntax::declarative_item::v_::subprogram: {
        auto& s = d->v.subprogram;
        if (s.spec->v_kind == vhdl::syntax::subprogram::v_::function)
            symbol(s.spec->designator, d->first__, d->__last, symbol_kind::function, "function");
        else
            symbol(s.spec->designator, d->first__, d->__last, symbol_kind::op, "procedure");
        for (auto it: s.spec->parameters)
            it->traverse(*this);
        close_symbol();
    }   break;

    case vhdl::syntax::declarative_item::v_::subprogram_body: {
        auto& s = d->v.subprogram_body;
        if (s.spec->v_kind == vhdl::syntax::subprogram::v_::function)
            symbol(s.spec->designator, d->first__, d->__last, symbol_kind::function, "function");
        else
            symbol(s.spec->designator, d->first__, d->__last, symbol_kind::op, "procedure");
        for (auto it: s.spec->parameters)
            it->traverse(*this);
        for (auto it: s.decls)
            it->traverse(*this);
        for (auto it: s.stmts)
            it->traverse(*this);
        close_symbol();
    }   break;

    case vhdl::syntax::declarative_item::v_::config_spec:
        break;

    case vhdl::syntax::declarative_item::v_::use:
        break;

    default:
        break;
    }
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::object* i)
{
    for (auto it: i->identifier)
    {

    //
    switch (i->v_kind) {
    case vhdl::syntax::object::v_::signal:
        symbol(it, i->decl->first__, i->decl->__last, symbol_kind::object, "object");
        close_symbol();
        break;

    case vhdl::syntax::object::v_::constant:
        symbol(it, i->decl->first__, i->decl->__last, symbol_kind::object, "object");
        close_symbol();
        break;

    case vhdl::syntax::object::v_::variable:
        symbol(it, i->decl->first__, i->decl->__last, symbol_kind::object, "object");
        close_symbol();
        break;

    case vhdl::syntax::object::v_::file:
        symbol(it, i->decl->first__, i->decl->__last, symbol_kind::object, "object");
        close_symbol();
        break;

    default:
        break;
    }

    }
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::interface* i)
{
    for (auto it: i->identifier)
    {

    //
    switch (i->v_kind) {
    case vhdl::syntax::interface::v_::signal:
        symbol(it, i->decl->first__, i->decl->__last, symbol_kind::interface, "interface");
        close_symbol();
        break;

    case vhdl::syntax::interface::v_::constant:
        symbol(it, i->decl->first__, i->decl->__last, symbol_kind::interface, "interface");
        close_symbol();
        break;

    case vhdl::syntax::interface::v_::variable:
        symbol(it, i->decl->first__, i->decl->__last, symbol_kind::interface, "interface");
        close_symbol();
        break;

    case vhdl::syntax::interface::v_::file:
        symbol(it, i->decl->first__, i->decl->__last, symbol_kind::interface, "interface");
        close_symbol();
        break;

    default:
        break;
    }

    }
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::association_element* a)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::entity_specification* e)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::configuration_specification* c)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::binding_indication* b)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::name* n)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::expression* e)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::element_association* e)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::choice* c)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::sequential_statement* s)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::target* t)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::waveform_element* w)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::case_statement_alternative* c)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::concurrent_statement* c)
{
    switch (c->v_kind) {
    case vhdl::syntax::concurrent_statement::v_::block:
        break;
    case vhdl::syntax::concurrent_statement::v_::process: {
        auto& p = c->v.process;
        if (!c->label.has_value())
            break;
        symbol(*c->label, c->first__, c->__last, symbol_kind::event, "process");
        for (auto it: p.decls)
            it->traverse(*this);
        close_symbol();
    }   break;
    case vhdl::syntax::concurrent_statement::v_::pcall:
        break;
    case vhdl::syntax::concurrent_statement::v_::assertion:
        break;
    case vhdl::syntax::concurrent_statement::v_::con_assign:
        break;
    case vhdl::syntax::concurrent_statement::v_::sel_assign:
        break;
    case vhdl::syntax::concurrent_statement::v_::inst: {
        auto& i = c->v.inst;
        if (!c->label.has_value())
            break;
        symbol(*c->label, c->first__, c->__last, symbol_kind::constructor, "inst");
        if (i.generic__ && i.gl__ && i.__gr)
        {
            symbol("generic map", *i.generic__, *i.generic__, *i.__gr, symbol_kind::ns);
            for (auto it: i.gens)
                it->traverse(*this);
            close_symbol();
        }
        if (i.port__ && i.pl__ && i.__pr)
        {
            symbol("port map", *i.port__, *i.port__, *i.__pr, symbol_kind::ns);
            for (auto it: i.ports)
                it->traverse(*this);
            close_symbol();
        }
        close_symbol();
    }   break;
    case vhdl::syntax::concurrent_statement::v_::for_gen:
        break;
    case vhdl::syntax::concurrent_statement::v_::if_gen:
        break;

    default:
        break;
    }
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::con_waveform* c)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::sel_waveform* s)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::instantiated_unit* i)
{
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::design_file* d)
{
    return true;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::design_unit* d)
{
    for (auto it: d->contexts)
        it->traverse(*this);

    switch (d->v_kind) {
    case vhdl::syntax::design_unit::v_::entity: {
        auto& e = d->v.entity;
        symbol(e.identifier, d->first__, d->__last, symbol_kind::clazz);
        if (e.generic__ && e.gl__ && e.__gr)
        {
            symbol("generic", *e.generic__, *e.generic__, *e.__gr, symbol_kind::ns);
            for (auto it: e.gens)
                it->traverse(*this);
            close_symbol();
        }
        if (e.port__ && e.pl__ && e.__pr)
        {
            symbol("port", *e.port__, *e.port__, *e.__pr, symbol_kind::ns);
            for (auto it: e.ports)
                it->traverse(*this);
            close_symbol();
        }

        for (auto it: e.decls)
            it->traverse(*this);

        close_symbol();
    }   break;
    
    case vhdl::syntax::design_unit::v_::architecture: {
        auto& a = d->v.architecture;
        symbol(a.identifier, d->first__, d->__last, symbol_kind::module);

        for (auto it: a.decls)
            it->traverse(*this);

        for (auto it: a.stmts)
            it->traverse(*this);

        close_symbol();
    }   break;
    
    case vhdl::syntax::design_unit::v_::package: {
        auto& p = d->v.package;
        symbol(p.identifier, d->first__, d->__last, symbol_kind::package);

        for (auto it: p.decls)
            it->traverse(*this);

        close_symbol();
    }   break;
    
    case vhdl::syntax::design_unit::v_::package_body: {
        auto& p = d->v.package_body;
        symbol(p.identifier, d->first__, d->__last, symbol_kind::property);

        for (auto it: p.decls)
            it->traverse(*this);

        close_symbol();
    }   break;
    
    case vhdl::syntax::design_unit::v_::configuration: {
        auto& c = d->v.configuration;
        symbol(c.identifier, d->first__, d->__last, symbol_kind::structure);

        for (auto it: c.decls)
            it->traverse(*this);

        close_symbol();
    }   break;
    
    default:
        break;
    }
    return false;
}

bool things::vhdl_document_symbol_provider::visit(vhdl::syntax::context_item* c)
{
    switch (c->v_kind) {
    case vhdl::syntax::context_item::v_::library_clause: {
        auto& l = c->v.library_clause;
        for (auto n: l.names)
        {
            symbol(n, n.location, n.location, symbol_kind::ns);
            close_symbol();
        }
    }   break;
    
    default:
        break;
    }
    return false;
}

things::sv_document_symbol_provider::sv_document_symbol_provider(
    slang::SourceManager& sm, rapidjson::Writer<rapidjson::StringBuffer>* w)
    : w(w), sm(sm)
{
}

void things::sv_document_symbol_provider::symbol(
    const slang::parsing::Token& name, const slang::SourceRange range,
    symbol_kind kind)
{
    auto start = range.start();
    auto end = range.end();
    if (range.start() == slang::SourceLocation::NoLocation ||
        range.end() == slang::SourceLocation::NoLocation ||
        name.location() == slang::SourceLocation::NoLocation)
        return;

    w->StartObject();
    w->Key("name");
    w->String(std::string(name.valueText()));
    w->Key("kind");
    w->Int(static_cast<int>(kind));
    w->Key("range");
    w->StartObject();
    w->Key("start");
    w->StartObject();
    w->Key("line");
    w->Int(sm.getLineNumber(range.start()) - 1);
    w->Key("character");
    w->Int(sm.getColumnNumber(range.start()) - 1);
    w->EndObject();
    w->Key("end");
    w->StartObject();
    w->Key("line");
    w->Int(sm.getLineNumber(range.end()) - 1);
    w->Key("character");
    w->Int(sm.getColumnNumber(range.end()) - 1);
    w->EndObject();
    w->EndObject();
    w->Key("selectionRange");
    w->StartObject();
    w->Key("start");
    w->StartObject();
    w->Key("line");
    w->Int(sm.getLineNumber(name.range().start()) - 1);
    w->Key("character");
    w->Int(sm.getColumnNumber(name.range().start()) - 1);
    w->EndObject();
    w->Key("end");
    w->StartObject();
    w->Key("line");
    w->Int(sm.getLineNumber(name.range().end()) - 1);
    w->Key("character");
    w->Int(sm.getColumnNumber(name.range().end()) - 1);
    w->EndObject();
    w->EndObject();
    w->Key("children");
    w->StartArray();
    // fill in the blanks
}

void things::sv_document_symbol_provider::close_symbol()
{
    w->EndArray();
    w->EndObject();
}

void things::sv_document_symbol_provider::handle(const slang::syntax::DataDeclarationSyntax& syntax)
{
    for (auto declarator : syntax.declarators)
    {
        symbol(declarator->name, syntax.sourceRange(), symbol_kind::variable);
        close_symbol();
    }
}

void things::sv_document_symbol_provider::handle(const slang::syntax::TypedefDeclarationSyntax& syntax)
{
    symbol(syntax.name, syntax.sourceRange(), symbol_kind::typeparameter);
    visitDefault(syntax);
    close_symbol();
}

void things::sv_document_symbol_provider::handle(const slang::syntax::ForwardTypedefDeclarationSyntax& syntax)
{
    symbol(syntax.name, syntax.sourceRange(), symbol_kind::typeparameter);
    close_symbol();
}

void things::sv_document_symbol_provider::handle(const slang::syntax::ForwardInterfaceClassTypedefDeclarationSyntax& syntax)
{
    symbol(syntax.name, syntax.sourceRange(), symbol_kind::typeparameter);
    close_symbol();
}

void things::sv_document_symbol_provider::handle(const slang::syntax::NetDeclarationSyntax& syntax)
{
    for (auto declarator : syntax.declarators)
    {
        symbol(declarator->name, syntax.sourceRange(), symbol_kind::variable);
        close_symbol();
    }
}

void things::sv_document_symbol_provider::handle(const slang::syntax::UserDefinedNetDeclarationSyntax& syntax)
{
    for (auto declarator : syntax.declarators)
    {
        symbol(declarator->name, syntax.sourceRange(), symbol_kind::variable);
        close_symbol();
    }
}

void things::sv_document_symbol_provider::handle(const slang::syntax::NetTypeDeclarationSyntax& syntax)
{
    symbol(syntax.name, syntax.sourceRange(), symbol_kind::variable);
    close_symbol();
}

void things::sv_document_symbol_provider::handle(const slang::syntax::PackageImportDeclarationSyntax& syntax)
{
}

void things::sv_document_symbol_provider::handle(const slang::syntax::TypeParameterDeclarationSyntax& syntax)
{
    for (auto declarator : syntax.declarators)
    {
        symbol(declarator->name, syntax.sourceRange(),
               symbol_kind::typeparameter);
        close_symbol();
    }
}

void things::sv_document_symbol_provider::handle(const slang::syntax::PortDeclarationSyntax& syntax)
{
    for (auto declarator : syntax.declarators)
    {
        symbol(declarator->name, syntax.sourceRange(), symbol_kind::variable);
        close_symbol();
    }
}

void things::sv_document_symbol_provider::handle(const slang::syntax::GenvarDeclarationSyntax& syntax)
{
    for (auto declarator : syntax.identifiers)
    {
        symbol(declarator->identifier, syntax.sourceRange(),
               symbol_kind::variable);
        close_symbol();
    }
}

void things::sv_document_symbol_provider::handle(const slang::syntax::ForVariableDeclarationSyntax& syntax)
{
}

void things::sv_document_symbol_provider::handle(const slang::syntax::ModuleDeclarationSyntax& syntax)
{
    switch (syntax.kind)
    {
    case slang::syntax::SyntaxKind::ModuleDeclaration:
        symbol(syntax.header->name, syntax.sourceRange(), symbol_kind::module);
        break;
    case slang::syntax::SyntaxKind::PackageDeclaration:
        symbol(syntax.header->name, syntax.sourceRange(), symbol_kind::package);
        break;
    case slang::syntax::SyntaxKind::InterfaceDeclaration:
        symbol(syntax.header->name, syntax.sourceRange(),
               symbol_kind::interface);
        break;
    case slang::syntax::SyntaxKind::ProgramDeclaration:
        symbol(syntax.header->name, syntax.sourceRange(), symbol_kind::ns);
        break;
    default:
        break;
    }
    visitDefault(syntax);
    close_symbol();
}

void things::sv_document_symbol_provider::handle(const slang::syntax::TimeUnitsDeclarationSyntax& syntax)
{
    symbol(syntax.time, syntax.sourceRange(), symbol_kind::variable);
    close_symbol();
}

void things::sv_document_symbol_provider::handle(const slang::syntax::FunctionDeclarationSyntax& syntax)
{
    switch (syntax.kind)
    {
    case slang::syntax::SyntaxKind::FunctionDeclaration:
        symbol(syntax.prototype->name->getLastToken(), syntax.sourceRange(),
               symbol_kind::function);
        break;
    case slang::syntax::SyntaxKind::TaskDeclaration:
        symbol(syntax.prototype->name->getLastToken(), syntax.sourceRange(),
               symbol_kind::function);
        break;
    default:
        break;
    }
    visitDefault(syntax);
    close_symbol();
}

void things::sv_document_symbol_provider::handle(const slang::syntax::LetDeclarationSyntax& syntax)
{
    symbol(syntax.identifier, syntax.sourceRange(), symbol_kind::function);
    close_symbol();
}

void things::sv_document_symbol_provider::handle(const slang::syntax::PropertyDeclarationSyntax& syntax)
{
}

void things::sv_document_symbol_provider::handle(const slang::syntax::SequenceDeclarationSyntax& syntax)
{
}

void things::sv_document_symbol_provider::handle(const slang::syntax::ClassDeclarationSyntax& syntax)
{
    symbol(syntax.name, syntax.sourceRange(), symbol_kind::clazz);
    visitDefault(syntax);
    close_symbol();
}

void things::sv_document_symbol_provider::handle(const slang::syntax::ClassPropertyDeclarationSyntax& syntax)
{
}

void things::sv_document_symbol_provider::handle(const slang::syntax::ClassMethodPrototypeSyntax& syntax)
{
}

void things::sv_document_symbol_provider::handle(const slang::syntax::ModportDeclarationSyntax& syntax)
{
}

void things::sv_document_symbol_provider::handle(const slang::syntax::ClockingDeclarationSyntax& syntax)
{
}

void things::sv_document_symbol_provider::handle(const slang::syntax::UdpOutputPortDeclSyntax& syntax)
{
}

void things::sv_document_symbol_provider::handle(const slang::syntax::UdpInputPortDeclSyntax& syntax)
{
}

void things::sv_document_symbol_provider::handle(const slang::syntax::UdpDeclarationSyntax& syntax)
{
}

void things::sv_document_symbol_provider::handle(const slang::syntax::SpecparamDeclarationSyntax& syntax)
{
}

void things::sv_document_symbol_provider::handle(const slang::syntax::PathDeclarationSyntax& syntax)
{
}

void things::sv_document_symbol_provider::handle(const slang::syntax::ConditionalPathDeclarationSyntax& syntax)
{
}

void things::sv_document_symbol_provider::handle(const slang::syntax::IfNonePathDeclarationSyntax& syntax)
{
}

void things::sv_document_symbol_provider::handle(const slang::syntax::PulseStyleDeclarationSyntax& syntax)
{
}

void things::sv_document_symbol_provider::handle(const slang::syntax::ConstraintDeclarationSyntax& syntax)
{
}

void things::sv_document_symbol_provider::handle(const slang::syntax::CovergroupDeclarationSyntax& syntax)
{
}

void things::sv_document_symbol_provider::handle(const slang::syntax::ProceduralBlockSyntax& syntax)
{
    switch (syntax.kind)
    {
    case slang::syntax::SyntaxKind::InitialBlock:
        symbol(syntax.keyword, syntax.sourceRange(), symbol_kind::ns);
        break;
    case slang::syntax::SyntaxKind::FinalBlock:
        symbol(syntax.keyword, syntax.sourceRange(), symbol_kind::ns);
        break;
    case slang::syntax::SyntaxKind::AlwaysBlock:
        symbol(syntax.keyword, syntax.sourceRange(), symbol_kind::ns);
        break;
    case slang::syntax::SyntaxKind::AlwaysCombBlock:
        symbol(syntax.keyword, syntax.sourceRange(), symbol_kind::ns);
        break;
    case slang::syntax::SyntaxKind::AlwaysFFBlock:
        symbol(syntax.keyword, syntax.sourceRange(), symbol_kind::ns);
        break;
    case slang::syntax::SyntaxKind::AlwaysLatchBlock:
        symbol(syntax.keyword, syntax.sourceRange(), symbol_kind::ns);
        break;
    }
    close_symbol();
}
