
#include "sv/ast.h"

#include <unordered_set>
#include <exception>

#include "slang/ast/Compilation.h"
#include "slang/ast/symbols/CompilationUnitSymbols.h"
#include "slang/diagnostics/Diagnostics.h"
#include "slang/parsing/Parser.h"
#include "slang/parsing/Preprocessor.h"
#include "slang/syntax/SyntaxNode.h"
#include "slang/syntax/SyntaxTree.h"
#include "slang/text/SourceManager.h"
#include "slang/util/BumpAllocator.h"

sv::ast::ast(std::string f, std::shared_ptr<sv::library_manager> m,
               std::string w)
    : filename(f), library_manager(m), worklibrary(w), invalidated_(true)
{

}

bool sv::ast::update()
{
    // check if invalidated
    if (!invalidated_)
        return true;

    auto buffer = sm.readSource(filename);
    if (!buffer)
    {
        main_file.reset();
        return false;
    }

    slang::parsing::PreprocessorOptions ppo;
    slang::parsing::LexerOptions lo;
    slang::parsing::ParserOptions po;
    slang::ast::CompilationOptions co;

    slang::Bag options;
    options.set(ppo);
    options.set(lo);
    options.set(po);
    options.set(co);

    main_file = slang::syntax::SyntaxTree::fromBuffer(buffer, sm, options);

    auto lib = library_manager->get(worklibrary);

    for (auto& [node, _]: main_file->getMetadata().nodeMap) {
        auto decl        = &node->as<slang::syntax::ModuleDeclarationSyntax>();
        auto name        = std::string(decl->header->name.valueText());
        auto loc         = decl->header->moduleKeyword.location();
        auto line_number = (int) sm.getLineNumber(loc);
        auto coln_number = (int) sm.getColumnNumber(loc);
        auto file        = std::string(sm.getFileName(loc));
        switch (decl->kind) {
        case slang::syntax::SyntaxKind::ModuleDeclaration:    lib->put(sv::library_cell_kind::module,    line_number, coln_number, name, std::nullopt, file, 0); break;
        case slang::syntax::SyntaxKind::InterfaceDeclaration: lib->put(sv::library_cell_kind::interface, line_number, coln_number, name, std::nullopt, file, 0); break;
        case slang::syntax::SyntaxKind::PackageDeclaration:   lib->put(sv::library_cell_kind::package,   line_number, coln_number, name, std::nullopt, file, 0); break;
        case slang::syntax::SyntaxKind::ProgramDeclaration:   lib->put(sv::library_cell_kind::program,   line_number, coln_number, name, std::nullopt, file, 0); break;
        default:
            throw std::logic_error("Unknown decl kind");
        }
    }

    compilation.addSyntaxTree(main_file);

    std::unordered_set<std::string_view> known_names;
    auto add_known_names = [&](const std::shared_ptr<slang::syntax::SyntaxTree> tree) {
        for (auto& [node, _] : tree->getMetadata().nodeMap)
        {
            auto decl = &node->as<slang::syntax::ModuleDeclarationSyntax>();
            std::string_view name = decl->header->name.valueText();
            if (!name.empty())
                known_names.emplace(name);
        }
        for (auto class_decl : tree->getMetadata().classDecls)
        {
            std::string_view name = class_decl->name.valueText();
            if (!name.empty())
                known_names.emplace(name);
        }
    };

    for (auto tree: compilation.getSyntaxTrees())
        add_known_names(tree);

    std::unordered_set<std::string_view> missing_names;
    auto find_missing_names = [&](const std::shared_ptr<slang::syntax::SyntaxTree> tree, std::unordered_set<std::string_view>& names) {
        for (auto name : tree->getMetadata().globalInstances)
            if (known_names.find(name) == known_names.end())
                names.emplace(name);

        for (auto class_package_name : tree->getMetadata().classPackageNames)
        {
            auto name = class_package_name->identifier.valueText();
            if (!name.empty() &&
                known_names.find(name) == known_names.end())
                names.emplace(name);
        }

        for (auto imports : tree->getMetadata().packageImports)
        {
            for (auto import_item : imports->items)
            {
                auto name = import_item->package.valueText();
                if (!name.empty() &&
                    known_names.find(name) == known_names.end())
                    continue;
                names.emplace(name);
            }
        }
    };

    for (auto tree : compilation.getSyntaxTrees())
        find_missing_names(tree, missing_names);

    std::unordered_set<std::string_view> next_missing_names;
    while (true) {
        for (auto name: missing_names) {
            auto [kind, line, column, id1, id2, file, time] = lib->get(std::string(name), std::nullopt);

            if (kind == sv::library_cell_kind::invalid)
                continue;

            auto buffer = sm.readSource(file);
            if (!buffer)
            {
                continue;
            }
            auto tree = slang::syntax::SyntaxTree::fromBuffer(buffer, sm, options);
            tree->isLibrary = true;
            compilation.addSyntaxTree(tree);
            add_known_names(tree);
            find_missing_names(tree, next_missing_names);
        }

        if (next_missing_names.empty())
            break;

        missing_names = std::move(next_missing_names);
        next_missing_names.clear();
    }

    invalidated_ = false;
    return false;
}

void sv::ast::invalidate_main_file()
{
    invalidated_ = true;
}

const std::tuple<slang::Diagnostics, slang::Diagnostics>
sv::ast::get_diagnostics()
{
    return std::make_tuple(compilation.getParseDiagnostics(),
                           compilation.getSemanticDiagnostics());
}
