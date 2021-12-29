
#include "client.h"

lsp::client::client(lsp::frontend* fe)
: frontend(fe)
{

}

bool lsp::client::notify(std::string method, std::optional<json::string> params)
{
    auto message = std::make_shared<lsp::outgoing_notification>();
    message->method = std::move(method);
    message->params = std::move(params);
    return frontend->send(message);
}

bool lsp::client::request(
    std::string method,
    std::optional<json::string> params,
    std::function<void(std::shared_ptr<lsp::incoming_response>)> on_response
) {
    auto id = frontend->get_id_for_next_outgoing_request();
    auto message = std::make_shared<lsp::outgoing_request>(std::move(on_response));
    message->id = id;
    message->method = method;
    message->params = std::move(params);
    return frontend->send(message);
}

