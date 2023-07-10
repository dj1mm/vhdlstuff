
#include "fast_parser.h"

#include "slang/parsing/Parser.h"
#include "slang/parsing/Preprocessor.h"
#include "slang/syntax/SyntaxTree.h"
#include "slang/util/Bag.h"
#include "slang/util/BumpAllocator.h"

sv::fast_parser::fast_parser(slang::SourceManager* sm, std::string file)
: sm(sm), file(file)
{

}

std::vector<std::tuple<sv::library_cell_kind, unsigned, unsigned, std::string,
                       std::optional<std::string>, std::string, time_t>>
sv::fast_parser::parse()
{
    std::vector<
        std::tuple<sv::library_cell_kind, unsigned, unsigned, std::string,
                   std::optional<std::string>, std::string, time_t>>
        result;
    auto buffer = sm->readSource(file);
    if (!buffer)
    {
        return {};
    }

    slang::parsing::PreprocessorOptions ppo;
    slang::parsing::LexerOptions lo;
    slang::parsing::ParserOptions po;

    slang::Bag options;
    options.set(ppo);
    options.set(lo);
    options.set(po);

    auto tree = slang::syntax::SyntaxTree::fromBuffer(buffer, *sm, options);
    for (auto& [node, _]: tree->getMetadata().nodeMap) {
        auto decl        = &node->as<slang::syntax::ModuleDeclarationSyntax>();
        auto name        = std::string(decl->header->name.valueText());
        auto loc         = decl->header->moduleKeyword.location();
        auto line_number = (unsigned) sm->getLineNumber(loc);
        auto coln_number = (unsigned) sm->getColumnNumber(loc);
        auto file        = std::string(sm->getFileName(loc));
        switch (decl->kind) {
        case slang::syntax::SyntaxKind::ModuleDeclaration:    result.push_back(std::make_tuple(sv::library_cell_kind::module,    line_number, coln_number, name, std::nullopt, file, 0)); break;
        case slang::syntax::SyntaxKind::InterfaceDeclaration: result.push_back(std::make_tuple(sv::library_cell_kind::interface, line_number, coln_number, name, std::nullopt, file, 0)); break;
        case slang::syntax::SyntaxKind::PackageDeclaration:   result.push_back(std::make_tuple(sv::library_cell_kind::package,   line_number, coln_number, name, std::nullopt, file, 0)); break;
        case slang::syntax::SyntaxKind::ProgramDeclaration:   result.push_back(std::make_tuple(sv::library_cell_kind::program,   line_number, coln_number, name, std::nullopt, file, 0)); break;
        default:
            break;
        }

    }
    for (auto class_decl: tree->getMetadata().classDecls) {
        std::string_view name = class_decl->name.valueText();
        if (name.empty())
            continue;

        // do something
    }

    return result;
}
