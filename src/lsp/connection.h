
#ifndef LSP_CONNECTION_H
#define LSP_CONNECTION_H

#include <atomic>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

namespace lsp
{

class journal_writer
{
    public:
    journal_writer(const std::string&);
    ~journal_writer();

    bool is_valid();

    void dump_read(const std::string&);
    void dump_write(const std::string&);

    std::ofstream output;
    std::mutex lock;
};

class connection
{
    public:
    virtual std::optional<std::string> read() = 0;
    virtual void write(const std::string& message) = 0;

    virtual bool good() = 0;

    virtual bool tee(std::string&) final;

    protected:

    struct message_header
    {
        unsigned int content_length = 0;
        std::string content_type = "";
    };

    virtual message_header read_message_header() = 0;

    std::optional<journal_writer> tee_;

};

class stdio: public connection
{
    public:
    std::optional<std::string> read();
    void write(const std::string& message);

    bool good();

    protected:

    std::atomic_bool good_ = true;
    std::mutex       lock_;

    message_header read_message_header();
};

class journal_reader
{
    std::ifstream input;
    int line_number;

    public:
    journal_reader(const std::string&);
    ~journal_reader();

    struct transactions
    {
        bool eof;
        std::list<std::tuple<int, std::string>> requests;
        std::list<std::tuple<int, bool, std::string>> responses;
    };
    transactions next();
};

class replay: public connection
{
    std::string filename;
    journal_reader reader;
    journal_reader::transactions current;

    public:
    replay(std::string&);
    ~replay();

    std::optional<std::string> read();
    void write(const std::string&);

    bool good();

    protected:

    std::atomic_bool stopped = false;

    message_header read_message_header();
    bool compare_expected_response_vs_actual_write(std::string&, std::string&);
    std::optional<std::string> wait_for_response_or_else_get_next_request();
};

}

#endif
