
#include "connection.h"

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

