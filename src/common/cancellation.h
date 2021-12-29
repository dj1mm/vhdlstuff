
#ifndef COMMON_CANCELLATION_H
#define COMMON_CANCELLATION_H

#include <atomic>
#include <memory>

namespace common
{

class cancellation_state
{
    public:
    cancellation_state();
    cancellation_state(const cancellation_state&) = delete;
    cancellation_state(cancellation_state&&) = delete;
    cancellation_state& operator=(const cancellation_state&) = delete;
    cancellation_state& operator=(cancellation_state&&) = delete;
    ~cancellation_state() = default;

    std::atomic_bool is_cancelled;
};

// forward declaration needed by cancellation_source
class cancellation_token;

class cancellation_source
{
    public:
    void request_cancellation();
    bool is_cancelled();

    cancellation_token token();

    private:

    std::shared_ptr<cancellation_state> state;

    friend cancellation_token;
};

class cancellation_token
{
    public:
    cancellation_token(cancellation_source* source);

    cancellation_token() = default;
    cancellation_token(const cancellation_token&) = default;
    cancellation_token(cancellation_token&&) = default;
    cancellation_token& operator=(const cancellation_token&) = default;
    cancellation_token& operator=(cancellation_token&&) = default;
    ~cancellation_token() = default;

    bool is_cancelled();
    operator bool();

    private:

    std::shared_ptr<cancellation_state> state;
};

}

#endif
