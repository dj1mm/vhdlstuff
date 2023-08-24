
#include "server.h"

lsp::server::server(lsp::connection* connection)
: started_(false),shutdown_(false)
{
    frontend = std::make_unique<lsp::frontend>(connection, this);

    using namespace std::placeholders;
    frontend->bind("initialize",  std::bind(&lsp::server::on_initialize , this, _1));
    frontend->bind("initialized", std::bind(&lsp::server::on_initialized, this, _1));
    frontend->bind("shutdown",    std::bind(&lsp::server::on_shutdown   , this, _1));
    frontend->bind("exit",        std::bind(&lsp::server::on_exit       , this, _1));
}

bool lsp::server::run()
{
    setup();

    auto exited = frontend->forever_loop();
    return exited && shutdown_;
}

void lsp::server::on_initialize(std::shared_ptr<lsp::incoming_request> request)
{
    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> w(s);
    
    w.StartObject();
    w.Key("capabilities");
        w.StartObject();
        w.Key("textDocumentSync");
            w.StartObject();
            w.Key("openClose"); w.Bool(true);
            w.Key("change");    w.Uint(2);
            w.Key("save");      w.Bool(true);
            w.EndObject();
        // todo fill in the blanks for hoverProvider
        // todo fill in the blanks for definitionProvider
        w.EndObject();
    w.EndObject();
 
    json::string json = s.GetString();
    request->reply(json);
}

void lsp::server::on_initialized(std::shared_ptr<lsp::incoming_notification> notification)
{

}

void lsp::server::on_shutdown(std::shared_ptr<lsp::incoming_request> request)
{
    shutdown_ = true;
    request->reply(json::null_value);
}

void lsp::server::on_exit(std::shared_ptr<lsp::incoming_notification> notification)
{

}


