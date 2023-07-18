
#include "folding_range_provider.h"

void things::vhdl_folding_range_provider::foldable(std::optional<common::location> from, std::optional<common::location> to)
{
    if (!from || !to)
        return;

    if (from->end.line >= to->begin.line + 2)
        return;

    w->StartObject();
    w->Key("startLine");      w->Int(from->end.line-1);
    w->Key("startCharacter"); w->Int(from->end.column);
    w->Key("endLine");        w->Int(to->begin.line-2);
    w->Key("endCharacter");   w->Int(to->begin.column);
    w->EndObject();
}

void things::vhdl_folding_range_provider::foldable(common::location from, common::location to)
{
    if (from.end.line >= to.begin.line + 2)
        return;

    w->StartObject();
    w->Key("startLine");      w->Int(from.end.line-1);
    w->Key("startCharacter"); w->Int(from.end.column);
    w->Key("endLine");        w->Int(to.begin.line-2);
    w->Key("endCharacter");   w->Int(to.begin.column);
    w->EndObject();
}

things::vhdl_folding_range_provider::vhdl_folding_range_provider(rapidjson::Writer<rapidjson::StringBuffer>* w)
: w(w)
{

}

things::vhdl_folding_range_provider::~vhdl_folding_range_provider()
{

};

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::component_specification* c)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::configuration_item* c)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::block_configuration* b)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::component_configuration* c)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::subprogram* s)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::signature* s)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::type_definition* t)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::subtype* s)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::index_subtype* i)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::range* r)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::constraint* c)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::unit_declaration* u)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::physical_literal* p)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::element_declaration* e)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::declarative_item* d)
{
    switch (d->v_kind) {
    case vhdl::syntax::declarative_item::v_::subprogram: {
        auto& s = d->v.subprogram;
        foldable(s.pl__, s.__pr);
    }   break;
    case vhdl::syntax::declarative_item::v_::subprogram_body: {
        auto& s = d->v.subprogram_body;
        foldable(s.pl__, s.__pr);
        foldable(s.is__, s.__begin__);
        foldable(s.__begin__, s.__end);
    }   return true;
    case vhdl::syntax::declarative_item::v_::component: {
        auto& c = d->v.component;
        foldable(c.pl__, c.__pr);
        foldable(c.gl__, c.__gr);
    }   break;
    default:
        break;
    }
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::object* i)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::interface* i)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::association_element* a)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::entity_specification* e)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::configuration_specification* c)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::binding_indication* b)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::name* n)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::expression* e)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::element_association* e)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::choice* c)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::sequential_statement* s)
{
    switch (s->v_kind) {
    case vhdl::syntax::sequential_statement::v_::if_stmt: {
        auto& i = s->v.if_stmt;
        foldable(i.then__, i.__end);
    }   break;
    case vhdl::syntax::sequential_statement::v_::for_loop: {
        auto& f = s->v.for_loop;
        foldable(f.loop__, f.__end);
    }   break;
    case vhdl::syntax::sequential_statement::v_::while_loop: {
        auto& l = s->v.while_loop;
        foldable(l.loop__, l.__end);
    }   break;
    default:
        break;
    }
    return true;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::target* t)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::waveform_element* w)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::case_statement_alternative* c)
{
    return true;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::concurrent_statement* c)
{
    switch (c->v_kind) {
    case vhdl::syntax::concurrent_statement::v_::inst: {
        auto& i = c->v.inst;
        foldable(i.gl__, i.__gr);
        foldable(i.pl__, i.__pr);
    }   break;
    case vhdl::syntax::concurrent_statement::v_::process: {
        auto& p = c->v.process;
        foldable(p.process__, p.__begin__);
        foldable(p.__begin__, p.__end);
    }   return true;
    default:
        break;
    }
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::con_waveform* c)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::sel_waveform* s)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::instantiated_unit* i)
{
    return false;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::design_file* d)
{
    return true;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::design_unit* d)
{
    switch (d->v_kind)
    {
    case vhdl::syntax::design_unit::v_::entity: {
        auto& e = d->v.entity;
        foldable(e.gl__, e.__gr);
        foldable(e.pl__, e.__pr);
    }   break;
    case vhdl::syntax::design_unit::v_::architecture: {
        auto& a = d->v.architecture;
        foldable(a.is__, a.__begin__);
        foldable(a.__begin__, a.__end);
    }   break;
    case vhdl::syntax::design_unit::v_::package: {
        auto& p = d->v.package;
        foldable(p.is__, p.__end);
    }   break;
    case vhdl::syntax::design_unit::v_::package_body: {
        auto& p = d->v.package_body;
        foldable(p.is__, p.__end);
    }   break;
    case vhdl::syntax::design_unit::v_::configuration: {
        auto& c = d->v.configuration;
        foldable(c.is__, c.__end);
    }   break;
    
    default:
        break;
    }
    return true;
}

bool things::vhdl_folding_range_provider::visit(vhdl::syntax::context_item* c)
{
    return false;
}

things::sv_folding_range_provider::sv_folding_range_provider(
    slang::SourceManager& sm, rapidjson::Writer<rapidjson::StringBuffer>* w)
    : w(w), sm(sm)
{
}

void things::sv_folding_range_provider::foldable(
    const slang::parsing::Token& from, const slang::parsing::Token& to)
{
    if (sm.getLineNumber(from.location()) >= sm.getLineNumber(to.location())+2)
        return;

    w->StartObject();
    w->Key("startLine");      w->Int(sm.getLineNumber(from.location()) - 1);
    w->Key("startCharacter"); w->Int(sm.getColumnNumber(from.location()) - 1);
    w->Key("endLine");        w->Int(sm.getLineNumber(to.location()) - 2);
    w->Key("endCharacter");   w->Int(sm.getColumnNumber(to.location()) - 1);
    w->EndObject();
}

void things::sv_folding_range_provider::foldable(
    const slang::SourceLocation from, const slang::SourceLocation to)
{
    if (sm.getLineNumber(from) >= sm.getLineNumber(to)+2)
        return;

    w->StartObject();
    w->Key("startLine");      w->Int(sm.getLineNumber(from)-1);
    w->Key("startCharacter"); w->Int(sm.getColumnNumber(from)-1);
    w->Key("endLine");        w->Int(sm.getLineNumber(to)-2);
    w->Key("endCharacter");   w->Int(sm.getColumnNumber(to)-1);
    w->EndObject();
}

void things::sv_folding_range_provider::handle(const slang::syntax::StructUnionTypeSyntax& syntax)
{
    foldable(syntax.openBrace, syntax.closeBrace);
    visitDefault(syntax);
}

void things::sv_folding_range_provider::handle(const slang::syntax::EnumTypeSyntax& syntax)
{
    foldable(syntax.openBrace, syntax.closeBrace);
    visitDefault(syntax);
}

void things::sv_folding_range_provider::handle(const slang::syntax::ModuleDeclarationSyntax& syntax)
{
    foldable(syntax.header->semi, syntax.endmodule);
    visitDefault(syntax);
}

void things::sv_folding_range_provider::handle(const slang::syntax::ParameterPortListSyntax& syntax)
{
    foldable(syntax.openParen, syntax.closeParen);
    visitDefault(syntax);
}

void things::sv_folding_range_provider::handle(const slang::syntax::NonAnsiPortListSyntax& syntax)
{
    foldable(syntax.openParen, syntax.closeParen);
    visitDefault(syntax);
}

void things::sv_folding_range_provider::handle(const slang::syntax::AnsiPortListSyntax& syntax)
{
    foldable(syntax.openParen, syntax.closeParen);
    visitDefault(syntax);
}

void things::sv_folding_range_provider::handle(const slang::syntax::ProceduralBlockSyntax& syntax)
{
    // For always, alwayscomb, alwaysff, alwayslatch, final, initial statements
    foldable(syntax.keyword, syntax.statement->getLastToken());
    visitDefault(syntax);
}

void things::sv_folding_range_provider::handle(const slang::syntax::ConditionalStatementSyntax& syntax)
{
    foldable(syntax.ifKeyword, syntax.statement->getLastToken());
    visitDefault(syntax);
}

void things::sv_folding_range_provider::handle(const slang::syntax::LoopGenerateSyntax& syntax)
{
    foldable(syntax.keyword, syntax.block->getLastToken());
    visitDefault(syntax);
}

void things::sv_folding_range_provider::handle(const slang::syntax::IfGenerateSyntax& syntax)
{
    foldable(syntax.keyword, syntax.block->getLastToken());
    visitDefault(syntax);
}

void things::sv_folding_range_provider::handle(const slang::syntax::ElseClauseSyntax& syntax)
{
    if (syntax.clause->isKind(slang::syntax::SyntaxKind::ConditionalStatement))
    {
        // else if statements and else if generate
        foldable( syntax.elseKeyword, syntax.clause->getLastToken());
        visitDefault(*syntax.clause);
    }
    else
    {
        // else statements
        foldable(syntax.elseKeyword, syntax.clause->getLastToken());
        visitDefault(syntax);
    } 
}

void things::sv_folding_range_provider::handle(const slang::syntax::ContinuousAssignSyntax& syntax)
{
    foldable(syntax.assign, syntax.semi);
    visitDefault(syntax);
}

void things::sv_folding_range_provider::handle(const slang::syntax::CaseStatementSyntax& syntax)
{
    foldable(syntax.closeParen, syntax.endcase);
    visitDefault(syntax);
}

void things::sv_folding_range_provider::handle(const slang::syntax::StandardCaseItemSyntax& syntax)
{
    foldable(syntax.colon, syntax.getLastToken());
    visitDefault(syntax);
}

void things::sv_folding_range_provider::handle(const slang::syntax::DefaultCaseItemSyntax& syntax)
{
    foldable(syntax.colon, syntax.getLastToken());
    visitDefault(syntax);
}

void things::sv_folding_range_provider::handle(const slang::syntax::PatternCaseItemSyntax& syntax)
{
    foldable(syntax.colon, syntax.getLastToken());
    visitDefault(syntax);
}


void things::sv_folding_range_provider::handle(const slang::syntax::LoopStatementSyntax& syntax)
{
    foldable(syntax.repeatOrWhile, syntax.statement->getLastToken());
    visitDefault(syntax);
}

void things::sv_folding_range_provider::handle(const slang::syntax::ForLoopStatementSyntax& syntax)
{
    foldable(syntax.forKeyword, syntax.statement->getLastToken());
    visitDefault(syntax);
}

void things::sv_folding_range_provider::handle(const slang::syntax::ForeachLoopStatementSyntax& syntax)
{
    foldable(syntax.keyword, syntax.statement->getLastToken());
    visitDefault(syntax);
}


void things::sv_folding_range_provider::handle(const slang::syntax::ParameterValueAssignmentSyntax& syntax)
{
    foldable(syntax.openParen, syntax.closeParen);
    visitDefault(syntax);
}

void things::sv_folding_range_provider::handle(const slang::syntax::HierarchicalInstanceSyntax& syntax)
{
    foldable(syntax.openParen, syntax.closeParen);
    visitDefault(syntax);
}


void things::sv_folding_range_provider::handle(const slang::syntax::ClassDeclarationSyntax& syntax)
{
    foldable(syntax.classKeyword, syntax.endClass);
    visitDefault(syntax);
}

void things::sv_folding_range_provider::handle(const slang::syntax::FunctionDeclarationSyntax& syntax)
{
    if (syntax.parent->isKind(slang::syntax::SyntaxKind::ClassMethodDeclaration))
    {
        // if this is true, we are looking at a method declaration. We already
        // reported the folding range of this method so just visit the children
        visitDefault(syntax);
        return;
    }
    // this is a normal function or task declaration
    foldable(syntax.getFirstToken(), syntax.getLastToken());
    visitDefault(syntax);
}

void things::sv_folding_range_provider::handle(const slang::syntax::ClassMethodDeclarationSyntax& syntax)
{
    foldable(syntax.getFirstToken(), syntax.getLastToken());
    visitDefault(syntax);
}

