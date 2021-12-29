
#include "frontend.h"

#include "connection.h"
#include "server.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"

#include <cstdarg>

lsp::incoming_request::incoming_request(lsp::frontend* f, int i, common::cancellation_token&& t)
: frontend_(f), internal_request_id_(i), replied_(false), token_(std::move(t))
{}

lsp::incoming_request::~incoming_request()
{
    if (!replied_)
    {
        frontend_->diagnose("Havent replied to request: %s", method.c_str());
        // assert(false && "Server must reply to all requests");
    }
}

bool lsp::incoming_request::is_cancelled()
{
    return token_.is_cancelled();
}

void lsp::incoming_request::reply(std::optional<std::variant<int, bool, std::string, json::string, json::null>> data)
{
    auto message = std::make_shared<lsp::outgoing_response>();
    message->state = lsp::outgoing_response::success;
    if (std::holds_alternative<int>(id))              message->id = std::get<int>(id);
    else if (std::holds_alternative<std::string>(id)) message->id = std::get<std::string>(id);
    else                                              message->id = json::null_value;
    message->result = std::move(data);
    message->request = weak_from_this();

    frontend_->send(message);
}

void lsp::incoming_request::error(int code, std::string m, std::optional<json::string> data)
{
    auto message = std::make_shared<lsp::outgoing_response>();
    message->state = lsp::outgoing_response::error;
    if (std::holds_alternative<int>(id))              message->id = std::get<int>(id);
    else if (std::holds_alternative<std::string>(id)) message->id = std::get<std::string>(id);
    else                                              message->id = json::null_value;
    message->code = code;
    message->message = m;
    message->data = data;
    message->request = weak_from_this();

    frontend_->send(message);
}

lsp::outgoing_request::outgoing_request(
    std::function<void(std::shared_ptr<lsp::incoming_response>)> cb)
    : on_response_(cb)
{
}

json::string lsp::outgoing_request::to_json() const
{
    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);

    writer.StartObject();

    writer.Key("jsonrpc");
    writer.String("2.0");

    writer.Key("id");
    if (std::holds_alternative<int>(id))
        writer.Int(std::get<int>(id));
    else if (std::holds_alternative<std::string>(id))
        writer.String(std::get<std::string>(id));
    else
        throw std::runtime_error("invalid request id");

    if (params.has_value()) {
        auto& json = params.value();
        rapidjson::Type type;
        switch (json.str.at(0)) {
        case '{': type = rapidjson::kObjectType; break;
        case '[': type = rapidjson::kArrayType;  break;
        default:  type = rapidjson::kNullType;   break;
        }
        writer.Key("params");
        writer.RawValue(json.str.c_str(), json.str.length(), type);
    }

    writer.Key("method");
    writer.String(method);

    writer.EndObject();
    return s.GetString();

}

json::string lsp::outgoing_response::to_json() const
{
    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);

    writer.StartObject();

    writer.Key("jsonrpc");
    writer.String("2.0");

    writer.Key("id");
    if (std::holds_alternative<int>(id))
        writer.Int(std::get<int>(id));
    else if (std::holds_alternative<std::string>(id))
        writer.String(std::get<std::string>(id));
    else if (std::holds_alternative<json::null>(id))
        writer.Null();

    if (state == success && result.has_value())
    {
        writer.Key("result");
        auto& r = result.value();
        if (std::holds_alternative<int>(r))
            writer.Int(std::get<int>(r));
        else if (std::holds_alternative<std::string>(r))
            writer.String(std::get<std::string>(r));
        else if (std::holds_alternative<bool>(r))
            writer.Bool(std::get<bool>(r));
        else if (std::holds_alternative<json::null>(r))
            writer.Null();
        else if (std::holds_alternative<json::string>(r))
        {
            auto json = std::get<json::string>(r);
            rapidjson::Type type;
            switch (json->at(0)) {
            case '{': type = rapidjson::kObjectType; break;
            case '[': type = rapidjson::kArrayType;  break;
            default:  type = rapidjson::kNullType;   break;
            }
            writer.RawValue(json->c_str(), json->length(), type);
        }
        else
            throw std::runtime_error("unknown response result");
    }

    if (state == error)
    {
        writer.Key("error");
        writer.StartObject();

        writer.Key("code");
        writer.Int(code);

        writer.Key("message");
        writer.String(message);

        if (data.has_value()) {
            auto d = data.value();
            auto length = d->length();
            rapidjson::Type type;
            switch (d->at(0)) {
            case '{': type = rapidjson::kObjectType; break;
            case '"': type = rapidjson::kStringType; break;
            case '[': type = rapidjson::kArrayType;  break;
            case 'n':
            case 'N': type = rapidjson::kNullType;     break;
            case 't':
            case 'T': type = rapidjson::kTrueType;     break;
            case 'f':
            case 'F': type = rapidjson::kFalseType;  break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': type = rapidjson::kNumberType; break;
            default:  type = rapidjson::kNullType;   break;
            }
            writer.Key("data");
            writer.RawValue(d->c_str(), d->length(), type);
        }
        writer.EndObject();
    }

    writer.EndObject();
    return s.GetString();

}

json::string lsp::outgoing_notification::to_json() const
{
    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);

    writer.StartObject();

    writer.Key("jsonrpc");
    writer.String("2.0");

    writer.Key("method");
    writer.String(method);

    if (params.has_value()) {
        auto& json = params.value();
        rapidjson::Type type;
        switch (json.str.at(0)) {
        case '{': type = rapidjson::kObjectType; break;
        case '[': type = rapidjson::kArrayType;  break;
        default:  type = rapidjson::kNullType;   break;
        }
        writer.Key("params");
        writer.RawValue(json.str.c_str(), json.str.length(), type);
    }

    writer.EndObject();
    return s.GetString();

}

lsp::frontend::frontend(lsp::connection* connection, lsp::server* server)
: connection_(connection), server_(server), is_running_(true)
{}

bool lsp::frontend::forever_loop()
{
    while (is_running_)
    {
        auto msg = connection_->read();

        if (!connection_->good())
        {
            return false;
        }

        if (!msg.has_value())
            continue;

        handle(std::move(msg.value()));
    }

    return !is_running_;
}

void lsp::frontend::on_diagnose(std::function<void(std::string)> callback)
{
    on_diagnose_ = callback;
}

void lsp::frontend::bind(std::string method, std::function<void(std::shared_ptr<lsp::incoming_notification>)> callback)
{
    notification_handlers[method] = callback;
}

void lsp::frontend::bind(std::string method, std::function<void(std::shared_ptr<lsp::incoming_request>)> callback)
{
    request_handlers[method] = callback;
}

bool lsp::frontend::handle(std::string msg)
{
    rapidjson::Document d;
    d.Parse(msg);

    if (d.HasParseError())
    {
        diagnose("Jsonrpc message could not be parsed");
        return false;
    }

    if (!d.IsObject())
    {
        diagnose("Jsonrpc message is not an object");
        return false;
    }

    bool is_request = d.HasMember("id") && d.HasMember("method");
    bool is_response = d.HasMember("id") && !d.HasMember("method");
    bool is_notification = !d.HasMember("id") && d.HasMember("method");

    if (is_request)
    {
        std::optional<json::string> params;
        if (d.HasMember("params"))
        {
            rapidjson::StringBuffer s;
            rapidjson::Writer<rapidjson::StringBuffer> writer(s);
            d["params"].Accept(writer);
            params = std::make_optional<json::string>(s.GetString());
        }

        std::variant<int, std::string> id;
        if (d["id"].IsInt())
            id = d["id"].GetInt();
        else if (d["id"].IsString())
            id = d["id"].GetString();
        else
        {
            diagnose("Jsonrpc message is a request with invalid id");
            return false;
        }

        return handle(id, d["method"].GetString(), std::move(params));
    }
    else if (is_response)
    {
        std::variant<int, std::string, json::null> id;
        if (d["id"].IsInt())
            id = d["id"].GetInt();
        else if (d["id"].IsString())
            id = d["id"].GetString();
        else if (d["id"].IsNull())
            id = json::null_value;
        else
        {
            diagnose("Jsonrpc message is a response with invalid id");
            return false;
        }

        if (d.HasMember("result"))
        {
            rapidjson::StringBuffer s;
            rapidjson::Writer<rapidjson::StringBuffer> writer(s);
            d["result"].Accept(writer);

            std::optional<std::variant<int, bool, std::string, json::string, json::null>> result;
            if (d["result"].IsInt())
                result = d["result"].GetInt();
            else if (d["result"].IsString())
                result = std::string(d["result"].GetString());
            else if (d["result"].IsNull())
                result = json::null_value;
            else if (d["result"].IsBool())
                result = d["result"].GetBool();
            else if (d["result"].IsObject())
                result = json::string(s.GetString());
            else
                diagnose("Jsonrpc message is a response with invalid result");

            return handle(id, result);
        }

        if (d.HasMember("error"))
        {
            int code = d["error"]["id"].GetInt();
            std::string message = d["error"]["message"].GetString();
            std::optional<json::string> data;
            if (d["error"].HasMember("data"))
            {
                rapidjson::StringBuffer s;
                rapidjson::Writer<rapidjson::StringBuffer> writer(s);
                d["result"]["data"].Accept(writer);
                data = s.GetString();
            }

            return handle(id, code, std::move(message), std::move(data));
        }

        diagnose("Couldnt tell if jsonrpc response is success or error");
        return false;
    }
    else if (is_notification)
    {
        std::optional<json::string> params;
        if (d.HasMember("params"))
        {
            rapidjson::StringBuffer s;
            rapidjson::Writer<rapidjson::StringBuffer> writer(s);
            d["params"].Accept(writer);
            params = std::make_optional<json::string>(s.GetString());
        }

        return handle(d["method"].GetString(), std::move(params));
    }

    diagnose("Jsonrpc message isn't a request, response nor notification");
    return false;
}

bool lsp::frontend::handle(std::string method, std::optional<json::string> params)
{
    if (method != "exit" && !server_->started_)
    {
        diagnose("Ignoring notification '%s' before initialization", method.c_str());
        return false;
    }

    if (method == "exit")
    {
        handle_exit_notification();
    }

    if (method == "$/cancelRequest")
    {
        lsp::incoming_notification message;
        message.method = method;
        message.params = std::move(params);
        return handle_cancel_request_notification(std::move(message));
    }
    auto it = notification_handlers.find(method);
    if (it == notification_handlers.end())
    {
        diagnose("Ignoring unhandled notification %s", method.c_str());
        return false;
    }

    auto message = std::make_shared<lsp::incoming_notification>();
    message->method = method;
    message->params = std::move(params);

    it->second(message);

    return true;
}

bool lsp::frontend::handle(
    std::variant<int, std::string> id, std::string method,
    std::optional<json::string> params
) {
    if (method == "initialize" && !server_->started_)
    {
        server_->started_ = true;
    }
    else if (method != "initialize" && !server_->started_)
    {
        diagnose("Couldn't handle request '%s' before initialization", method.c_str());

        auto message = std::make_shared<lsp::outgoing_response>();
        message->state = lsp::outgoing_response::error;
        message->code = lsp::error_code::server_not_initialized;
        message->message = "server not yet initialized";
        if (std::holds_alternative<int>(id))              message->id = std::get<int>(id);
        else if (std::holds_alternative<std::string>(id)) message->id = std::get<std::string>(id);
        else                                              message->id = json::null_value;

        send(message);
        return false;
    }

    if (method == "shutdown")
    {
        server_->shutdown_ = true;
    }

    auto it = request_handlers.find(method);
    if (it == request_handlers.end())
    {
        diagnose("Request handler not found for %s", method.c_str());

        auto message = std::make_shared<lsp::outgoing_response>();
        message->state = lsp::outgoing_response::error;
        message->code = lsp::error_code::method_not_found;
        message->message = "method not found";
        if (std::holds_alternative<int>(id))              message->id = std::get<int>(id);
        else if (std::holds_alternative<std::string>(id)) message->id = std::get<std::string>(id);
        else                                              message->id = json::null_value;

        send(message);
        return false;
    }

    auto internal_request_id = 0;
    {
        std::lock_guard<std::mutex> guard(incoming_requests_mutex);
        internal_request_id = ++next_internal_request_id;
    }

    common::cancellation_source source;

    auto message = std::make_shared<lsp::incoming_request>(this, internal_request_id, source.token());
    message->id = id;
    message->method = method;
    message->params = std::move(params);
    incoming_requests_in_flight[id] = std::make_pair(source, message);

    it->second(message);

    return true;
}

bool lsp::frontend::handle(
    std::variant<int, std::string, json::null> id,
    std::optional<std::variant<int, bool, std::string, json::string, json::null>> result
) {
    std::variant<int, std::string> _id;
    if (std::holds_alternative<int>(id))
    {
        _id = std::get<int>(id);
    }
    else if (std::holds_alternative<std::string>(id))
    {
        _id = std::get<std::string>(id);
    }
    else
    {
        diagnose("Received success response with unknown id");
        return false;
    }

    auto reply = std::make_shared<lsp::incoming_response>();
    {
        std::lock_guard<std::mutex> g(outgoing_requests_mutex);
        auto it = outgoing_requests_in_flight.find(_id);
        if (it == outgoing_requests_in_flight.end())
        {
            diagnose("Received success response but no requests were made");
            return false;
        }
        reply->state = lsp::incoming_response::success;
        reply->id = std::move(id);
        reply->result = std::move(result);
        reply->request = it->second;
        outgoing_requests_in_flight.erase(it);

        if (!reply->request->on_response_)
        {
            diagnose("Received success response but no handler was provided");
            return true;
        }
    }
    reply->request->on_response_(reply);
    return true;
}

bool lsp::frontend::handle(
    std::variant<int, std::string, json::null> id,
    int code, std::string message, std::optional<json::string> data
) {
    std::variant<int, std::string> _id;
    if (std::holds_alternative<int>(id))
    {
        _id = std::get<int>(id);
    }
    else if (std::holds_alternative<std::string>(id))
    {
        _id = std::get<std::string>(id);
    }
    else
    {
        diagnose("Received error response with unknown id");
        return false;
    }

    auto error = std::make_shared<lsp::incoming_response>();
    error->state = lsp::incoming_response::error;
    error->id = std::move(id);
    error->code = code;
    error->message = std::move(message);
    error->data = std::move(data);
    {
        std::lock_guard<std::mutex> g(outgoing_requests_mutex);
        auto it = outgoing_requests_in_flight.find(std::get<int>(id));
        if (it == outgoing_requests_in_flight.end())
        {
            diagnose("Received error response but no requests were made");
            return false;
        }
        error->request = it->second;
        outgoing_requests_in_flight.erase(it);
    }

    if (!error->request->on_response_)
    {
        diagnose("Received error response but no handler was provided");
        return true;
    }
    error->request->on_response_(error);
    return true;
}

bool lsp::frontend::handle_exit_notification()
{
    is_running_ = false;
    return true;
}

bool lsp::frontend::handle_cancel_request_notification(lsp::incoming_notification notification)
{
    rapidjson::Document d;
    d.Parse(notification.params.value().str);

    if (d.HasParseError())
    {
        diagnose("Unable to parse cancel request");
        return false;
    }

    if (!d.IsObject())
    {
        diagnose("Invalid cancel request");
        return false;
    }

    if (!d.HasMember("id"))
    {
        diagnose("Cancel request missing id");
        return false;
    }

    std::string id;
    if (d["id"].IsInt())
    {
        id = std::to_string(d["id"].GetInt());
    }
    else if (d["id"].IsString())
    {
        id = d["id"].GetString();
    }
    else
    {
        diagnose("Cancel request malformed");
        return false;
    }

    std::lock_guard<std::mutex> g(incoming_requests_mutex);
    auto it = incoming_requests_in_flight.find(id);
    if (it != incoming_requests_in_flight.end())
    {
        it->second.first.request_cancellation();
    }
    return true;
}

int lsp::frontend::get_id_for_next_outgoing_request()
{
    return ++next_outgoing_request_id;
}

bool lsp::frontend::send(std::shared_ptr<lsp::outgoing_notification> notification)
{
    connection_->write(notification->to_json().str);
    return true;
}

bool lsp::frontend::send(std::shared_ptr<lsp::outgoing_request> request)
{
    {
        std::lock_guard<std::mutex> g(outgoing_requests_mutex);
        outgoing_requests_in_flight[request->id] = request;
    }

    connection_->write(request->to_json().str);

    return true;
}

bool lsp::frontend::send(std::shared_ptr<lsp::outgoing_response> response)
{
    if (!response->request.expired())
    {
        auto request = response->request.lock();
        if (request->replied_)
        {
            diagnose("Only one reply per request");
            // assert(false && "Server must provide one reply per request");
            return false;
        }

        request->replied_ = true;
        std::lock_guard<std::mutex> g(incoming_requests_mutex);
        auto it = incoming_requests_in_flight.find(request->id);
        if (it != incoming_requests_in_flight.end())
        {
            if (it->second.second->internal_request_id_ == request->internal_request_id_)
            {
                incoming_requests_in_flight.erase(it);
            }
        }
    }

    connection_->write(response->to_json().str);

    return true;
}

void lsp::frontend::diagnose(const char* format, ...)
{
    if (!on_diagnose_)
        return;

    va_list args;
    va_start(args, format);
    auto diagnostic = sprintf(format, args);
    va_end(args);

    on_diagnose_(diagnostic);

}

std::string inline lsp::frontend::sprintf(const char* format, va_list args)
{

    // https://stackoverflow.com/questions/4182071/

    // first attempt with buffer 256. Idk if enough tho ¯\_(ツ)_/¯
    char buffer[256];
    const size_t n = vsnprintf(buffer, sizeof buffer, format, args) + 1;
    if (n <= sizeof buffer)
        return std::string(buffer, n);

    // nop it wasnt enough. Create a string of size n - previously reported
    // from sprintf, fill it in, and diagnose
    std::string diagnostic(n, ' ');
    vsnprintf(diagnostic.data(), n, format, args);

    return std::move(diagnostic);
}

