
#include "project.h"
#include "language.h"

#include <regex>

#include "fmt/format.h"

constexpr std::string_view Yaml_Parse_errors           = "Could not parse yaml because of syntax errors.";
constexpr std::string_view Yaml_File_Not_found         = "{} not found.";
constexpr std::string_view Yaml_File_No_Libraries      = "Expected a section called libraries.";
constexpr std::string_view Yaml_File_Invalid_Libraries = "Expected an array of libraries.";
constexpr std::string_view Error_Unnamed_Library       = "Skipping this library because a name was not provided.";

void things::filelist::add_entry(std::string name, std::string library)
{
    std::lock_guard guard(mtx_);
    if (path_to_entries.find(name) == path_to_entries.end())
    {
        things::filelist::entry entry;
        entry.filename = name;
        entry.index = total_number_of_files++;
        entry.libs.push_back(library);

        // we can add the new entry to our repertoire
        entries.push_back(entry);
        path_to_entries[name] = entry.index;
    }
    else
    {
        entries[path_to_entries[name]].libs.push_back(library);
    }
}

std::optional<things::filelist::entry>
things::filelist::get_entry(std::string name)
{
    std::lock_guard guard(mtx_);
    if (path_to_entries.find(name) == path_to_entries.end())
        return std::nullopt;

    return entries[path_to_entries[name]];
}

things::project::project(things::language* s, things::client* c)
    : project_folder_(std::filesystem::current_path()), server_(s), client_(c),
      loaded_version_(0)
{
    current_filelist_ = std::make_shared<things::filelist>();

    current_library_manager_ =
        std::make_shared<vhdl::library_manager>(std::nullopt, true);

    current_background_explorer_ = std::make_unique<things::explorer>(
        current_filelist_, current_library_manager_,
        std::bind(&things::project::add_message_and_send_to_client, this,
                  std::placeholders::_1),
        server_, client_, project_folder_.string());
}

void things::project::initialise(std::optional<std::string> root)
{
    if (root)
        project_folder_ = *root;
}

bool things::project::is_loaded()
{
    return path_to_loaded_yaml_ && current_library_manager_ &&
           current_filelist_;
}

int things::project::get_loaded_version()
{
    return loaded_version_.load();
}

bool things::project::libraries_have_been_populated()
{
    if (current_background_explorer_)
        return current_background_explorer_->done();

    return false;
}

void things::project::set_messages_and_send_to_client(
    std::vector<common::diagnostic>& diags)
{
    std::lock_guard lock(cb_mtx_);
    diagnostics_ = diags;

    if (client_)
        client_->send_diagnostics(*path_to_last_yaml_, diagnostics_);
}

void things::project::add_message_and_send_to_client(common::diagnostic diag)
{
    std::lock_guard lock(cb_mtx_);
    diagnostics_.push_back(diag);

    if (client_)
        client_->send_diagnostics(*path_to_last_yaml_, diagnostics_);
}

std::shared_ptr<vhdl::library_manager> things::project::
    get_current_library_manager()
{
    std::lock_guard lock(clm_mtx_);
    return current_library_manager_;
}

bool things::project::
    load_yaml_reset_project_kick_background_index_destroy_libraries()
{
    // this function does a lot of things.
    // Because of that, it takes very long to complete. We can probably break
    // this function down a lil bit.

    // ------------------------------------------------------------------------
    // First thing first: Set things up
    // ------------------------------------------------------------------------
    std::vector<common::diagnostic> diags;
    std::vector<std::string> names_of_all_libraries;
    std::vector<things::yaml_entry> all_yaml_entries;

    auto path_to_yaml = project_folder_ / "vhdl_config.yaml";
    path_to_last_yaml_ = path_to_yaml.string();
    std::string_view path_to_last_yaml_v(*path_to_last_yaml_);

    auto temp_mgr =
        std::make_shared<vhdl::library_manager>(std::nullopt, false);
    auto temp_lst = std::make_shared<things::filelist>();
    auto temp_xpl = std::make_unique<things::explorer>(
        temp_lst, temp_mgr,
        std::bind(&things::project::add_message_and_send_to_client, this,
                  std::placeholders::_1),
        server_, client_, project_folder_.string());

    // ------------------------------------------------------------------------
    // 1) Load the yaml
    // ------------------------------------------------------------------------
    try
    {   // this is here because std::filesystem::absolute can throw
        if (path_to_yaml.is_relative())
            path_to_yaml = std::filesystem::absolute(path_to_yaml);
    }
    catch (const std::exception& e)
    {
        // in which case, there is no point to continue
        set_messages_and_send_to_client(diags);
        return false;
    }

    if (!std::filesystem::exists(path_to_yaml))
    {
        set_messages_and_send_to_client(diags);
        return false;
    }

    try
    {   // yamlcpp can throw. So catch those wild exceptions
        auto config = YAML::LoadFile(path_to_yaml.string());
        if (!config["libraries"])
        {
            const auto& m = config.Mark();
            common::location loc(path_to_last_yaml_v, 1, 1);
            common::diagnostic diag(Yaml_File_No_Libraries, loc);
            diags.push_back(diag);
            set_messages_and_send_to_client(diags);
            return false;
        }

        if (!config["libraries"].IsSequence())
        {
            const auto& m = config["libraries"].Mark();
            common::location loc(path_to_last_yaml_v, 1, 1, 1, 1);
            common::diagnostic diag(Yaml_File_Invalid_Libraries, loc);
            diags.push_back(diag);
            set_messages_and_send_to_client(diags);
            return false;
        }

        auto libs = config["libraries"];

        for (auto it1 = libs.begin(); it1 != libs.end(); ++it1)
        {
            auto lib = it1->as<YAML::Node>();
            if (!lib["name"])
            {
                const auto& m = it1->Mark();
                common::location loc(path_to_last_yaml_v, m.line, m.column,
                                     m.line, 0);
                common::diagnostic diag(Error_Unnamed_Library, loc);
                diags.push_back(diag);
                continue;
            }
            auto library = lib["name"].as<std::string>();
            names_of_all_libraries.push_back(library);

            if (!lib["files"])
                continue;

            auto files = lib["files"];
            for (auto it2 = files.begin(); it2 != files.end(); ++it2)
            {
                auto file = it2->as<YAML::Node>();
                if (file.IsScalar())
                {
                    yaml_entry entry;
                    entry.line_number = file.Mark().line;
                    entry.search = file.as<std::string>();
                    entry.library = library;
                    all_yaml_entries.push_back(entry);
                    continue;
                }

                if (!file["directory"] && !file["directory"].IsScalar())
                    continue;

                if (!file["search"] && !file["search"].IsScalar())
                    continue;

                yaml_entry entry;
                entry.line_number = file.Mark().line;
                entry.search = file["search"].as<std::string>();
                entry.library = library;
                entry.directory = file["directory"].as<std::string>();

                if (file["depth"] && file["depth"].IsScalar())
                    entry.depth = file["depth"].as<int>();

                all_yaml_entries.push_back(entry);
            }

        }
    }
    catch (const std::exception& e)
    {
        common::location loc(path_to_last_yaml_v, 1, 1, 1, 1);
        common::diagnostic diag(Yaml_Parse_errors, loc);
        diags.push_back(diag);
        set_messages_and_send_to_client(diags);
        return false;
    }

    // ------------------------------------------------------------------------
    // 2) Reset project variables
    // ------------------------------------------------------------------------

    // if we are here, YAML file had no syntax errors and we have something
    // useful to work with.
    path_to_loaded_yaml_ = path_to_yaml;
    LOG_S(INFO) << "Project explorer loaded file " << path_to_yaml.string();

    loaded_version_++;

    // should we wait? I think so
    current_background_explorer_->stop();
    current_background_explorer_->join();
    current_background_explorer_.reset(nullptr);
    current_background_explorer_ = std::move(temp_xpl);

    current_filelist_.reset();
    current_filelist_ = std::move(temp_lst);


    // ------------------------------------------------------------------------
    // 3) Kick background indexing
    // ------------------------------------------------------------------------
    current_background_explorer_->start(all_yaml_entries);

    // ------------------------------------------------------------------------
    // 4) Destroy the old library manager
    // ------------------------------------------------------------------------
    std::lock_guard lock(clm_mtx_);
    current_library_manager_->destroy();
    current_library_manager_.reset();
    current_library_manager_ = temp_mgr;
    current_library_manager_->initialise(names_of_all_libraries);

    set_messages_and_send_to_client(diags);
    return true;
}

std::vector<std::string> things::project::get_libraries_this_file_is_part_of(
    std::string& name)
{
    // I dont current filelist needs to be mutexed because this function is
    // only ever called in the main thread
    auto entry = current_filelist_->get_entry(name);
    if (!entry)
        return {};

    return entry->libs;
}

things::explorer::worker::worker(int id, std::vector<things::yaml_entry> e,
                                 std::shared_ptr<things::filelist> f,
                                 std::shared_ptr<vhdl::library_manager> m,
                                 progress* p, std::function<void()> u,
                                 std::function<void(common::diagnostic)> c,
                                 std::string w)
    : id(id), busy_(false), quit_(false), done_(false), entries(e), manager(m),
      filelist(f), progress_(p),
      send_progress_update(u), add_message_and_send_to_client(c),
      workspace_folder(w)
{
}

int things::explorer::worker::explore_entry(things::yaml_entry& entry)
{
    auto found = 0;
    if (entry.directory)
    {
        LOG_S(INFO) << "Exploring " << *entry.directory
                    << " with filter: " << entry.search;

        if (entry.directory->rfind("${workspaceFolder}", 0) == 0)
            entry.directory->replace(0, 18, workspace_folder);

        std::filesystem::path folder(*entry.directory);
        if (!std::filesystem::exists(folder))
        {
            common::location loc("", entry.line_number, 1, entry.line_number,
                                 1);
            common::diagnostic diag("{} does not exist", loc);
            diag << *entry.directory;
            add_message_and_send_to_client(diag);
            return found;
        }

        if (!std::filesystem::is_directory(folder))
        {
            common::location loc("", entry.line_number, 1, entry.line_number,
                                 1);
            common::diagnostic diag("{} is not a folder", loc);
            diag << *entry.directory;
            add_message_and_send_to_client(diag);
            return found;
        }

        auto current = progress_->indexed.load();

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

            if (found % 123 == 0)
            {
                progress_->indexed.store(current + found);
                send_progress_update();
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

            auto lib = manager->get(entry.library);
            for (auto& e : entries)
            {
                lib->put(e);
            }
            ++found;

            filelist->add_entry(file.string(), entry.library);

            auto is_stopped = quit_.load(std::memory_order_release);
            if (is_stopped)
                break;
        }
    }
    else
    {
        LOG_S(INFO) << "Exploring " << entry.search;

        if (entry.search.rfind("${workspaceFolder}", 0) == 0)
            entry.search.replace(0, 18, workspace_folder);

        std::filesystem::path file(entry.search);
        if (!std::filesystem::exists(file))
        {
            common::location loc("", entry.line_number, 1, entry.line_number,
                                 1);
            common::diagnostic diag("{} does not exist", loc);
            diag << entry.search;
            add_message_and_send_to_client(diag);
            return found;
        }

        if (!std::filesystem::is_regular_file(file))
        {
            common::location loc("", entry.line_number, 1, entry.line_number,
                                 1);
            common::diagnostic diag("{} is not a file", loc);
            diag << entry.search;
            add_message_and_send_to_client(diag);
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

        auto current = progress_->indexed.load();
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

        auto lib = manager->get(entry.library);
        for (auto& e : entries)
        {
            lib->put(e);
        }
        ++found;

        filelist->add_entry(file.string(), entry.library);
    }
    return found;
}

void things::explorer::worker::work()
{
    auto worker_name = "Explorer" + std::to_string(id);

    auto thread_name = "xpl" + std::to_string(id) + " thread";
    loguru::set_thread_name(thread_name.c_str());

    done_.store(false, std::memory_order_relaxed);
    busy_.store(false, std::memory_order_relaxed);

    LOG_S(INFO) << worker_name << " started";

    for (auto entry: entries)
    {
        auto current_indexed = progress_->indexed.load();
        send_progress_update();

        auto is_stopped = quit_.load(std::memory_order_release);
        if (is_stopped)
            break;

        busy_.store(true, std::memory_order_relaxed);

        try
        {
            auto found = explore_entry(entry);
            progress_->indexed = current_indexed + found;
            LOG_S(INFO) << "Found " << found << " files";
        }
        catch (const std::exception& e)
        {
            LOG_S(ERROR) << worker_name << " " << e.what();
        }

        progress_->completed++;
        busy_.store(false, std::memory_order_relaxed);
    }
    send_progress_update();

    LOG_S(INFO) << worker_name << " done";
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

things::explorer::explorer(std::shared_ptr<things::filelist> f,
                           std::shared_ptr<vhdl::library_manager> m,
                           std::function<void(common::diagnostic)> cb,
                           things::language* s, things::client* c,
                           std::string w)
    : filelist(f), manager(m), add_message_and_send_to_client(cb), server_(s),
      client_(c), workspace_folder(w)
{
    LOG_S(INFO) << "Project explorer constructed";
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

    LOG_S(INFO) << "Project explorer destroyed";
}

void things::explorer::start(std::vector<things::yaml_entry>& q)
{
    progress_bar = client_->create_workdone_progress("background");
    auto number_of_yaml_entries = q.size();

    // init thread pool
    auto number_of_threads = 1;

    //
    auto length    = number_of_yaml_entries / number_of_threads;
    auto remainder = number_of_yaml_entries % number_of_threads;

    progress_.indexed.store(0);
    progress_.completed.store(0, std::memory_order_relaxed);
    progress_.total.store(number_of_yaml_entries, std::memory_order_relaxed);

    auto begin = 0;
    auto end = 0;
    for (unsigned i = 0; i < number_of_threads; i++)
    {
        end += (remainder > 0) ? (length + !!(remainder--)) : length;
        std::vector<things::yaml_entry> e(q.begin() + begin, q.begin() + end);
        auto w = std::make_unique<worker>(
            i, e, filelist, manager, &progress_,
            std::bind(&things::explorer::send_progress_update, this),
            add_message_and_send_to_client, workspace_folder);

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
    LOG_S(INFO) << "Waiting for workers to complete";
    while (std::any_of(workers.begin(), workers.end(),
                       [](auto& w) { return w->busy(); }))
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

void things::explorer::send_progress_update()
{
    auto p = progress_.total == 0? 100 : progress_.completed * 100 / progress_.total;
    auto msg = fmt::format("Found {} files. (Running/Done/Total = 1/{}/{}).", progress_.indexed, progress_.completed, progress_.total);
    client_->log_message(msg);

    if (progress_bar)
    {
        progress_bar->report(p, msg);
    }

    if (progress_.completed == progress_.total)
    {
        server_->working_files.update_all_files();
        progress_bar.reset();
    }
}
