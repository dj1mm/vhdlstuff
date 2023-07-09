
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
        auto decl = &node->as<slang::syntax::ModuleDeclarationSyntax>();
        std::string_view name = decl->header->name.valueText();
        if (name.empty())
            continue;

        // do something

    }
    for (auto class_decl: tree->getMetadata().classDecls) {
        std::string_view name = class_decl->name.valueText();
        if (name.empty())
            continue;

        // do something
    }

    return result;
}
