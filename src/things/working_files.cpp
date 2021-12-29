
#include "working_files.h"

#include "client.h"
#include "language.h"
#include "project.h"

#include "common/loguru.h"
#include "common/scope_guard.h"
#include "vhdl/ast.h"
#include "vhdl/binder.h"

namespace Err
{
    std::string_view File_not_found = "{} not found.";
    std::string_view File_not_in_lib = "To enjoy full vhdl language services, please add this file to vhdl_config.yaml.";
    std::string_view Loading_prj = "To enjoy full vhdl language services, make sure there is a valid vhdl_config.yaml configuration.";
    std::string_view Library_not_loaded = "Indexer is still running. Ignoring {} errors.";
};

things::working_file::working_file(
    std::string file,
    std::function<void(std::string, std::vector<common::diagnostic>)> cb,
    things::project* prj)
    : file_(file), policy(run_on_main_thread), send_diagnostics_(cb),
      project_(prj), current_project_version_(0), library_fully_loaded_(false)
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

void things::working_file::run_with_ast(
    std::function<void(std::shared_ptr<vhdl::ast>)> callback)
{
    auto run_that = [this, that = std::move(callback)](bool is_superseded) {
        if (is_superseded)
        {
            that(nullptr);
            return;
        }

        make_sure_this_is_latest_project_version_();

        auto was_already_uptodate = ast->update();

        if (!was_already_uptodate)
            send_diagnostics_back_to_client_if_needed_();

        that(ast);
    };

    return add_task("run_with_ast", std::move(run_that));
}

void things::working_file::update()
{
    auto analyse_and_diagnose = [this](bool is_superseded) {
        if (is_superseded)
            return;

        make_sure_this_is_latest_project_version_();

        if (list_of_potentially_invalidated_reference_files.size())
        {
            for (auto it : list_of_potentially_invalidated_reference_files)
            {
                ast->invalidate_reference_file(it);
            }
            list_of_potentially_invalidated_reference_files.clear();
        }

        ast->invalidate_main_file();
        ast->update();

        send_diagnostics_back_to_client_if_needed_();
    };

    return add_task("update", std::move(analyse_and_diagnose));
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

void things::working_file::make_sure_this_is_latest_project_version_()
{
    auto libmgr = project_->get_current_library_manager();

    if (!ast ||
        current_project_version_ != project_->get_loaded_version() ||
        library_fully_loaded_ != project_->libraries_have_been_populated())
    {
        current_project_version_ = project_->get_loaded_version();
        library_fully_loaded_ = project_->libraries_have_been_populated();
        work_libraries_ = project_->get_libraries_this_file_is_part_of(file_);
        std::optional<std::string> work;
        if (work_libraries_.size() != 0)
            work = work_libraries_[0];
        ast = std::make_shared<vhdl::ast>(file_, libmgr, work.value_or("work"));
    }
}

void things::working_file::send_diagnostics_back_to_client_if_needed_()
{
    if (send_diagnostics_ && !stopped_.load())
    {
        auto [parse_errors, semantic_errors] = ast->get_diagnostics();

        std::vector<common::diagnostic> diags;
        diags.reserve(parse_errors.size() /* + semantic_errors.size() */);

        common::location loc(file_);
        if (!ast->get_main_file())
        {
            common::diagnostic diag(Err::File_not_found, loc);
            diags.push_back(diag);
        }

        if (current_project_version_ == 0)
        {
            common::diagnostic diag(Err::Loading_prj, loc);
            diags.push_back(diag);
        }

        // else if is what we want. Only complain about the config file once
        else if (!library_fully_loaded_)
        {
            common::diagnostic diag(Err::Library_not_loaded, loc);
            diag << semantic_errors.size();
            diags.push_back(diag);
        }
        else if (work_libraries_.size() == 0)
        {
            common::diagnostic diag(Err::File_not_in_lib, loc);
            diags.push_back(diag);
        }

        diags.insert(diags.end(), parse_errors.begin(), parse_errors.end());
        // diags.insert(diags.end(), semantic_errors.begin(), semantic_errors.end());
        send_diagnostics_(file_, diags);
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

void things::working_files::run_with_ast(
    std::string file, std::function<void(std::shared_ptr<vhdl::ast>)> callback)
{
    if (working_files_.find(file) == working_files_.end())
        return;

    auto wf = working_files_.find(file)->second.get();
    wf->run_with_ast(std::move(callback));
}

bool things::working_files::update(std::string file)
{
    auto new_file = false;
    if (working_files_.find(file) == working_files_.end())
    {
        auto wf = std::make_shared<working_file>(
            file,
            std::bind(&things::client::send_diagnostics, client_, std::placeholders::_1, std::placeholders::_2),
            &server_->project);

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
            wf->list_of_potentially_invalidated_reference_files.push_back(file);
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

