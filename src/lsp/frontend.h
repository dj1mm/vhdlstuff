
#ifndef LSP_FRONTEND_H
#define LSP_FRONTEND_H

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <variant>

#include "common/cancellation.h"
#include "common/json.h"

namespace lsp
{

// forward declaration needed by incoming_request
class frontend;
class connection;

struct incoming_request: std::enable_shared_from_this<incoming_request>
{
    incoming_request(lsp::frontend*, int, common::cancellation_token&&);
    incoming_request() = default;
    incoming_request(const incoming_request&) = default;
    incoming_request(incoming_request&&) = default;
    incoming_request& operator=(const incoming_request&) = default;
    incoming_request& operator=(incoming_request&&) = default;
    ~incoming_request();

    std::variant<int, std::string> id;
    std::string method;
    std::optional<json::string> params;

    bool is_cancelled();

    void reply(std::optional<std::variant<int, bool, std::string, json::string, json::null>> data);
    void error(int code, std::string message, std::optional<json::string> data);

    private:

    lsp::frontend* frontend_ = nullptr;
    int internal_request_id_ = 0;
    bool replied_ = false;

    common::cancellation_token token_;

    friend class frontend;
};

// forward declaration needed by outgoing_request
struct incoming_response;

struct outgoing_request
{
    outgoing_request(
        std::function<void(std::shared_ptr<lsp::incoming_response>)>);

    std::variant<int, std::string> id;
    std::string method;
    std::optional<json::string> params;

    json::string to_json() const;

    private:

    std::function<void(std::shared_ptr<lsp::incoming_response>)> on_response_;

    friend class frontend;
};

namespace error_code
{
    // Defined by JSON RPC
    inline constexpr int parse_error = -32700;
    inline constexpr int invalid_request = -32600;
    inline constexpr int method_not_found = -32601;
    inline constexpr int invalid_params = -32602;
    inline constexpr int internal_error = -32603;
    inline constexpr int server_error_start = -32099;
    inline constexpr int server_error_end = -32000;
    inline constexpr int server_not_initialized = -32002;
    inline constexpr int unknown_error_code = -32001;

    // Defined by the protocol
    inline constexpr int request_cancelled = -32800;
    inline constexpr int content_modified = -32801;
};

struct incoming_response
{
    enum { invalid, success, error } state = invalid;

    std::variant<int, std::string, json::null> id; // on success
    std::optional<std::variant<int, bool, std::string, json::string, json::null>> result;
    int code = 0; // on error
    std::string message;
    std::optional<json::string> data;

    std::shared_ptr<lsp::outgoing_request> request;
};

struct outgoing_response
{
    enum { invalid, success, error } state = invalid;

    std::variant<int, std::string, json::null> id;
    std::optional<std::variant<int, bool, std::string, json::string, json::null>> result;
    int code;
    std::string message;
    std::optional<json::string> data;

    json::string to_json() const;

    std::weak_ptr<incoming_request> request;
};

struct incoming_notification
{
    std::string method;
    std::optional<json::string> params;
};

struct outgoing_notification
{
    std::string method;
    std::optional<json::string> params;

    json::string to_json() const;
};

// forward declaration needed by frontend
class server;
class client;

class frontend
{
    public:
    frontend(lsp::connection* connection, lsp::server* server);

    bool forever_loop();

    void on_diagnose(std::function<void(std::string)> callback);

    // bind notification. When notification is received, call passed callback
    void bind(std::string method, std::function<void(std::shared_ptr<lsp::incoming_notification>)> callback);

    // bind request. When request is received, call passed callback
    void bind(std::string method, std::function<void(std::shared_ptr<lsp::incoming_request>)> callback);

    private:

    // handle a jsonrpc message coming from connection
    bool handle(std::string message);

    // got jsonrpc message and it is valid json
    // checked and it is an incoming request. handle it
    bool handle(
        std::variant<int, std::string> id, std::string method,
        std::optional<json::string> params
    );

    // got jsonrpc message and it is valid json
    // checked and it is an incoming successful response. handle it
    bool handle(
        std::variant<int, std::string, json::null> id,
        std::optional<std::variant<int, bool, std::string, json::string, json::null>> result
    );

    // got jsonrpc message and it is valid json
    // checked and it is an incoming error response. handle it
    bool handle(
        std::variant<int, std::string, json::null> id,
        int code, std::string message, std::optional<json::string> data
    );

    // got jsonrpc message and it is valid json
    // checked and it is an incoming notification. handle it
    bool handle(std::string method, std::optional<json::string> params);

    // special notifications need special handlers
    bool handle_exit_notification();
    bool handle_cancel_request_notification(lsp::incoming_notification notification);

    // we wanna send a request. get request id
    int get_id_for_next_outgoing_request();

    // lsp message is coming from us, the server and we want to send it to the
    // client
    bool send(std::shared_ptr<lsp::outgoing_notification> notification);
    bool send(std::shared_ptr<lsp::outgoing_request> request);
    bool send(std::shared_ptr<lsp::outgoing_response> response);

    std::map<std::string, std::function<void(std::shared_ptr<lsp::incoming_notification>)>> notification_handlers;
    std::map<std::string, std::function<void(std::shared_ptr<lsp::incoming_request>)>>      request_handlers;

    int next_outgoing_request_id = 0;
    mutable std::mutex outgoing_requests_mutex;
    std::map<
        std::variant<int, std::string>,
        std::shared_ptr<lsp::outgoing_request>
    > outgoing_requests_in_flight;

    int next_internal_request_id = 0;
    mutable std::mutex incoming_requests_mutex;
    std::map<
        std::variant<int, std::string>,
        std::pair<common::cancellation_source, std::shared_ptr<lsp::incoming_request>>
    > incoming_requests_in_flight;

    lsp::connection* connection_ = nullptr;
    lsp::server* server_ = nullptr;
    std::atomic_bool is_running_ = false;

    friend class client;
    friend class incoming_request;

    //
    // string printf utility- returns a formatted std::string as per format and
    // variable list of argument
    //
    void diagnose(const char* format, ...);

    //
    // string printf utility- returns a formatted std::string as per format and
    // variable list of argument
    //
    std::string inline sprintf(const char* format, va_list args);

    std::function<void(std::string)> on_diagnose_;


};

}

#endif
