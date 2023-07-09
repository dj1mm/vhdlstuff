
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "version.h"

#include "sv/ast.h"
#include "sv/library_manager.h"

#include "slang/parsing/Preprocessor.h"
#include "slang/parsing/Parser.h"
#include "slang/ast/Compilation.h"
#include "slang/util/BumpAllocator.h"
#include "slang/text/SourceManager.h"
#include "slang/syntax/SyntaxPrinter.h"
#include "slang/diagnostics/DiagnosticEngine.h"
#include "slang/diagnostics/TextDiagnosticClient.h"

// this define is needed before the include format.h. Otherwise will get an
// `undefined reference to `fmt::v8::vformat` compile error
#define FMT_HEADER_ONLY
#include "fmt/args.h"
#include "fmt/format.h"


#include "vhdl/ast.h"
#include "vhdl/lexer.h"
#include "vhdl/library_manager.h"
#include "vhdl/parser.h"
#include "vhdl/binder.h"

#include "vhdl_syntax_debug.h"

#include "args.hxx"
#include "loguru.h"

int debug_tokens(std::string file, std::filesystem::path path, bool stats = false)
{
    auto zero = std::chrono::high_resolution_clock::now();

    if (path.extension() == ".sv")
    {
        slang::SourceManager sm;
        // auto sm = std::make_shared<slang::SourceManager>();
        slang::BumpAllocator alloc;
        slang::Diagnostics diagnostics;
        slang::parsing::Preprocessor pp(sm, alloc, diagnostics);

        auto buffer = sm.readSource(file);
        if (!buffer)
            throw std::invalid_argument(fmt::format("Unable to open {}", file));

        pp.pushSource(buffer);

        slang::syntax::SyntaxPrinter output;
        while (true) {
            auto token = pp.next();
            output.print(token);
            if (token.kind == slang::parsing::TokenKind::EndOfFile) break;
        }

        std::cout << output.str() << std::endl;
        return 0;
    }

    // else this is a vhdl file
    std::ifstream content(file);
    if (!content.good())
        throw std::invalid_argument(fmt::format("Unable to open {}", file));

    content.seekg(0, std::ios::end);
    auto size = content.tellg();
    std::string buffer(size, ' ');
    content.seekg(0);
    content.read(&buffer[0], size);

    auto p = std::filesystem::path(file);
    if (p.is_relative()) {
        p = std::filesystem::canonical(file);
    }

    common::stringtable st;
    std::vector<common::diagnostic> diags;

    auto one = std::chrono::high_resolution_clock::now();
    vhdl::lexer lexer(&buffer[0], &buffer[buffer.length()], &st, &diags, p.string());
    lexer.scan();

    auto two = std::chrono::high_resolution_clock::now();
    while (lexer.current_token() != vhdl::token::eof)
    {
        std::cout << lexer.scan().debug() << "\n";
    }
    auto three = std::chrono::high_resolution_clock::now();
    if (stats)
    {
        std::cout << "Lexer took: " << std::chrono::duration_cast<std::chrono::microseconds>(two-one).count()    << "us"
                             "  / " << std::chrono::duration_cast<std::chrono::milliseconds>(three-two).count()  << "ms"
                             "  / " << std::chrono::duration_cast<std::chrono::milliseconds>(three-zero).count() << "ms\n";
    }

    return diags.size() == 0? 0 : 1 << 2;
}

int debug_analysis(std::string file, std::filesystem::path path, std::string work, bool ast = false, bool stats = false)
{
    auto cwd = std::filesystem::current_path();

    if (path.extension() == ".sv")
    {
        auto manager = std::make_shared<sv::library_manager>(cwd.string());
        sv::ast tree(path.string(), manager, work);
        tree.update();
        manager.reset();
        
        auto [parse_errors, semantic_errors ] = tree.get_diagnostics();
        auto number_of_parse_errors = parse_errors.size();
        auto number_of_semantic_errors = semantic_errors.size();

        
        slang::DiagnosticEngine diagnostic_engine(tree.sm);
        auto diagnostic_client = std::make_shared<slang::TextDiagnosticClient>();
        diagnostic_engine.addClient(diagnostic_client);
        for (auto& it : parse_errors)
        {
            diagnostic_engine.issue(it);
        }

        for (auto& it : semantic_errors)
        {
            diagnostic_engine.issue(it);
        }
        std::string errors = diagnostic_client->getString();
        std::cout << errors << std::endl;
        return 0;
    }

    // else this is a vhdl file
    auto manager = std::make_shared<vhdl::library_manager>(cwd.string());

    vhdl::ast tree(path.string(), manager, work);
    tree.update();
    
    auto [parse_errors, semantic_errors ] = tree.get_diagnostics();
    auto number_of_parse_errors = parse_errors.size();
    auto number_of_semantic_errors = semantic_errors.size();
    for (auto& it : parse_errors)
    {
        fmt::dynamic_format_arg_store<fmt::format_context> args;
        for (auto& arg : it.args)
        {
            if (std::holds_alternative<std::string>(arg))
                args.push_back(std::get<std::string>(arg));
            if (std::holds_alternative<int>(arg))
                args.push_back(std::get<int>(arg));
        }
        std::cout << it.location << "\n";
        std::cout << fmt::vformat(it.format, args) << "\n\n";
    }

    if (ast)
    {
        vhdl::syntax::vhdl_syntax_debug d(std::cout);
        tree.get_main_file()->traverse(d);
    }

    if (stats)
    {
        std::cout << number_of_parse_errors << " errors" << "\n";
    }

    int result = 0;
    result |= (number_of_parse_errors    == 0 ? 0 : 1 << 2);
    // result |= (number_of_semantic_errors == 0 ? 0 : 1 << 3);
    return result;
}

int main(int argc, char** argv)
{
    loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
    loguru::init(argc, argv, "--verbosity");

    // make a new ArgumentParser
    const std::vector<std::string> arguments{argv + 1, argv + argc};
    args::ArgumentParser parser("Debug stuffs");


    args::Positional<std::string> f(parser, "file"  , "compile this file");
    args::ValueFlag<std::string>  w(parser, "name"  , "work library",               {     "work"}, "work");
    args::ValueFlag<std::string>  t(parser, "path"  , "write trace to this file",   {     "trace"});

    args::Flag                    k(parser, "tokens", "debug tokens",               {     "tokens"});
    args::Flag                    p(parser, "ast"   , "debug the parse ast",        {     "ast"});

    args::Flag                   s(parser, "stats"  , "print statistics",           {'s', "stats"});
    args::Flag                   v(parser, "version", "output version information", {'v', "version"});
    args::HelpFlag               h(parser, "help"   , "print this help message"   , {'h', "help"});

    int status = 0;
    try
    {
        parser.ParseArgs(arguments);

        if (v)
        {
            std::cout << "Debugstuff " << things::build_hash << " (" << things::build_branch << ") " << things::build_tag << "\n";
            return status;
        }

        if (!f)
        {
            throw args::ParseError("File is required");
        }

        auto path = std::filesystem::path(f.Get());
        if (path.is_relative())
        {
            path = std::filesystem::canonical(path);
        }

        if (k)
        {
            return debug_tokens(f.Get(), path, s);
        }

        return debug_analysis(f.Get(), path, w.Get(), p, s);
    }
    catch (const args::Completion& e)
    {
        LOG_S(ERROR) << e.what();
    }
    catch (const args::Help&)
    {
        std::cout << parser;
        std::cout << "Build " << things::build_hash << " (" << things::build_branch << ") " << things::build_tag << "\n";
        std::cout << "Debug stuffs" << std::endl;
    }
    catch (const args::ParseError& e)
    {
        std::cerr << parser;
        std::cerr << e.what() << "\n\nDo " << argv[0]
                  << " --help for more information" << std::endl;
        status = 1;
    }
    catch (const args::Error& e)
    {
        std::cerr << parser;
        std::cerr << "\n\nThere was an unexpected error. Do " << argv[0]
                  << " --help for more information" << std::endl;
        status = 1;
    }
    catch (const std::invalid_argument& e)
    {
        LOG_S(ERROR) << "Invalid argument exception thrown: " << e.what();
        std::cerr << e.what() << std::endl;
        status = 1;
    }
    catch (const std::logic_error& e)
    {
        LOG_S(ERROR) << "Logic error thrown: " << e.what();
        status = 1;
    }
    catch (const std::exception& e)
    {
        LOG_S(ERROR) << "Uncaught exception " << e.what();
        status = 1;
    }

    return status;
}

