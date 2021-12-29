
#include "client.h"

#include "lsp/frontend.h"
#include "lsp/structures.h"

// this define is needed before the include format.h. Otherwise will get an
// `undefined reference to `fmt::v8::vformat` compile error
#define FMT_HEADER_ONLY
#include "fmt/args.h"
#include "fmt/format.h"

lsp::diagnostic to_lsp_diagnostic(common::diagnostic sd)
{
    lsp::diagnostic diag;
    fmt::dynamic_format_arg_store<fmt::format_context> args;
    for (auto& arg : sd.args)
    {
        if (std::holds_alternative<std::string>(arg)) args.push_back(std::get<std::string>(arg));
        if (std::holds_alternative<int        >(arg)) args.push_back(std::get<int        >(arg));
    }
    diag.message = fmt::vformat(sd.format, args);
    diag.range.start.line      = sd.location.begin.line-1;
    diag.range.start.character = sd.location.begin.column-1;
    diag.range.end.line        = sd.location.end.line-1;
    diag.range.end.character   = sd.location.end.column-1;
    return diag;
}

things::workdone_progress_bar::workdone_progress_bar(things::client* c, std::string t)
    :client_(c), token_(t), in_progress_(false)
{

}

// things::workdone_progress_bar::workdone_progress_bar(const workdone_progress_bar&)
// {
// 
// }

things::workdone_progress_bar::workdone_progress_bar(workdone_progress_bar&& rhs)
{
    client_ = std::move(rhs.client_);
    in_progress_ = rhs.in_progress_;
    token_ = rhs.token_;
    rhs.in_progress_ = false;
}

// things::workdone_progress_bar& things::workdone_progress_bar::operator=(const workdone_progress_bar&)
// {
//     return *this;
// }

things::workdone_progress_bar& things::workdone_progress_bar::operator=(workdone_progress_bar&& rhs)
{
    client_ = std::move(rhs.client_);
    in_progress_ = rhs.in_progress_;
    rhs.in_progress_ = false;
    return *this;
}

things::workdone_progress_bar::~workdone_progress_bar()
{
    if (client_)
        client_->workdone_progress_bar_visible.store(false);
    if (in_progress_)
    {
        lsp::workdone_progress_end_params params;
        params.token = token_;
        auto json = serialize::to_json(params);
        client_->notify("$/progress", json);
        in_progress_ = false;
    }
}

void things::workdone_progress_bar::report(int percentage, std::string message)
{
    if (percentage >= 100 && in_progress_)
    {
        lsp::workdone_progress_end_params params;
        params.token = token_;
        params.message = message;
        auto json = serialize::to_json(params);
        client_->notify("$/progress", json);
        in_progress_ = false;
    }
    else if (percentage >= 100)
    {
        // do nothing
    }
    else if (!in_progress_)
    {
        lsp::workdone_progress_begin_params params;
        params.token = token_;
        params.message = message;
        params.percentage = percentage;
        params.title = "Indexing";
        auto json = serialize::to_json(params);
        client_->notify("$/progress", json);
        in_progress_ = true;
    }
    else
    {
        lsp::workdone_progress_report_params params;
        params.token = token_;
        params.message = message;
        params.percentage = percentage;
        auto json = serialize::to_json(params);
        client_->notify("$/progress", json);
    }
}

things::client::client(lsp::frontend* fe)
: lsp::client(fe), workdone_progress_bar_visible(false)
{

}

things::client::~client()
{
    
}

void things::client::show_message(std::string message)
{
    lsp::show_message_params params;
    params.type = lsp::message_type::log;
    params.message = message;

    auto json = serialize::to_json(params);
    notify("window/showMessage", json);
}

void things::client::log_message(std::string message)
{
    lsp::log_message_params params;
    params.type = lsp::message_type::log;
    params.message = message;

    auto json = serialize::to_json(params);
    notify("window/logMessage", json);
}

void things::client::send_diagnostics(
    std::string file, std::vector<common::diagnostic> diagnostics)
{
    lsp::publish_diagnostics_params params;
    params.uri.set_path(file);
    for (auto& it : diagnostics)
    {
        params.diagnostics.push_back(to_lsp_diagnostic(it));
    }

    auto json = serialize::to_json(params);
    notify("textDocument/publishDiagnostics", json);
}

std::optional<things::workdone_progress_bar> things::client::create_workdone_progress(std::string t)
{
    if (workdone_progress_bar_visible.load())
        return std::nullopt;

    workdone_progress_bar_visible.store(true);

    lsp::workdone_progress_create_params params;
    params.token = t;
    auto json = serialize::to_json(params);
    request("window/workDoneProgress/create", json);

    auto bar = std::make_optional<things::workdone_progress_bar>(this, t);
    return bar;
}

