
#include <algorithm>
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
                result.valid = true;
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
                result.valid = true;
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
                result.valid = true;
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
    result.valid = false;
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

void lsp::replay::print_status()
{
    LOG_S(1) << "Journal file:    " << number_of_requests_in_the_journal << " requests and " << number_of_responses_in_the_journal << " responses";
    LOG_S(1) << "Language server: " << number_of_requests_serviced_by_the_language_server << " requests and " << number_of_responses_generated_by_the_language_server << " responses";

    LOG_S(1) << "Matches / Ignored / Timed out / Unhandled : " << (number_of_matches + number_of_ooo_matches) << " / " << number_of_ignores << " / " << number_of_timeouts << " / " << unhandled_responses.size();
}

std::optional<std::string> lsp::replay::read()
{
    auto request = wait_for_response_or_else_get_next_request();
    if (!request)
    {
        stopped.store(true);
        return request;
    }
    number_of_requests_serviced_by_the_language_server++;
    return request;
}

void lsp::replay::write(const std::string& message)
{
    number_of_responses_generated_by_the_language_server++;
    response_queue.push(message);
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
    while (current.valid)
    {
        while (current.requests.size() > 0)
        {
            auto req = current.requests.front();
            current.requests.pop_front();
            number_of_requests_in_the_journal++;
            LOG_S(1) << "REQ: " << filename << ":" << std::get<0>(req);
            return std::get<std::string>(req);
        }

        if (current.responses.size() > 0)
        {
            while (current.responses.size() > 0)
            {
                auto expected = current.responses.front();
                current.responses.pop_front();
                number_of_responses_in_the_journal++;

                if (unhandled_responses.size() > 0)
                {
                    auto response = std::find_if(unhandled_responses.begin(), unhandled_responses.end(), [&] (const auto& r) { return r == std::get<2>(expected); });
                    if (response != std::end(unhandled_responses))
                    {
                        LOG_S(1) << "MATCH OOO: " << filename << ":" << std::get<0>(expected) << ": " << *response;
                        unhandled_responses.erase(response);
                        number_of_ooo_matches++;
                        continue;
                    }

                }
                auto timedout_or_matched = false;
                while (!timedout_or_matched)
                {
                    auto value = response_queue.pop(std::chrono::seconds(10));
                    if (!value)
                    {
                        LOG_S(ERROR) << "TIMEOUT: " << filename << ":" << std::get<0>(expected) << ": " << std::get<2>(expected);
                        number_of_timeouts++;
                        timedout_or_matched = true;
                    }
                    else if (*value == std::get<2>(expected))
                    {
                        LOG_S(1) << "MATCH: " << filename << ":" << std::get<0>(expected) << ": " << *value;
                        number_of_matches++;
                        timedout_or_matched = true;
                    }
                    else
                    {
                        LOG_S(1) << "IGNORED: " << filename << ":" << std::get<0>(expected) << ": " << *value;
                        unhandled_responses.push_back(*value);
                        number_of_ignores++;
                    }
                }
            }
        }

        current = reader.next();
    }
    return std::nullopt;
}

