
#include "cancellation.h"

common::cancellation_state::cancellation_state(): is_cancelled(false)
{

}

void common::cancellation_source::request_cancellation()
{
    state->is_cancelled.store(true, std::memory_order_relaxed);
}

bool common::cancellation_source::is_cancelled()
{
    return state->is_cancelled.load(std::memory_order_relaxed);
}

common::cancellation_token common::cancellation_source::token()
{
    return common::cancellation_token(this);
}

common::cancellation_token::cancellation_token(cancellation_source* source)
: state(source->state)
{

}

bool common::cancellation_token::is_cancelled()
{
    if (!state)
        return false;
    return state->is_cancelled.load(std::memory_order_relaxed);
}

common::cancellation_token::operator bool()
{
    return is_cancelled();
}

