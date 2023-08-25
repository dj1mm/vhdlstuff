
#include <algorithm>
#include <vector>
#include <string_view>

#include "rapidjson/error/en.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"

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

template <typename OutputHandler> struct expand_all_macros
{
    expand_all_macros(OutputHandler& out, std::filesystem::path cwd)
    : out_(out), cwd_(cwd)
    {
    }

    bool Null()
    {
        return out_.Null();
    }

    bool Bool(bool b)
    {
        return out_.Bool(b);
    }

    bool Int(int i)
    {
        return out_.Int(i);
    }

    bool Uint(unsigned u)
    {
        return out_.Uint(u);
    }

    bool Int64(int64_t i)
    {
        return out_.Int64(i);
    }

    bool Uint64(uint64_t u)
    {
        return out_.Uint64(u);
    }

    bool Double(double d)
    {
        return out_.Double(d);
    }

    bool RawNumber(const char* str, rapidjson::SizeType length, bool copy)
    {
        return out_.RawNumber(str, length, copy);
    }

    bool String(const char* str, rapidjson::SizeType length, bool)
    {
        if (this_is_a_yaml_object_key)
            return out_.String(str, length, false);

        std::string_view input(str);

        auto start_pos = input.find("${file:");
        if (start_pos == std::string::npos)
            return out_.String(str, length, false);

        auto end_pos = input.find("}", start_pos);
        if (end_pos == std::string::npos || end_pos <= start_pos)
            return out_.String(str, length, false);

        std::string result(input);
        std::string replacement = "file://" + cwd_;

        auto folder_or_file = result.substr(start_pos+7, end_pos-start_pos-7);
        if (folder_or_file.size())
            replacement += "/" + folder_or_file;
        result.replace(start_pos, end_pos-start_pos+1, replacement);
        return out_.String(&result[0], result.size(), true);
    }

    bool StartObject()
    {
        return out_.StartObject();
    }

    bool Key(const char* str, rapidjson::SizeType length, bool copy)
    {
        this_is_a_yaml_object_key = true;
        auto result = String(str, length, copy);
        this_is_a_yaml_object_key = false;
        return result;
    }

    bool EndObject(rapidjson::SizeType memberCount)
    {
        return out_.EndObject(memberCount);
    }

    bool StartArray()
    {
        return out_.StartArray();
    }

    bool EndArray(rapidjson::SizeType elementCount)
    {
        return out_.EndArray(elementCount);
    }

    OutputHandler& out_;
    std::string cwd_;
    bool this_is_a_yaml_object_key = false;
};

lsp::replay::replay(std::string& filename)
: filename(filename), reader(filename), current(reader.next()),
  path_to_journal(std::filesystem::path(filename).parent_path())
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

bool lsp::replay::compare_expected_response_vs_actual_write(
    std::string& expected, std::string& actual)
{
    return expected == actual;
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

            rapidjson::StringBuffer sb;
            rapidjson::Writer<rapidjson::StringBuffer> w(sb);
            expand_all_macros<rapidjson::Writer<rapidjson::StringBuffer>>
                filter(w, path_to_journal);

            rapidjson::Reader reader;
            rapidjson::StringStream ss(std::get<std::string>(req).c_str());
            reader.Parse(ss, filter);
            return sb.GetString();
        }

        if (current.responses.size() > 0)
        {
            while (current.responses.size() > 0)
            {
                auto [line, something, expect] = current.responses.front();
                rapidjson::StringBuffer sb;
                rapidjson::Writer<rapidjson::StringBuffer> w(sb);
                expand_all_macros<rapidjson::Writer<rapidjson::StringBuffer>>
                    filter(w, path_to_journal);

                rapidjson::Reader reader;
                rapidjson::StringStream ss(expect.c_str());
                reader.Parse(ss, filter);
                std::string expected(sb.GetString());

                current.responses.pop_front();
                number_of_responses_in_the_journal++;

                if (unhandled_responses.size() > 0)
                {
                    auto response = std::find_if(unhandled_responses.begin(), unhandled_responses.end(), [&] (const auto& r) { return r == expected; });
                    if (response != std::end(unhandled_responses))
                    {
                        LOG_S(1) << "MATCH OOO: " << filename << ":" << line << ": " << *response;
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
                        LOG_S(ERROR) << "TIMEOUT: " << filename << ":" << line << ": " << expected;
                        number_of_timeouts++;
                        timedout_or_matched = true;
                    }
                    else if (*value == expected)
                    {
                        LOG_S(1) << "MATCH: " << filename << ":" << line << ": " << *value;
                        number_of_matches++;
                        timedout_or_matched = true;
                    }
                    else
                    {
                        LOG_S(1) << "IGNORED: " << filename << ":" << line << ": " << *value;
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

