
# Abstract

Project: *vhdlstuff*

Vhdlstuff is a tool that tries to provide standard language features such as
breadcrumbs, go to symbols, code errors and all those related goodness to the
vhdl language. It has been designed to be text editor agnostic. To do this,
vhdlstuff implements the
[Language Server Protocol - 3.16](https://github.com/Microsoft/language-server-protocol/blob/gh-pages/_specifications/specification-3-16.md)
from Microsoft.

As a refresher, the language server protocol tries to specify a standard
communication protocol between a text editor and external tools such as
vhdlstuff. The protocol has the notion of clients and servers. In theory, it
should be possible for any client - such as any text editor - *speaking* the
language server protocol, to *talk* to any server - such as vhdlstuff.

# Introduction

A copy of the implemented language server protocol specification is saved in
`docs/lsp_specification.md`.

The gist of the protocol is that a language server communicates to a language
client via standard input and output (stdio). Messages are sent to and from the
server in the form of JSON RPC messages. While the server is busy working on a
message, it should still be able to accept subsequent messages.  
Aka asynchronous processing!

Vhdlstuff, being written in vanilla c++, needs a whole lot of infrastructure
around it to allow it to function as a language server.

This infrastructure has been greatly inspired and in some places shamefully
lifted from [clangd](https://clangd.llvm.org/) and
[ccls](https://github.com/MaskRay/ccls/).

# Frontend and the application

Vhdlstuff seperates the task of receiving a JSON RPC command and its execution
into a frontend and a backend (also known as the application or the server).

The frontend has the responsibility of receiving a JSON RPC command and calling
the correct function in the application.

When a JSON RPC message is received, the frontend interpretes it to see whether
this is an incoming request, a notification or a response. Once interpreted,
the frontend decodes the incoming message and calls the correct function in the
application.

On the otherhand, the application is essentially a bunch of callbacks. In
vhdlstuff these callbacks are called handlers and are called by the frontend.
The application processes the incoming message and generates response(s) to send
back to the server.  
This is what programmers have to write!

Note that the application is still running in the main thread and therefore,
long running functions should be run asynchronously.

> Note to self, it is definitely possible to modify the frontend to call every
> function in the application 'on its own thread'. The only reason I can think
> not to do so is maxing out the threads and stalling requests from the client.
> The benefit is that threads are abstracted away (just like an http server).

## Incoming request

This is how a callback in the application is written to process an incoming
request. Note that a request ***must*** send a response back to the client.

```cpp
void the_callback(std::shared_ptr<lsp::incoming_request> request)
{
    // if it takes a long time to process this request, processing should be
    // done on a seperate thread. Something like this
    auto task = [](auto request) -> void {
        auto params = serialize::from_json(request->params);
        // long running processings here
        auto json = serialize::to_json(result);
        request->reply(json);
    }
    thread_pool->run(std::move(task), std::move(request));
}
```

This is the c++ declaration for incoming requests:

```cpp
struct incoming_request
{
    std::variant<int, std::string> id;
    std::string method;
    std::optional<json::string> params;
    common::cancellation_token& token();
    void reply(std::optional<data> result);
    void error(int code,std::string message,std::optional<json::string> data);
};
```

## Incoming notification

This is how a callback in the application is written to process an incoming
notification. Note that a notification ***must not*** send a response back to
the client.

```cpp
void the_callback(std::shared_ptr<lsp::incoming_notification> notification)
{
    auto params = serialize::from_json(notification->params);
    // if it is a quick thing to do, do it right here
    auto json = serialize::to_json(result);
    request->reply(json);
}
```

This is the c++ declaration for incoming notification:

```cpp
struct incoming_notification
{
    std::string method;
    std::optional<json::string> params;
};
```

## Incoming response

The language server protocol allows the language server to send requests to a
client. For example, see the `window/workDoneProgress/create` request. This is
sent from the server to the client.

In this case, it is expected the client to respond back. This is how a callback
in the application is written to process an incoming response.

```cpp
void the_callback(std::shared_ptr<lsp::incoming_response> response)
{
    auto result = serialize::from_json(response->result);
    // if it is a quick thing to do, do it right here. Nothing to send back
}
```

This is the c++ declaration for incoming response:

```cpp
struct incoming_response
{
    enum { invalid, success, error } state = invalid;
    std::variant<int, std::string, json::null> id;
    std::optional<std::variant<int, bool, std::string, json::string, json::null>> result;
    int code;
    std::string message;
    std::optional<json::string> data;
    std::shared_ptr<lsp::outgoing_request> request;
};
```

# The frontend

This section describes the very boring implementation of the frontend and its
related classes.

## Connnection

The connection represents the way the language server and the language client
interract. Currently, vhdlstuff only supports the language server protocol
communication over standard input and output (stdio).

A connection object must have a blocking read and a thread safe write function.

```cpp
class lsp::stdio: lsp::connection
{
    // this is a blocking read function.
    // Returns std::nullopt on error
    // Or a string corresponding to an incoming lsp message
    std::optional<std::string>&& read();

    // this is a thread safe write function.
    // It outputs an outgoing lsp message to whatever stream
    void write(std::string&&);
};
```

## Frontend

The frontend is the intermediate between the application (the server) and the
connection to its client.

1. The frontend handles incoming language server protocol messages from the
   client and calls the respective function in the backend/application/server.
2. The frontend handles outgoing language server protocol messages from the
   server to the client.
3. The frontend takes care of request cancellation notifications coming from
   the client. When a request cancellation notification is received, the
   frontend toggles a cancellation token that is seen by the application. It is
   then up to the application to decide what it does with this.
4. The frontend handles the intricacies of the protocol.
   - It ensures that requests are only processed after 'initialize'
   - It ensures that notifications are only processed after 'initialize'
   - It ignores unknown notifications
   - It handles unknown requests
5. The frontend checks whether the language server protocol is followed.
   - It does this by keeping track of incoming requests.
     It makes sure the server has sent ***one*** reply to that request.
   - And also keeps track of outgoing requests and checks that the client has
     replied to our requests.

```cpp
class frontend
{
    // Blocking function. Return true if exit notification received
    bool forever_loop();

    // Handle message received from client. Decodes whether it is a request, a
    // notification or a response and handles it. Return true if handled and
    // false otherwise.
    void handle(std::string);

    // bind handler for notifications and requests coming from the client
    void bind(std::string, std::function<lsp::server(std::shared_ptr<lsp::incoming_notification>)>);
    void bind(std::string, std::function<lsp::server(std::shared_ptr<lsp::incoming_request>)>);

    // Send messages to the client
    bool send(std::shared_ptr<lsp::outgoing_notification>);
    bool send(std::shared_ptr<lsp::outgoing_request>);
    bool send(std::shared_ptr<lsp::outgoing_response>);

    int callback_id = 0;
    mutable std::mutex callback_mutex;

    std::map<std::string, std::function<void(std::shared_ptr<lsp::incoming_notification>)>> notification_handlers;
    std::map<std::string, std::function<void(std::shared_ptr<lsp::incoming_request>)>>      request_handlers;

    std::map<id, std::pair<common::cancellation_source, std::shared_ptr<lsp::incoming_request>>> incoming_requests_in_flight;
    std::map<id, std::shared_ptr<lsp::outgoing_request>> outgoing_requests_in_flight;
};
```

## Server

The server is the backbone to the application (the code written by the
programmer). The server accepts a connection object and is run by calling its
`run()` function.

The server blocks on `run()` until an exit notification, sent by the client, is
received and is processed by the frontend. There is no other way to stop the
server. This is because the lifetime of the server is managed by the client.

Internally, the server owns and manages its own frontend. It uses it to send
language server protocol messages over the connection.

```cpp
class lsp::server
{
    server(lsp::connection*);


    // This is where the frontend is told about what method corresponds to what
    // callback function.
    bool setup();
    bool run();

    private:

    // request and notification handlers. Each handler should be bound to a
    // method in setup.
    void on_initialise (std::shared_ptr<lsp::incoming_request>);
    void on_initialised(std::shared_ptr<lsp::incoming_notification>);
    void on_shutdown   (std::shared_ptr<lsp::incoming_request);
    void on_exit       (std::shared_ptr<lsp::incoming_notification>);
};
```

## Client

The client object is how the application is expected to send requests or
notifications to the client.

```cpp
class lsp::client
{
    client(lsp::frontend*);

    // send a notification or a request to client
    bool notify(std::string, std::optional<json::string>);
    bool request(std::string, std::optional<json::string>, on_resp);

};
```
