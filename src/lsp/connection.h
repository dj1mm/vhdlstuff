
#ifndef LSP_CONNECTION_H
#define LSP_CONNECTION_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
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
        bool valid;
        std::list<std::tuple<int, std::string>> requests;
        std::list<std::tuple<int, bool, std::string>> responses;
    };
    transactions next();
};

template<typename T>
class queue
{
    std::mutex mutex;
    std::condition_variable condition;
    std::deque<T> container;
    bool stopped = false;

    public:
    queue() = default;
    queue(queue&&) = default;
    queue& operator=(queue&&) = default;
    ~queue() = default;

    // no copy
    queue(const queue&) = delete;
    queue& operator=(const queue&) = delete;

    void push(const T value) noexcept
    {
        std::unique_lock g(mutex);
        container.push_back(value);
        condition.notify_one();
    }

    std::optional<T> pop() noexcept
    {
        std::unique_lock g(mutex);
        condition.wait(g, [&] { return !container.empty() || stopped; });
        if (container.empty())
            return std::nullopt;

        auto value = container.front();
        container.pop_front();
        return value;
    }

    template<class R, class P>
    std::optional<T> pop(const std::chrono::duration<R, P>& timeout) noexcept
    {
        std::unique_lock g(mutex);
        condition.wait_for(g, timeout, [&] { return !container.empty() || stopped; });
        if (container.empty())
            return std::nullopt;

        auto value = container.front();
        container.pop_front();
        return value;
    }

    void stop() noexcept
    {
        std::unique_lock g(mutex);
        stopped = true;
        condition.notify_all();
    }
};

class replay: public connection
{
    std::string filename;
    journal_reader reader;
    journal_reader::transactions current;
    std::list<std::string> unhandled_responses;

    int number_of_requests_in_the_journal = 0;
    int number_of_responses_in_the_journal = 0;
    int number_of_requests_serviced_by_the_language_server = 0;
    int number_of_responses_generated_by_the_language_server = 0;
    int number_of_ooo_matches = 0;
    int number_of_matches = 0;
    int number_of_ignores = 0;
    int number_of_timeouts = 0;

    queue<std::string> response_queue;

    public:
    replay(std::string&);
    ~replay();

    void print_status();

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
