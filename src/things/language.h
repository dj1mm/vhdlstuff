
#ifndef THINGS_LANGUAGE_H
#define THINGS_LANGUAGE_H

#include <functional>
#include <memory>
#include <string>

#include "lsp/connection.h"
#include "lsp/server.h"
#include "lsp/structures.h"

#include "client.h"
#include "project.h"
#include "working_files.h"

namespace things
{

struct capabilities
{
    std::optional<std::filesystem::path> root;
};

class language: public lsp::server
{
    public:

    language(lsp::connection*);
    virtual ~language();

    virtual void setup();

    // override the default behaviour of these handlers
    void on_initialize (std::shared_ptr<lsp::incoming_request>);
    void on_initialized(std::shared_ptr<lsp::incoming_notification>);
    void on_shutdown   (std::shared_ptr<lsp::incoming_request>);
    void on_exit       (std::shared_ptr<lsp::incoming_notification>);

    //
    void on_text_document_did_open     (std::shared_ptr<lsp::incoming_notification>);
    void on_text_document_did_close    (std::shared_ptr<lsp::incoming_notification>);
    void on_text_document_did_change   (std::shared_ptr<lsp::incoming_notification>);
    void on_text_document_did_save     (std::shared_ptr<lsp::incoming_notification>);
    void on_text_document_folding_range(std::shared_ptr<lsp::incoming_request>);
    void on_text_document_symbol       (std::shared_ptr<lsp::incoming_request>);
    void on_text_document_hover        (std::shared_ptr<lsp::incoming_request>);
    void on_text_document_definition   (std::shared_ptr<lsp::incoming_request>);

    //
    void on_workspace_did_change_watched_files(std::shared_ptr<lsp::incoming_notification>);

    void update_all_working_files();

    things::capabilities capabilities;

    private:

    things::client client;
    things::working_files working_files;
    things::project project;

    friend class working_files;
    friend class project;
    friend class explorer;

};


}

#endif
