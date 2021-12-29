
#ifndef LSP_CLIENT_H
#define LSP_CLIENT_H

#include "frontend.h"

namespace lsp
{

class client
{
    public:

    client(lsp::frontend*);
    virtual ~client() = default;

    bool notify(std::string method, std::optional<json::string>);
    bool request(
        std::string,
        std::optional<json::string>,
        std::function<void(std::shared_ptr<lsp::incoming_response>)> = nullptr
    );

    private:

    lsp::frontend* frontend;
    
};


}

#endif
