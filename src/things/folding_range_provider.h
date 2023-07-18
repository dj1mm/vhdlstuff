
#ifndef THINGS_FOLDING_RANGE_PROVIDER_H
#define THINGS_FOLDING_RANGE_PROVIDER_H

#include "vhdl_syntax.h"

#include "slang/syntax/SyntaxVisitor.h"
#include "slang/syntax/AllSyntax.h"
#include "slang/text/SourceManager.h"
#include "slang/text/SourceLocation.h"

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace things
{

class vhdl_folding_range_provider: public vhdl::syntax::visitor
{
    void foldable(std::optional<common::location> from, std::optional<common::location> to);
    void foldable(common::location from, common::location to);

    public:
    vhdl_folding_range_provider(rapidjson::Writer<rapidjson::StringBuffer>*);
    ~vhdl_folding_range_provider();

    virtual bool visit(vhdl::syntax::component_specification* c);
    virtual bool visit(vhdl::syntax::configuration_item* c);

    virtual bool visit(vhdl::syntax::block_configuration* b);
    virtual bool visit(vhdl::syntax::component_configuration* c);
    virtual bool visit(vhdl::syntax::subprogram* s);
    virtual bool visit(vhdl::syntax::signature* s);
    virtual bool visit(vhdl::syntax::type_definition* t);
    virtual bool visit(vhdl::syntax::subtype* s);
    virtual bool visit(vhdl::syntax::index_subtype* i);
    virtual bool visit(vhdl::syntax::range* r);
    virtual bool visit(vhdl::syntax::constraint* c);
    virtual bool visit(vhdl::syntax::unit_declaration* u);
    virtual bool visit(vhdl::syntax::physical_literal* p);
    virtual bool visit(vhdl::syntax::element_declaration* e);
    virtual bool visit(vhdl::syntax::declarative_item* d);
    virtual bool visit(vhdl::syntax::object* i);
    virtual bool visit(vhdl::syntax::interface* i);
    virtual bool visit(vhdl::syntax::association_element* a);
    virtual bool visit(vhdl::syntax::entity_specification* e);
    virtual bool visit(vhdl::syntax::configuration_specification* c);
    virtual bool visit(vhdl::syntax::binding_indication* b);
    virtual bool visit(vhdl::syntax::name* n);
    virtual bool visit(vhdl::syntax::expression* e);
    virtual bool visit(vhdl::syntax::element_association* e);
    virtual bool visit(vhdl::syntax::choice* c);
    virtual bool visit(vhdl::syntax::sequential_statement* s);
    virtual bool visit(vhdl::syntax::target* t);
    virtual bool visit(vhdl::syntax::waveform_element* w);
    virtual bool visit(vhdl::syntax::case_statement_alternative* c);
    virtual bool visit(vhdl::syntax::concurrent_statement* c);
    virtual bool visit(vhdl::syntax::con_waveform* c);
    virtual bool visit(vhdl::syntax::sel_waveform* s);
    virtual bool visit(vhdl::syntax::instantiated_unit* i);
    virtual bool visit(vhdl::syntax::design_file* d);
    virtual bool visit(vhdl::syntax::design_unit* d);
    virtual bool visit(vhdl::syntax::context_item* c);

    private:
    rapidjson::Writer<rapidjson::StringBuffer>* w;
};

class sv_folding_range_provider
    : public slang::syntax::SyntaxVisitor<sv_folding_range_provider>
{
    rapidjson::Writer<rapidjson::StringBuffer>* w;
    slang::SourceManager& sm;

    public:
    sv_folding_range_provider(slang::SourceManager&,
                              rapidjson::Writer<rapidjson::StringBuffer>*);

    void foldable(const slang::parsing::Token&, const slang::parsing::Token&);
    void foldable(const slang::SourceLocation, const slang::SourceLocation);

    void handle(const slang::syntax::StructUnionTypeSyntax&);
    void handle(const slang::syntax::EnumTypeSyntax&);
    void handle(const slang::syntax::ModuleDeclarationSyntax&);
    void handle(const slang::syntax::ParameterPortListSyntax&);
    void handle(const slang::syntax::NonAnsiPortListSyntax&);
    void handle(const slang::syntax::AnsiPortListSyntax&);
    void handle(const slang::syntax::ProceduralBlockSyntax&);
    void handle(const slang::syntax::ConditionalStatementSyntax&);
    void handle(const slang::syntax::LoopGenerateSyntax&);
    void handle(const slang::syntax::IfGenerateSyntax&);
    void handle(const slang::syntax::ElseClauseSyntax&);
    void handle(const slang::syntax::ContinuousAssignSyntax&);
    void handle(const slang::syntax::CaseStatementSyntax&);
    void handle(const slang::syntax::StandardCaseItemSyntax&);
    void handle(const slang::syntax::DefaultCaseItemSyntax&);
    void handle(const slang::syntax::PatternCaseItemSyntax&);
    void handle(const slang::syntax::LoopStatementSyntax&);
    void handle(const slang::syntax::ForLoopStatementSyntax&);
    void handle(const slang::syntax::ForeachLoopStatementSyntax&);
    void handle(const slang::syntax::ParameterValueAssignmentSyntax&);
    void handle(const slang::syntax::HierarchicalInstanceSyntax&);
    void handle(const slang::syntax::ClassDeclarationSyntax&);
    void handle(const slang::syntax::FunctionDeclarationSyntax&);
    void handle(const slang::syntax::ClassMethodDeclarationSyntax&);
};

}

#endif
