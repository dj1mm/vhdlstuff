
#include "connection.h"
#include "common/loguru.h"

lsp::journal_writer::journal_writer(const std::string& filename)
: output(filename)
{
    if (is_valid())
        output << "\n# TEST: <fill in the blanks>"
                  "\n" << std::endl;
}

lsp::journal_writer::~journal_writer()
{
    if (is_valid())
        output << "# EXIT: 0" << std::endl;
}

bool lsp::journal_writer::is_valid()
{
    return output.is_open();
}

void lsp::journal_writer::dump_read(const std::string& str)
{
    std::lock_guard<std::mutex> guard(lock);
    if (is_valid())
        output << str << "\n\n---\n" << std::endl;
}

void lsp::journal_writer::dump_write(const std::string& str)
{
    std::lock_guard<std::mutex> guard(lock);
    if (is_valid())
        output << "# MATCH: " << str << "\n\n---\n" << std::endl;
}

bool lsp::connection::tee(std::string& filename)
{
    tee_.emplace(filename);
    return tee_->is_valid();
}

lsp::connection::message_header lsp::stdio::read_message_header()
{
    std::string str;
    const std::string_view content_length("Content-Length: ");

    lsp::connection::message_header header;

    while (good_ && !std::cin.fail() && !std::cin.eof())
    {
        auto c = std::cin.get();
        if (c == '\n') {
            if (str.empty())
                return header;
            if (!str.compare(0, content_length.size(), content_length))
                header.content_length = atoi(str.c_str() + content_length.size());
            str.clear();
        } else if (c != '\r') {
            str += c;
        }
    }

    good_ = false;
    header.content_length = 0;
    return header;
}

std::optional<std::string> lsp::stdio::read()
{
    auto header = read_message_header();

    std::string str;
    str.resize(header.content_length);
    for (int i = 0; i < header.content_length; ++i)
    {
        if (std::cin.fail() || std::cin.eof())
        {
            good_ = false;
            break;
        }
        str[i] = std::cin.get();
    }

    if (!good_ || header.content_length == 0)
    {
        return std::nullopt;
    }

    if (tee_)
        tee_->dump_read(str);

    return std::move(str);
}

void lsp::stdio::write(const std::string& message)
{
    if (!good_ || !std::cout.good())
    {
        good_ = false;
        return;
    }

    std::lock_guard<std::mutex> guard(lock_);

    if (tee_)
        tee_->dump_write(message);

    std::cout <<
        "Content-Length: " << message.length() <<
#if WIN32
        "\n\n" << // this is needed for some reason that I cannot understand
#else
        "\r\n\r\n" <<
#endif
        message << std::flush;
}

bool lsp::stdio::good()
{
    return good_;
}

lsp::journal_reader::journal_reader(const std::string& filename)
: input(filename), line_number(0)
{
}

lsp::journal_reader::~journal_reader()
{

}

lsp::journal_reader::transactions lsp::journal_reader::next()
{
    std::list<std::tuple<int, std::string>> requests;
    std::list<std::tuple<int, bool, std::string>> responses;

    enum class state { idle, midline, eof };
    state current_state = state::idle;
    bool not_eof = false;
    std::string aggregated_line;
    int start_of_aggregated_line;
    std::string line;

    while (current_state != state::eof && !input.fail()) {
        auto next_state = current_state;
        auto not_eof = (bool) std::getline(input, line);
        if (not_eof) {
            line_number++;
        } else {
            current_state = state::eof;
            next_state = state::eof;
        }

        switch (current_state) {
        case state::idle:
            if (line == "---" && (requests.size() + responses.size()) > 0) {
                transactions result;
                result.valid = !not_eof;
                result.requests = requests;
                result.responses = responses;
                return result;
            }
            if (line == "---" || line.empty()) {
                break;
            }
            next_state = state::midline;
            aggregated_line += line;
            start_of_aggregated_line = line_number;
            break;
        case state::midline:
            if ((line.empty() || line == "---") && !aggregated_line.empty()) {
                if (aggregated_line.starts_with("# MATCH: "))
                    responses.push_back(std::make_tuple(start_of_aggregated_line, false, aggregated_line.substr(9)));
                else
                    requests.push_back(std::make_tuple(start_of_aggregated_line, aggregated_line));
                aggregated_line.clear();
            }
            if (line == "---") {
                transactions result;
                result.valid = !not_eof;
                result.requests = requests;
                result.responses = responses;
                return result;
            }
            if (line.empty()) {
                next_state = state::idle;
                break;
            }
            next_state = state::midline;
            aggregated_line += line;
            break;
        case state::eof:
            if (!aggregated_line.empty()) {
                if (aggregated_line.starts_with("# MATCH: "))
                    responses.push_back(std::make_tuple(start_of_aggregated_line, false, aggregated_line.substr(9)));
                else
                    requests.push_back(std::make_tuple(start_of_aggregated_line, aggregated_line));
                aggregated_line.clear();
            }
            {
                transactions result;
                result.valid = !not_eof;
                result.requests = requests;
                result.responses = responses;
                return result;
            }
            break;
        default:
            break;
        }
        current_state = next_state;
    }
    transactions result;
    result.valid = true;
    result.requests = requests;
    result.responses = responses;
    return result;
}

lsp::replay::replay(std::string& filename)
: filename(filename), reader(filename), current(reader.next())
{
}

lsp::replay::~replay()
{
    if (!current.valid || !stopped.load())
    {
        LOG_S(1) << "EOF not reached: server exitted prematurely";
    }

    stopped.store(true);
}

std::optional<std::string> lsp::replay::read()
{
    auto request = wait_for_response_or_else_get_next_request();
    if (!request)
    {
        stopped.store(true);
    }
    return request;
}

void lsp::replay::write(const std::string& message)
{
    LOG_S(1) << "RESP: " << message;
}

bool lsp::replay::good()
{
    return !stopped.load();
}

lsp::connection::message_header lsp::replay::read_message_header()
{
    return {};
}

std::optional<std::string>
lsp::replay::wait_for_response_or_else_get_next_request()
{
    while (!current.valid)
    {
        if (current.requests.size() > 0)
        {
            auto req = current.requests.front();
            current.requests.pop_front();
            LOG_S(1) << filename << ":" << std::get<0>(req) << ": " << std::get<1>(req);
            return std::get<std::string>(req);
        }

        if (current.responses.size() > 0)
        {
            // wait for responses here
            // for (auto resp: current.responses) {
            //     LOG_S(1) << "CHECK MATCH: Line" << std::get<0>(resp) << ": " << std::get<2>(resp);
            // }
        }

        current = reader.next();
    }
    return std::nullopt;
}

