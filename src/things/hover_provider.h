
#ifndef THINGS_HOVER_PROVIDER_H
#define THINGS_HOVER_PROVIDER_H

#include "fmt/format.h"
#include "vhdl_syntax.h"

#include "slang/ast/ASTVisitor.h"
#include "slang/ast/Compilation.h"
#include "slang/syntax/SyntaxVisitor.h"
#include "slang/syntax/AllSyntax.h"
#include "slang/syntax/SyntaxPrinter.h"
#include "slang/text/SourceManager.h"
#include "slang/text/SourceLocation.h"

#include "lsp/structures.h"

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace things
{

class vhdl_hover_provider: public vhdl::syntax::visitor
{
    template <typename... T>
    void hover(std::string fmt, T&&... args)
    {
        if (found)
            return;

        auto f = fmt::vformat(fmt, fmt::make_format_args(args...));
        w->StartObject();
        w->Key("kind"); w->String("markdown");
        w->Key("contents"); w->String(fmt::format("```\n{}\n```", f));
        w->EndObject();
        found = true;
    }

    void hover_identifier_denotes_entity(std::string_view name, std::vector<vhdl::node::named_entity*> denotes)
    {
        if (found)
            return;

        std::string str;
        switch (denotes.size()) {
        case 0:
            str = fmt::format("`{}` not found", name);
            break;
        case 1:
            hover(denotes[0]);
            return;
        default:
            str = fmt::format("{} overloads for `{}`", denotes.size(), name);
            break;
        }

        w->StartObject();
        w->Key("kind"); w->String("markdown");
        w->Key("contents"); w->String(str);
        w->EndObject();
        found = true;
    }

    public:
    vhdl_hover_provider(rapidjson::Writer<rapidjson::StringBuffer>*, bool&, common::position);
    ~vhdl_hover_provider();

    void hover(vhdl::node::named_entity*);
    void hover(vhdl::syntax::type_definition* t, int i);
    void hover(vhdl::syntax::element_declaration* e, int);
    void hover(vhdl::syntax::declarative_item* d);
    void hover(vhdl::syntax::object* i, int);
    void hover(vhdl::syntax::interface* i, int);
    void hover(vhdl::syntax::design_unit* d);
    void hover(vhdl::syntax::context_item* c, int);

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
    bool& found;
    common::position position;
};

bool sv_hover_provider(rapidjson::Writer<rapidjson::StringBuffer>*,
                       slang::ast::Compilation&, slang::SourceManager&,
                       std::string file, lsp::position);

class find_declaration_under_position
    : public slang::syntax::SyntaxVisitor<find_declaration_under_position>
{
    std::string file;
    lsp::position position;
    slang::SourceManager& sm;

    bool position_is_within_source_range(const slang::SourceRange);

    public:
    const slang::syntax::SyntaxNode* node;
    find_declaration_under_position(slang::SourceManager&, std::string file,
                                    lsp::position);

    void handle(const slang::syntax::DataDeclarationSyntax&);
    void handle(const slang::syntax::TypedefDeclarationSyntax&);
    void handle(const slang::syntax::ForwardTypedefDeclarationSyntax&);
    void handle(const slang::syntax::ForwardInterfaceClassTypedefDeclarationSyntax&);
    void handle(const slang::syntax::NetDeclarationSyntax&);
    void handle(const slang::syntax::UserDefinedNetDeclarationSyntax&);
    void handle(const slang::syntax::NetTypeDeclarationSyntax&);
    void handle(const slang::syntax::PackageImportDeclarationSyntax&);
    void handle(const slang::syntax::ParameterDeclarationSyntax&);
    void handle(const slang::syntax::TypeParameterDeclarationSyntax&);
    void handle(const slang::syntax::PortDeclarationSyntax&);
    void handle(const slang::syntax::ImplicitAnsiPortSyntax& syntax);
    void handle(const slang::syntax::ExplicitAnsiPortSyntax& syntax);
    void handle(const slang::syntax::GenvarDeclarationSyntax&);
    void handle(const slang::syntax::ForVariableDeclarationSyntax&);
    void handle(const slang::syntax::ModuleDeclarationSyntax&);
    void handle(const slang::syntax::TimeUnitsDeclarationSyntax&);
    void handle(const slang::syntax::FunctionDeclarationSyntax&);
    void handle(const slang::syntax::LetDeclarationSyntax&);
    void handle(const slang::syntax::PropertyDeclarationSyntax&);
    void handle(const slang::syntax::SequenceDeclarationSyntax&);
    void handle(const slang::syntax::ClassDeclarationSyntax&);
    void handle(const slang::syntax::ClassPropertyDeclarationSyntax&);
    void handle(const slang::syntax::ClassMethodPrototypeSyntax&);
    void handle(const slang::syntax::ModportDeclarationSyntax&);
    void handle(const slang::syntax::ClockingDeclarationSyntax&);
    void handle(const slang::syntax::UdpOutputPortDeclSyntax&);
    void handle(const slang::syntax::UdpInputPortDeclSyntax&);
    void handle(const slang::syntax::UdpDeclarationSyntax&);
    void handle(const slang::syntax::SpecparamDeclarationSyntax&);
    void handle(const slang::syntax::PathDeclarationSyntax&);
    void handle(const slang::syntax::ConditionalPathDeclarationSyntax&);
    void handle(const slang::syntax::IfNonePathDeclarationSyntax&);
    void handle(const slang::syntax::PulseStyleDeclarationSyntax&);
    void handle(const slang::syntax::ConstraintDeclarationSyntax&);
    void handle(const slang::syntax::CovergroupDeclarationSyntax&);
    void handle(const slang::syntax::ProceduralBlockSyntax&);
};

class find_instance_body_or_package_under_position
    : public slang::ast::ASTVisitor<
          find_instance_body_or_package_under_position, false, false>
{
    std::string file;
    lsp::position position;
    slang::SourceManager& sm;
    const slang::ast::InstanceBodySymbol* symbol;

    public:
    find_instance_body_or_package_under_position(slang::SourceManager&,
                                                 std::string file,
                                                 lsp::position);
    ~find_instance_body_or_package_under_position();

    void handle(const slang::ast::Scope& scope);
};

class find_innermost_scope_and_then_denoting_name
    : public slang::ast::ASTVisitor<find_innermost_scope_and_then_denoting_name,
                                    true, true>
{
    lsp::position position;
    slang::SourceManager& sm;
    const slang::ast::InstanceBodySymbol* symbol;

    public:
    find_innermost_scope_and_then_denoting_name(slang::SourceManager&,
                                                lsp::position position);

    void handle (const slang::ast::IntegerLiteral&);
    void handle (const slang::ast::RealLiteral&);
    void handle (const slang::ast::TimeLiteral&);
    void handle (const slang::ast::UnbasedUnsizedIntegerLiteral&);
    void handle (const slang::ast::NullLiteral&);
    void handle (const slang::ast::UnboundedLiteral&);
    void handle (const slang::ast::StringLiteral&);
    void handle (const slang::ast::NamedValueExpression&);
    void handle (const slang::ast::HierarchicalValueExpression&);
    void handle (const slang::ast::UnaryExpression&);
    void handle (const slang::ast::BinaryExpression&);
    void handle (const slang::ast::ConditionalExpression&);
    void handle (const slang::ast::InsideExpression&);
    void handle (const slang::ast::AssignmentExpression&);
    void handle (const slang::ast::ConcatenationExpression&);
    void handle (const slang::ast::ReplicationExpression&);
    void handle (const slang::ast::StreamingConcatenationExpression&);
    void handle (const slang::ast::ElementSelectExpression&);
    void handle (const slang::ast::RangeSelectExpression&);
    void handle (const slang::ast::MemberAccessExpression&);
    void handle (const slang::ast::CallExpression&);
    void handle (const slang::ast::ConversionExpression&);
    void handle (const slang::ast::DataTypeExpression&);
    void handle (const slang::ast::TypeReferenceExpression&);
    void handle (const slang::ast::LValueReferenceExpression&);
    void handle (const slang::ast::SimpleAssignmentPatternExpression&);
    void handle (const slang::ast::StructuredAssignmentPatternExpression&);
    void handle (const slang::ast::ReplicatedAssignmentPatternExpression&);
    void handle (const slang::ast::EmptyArgumentExpression&);
    void handle (const slang::ast::OpenRangeExpression&);
    void handle (const slang::ast::DistExpression&);
    void handle (const slang::ast::NewArrayExpression&);
    void handle (const slang::ast::NewClassExpression&);
    void handle (const slang::ast::CopyClassExpression&);
    void handle (const slang::ast::MinTypMaxExpression&);
    
    void handle (const slang::ast::Statement&);

    void handle(const slang::ast::Scope&);
};
}

#endif
