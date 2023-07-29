
#ifndef SV_AST_H
#define SV_AST_H

#include "sv/library_manager.h"

#include "slang/util/Bag.h"
#include "slang/ast/Compilation.h"
#include "slang/text/SourceManager.h"
#include "slang/diagnostics/Diagnostics.h"
#include "slang/syntax/SyntaxTree.h"

namespace sv {

class ast
{
    public:
    ast(std::string, std::shared_ptr<sv::library_manager>, std::string,
        std::vector<std::string>&);
    ast(const ast&) = delete;
    ast(ast&&) = default;
    ast& operator=(const ast&) = delete;
    ast& operator=(ast&&) = default;
    ~ast() = default;

    // this function can take a while to return because all the parsing and
    // semantic analysis will be done there.
    //
    // Returns true if ast was already up to date and nothing was done.
    // Returns false otherwise.
    // In this case, it means that the ast was rebuilt/updated. I think this
    // can be used by the caller to determine whether there are new diagnostics
    // to send to the client
    bool update();

    // this function will return quickly
    void invalidate_main_file();

    // this function will return quickly
    void invalidate_reference_file(std::string&);

    // this function will return quickly
    // Note that the main file might be out of date if invalidate_x() functions
    // were called between the last update() and this one. However, the main
    // file is still useful.
    // This function can return nullptr. In which case, it means that file not
    // found
    std::shared_ptr<slang::syntax::SyntaxTree> get_main_file();

    slang::SourceManager& get_source_manager();

    // this function will return quickly
    // return the current parse errors and semantic errors.
    //
    // Note that the returned errors might be out of date if the invalidate_x()
    // functions were called between the last update() and this one. However,
    // the diags are still useful.
    const std::tuple<slang::Diagnostics, slang::Diagnostics> get_diagnostics();

    // Get work library where the main design units in this ast shall be stored
    std::string get_work_library_name();

    // Return true if no invalidate_x() functions were called between the last
    // update() and this one.
    bool is_uptodate();

    std::string get_filename();

    private:

    std::string filename;
    std::string worklibrary;
    std::vector<std::string> incdirs;

    std::shared_ptr<slang::syntax::SyntaxTree> main_file;
    std::shared_ptr<sv::library_manager> library_manager;

    public:
    slang::ast::Compilation compilation;

    slang::SourceManager sm;

    bool invalidated_;
};

bool is_a_sv_file(std::string&);

};

#endif
