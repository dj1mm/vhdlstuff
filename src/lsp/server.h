
#ifndef LSP_SERVER_H
#define LSP_SERVER_H

#include <memory>

#include "connection.h"
#include "frontend.h"
#include "structures.h"

namespace lsp
{

class server
{
    public:

    server(lsp::connection*);
    server(const server&) = delete;
    server(server&&) = default;
    server& operator=(const server&) = delete;
    server& operator=(server&&) = default;
    virtual ~server() = default;

    virtual void setup() = 0;
    virtual bool run() final;

    protected:

    std::unique_ptr<lsp::frontend> frontend;

    private:

    virtual void on_initialize(std::shared_ptr<lsp::incoming_request>);
    virtual void on_initialized(std::shared_ptr<lsp::incoming_notification>);
    virtual void on_shutdown(std::shared_ptr<lsp::incoming_request>);
    virtual void on_exit(std::shared_ptr<lsp::incoming_notification>);

    bool started_;
    bool shutdown_;

    friend class frontend;

};

}

#endif
