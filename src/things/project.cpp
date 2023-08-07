
#include "project.h"
#include "language.h"

#include <regex>
#include <utility>

#include "fmt/format.h"

#include "slang/diagnostics/Diagnostics.h"
#include "slang/parsing/Preprocessor.h"
#include "slang/util/BumpAllocator.h"
#include "sv/ast.h"
#include "sv/fast_parser.h"
#include "vhdl/ast.h"
#include "vhdl/fast_parser.h"

bool things::config::file_spec::is_path() const
{
    return content.index() == 0;
}

bool things::config::file_spec::is_file_query() const
{
    return content.index() == 1;
}

const std::string& things::config::file_spec::as_path() const
{
    return std::get<0>(content);
}

const things::config::file_filter&
things::config::file_spec::as_file_query() const
{
    return std::get<1>(content);
}

std::string things::config::file_spec::to_string()
{
    if (is_path())
        return fmt::format("{{path: \"{}\"}}", as_path());
    else if (is_file_query() && as_file_query().depth)
        return fmt::format("{{dir: \"{}\", search: \"{}\", depth: {}}}",
                           as_file_query().directory,
                           as_file_query().search,
                           as_file_query().depth.value());
    else if (is_file_query())
        return fmt::format("{{dir: \"{}\", search: \"{}\"}}",
                           as_file_query().directory,
                           as_file_query().search);
    else
        return "UNKNOWN FILE SPECIFICATION";
}

void things::filelist::add_entry(std::string name, things::config::file_spec* spec)
{
    std::lock_guard guard(mtx_);
    if (path_to_entries.find(name) == path_to_entries.end())
    {
        // this is the first time we see this file name - so we should just add
        // it to our filelist.
        auto entry  = std::make_unique<things::filelist::entry>();
        entry->next = entry.get();
        entry->spec = spec;

        // we can add the new entry to our repertoire
        path_to_entries[name] = entry.get();
        entries.push_back(std::move(entry));

        // we just added a new entry to the filelist - increment the count
        total_number_of_files++;
    }
    else
    {
        // our filelist already contains an entry for this file - so it seems
        // the vhdl_config.yaml maps this file to more than one library. Now,
        // while adding the file to our filelist, we shall link it to the
        // existing entries.
        auto existing_entry = path_to_entries[name];

        auto entry  = std::make_unique<things::filelist::entry>();
        entry->next = existing_entry;
        entry->spec = spec;

        // this time, we add the new entry to the end of the chain of existing
        // entries
        while (existing_entry->next != entry->next)
            existing_entry = existing_entry->next;
        existing_entry->next = entry.get();

        // we can add the entry to our repertoire
        entries.push_back(std::move(entry));
    }
}

things::filelist::entry* things::filelist::get_entry(std::string name)
{
    std::lock_guard guard(mtx_);
    auto entry = path_to_entries.find(name);
    if (entry == path_to_entries.end())
        return nullptr;

    return entry->second;
}

things::project::project(std::function<void()> cb, things::client* c)
    : project_folder_(std::filesystem::current_path()), client_(c),
      loaded_version_(0), on_all_requests_completed(cb)
{
    current_filelist_ = std::make_shared<things::filelist>();

    current_library_manager_ =
        std::make_shared<vhdl::library_manager>(std::nullopt, true);
    current_sv_library_manager_ =
        std::make_shared<sv::library_manager>(std::nullopt, true);

    current_background_explorer_ = std::make_unique<things::explorer>(
        loaded_version_, current_filelist_, current_library_manager_,
        current_sv_library_manager_,
        path_to_loaded_yaml_.value_or("").string(),
        on_all_requests_completed, client_, project_folder_.string());
}

void things::project::set_project_folder(std::string folder)
{
    // this is the folder from which we will look for a vhdl_config.yaml file
    project_folder_ = folder;
}

bool things::project::is_loaded()
{
    return path_to_loaded_yaml_ && current_library_manager_ &&
           current_sv_library_manager_ &&
           current_filelist_;
}

int things::project::get_loaded_version()
{
    if (!is_loaded())
        return -1;

    return loaded_version_.load();
}

bool things::project::libraries_have_been_populated()
{
    if (!is_loaded())
        return false;

    if (current_background_explorer_)
        return current_background_explorer_->done();

    return false;
}

std::shared_ptr<vhdl::library_manager> things::project::
    get_current_library_manager()
{
    std::lock_guard lock(clm_mtx_);
    return current_library_manager_;
}

std::shared_ptr<sv::library_manager> things::project::
    get_current_sv_library_manager()
{
    std::lock_guard lock(clm_mtx_);
    return current_sv_library_manager_;
}

bool things::project::
    reload_yaml_reset_project_kick_background_index_destroy_libraries()
{
    // this function does a lot of things.
    // Because of that, it takes very long to complete. We can probably break
    // this function down a lil bit.

    // ------------------------------------------------------------------------
    // First thing first: Set things up
    // ------------------------------------------------------------------------


    auto path_to_yaml = project_folder_ / "vhdl_config.yaml";
    path_to_loaded_yaml_ = path_to_yaml;
    loaded_version_++;

    auto temp_mgr =
        std::make_shared<vhdl::library_manager>(std::nullopt, false);
    auto temp_svm = std::make_shared<sv::library_manager>(std::nullopt, false);
    auto temp_lst = std::make_shared<things::filelist>();
    auto temp_xpl = std::make_unique<things::explorer>(loaded_version_,
        temp_lst, temp_mgr, temp_svm,
        path_to_loaded_yaml_.value_or("").string(),
        on_all_requests_completed, client_, project_folder_.string());

    // ------------------------------------------------------------------------
    // 1) Load the yaml
    // ------------------------------------------------------------------------
    std::unique_ptr<things::config::root> vhdl_config;
    try
    {
        auto node = YAML::LoadFile(path_to_yaml.string());
        auto root = node.as<things::config::root>();
        vhdl_config = std::make_unique<things::config::root>(root);
        LOG_S(INFO) << "ProjectManager: loaded " << path_to_yaml.string()
                    << " ver" << loaded_version_;
    }
    catch (const YAML::Exception& e)
    {
        lsp::diagnostic diag;
        diag.message               = e.msg;
        diag.range.start.line      = e.mark.line-1;
        diag.range.start.character = e.mark.column-1;
        diag.range.end.line        = e.mark.line-1;
        diag.range.end.character   = e.mark.column-1;
        client_->send_persistent_diagnostic(path_to_yaml.string(), diag);
        LOG_S(ERROR) << "ProjectManager: error loading " << path_to_yaml.string()
                     << " ver" << loaded_version_ << ": " << e.msg;
        return false;
    }
    catch (const std::exception& e)
    {
        lsp::diagnostic diag;
        diag.message               = e.what();
        diag.range.start.line      = 0;
        diag.range.start.character = 0;
        diag.range.end.line        = 0;
        diag.range.end.character   = 0;
        client_->send_persistent_diagnostic(path_to_yaml.string(), diag);
        LOG_S(ERROR) << "ProjectManager: error loading " << path_to_yaml
                     << " ver" << loaded_version_ << ": " << e.what();
        return false;
    }

    // ------------------------------------------------------------------------
    // 1.5) Extract whatever useful information we need
    // ------------------------------------------------------------------------
    things::config::file_specs_ptr filelist_specifications;
    for (auto& library: vhdl_config->vhdl) {
        for (auto& file: library.files) {
            filelist_specifications.push_back(&file);
            file.library = &library;
        }
    }
    auto& library = vhdl_config->sv;
        for (auto& file: library.files) {
            filelist_specifications.push_back(&file);
            file.library = &library;
        }

    std::vector<std::string> names_of_all_vhdl_libraries;
    for (auto library: vhdl_config->vhdl)
        names_of_all_vhdl_libraries.push_back(library.name);
    
    // ------------------------------------------------------------------------
    // 2) Reset project variables
    // ------------------------------------------------------------------------

    // if we are here, YAML file had no syntax errors and we have something
    // useful to work with.

    // should we wait? I think so
    current_background_explorer_->stop();
    current_background_explorer_->join();
    current_background_explorer_.reset(nullptr);
    current_background_explorer_ = std::move(temp_xpl);

    // It is important that all the background explorer works are fully stopped
    // before resetting the filelist to avoid segfaults
    current_filelist_.reset();
    current_filelist_ = std::move(temp_lst);
    current_filelist_->vhdl_config = std::move(vhdl_config);

    // ------------------------------------------------------------------------
    // 3) Kick background indexing
    // ------------------------------------------------------------------------
    current_background_explorer_->start(filelist_specifications);

    // ------------------------------------------------------------------------
    // 4) Destroy the old library manager
    // ------------------------------------------------------------------------
    std::lock_guard lock(clm_mtx_);
    current_library_manager_->destroy();
    current_library_manager_.reset();
    current_library_manager_ = temp_mgr;
    current_library_manager_->initialise(names_of_all_vhdl_libraries);

    current_sv_library_manager_->destroy();
    current_sv_library_manager_.reset();
    current_sv_library_manager_ = temp_svm;

    client_->clear_persistent_diagnostic(path_to_yaml.string());
    return true;
}

std::vector<std::string> things::project::get_libraries_this_file_is_part_of(
    std::string& name)
{
    // I dont think current filelist needs to be mutexed because this function
    // is only ever called in the main thread
    auto entry = current_filelist_->get_entry(name);
    if (!entry)
        return {};

    std::vector<std::string> libs;
    for (auto head = entry;; entry = entry->next)
    {
        libs.push_back(entry->spec->library->name);

        if (entry->next == head)
            break;
    }
    return libs;
}

std::vector<std::string> things::project::get_incdirs_this_file_needs(
    std::string& name)
{
    // I dont think current filelist needs to be mutexed because this function
    // is only ever called in the main thread
    auto entry = current_filelist_->get_entry(name);
    if (!entry)
        return {};

    std::vector<std::string> incdirs;
    for (auto& folder: entry->spec->library->incdirs)
        incdirs.push_back(folder.path);
    return incdirs;
}

things::compass::compass(int total, std::function<void()> callback,
                         std::optional<things::workdone_progress_bar> bar)
    : number_of_files_found(0),
      number_of_requests_completed(0),
      total_number_of_requests(total),
      on_all_requests_completed(callback),
      progress_bar(std::move(bar))
{

}

void things::compass::i_just_completed_a_request(int found)
{
    unsigned int indexed = 0;
    unsigned int total = 0;
    unsigned int completed = 0;
    {
        std::lock_guard<std::mutex> lock(mutex);
        number_of_files_found += found;
        indexed = number_of_files_found;
        total = total_number_of_requests;
        completed = ++number_of_requests_completed;
    }
    auto p = total == 0 ? 100 : completed * 100 / total;
    auto msg = fmt::format("Found {} files. (Done/Total = {}/{}).",
                           indexed, completed, total);

    if (progress_bar)
    {
        progress_bar->report(p, msg);
    }

    if (completed == total && on_all_requests_completed)
    {
        on_all_requests_completed();
        on_all_requests_completed = nullptr;
        progress_bar.reset();
    }
}

int things::compass::get_number_of_files_found()
{
    std::lock_guard<std::mutex> lock(mutex);
    return number_of_files_found;
}

things::explorer::worker::worker(int v, int i,
                                 things::config::file_specs_ptr s,
                                 std::shared_ptr<things::filelist> f,
                                 std::shared_ptr<vhdl::library_manager> m,
                                 std::shared_ptr<sv::library_manager> svm,
                                 std::shared_ptr<things::compass> p,
                                 std::string y, things::client* c ,
                                 std::string w)
    : busy_(false), quit_(false), done_(false), specs(s), manager(m),
      sv_manager(svm), filelist(f), progress(p),
      client_(c), workspace_folder(w), version(v), id(i), path_to_yaml(y)
{
    header = fmt::format("Worker{}.{}: ", version, i);
}

int things::explorer::worker::explore_spec(things::config::file_spec* spec)
{
    auto found = 0;
    if (spec->is_file_query())
    {
        auto& entry = spec->as_file_query();
        auto directory = entry.directory;

        if (directory.rfind("${workspaceFolder}", 0) == 0)
            directory.replace(0, 18, workspace_folder);

        std::filesystem::path folder(directory);
        if (!std::filesystem::exists(folder))
        {
            lsp::diagnostic diag;
            diag.message = fmt::format("{} does not exist", directory);
            diag.range.start.line      = spec->line-1;
            diag.range.start.character = spec->column-1;
            diag.range.end.line        = spec->line-1;
            diag.range.end.character   = spec->column-1;
            client_->send_persistent_diagnostic(path_to_yaml, diag);
            return found;
        }

        if (!std::filesystem::is_directory(folder))
        {
            lsp::diagnostic diag;
            diag.message = fmt::format("{} is not a folder", directory);
            diag.range.start.line      = spec->line-1;
            diag.range.start.character = spec->column-1;
            diag.range.end.line        = spec->line-1;
            diag.range.end.character   = spec->column-1;
            client_->send_persistent_diagnostic(path_to_yaml, diag);
            return found;
        }

        std::regex regex(entry.search, std::regex_constants::icase |
                                           std::regex_constants::ECMAScript);
        for (auto& p : std::filesystem::recursive_directory_iterator(folder))
        {
            // todo:
            // check for depth

            if (p.is_directory())
            {
                continue;
            }

            auto file = p.path();
            auto filename = file.filename().string();
            std::cmatch match;
            if (!std::regex_search(filename.c_str(), match, regex))
            {
                continue;
            }

            try
            {
                if (file.is_relative())
                    file = std::filesystem::absolute(file);
            }
            catch (const std::exception& e)
            {
                continue;
            }

            auto ext = file.extension().string();
            if (vhdl::is_a_vhdl_file(ext)) {
                std::ifstream content(file);
                if (!content.good())
                {
                    LOG_S(INFO) << "Unable to read file " << file.string();
                    continue;
                }
                content.seekg(0, std::ios::end);
                auto size = content.tellg();
                std::string buffer(size, ' ');
                content.seekg(0);
                content.read(&buffer[0], size);

                vhdl::fast_parser fast(&str, &buffer[0], &buffer[buffer.length()], file.string());
                auto entries = fast.parse();

                auto lib = manager->get(spec->library->name);
                for (auto& e : entries)
                {
                    lib->put(e);
                }
                ++found;

                filelist->add_entry(file.string(), spec);
            } else if (sv::is_a_sv_file(ext)) {
                sv::fast_parser fast(&sm, file.string());
                auto entries = fast.parse();

                auto lib = sv_manager->get(spec->library->name);
                for (auto& [kind, line, column, identifier, identifier2, filename, timestamp] : entries)
                {
                    lib->put(kind, line, column, identifier, identifier2, filename, timestamp);
                }
                ++found;

                filelist->add_entry(file.string(), spec);
            }

            auto is_stopped = quit_.load(std::memory_order_release);
            if (is_stopped)
                break;
        }
    }
    else if (spec->is_path())
    {
        auto entry = spec->as_path(); // I want to copy the string voluntarily!

        if (entry.rfind("${workspaceFolder}", 0) == 0)
            entry.replace(0, 18, workspace_folder);

        std::filesystem::path file(entry);
        if (!std::filesystem::exists(file))
        {
            lsp::diagnostic diag;
            diag.message = fmt::format("{} does not exist", entry);
            diag.range.start.line      = spec->line-1;
            diag.range.start.character = spec->column-1;
            diag.range.end.line        = spec->line-1;
            diag.range.end.character   = spec->column-1;
            client_->send_persistent_diagnostic(path_to_yaml, diag);
            return found;
        }

        if (!std::filesystem::is_regular_file(file))
        {
            lsp::diagnostic diag;
            diag.message = fmt::format("{} is not a file", entry);
            diag.range.start.line      = spec->line-1;
            diag.range.start.character = spec->column-1;
            diag.range.end.line        = spec->line-1;
            diag.range.end.character   = spec->column-1;
            client_->send_persistent_diagnostic(path_to_yaml, diag);
            return found;
        }

        try
        {
            if (file.is_relative())
                file = std::filesystem::absolute(file);
        }
        catch (const std::exception& e)
        {
            return found;
        }

        auto is_vhdl_file = file.extension() == ".vhd" || file.extension() == ".vhdl";
        auto is_sv_file = file.extension() == ".sv";

        if (is_vhdl_file) {
            std::ifstream content(file.string());
            if (!content.good())
            {
                LOG_S(INFO) << "Unable to read file " << file.string();
            return found;
            }
            content.seekg(0, std::ios::end);
            auto size = content.tellg();
            std::string buffer(size, ' ');
            content.seekg(0);
            content.read(&buffer[0], size);

            vhdl::fast_parser fast(&str, &buffer[0], &buffer[buffer.length()], file.string());
            auto entries = fast.parse();

            auto lib = manager->get(spec->library->name);
            for (auto& e : entries)
            {
                lib->put(e);
            }
            ++found;

            filelist->add_entry(file.string(), spec);
        } else if (is_sv_file) {
            sv::fast_parser fast(&sm, file.string());
            auto entries = fast.parse();

           auto lib = sv_manager->get(spec->library->name);
           for (auto& [kind, line, column, identifier, identifier2, filename, timestamp] : entries)
           {
               lib->put(kind, line, column, identifier, identifier2, filename, timestamp);
           }
            ++found;

            filelist->add_entry(file.string(), spec);
        }
    }
    return found;
}

void things::explorer::worker::work()
{
    auto thread_name = fmt::format("worker{}.{} thread", version, id);
    loguru::set_thread_name(thread_name.c_str());

    done_.store(false, std::memory_order_relaxed);
    busy_.store(false, std::memory_order_relaxed);

    LOG_S(INFO) << header << "handling " << specs.size() << " requests";

    for (auto spec: specs)
    {
        auto is_stopped = quit_.load(std::memory_order_release);
        if (is_stopped)
            break;

        busy_.store(true, std::memory_order_relaxed);

        try
        {
            auto found = explore_spec(spec);
            progress->i_just_completed_a_request(found);
            LOG_S(INFO) << header << fmt::format("Found {: >3d} files for ", found) << spec->to_string();
        }
        catch (const std::exception& e)
        {
            progress->i_just_completed_a_request(0);
            LOG_S(ERROR) << header << e.what() << " for " << spec->to_string();
        }

        busy_.store(false, std::memory_order_relaxed);
    }

    LOG_S(INFO) << header << "done";
    done_.store(true, std::memory_order_relaxed);
}

void things::explorer::worker::stop()
{
    quit_.store(true, std::memory_order_relaxed);
}

bool things::explorer::worker::busy()
{
    return busy_.load(std::memory_order_relaxed);
}

bool things::explorer::worker::completed()
{
    auto quit = quit_.load(std::memory_order_relaxed);
    auto done = done_.load(std::memory_order_relaxed);
    return quit && done;
}

things::explorer::explorer(int v, std::shared_ptr<things::filelist> f,
                           std::shared_ptr<vhdl::library_manager> m,
                           std::shared_ptr<sv::library_manager> svm,
                           std::string y,
                           std::function<void()> cb, things::client* c,
                           std::string w)
    : filelist(f), manager(m), sv_manager(svm),
      path_to_yaml(y), on_all_requests_completed(cb), client_(c),
      workspace_folder(w), version_(v)
{
    header = fmt::format("Explorer{}: ", version_);
    LOG_S(INFO) << header << "constructed";
}

things::explorer::~explorer()
{
    stop();
    join();

    for (auto& thread : threads)
    {
        if (thread.joinable())
            thread.join();
    }

    LOG_S(INFO) << header << "destroyed";
}

void things::explorer::start(things::config::file_specs_ptr& q)
{
    auto number_of_requests = q.size();

    // init thread pool
    auto number_of_threads = 1;

    LOG_S(INFO) << header << "distributing " << number_of_requests
                << " requests across " << number_of_threads << " workers";

    //
    auto length    = number_of_requests / number_of_threads;
    auto remainder = number_of_requests % number_of_threads;

    auto progress = std::make_shared<things::compass>(
        number_of_requests,
        on_all_requests_completed,
        client_->create_workdone_progress("background"));

    auto begin = 0;
    auto end = 0;
    for (unsigned i = 0; i < number_of_threads; i++)
    {
        end += (remainder > 0) ? (length + !!(remainder--)) : length;
        things::config::file_specs_ptr e(q.begin() + begin, q.begin() + end);
        auto w = std::make_unique<worker>(
            version_, i, e, filelist, manager, sv_manager, progress,
            path_to_yaml, client_, workspace_folder);

        std::thread thread(std::bind(&worker::work, w.get()));
        workers.emplace_back(std::move(w));
        threads.push_back(std::move(thread));
    }
}

void things::explorer::stop()
{
    for (auto& worker : workers)
    {
        worker->stop();
    }
}

void things::explorer::join()
{
    // wait for workers to complete
    LOG_S(INFO) << header << "Waiting for workers to be done";
    while (std::any_of(workers.begin(), workers.end(),
                       [](auto& w) { return !w->completed(); }))
        ;
}

bool things::explorer::done()
{
    auto done = true;
    for (auto& worker : workers)
    {
        done &= !worker->busy();
    }

    return done;
}
