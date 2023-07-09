
#ifndef SV_FAST_PARSER_H
#define SV_FAST_PARSER_H

#include "slang/text/SourceManager.h"
#include "sv/library_manager.h"

namespace sv
{

class fast_parser
{
    slang::SourceManager* sm;
    std::string file;

    public:
    fast_parser(slang::SourceManager*, std::string);
    fast_parser(const fast_parser&) = delete;
    fast_parser(fast_parser&&) = default;
    fast_parser& operator=(const fast_parser&) = delete;
    fast_parser& operator=(fast_parser&&) = default;
    ~fast_parser() = default;

    //
    // Fast parse the initialized file and look for design units
    //
    std::vector<std::tuple<library_cell_kind, unsigned, unsigned, std::string,
                           std::optional<std::string>, std::string, time_t>>
    parse();
};

}; // namespace sv

#endif
