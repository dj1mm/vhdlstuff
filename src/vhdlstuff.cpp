
#include <iostream>
#include <string>
#include <vector>

#include "args.hxx"
#include "loguru.h"

#include "version.h"
#include "things/language.h"

using arg_it = std::vector<std::string>::const_iterator;
using command = std::function<int(const std::string&, arg_it, arg_it)>;

int main(int argc, char** argv)
{

    // make a new ArgumentParser
    const std::vector<std::string> arguments{argv + 1, argv + argc};
    args::ArgumentParser parser("Stuffs for VHDL");

    args::HelpFlag h(parser, "help", "print this help message", {'h', "help"});
    parser.Prog(argv[0]);

    args::Flag                   v(parser, "version", "output version information", {'v', "version"});
    args::Flag                   s(parser, "stderr" , "output all logs to stderr",  {"s", "stderr"});
    args::ValueFlag<std::string> l(parser, "path"   , "output logs to this file",   {     "logfile"});
    args::ValueFlag<std::string> t(parser, "path"   , "write trace to this file",   {     "trace"});
    args::ValueFlag<std::string> j(parser, "path"   , "write journal to this file", {     "journal"});
    args::ValueFlag<std::string> r(parser, "path"   , "replay this journal file",   {     "replay"});

    int status = 0;
    try
    {
        auto next = parser.ParseArgs(arguments);

        if (s)
        {
            loguru::g_stderr_verbosity = loguru::Verbosity_MAX;
        }
        else if (r)
        {
            loguru::g_stderr_verbosity = loguru::Verbosity_1;
        }
        else
        {
            loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
        }

        loguru::init(argc, argv, "--verbosity");

        if (l)
        {
            loguru::add_file(get(l).c_str(), loguru::Truncate,
                             loguru::Verbosity_MAX);
        }

        if (v)
        {
            LOG_S(INFO) << "Vhdlstuff " << things::build_hash << " (" << things::build_branch << ") " << things::build_tag;
            std::cout << "Vhdlstuff " << things::build_hash << " (" << things::build_branch << ") " << things::build_tag << "\n";
            return status;
        }

        if (r)
        {
            lsp::replay connection(args::get(r));
            things::language server(&connection);
            server.run();

            connection.print_status();
        }
        else
        {
        lsp::stdio connection;
        if (j && connection.tee(args::get(j)))
        {
            LOG_S(INFO) << "Writing journal to " << get(j);
        }
        else if (j)
        {
            LOG_S(ERROR) << "Unable to write journal to " << get(j);
        }

        things::language server(&connection);
        server.run();
        }
    }
    catch (const args::Completion& e)
    {
        LOG_S(ERROR) << e.what();
    }
    catch (const args::Help&)
    {
        std::cout << parser;
        std::cout << "Build " << things::build_hash << " (" << things::build_branch << ") " << "\n";
        std::cout << "Run as a language server over stdin and stdout" << std::endl;
    }
    catch (const args::ParseError& e)
    {
        LOG_S(ERROR) << e.what();
        std::cerr << parser;
        std::cerr << e.what() << "\n\nDo " << argv[0]
                  << " --help for more information" << std::endl;
        status = 1;
    }
    catch (const args::Error& e)
    {
        LOG_S(ERROR) << e.what();
        std::cerr << parser;
        std::cerr << "\n\nThere was an unexpected error. Do " << argv[0]
                  << " --help for more information" << std::endl;
        status = 1;
    }
    catch (const std::invalid_argument& e)
    {
        LOG_S(ERROR) << "Invalid argument exception thrown: " << e.what();
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


