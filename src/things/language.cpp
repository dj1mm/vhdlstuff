
#include "language.h"

#include "common/loguru.h"
#include "vhdl/ast.h"

#include "definition_provider.h"
#include "document_symbol_provider.h"
#include "folding_range_provider.h"
#include "hover_provider.h"

things::language::language(lsp::connection* connection)
    : server(connection), client(frontend.get()), project(this, &client),
      working_files(this, &client, false)
// False there means that working_files will have a seperate thread for each
// file that is `opened`
{
    frontend->on_diagnose([](std::string diagnostic) {
        LOG_S(ERROR) << "Frontend: " << diagnostic;
    });

    LOG_S(INFO) << "Language Server constructed";
}

things::language::~language()
{
    LOG_S(INFO) << "Language Server destroyed";
}

void things::language::setup()
{
    using namespace std::placeholders;
    frontend->bind("textDocument/didOpen",        std::bind(&things::language::on_text_document_did_open,      this, _1));
    frontend->bind("textDocument/didSave",        std::bind(&things::language::on_text_document_did_save,      this, _1));
    frontend->bind("textDocument/didClose",       std::bind(&things::language::on_text_document_did_close,     this, _1));
    frontend->bind("textDocument/didChange",      std::bind(&things::language::on_text_document_did_change,    this, _1));
    frontend->bind("textDocument/foldingRange",   std::bind(&things::language::on_text_document_folding_range, this, _1));
    frontend->bind("textDocument/documentSymbol", std::bind(&things::language::on_text_document_symbol,        this, _1));
    frontend->bind("textDocument/hover",          std::bind(&things::language::on_text_document_hover,         this, _1));
    frontend->bind("textDocument/definition",     std::bind(&things::language::on_text_document_definition,    this, _1));

    //
    frontend->bind("workspace/didChangeWatchedFiles", std::bind(&things::language::on_workspace_did_change_watched_files, this, _1));

}

void things::language::on_initialize(
    std::shared_ptr<lsp::incoming_request> request)
{
    LOG_S(INFO) << "Language Server initializing";

    {
    rapidjson::Document r;
    r.Parse(request->params->str);
    if (r.HasMember("rootUri") && r["rootUri"].IsString()) {
        std::string raw_uri = r["rootUri"].GetString();

        if (raw_uri.compare(0, 7, "file://"))
            capabilities.root = raw_uri;
        else
            capabilities.root = raw_uri.substr(7);
    }
    }

    {
    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> w(s);
    
    w.StartObject();
    w.Key("capabilities");
        w.StartObject();
        w.Key("textDocumentSync");
            w.StartObject();
            w.Key("openClose"); w.Bool(true);
            w.Key("change");    w.Uint(0);
            w.Key("save");      w.Bool(true);
            w.EndObject();
        w.Key("hoverProvider"); w.Bool(true);
        w.Key("definitionProvider"); w.Bool(true);
        w.Key("foldingRangeProvider"); w.Bool(true);
        w.Key("documentSymbolProvider"); w.Bool(true);
        w.EndObject();
    w.EndObject();
 
    json::string json = s.GetString();
    request->reply(json);
    }

    project.initialise(capabilities.root);

}

void things::language::on_initialized(
    std::shared_ptr<lsp::incoming_notification> notification)
{
    LOG_S(INFO) << "Language Server initialized";
    project.load_yaml_reset_project_kick_background_index_destroy_libraries();
}

void things::language::on_shutdown(
    std::shared_ptr<lsp::incoming_request> request)
{
    LOG_S(INFO) << "Language Server shutdown";

    request->reply(json::null_value);
}

void things::language::on_exit(
    std::shared_ptr<lsp::incoming_notification> notification)
{
    LOG_S(INFO) << "Language Server exit";
}

void things::language::on_text_document_did_open(
    std::shared_ptr<lsp::incoming_notification> notification)
{
    LOG_S(INFO) << "Language Server textDocument/didOpen";

    auto param =
        serialize::from_json<lsp::text_document_did_open_save_close_params>(
            notification->params.value());
    working_files.update(param.text_document.uri.get_path());
}

void things::language::on_text_document_did_save(
    std::shared_ptr<lsp::incoming_notification> notification)
{
    LOG_S(INFO) << "Language Server textDocument/didSave";

    auto param =
        serialize::from_json<lsp::text_document_did_open_save_close_params>(
            notification->params.value());
    working_files.update(param.text_document.uri.get_path());
}

void things::language::on_text_document_did_close(
    std::shared_ptr<lsp::incoming_notification> notification)
{
    LOG_S(INFO) << "Language Server textDocument/didClose";

    auto param =
        serialize::from_json<lsp::text_document_did_open_save_close_params>(
            notification->params.value());
    working_files.remove(param.text_document.uri.get_path());
    client.send_diagnostics(param.text_document.uri.get_path(), {});
}

void things::language::on_text_document_did_change(
    std::shared_ptr<lsp::incoming_notification> notification)
{
    LOG_S(INFO) << "Language Server textDocument/didChange";
}

void things::language::on_text_document_folding_range(
    std::shared_ptr<lsp::incoming_request> request)
{
    LOG_S(INFO) << "Language Server textDocument/foldingRange";

    auto calculate_folding_ranges =
        [r = request](std::shared_ptr<vhdl::ast> ast) {

            if (!ast || !ast->get_main_file()) {
                r->reply(json::string("[]"));
                return;
            }

        {
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> w(s);

        w.StartArray();
        vhdl_folding_range_provider d(&w);
        ast->get_main_file()->traverse(d);
        w.EndArray();
    
        json::string json = s.GetString();
        r->reply(json);
        }
    };

    auto param = serialize::from_json<lsp::folding_range_params>(
        request->params.value());
    working_files.run_with_ast(param.text_document.uri.get_path(),
                               calculate_folding_ranges);
}

void things::language::on_text_document_symbol(
    std::shared_ptr<lsp::incoming_request> request)
{
    LOG_S(INFO) << "Language Server textDocument/documentSymbol";

    auto get_document_symbols = [r = request](std::shared_ptr<vhdl::ast> ast) {

        if (!ast || !ast->get_main_file()) {
            r->reply(json::string("[]"));
            return;
        }

        {
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> w(s);

        w.StartArray();
        vhdl_document_symbol_provider d(&w);
        ast->get_main_file()->traverse(d);
        
        w.EndArray();
    
        json::string json = s.GetString();
        r->reply(json);
        }
    };

    auto param = serialize::from_json<lsp::document_symbols_params>(
        request->params.value());
    working_files.run_with_ast(param.text_document.uri.get_path(),
                               get_document_symbols);
}

void things::language::on_text_document_hover(
    std::shared_ptr<lsp::incoming_request> request)
{
    LOG_S(INFO) << "Language Server textDocument/hover";

    auto param = serialize::from_json<lsp::text_document_hover_params>(
        request->params.value());

    common::position pos(param.position.line + 1, param.position.character + 1);
    auto get_hover = [r = request, pos](std::shared_ptr<vhdl::ast> ast) {
        if (!ast || !ast->get_main_file())
        {
            r->reply(json::null_value);
            return;
        }

        {
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> w(s);

        bool found = false;
        vhdl_hover_provider d(&w, found, pos);
        ast->get_main_file()->traverse(d);
    
        if (!found) {
            r->reply(json::null_value);
            return;
        }

        json::string json = s.GetString();
        r->reply(json);
        }
    };

    working_files.run_with_ast(param.text_document.uri.get_path(), get_hover);
}

void things::language::on_text_document_definition(
    std::shared_ptr<lsp::incoming_request> request)
{
    LOG_S(INFO) << "Language Server textDocument/definition";

    auto param = serialize::from_json<lsp::text_document_hover_params>(
        request->params.value());

    common::position pos(param.position.line + 1, param.position.character + 1);
    auto get_definition = [r = request, pos](std::shared_ptr<vhdl::ast> ast) {
        if (!ast || !ast->get_main_file())
        {
            r->reply(json::null_value);
            return;
        }

        {
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> w(s);

        bool found = false;
        vhdl_definition_provider d(&w, found, pos);
        ast->get_main_file()->traverse(d);
    
        if (!found) {
            r->reply(json::null_value);
            return;
        }

        json::string json = s.GetString();
        r->reply(json);
        }
    };

    working_files.run_with_ast(param.text_document.uri.get_path(),
                               get_definition);
}

void things::language::on_workspace_did_change_watched_files(
    std::shared_ptr<lsp::incoming_notification> notification)
{
    LOG_S(INFO) << "Language Server workspace/didChangeWatchedFiles";
    project.load_yaml_reset_project_kick_background_index_destroy_libraries();
}
