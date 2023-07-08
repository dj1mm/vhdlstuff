
#include "working_files.h"

#include "language.h"

#include "common/scope_guard.h"

#include "definition_provider.h"
#include "document_symbol_provider.h"
#include "folding_range_provider.h"
#include "hover_provider.h"

things::working_file::working_file(std::string file, things::client* client,
                                   things::project* prj)
    : file_(file), policy(run_on_main_thread), client_(client), project_(prj),
      current_project_version_(0), library_fully_loaded_(false)
{
}

things::working_file::~working_file()
{
    if (thread.joinable())
        // Should we detach or should we join here?
        //
        // I think we should detach. But valgrind occasionally reports an error
        // with pthreads. I think it's got to do with the actual thread running
        // beyond main, and valgrind complaining about a false positive. Even
        // with the many primitives (mutex_, conditional_variable_ etc...) that
        // ensures no threads are running when things::working_files is being
        // destroyed, we still see the valgrind complaint. Not too sure why.
        //
        // So let's join for now. The downside of join is that it causes the
        // things::working_files::remove() function to block.
        //
        // Possible solution to all of this:
        //
        // Implement some form of garbage collection.
        //
        // 1. In things::working_files::remove(), stop the thread and add the
        //    working file to the garbage bin.
        // 2. To ensure the trash can does not perpetually grow in size, every
        //    so often, we will check it, and throw away the files whose thread
        //    havs finally joined.
        // 3. When things::working_files is being destroyed, all the garbage
        //    will be passed onto the main thread, who will then check and
        //    throw the trash away.
        thread.detach();
}

void things::working_file::invalidate_potentially_referenced_file(std::string f)
{
    std::unique_lock<std::mutex> lock(mutex_to_invalidate_files_);
    list_of_potentially_referenced_files_now_invalid.push_back(f);
}

void things::working_file::forever_loop()
{
    while (true)
    {
        try
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [&]() { return !queue_.empty() || stopped_.load(); });
            if (stopped_.load())
                break;
            auto request = queue_.front();
            queue_.pop_front();

            if (request.action)
                request.action(request.is_superseded);
        }
        catch (const std::exception& e)
        {
        }

    }
}

void things::working_file::stop()
{
    std::unique_lock<std::mutex> lock(mutex_);
    stopped_.store(true);
    cv_.notify_one();
}

void things::working_file::add_task(std::string name,
                                    std::function<void(bool)> task)
{
    if (policy == run_on_main_thread)
    {
        // do the update right here
        task(false);
    }
    else
    {
        // invalidate everything on the queue
        for (auto& entry : queue_)
            entry.is_superseded = true;

        working_file::task request;
        request.name = name;
        request.action = std::move(task);
        std::unique_lock<std::mutex> lock(mutex_);
        queue_.push_back(request);
        cv_.notify_one();
    }
}


things::working_files::working_files(things::language* s, things::client* c,
                                     bool j)
    : number_of_running_threads_(0), server_(s), client_(c),
      everything_on_main_thread(j)
{

}

things::working_files::~working_files()
{
    for (auto& it : working_files_)
    {
        it.second->stop();
    }
    working_files_.clear();

    // wait for running threads = 0. Should we add a duration here?
    std::unique_lock<std::mutex> guard(mutex_);
    if (number_of_running_threads_ != 0)
        no_running_threads_.wait(guard);
}

bool things::working_files::update(std::string file)
{
    auto new_file = false;
    if (working_files_.find(file) == working_files_.end())
    {
        std::filesystem::path path(file);
        auto ext = path.extension().string();

        auto is_a_sv_file = (ext == ".sv");
        auto is_a_vhdl_file = (ext == ".vhd" || ext == ".vhdl");
        assert(is_a_sv_file || is_a_vhdl_file);

        auto wf = is_a_sv_file? std::make_shared<vhdl_working_file>(file, client_, &server_->project):
                                std::make_shared<vhdl_working_file>(file, client_, &server_->project);

        // if we want to run_everything_on_the_main_thread, simply create a
        // working_file object, and simply call its update() function.
        if (everything_on_main_thread)
        {
            wf->policy = working_file::run_on_main_thread;
            working_files_[file] = std::move(wf);
            new_file = true;
        }
        // However, if we dont mind creating threads, only then we execute all
        // of the following code
        else
        {
            auto cleanup = common::make_scope_guard([this]() {
                std::lock_guard<std::mutex> guard(mutex_);
                auto run_execution_policys = --number_of_running_threads_;
                if (run_execution_policys == 0)
                    no_running_threads_.notify_one();
            });

            wf->thread = std::thread([file, wf, C = std::move(cleanup)]() {
                loguru::set_thread_name("file thread");

                LOG_S(INFO) << file << ": started";

                wf->forever_loop();

                LOG_S(INFO) << file << ": done";
            });

            wf->policy = working_file::run_on_different_thread;
            working_files_[file] = std::move(wf);
            new_file = true;

            {
            std::lock_guard<std::mutex> guard(mutex_);
            ++number_of_running_threads_;
            }
        }
    }
    for (auto [name, wf] : working_files_)
    {
        if (name == file)
            wf->update();
        else
            wf->invalidate_potentially_referenced_file(file);
    }

    return new_file;
}

void things::working_files::remove(std::string file)
{
    auto it = working_files_.find(file);
    if (it != working_files_.end())
    {
        it->second->stop();
        working_files_.erase(it);
    }
}

void things::working_files::update_all_files()
{
    for (auto [name, wf] : working_files_)
    {
        wf->update();
    }
}

void things::working_files::folding_ranges(
    std::string file, std::shared_ptr<lsp::incoming_request> request)
{
    if (working_files_.find(file) == working_files_.end())
        return;

    auto wf = working_files_.find(file)->second.get();
    wf->folding_ranges(request);
}

void things::working_files::symbols(
    std::string file, std::shared_ptr<lsp::incoming_request> request)
{
    if (working_files_.find(file) == working_files_.end())
        return;

    auto wf = working_files_.find(file)->second.get();
    wf->symbols(request);
}

void things::working_files::hover(
    std::string file, std::shared_ptr<lsp::incoming_request> request,
    common::position pos)
{
    if (working_files_.find(file) == working_files_.end())
        return;

    auto wf = working_files_.find(file)->second.get();
    wf->hover(request, pos);
}

void things::working_files::definition(
    std::string file, std::shared_ptr<lsp::incoming_request> request,
    common::position pos)
{
    if (working_files_.find(file) == working_files_.end())
        return;

    auto wf = working_files_.find(file)->second.get();
    wf->definition(request, pos);
}

things::vhdl_working_file::vhdl_working_file(std::string file,
                                             things::client* client,
                                             things::project* prj)
    : working_file(file, client, prj)
{
}

void things::vhdl_working_file::update()
{
    auto analyse_and_diagnose = [this](bool is_superseded) {
        if (is_superseded)
        {
            return;
        }

        make_sure_this_is_latest_project_version();

        if (list_of_potentially_referenced_files_now_invalid.size())
        {
            std::lock_guard<std::mutex> lock(mutex_to_invalidate_files_);
            for (auto it : list_of_potentially_referenced_files_now_invalid)
            {
                ast->invalidate_reference_file(it);
            }
            list_of_potentially_referenced_files_now_invalid.clear();
        }

        ast->invalidate_main_file();
        ast->update();

        send_diagnostics_back_to_client_if_needed();
    };

    return add_task("update", std::move(analyse_and_diagnose));
}

void things::vhdl_working_file::folding_ranges(
    std::shared_ptr<lsp::incoming_request> r)
{
    auto calculate_folding_ranges = [r](std::shared_ptr<vhdl::ast> ast) {
        if (!ast || !ast->get_main_file()) {
            r->reply(json::string("[]"));
            return;
        }

        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> w(s);

        w.StartArray();
        vhdl_folding_range_provider d(&w);
        ast->get_main_file()->traverse(d);
        w.EndArray();
    
        json::string json = s.GetString();
        r->reply(json);
    };
    run_with_vhdl_ast(calculate_folding_ranges);
}

void things::vhdl_working_file::symbols(
    std::shared_ptr<lsp::incoming_request> r)
{
    auto get_document_symbols = [r](std::shared_ptr<vhdl::ast> ast) {
        if (!ast || !ast->get_main_file()) {
            r->reply(json::string("[]"));
            return;
        }

        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> w(s);

        w.StartArray();
        vhdl_document_symbol_provider d(&w);
        ast->get_main_file()->traverse(d);
        w.EndArray();
    
        json::string json = s.GetString();
        r->reply(json);
    };

    run_with_vhdl_ast(get_document_symbols);
}

void things::vhdl_working_file::hover(std::shared_ptr<lsp::incoming_request> r,
                                      common::position pos)
{
    auto get_hover = [r, pos](std::shared_ptr<vhdl::ast> ast) {
        if (!ast || !ast->get_main_file())
        {
            r->reply(json::null_value);
            return;
        }

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
    };

    run_with_vhdl_ast(get_hover);
}

void things::vhdl_working_file::definition(
    std::shared_ptr<lsp::incoming_request> r, common::position pos)
{
    auto get_definition = [r, pos](std::shared_ptr<vhdl::ast> ast) {
        if (!ast || !ast->get_main_file())
        {
            r->reply(json::null_value);
            return;
        }

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
    };

    run_with_vhdl_ast(get_definition);
}

void things::vhdl_working_file::run_with_vhdl_ast(
    std::function<void(std::shared_ptr<vhdl::ast>)> callback)
{
    auto run_that = [this, that = std::move(callback)](bool is_superseded) {
        if (is_superseded)
        {
            that(nullptr);
            return;
        }

        make_sure_this_is_latest_project_version();

        auto was_already_uptodate = ast->update();

        if (!was_already_uptodate)
            send_diagnostics_back_to_client_if_needed();

        that(ast);
    };

    return add_task("run_with_ast", std::move(run_that));
}

void things::vhdl_working_file::make_sure_this_is_latest_project_version()
{
    auto libmgr = project_->get_current_library_manager();

    auto new_project_version = project_->get_loaded_version();
    auto new_library_loaded_state = project_->libraries_have_been_populated();
    if (!ast ||
        current_project_version_ != new_project_version ||
        library_fully_loaded_ != new_library_loaded_state)
    {
        current_project_version_ = new_project_version;
        library_fully_loaded_ = new_library_loaded_state;
        work_libraries_ = project_->get_libraries_this_file_is_part_of(file_);
        std::optional<std::string> work;
        if (work_libraries_.size() != 0)
            work = work_libraries_[0];
        ast = std::make_shared<vhdl::ast>(file_, libmgr, work.value_or("work"));
    }
}

void things::vhdl_working_file::send_diagnostics_back_to_client_if_needed()
{
    if (!stopped_.load())
    {
        auto [parse_errors, semantic_errors] = ast->get_diagnostics();

        std::vector<common::diagnostic> diags;
        diags.reserve(parse_errors.size() /* + semantic_errors.size() */);

        common::location loc(file_);
        if (!ast->get_main_file())
        {
            common::diagnostic diag("{} not found.", loc);
            diags.push_back(diag);
        }

        if (current_project_version_ == 0)
        {
            common::diagnostic diag("To enjoy full vhdl language services, make sure there is a valid vhdl_config.yaml configuration.", loc);
            diags.push_back(diag);
        }

        // else if is what we want. Only complain about the config file once
        else if (!library_fully_loaded_)
        {
            common::diagnostic diag("Indexer is still running. Ignoring {} errors.", loc);
            diag << semantic_errors.size();
            diags.push_back(diag);
        }
        else if (work_libraries_.size() == 0)
        {
            common::diagnostic diag("To enjoy full vhdl language services, please add this file to vhdl_config.yaml.", loc);
            diags.push_back(diag);
        }

        diags.insert(diags.end(), parse_errors.begin(), parse_errors.end());
        // diags.insert(diags.end(), semantic_errors.begin(), semantic_errors.end());
        client_->send_diagnostics(file_, diags);
    }
}

