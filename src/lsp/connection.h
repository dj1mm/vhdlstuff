
#ifndef LSP_CONNECTION_H
#define LSP_CONNECTION_H

#include <atomic>
#include <fstream>
#include <iostream>
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

}

#endif
