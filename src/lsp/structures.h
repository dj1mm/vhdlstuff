
#ifndef LSP_STRUCTURES_H
#define LSP_STRUCTURES_H

#include <filesystem>
#include <optional>
#include <string>

#include "common/json.h"
#include "common/location.h"
#include "common/position.h"
#include "common/serialize.h"

namespace lsp
{

// wrapper of a std::string representing a document uri to improve semantics.
// Note that document_uri is really passed as a string over json - hence we
// serialize and deserialize this container as a std::string
//
// https://microsoft.github.io/language-server-protocol/specification#text-documents
struct document_uri
{
    document_uri(std::string uri = "")
    {
        raw_uri = uri;
    }

    bool operator==(const document_uri &o) const
    {
        return raw_uri == o.raw_uri;
    }

    bool operator<(const document_uri &o) const
    {
        return raw_uri < o.raw_uri;
    }

    void set_path(const std::filesystem::path& path)
    {
        raw_uri = path.string();
#if WIN32
        if (raw_uri[1] == ':') { // windows drive letters must always be 1 ch
            raw_uri[0] = std::tolower(raw_uri[0]);
            raw_uri.replace(raw_uri.begin() + 1,
                            raw_uri.begin() + 2, "%3A");
        }
        std::replace(raw_uri.begin(), raw_uri.end(), '\\', '/');
#endif

        // subset of reserved characters from the URI standard
        // http://www.ecma-international.org/ecma-262/6.0/#sec-uri-syntax-and-semantics
        std::string t;
        t.reserve(8 + raw_uri.size());
        // TODO: proper fix
#if WIN32
        t += "file:///";
#else
        t += "file://";
#endif

        for (char c: raw_uri)
            switch (c) {
            case ' ': t += "%20"; break;
            case '#': t += "%23"; break;
            case '$': t += "%24"; break;
            case '&': t += "%26"; break;
            case '(': t += "%28"; break;
            case ')': t += "%29"; break;
            case '+': t += "%2B"; break;
            case ',': t += "%2C"; break;
            case ';': t += "%3B"; break;
            case '?': t += "%3F"; break;
            case '@': t += "%40"; break;
            default:  t += c;     break;
            }

        raw_uri = std::move(t);
    }

    std::filesystem::path get_path() const
    {
        return get_string();
    }

    std::string get_string() const
    {
        if (raw_uri.compare(0, 7, "file://"))
            return raw_uri;

        std::string res;
        auto from_hex = [](unsigned char c) {
            return c - '0' < 10 ? c - '0' : (c | 32) - 'a' + 10;
        };
        for (
#if WIN32
        auto i = 8 // skipping the initial "/" on Windows
#else
        auto i = 7
#endif
            ; i < raw_uri.size(); i++) {
            if (i + 3 <= raw_uri.size() && raw_uri[i] == '%')
            {
                res.push_back(from_hex(raw_uri[i + 1]) * 16 +
                              from_hex(raw_uri[i + 2]));
                i += 2;
            }
            else
                res.push_back(raw_uri[i]);
        }
#if WIN32
        std::replace(res.begin(), res.end(), '\\', '/');
        if (res.size() > 1 && res[0] >= 'a' && res[0] <= 'z' && res[1] == ':')
            res[0] = toupper(res[0]);
#endif

        return res;
    }

    void set_uri(const std::string &uri)
    {
        raw_uri = uri;
    }

    std::string get_uri() const
    {
        return raw_uri;
    }

    private:
    std::string raw_uri;
};

void inline execute(serialize::json_reader& reader, lsp::document_uri& value)
{
    if (!reader.is_string())
        return;

    value.set_uri(reader.get_string());
}

void inline execute(serialize::json_writer& writer, lsp::document_uri& value)
{
    writer.string(value.get_uri());
}

// positions in a zero-based line and character offset document.
//
// https://microsoft.github.io/language-server-protocol/specification#position
struct position
{
    unsigned int line = 0;
    unsigned int character = 0;

    // explicitly define constructors and assignment operators as default. This
    // is needed because we overload constructors and assign operators later
    position() = default;
    position(const position&) = default;
    position(position&&) = default;

    position& operator=(const position&) = default;
    position& operator=(position&&) = default;
    ~position() = default;

    // initialise an lsp::position given a pair of line and character number
    position(unsigned l, unsigned c): line(l), character(c) {}

    // convert a common::position to its equivalent lsp::position object
    position(common::position position): line(position.line-1), character(position.column-1) {}

    // convert a common::position to its equivalent lsp::position object
    position& operator=(common::position position)
    {
        line = position.line-1;
        character = position.column-1;
        return *this;
    }

    bool operator==(const position &o) const
    {
        return line == o.line && character == o.character;
    }

    bool operator<(const position &o) const
    {
        return line != o.line ? line < o.line : character < o.character;
    }

    bool operator<=(const position &o) const
    {
        return line != o.line ? line < o.line : character <= o.character;
    }

    std::string to_string() const
    {
        return "";
    }
};

SERIALIZABLE_STRUCT_BEGIN(position)
SERIALIZABLE_STRUCT_MEMBER(line);
SERIALIZABLE_STRUCT_MEMBER(character);
SERIALIZABLE_STRUCT_END()

// range in a zero-based line and character offset document. A range consists of
// a start and an end position
//
// https://microsoft.github.io/language-server-protocol/specification#range
struct range
{
    position start;
    position end;

    // explicitly define constructors and assignment operators as default. This
    // is needed because we overload constructors and assign operators later
    range() = default;
    range(const range&) = default;
    range(range&&) = default;

    range& operator=(const range&) = default;
    range& operator=(range&&) = default;

    // initialise an lsp::range given a pair of start and end lsp::positions
    range(position s, position e): start(s), end(e)
    {
    }

    // initialise an lsp::range given a pair of start and end lsp::positions
    range(common::location location): start(location.begin), end(location.end)
    {
    }

    // convert a common::location to its equivalent lsp::range object
    range& operator=(common::location location)
    {
        start = location.begin;
        end = location.end;
        return *this;
    }

    bool operator==(const range &o) const
    {
        return start == o.start && end == o.end;
    }

    bool operator<(const range &o) const
    {
        return !(start == o.start) ? start < o.start : end < o.end;
    }

    bool includes(const range &o) const
    {
        return start <= o.start && o.end <= end;
    }

    bool intersects(const range &o) const
    {
        return start < o.end && o.start < end;
    }
};

SERIALIZABLE_STRUCT_BEGIN(range)
SERIALIZABLE_STRUCT_MEMBER(start)
SERIALIZABLE_STRUCT_MEMBER(end)
SERIALIZABLE_STRUCT_END()

// location in a zero-based line and character offset document
//
// https://microsoft.github.io/language-server-protocol/specification#location
struct location
{
    document_uri uri;
    lsp::range range;

    // explicitly define constructors and assignment operators as default. This
    // is needed because we overload constructors and assign operators later
    location() = default;
    location(const location&) = default;
    location(location&&) = default;

    location& operator=(const location&) = default;
    location& operator=(location&&) = default;

    // initialise an lsp::location given a document uri and an lsp::range
    location(document_uri u, lsp::range r): uri(u), range(r)
    {
    }

    // initialise an lsp::location given a pair of start and end lsp::positions
    location(common::location location): uri(""), range(location)
    {
    }

    // convert a common::location to its equivalent lsp::location object
    location& operator=(common::location location)
    {
        uri.set_path("");
        range = location;
        return *this;
    }

    bool operator==(const location &o) const
    {
        return uri == o.uri && range == o.range;
    }

    bool operator<(const location &o) const
    {
        return !(uri == o.uri) ? uri < o.uri : range < o.range;
    }
};

SERIALIZABLE_STRUCT_BEGIN(location)
SERIALIZABLE_STRUCT_MEMBER(uri)
SERIALIZABLE_STRUCT_MEMBER(range)
SERIALIZABLE_STRUCT_END()

// represent a link between a source and a target location
//
// https://microsoft.github.io/language-server-protocol/specification#locationlink
struct location_link {
  std::string target_uri;
  lsp::range target_range;
  lsp::range target_selection_range;

    explicit operator bool() const
    {
        return target_uri.size();
    }

    explicit operator location() &&
    {
        return {document_uri{std::move(target_uri)}, target_selection_range};
    }

    bool operator==(const location_link &o) const
    {
        return target_uri == o.target_uri &&
            target_selection_range == o.target_selection_range;
    }

    bool operator<(const location_link &o) const
    {
        if (target_uri == o.target_uri)
            return target_selection_range < o.target_selection_range;
        else
            return target_uri < o.target_uri;
    }
};

SERIALIZABLE_STRUCT_BEGIN(location_link)
SERIALIZABLE_STRUCT_MEMBER_RENAMED(target_uri, "targetUri")
SERIALIZABLE_STRUCT_MEMBER_RENAMED(target_range, "targetRange")
SERIALIZABLE_STRUCT_MEMBER_RENAMED(target_selection_range, "targetSelectionRange")
SERIALIZABLE_STRUCT_END()

// a related message and source code location for a diagnostic
struct diagnostic_related_information
{
    lsp::location location;
    std::string message;
};

SERIALIZABLE_STRUCT_BEGIN(diagnostic_related_information)
SERIALIZABLE_STRUCT_MEMBER(location)
SERIALIZABLE_STRUCT_MEMBER(message)
SERIALIZABLE_STRUCT_END()

// a diagnostic is either a compiler error, warning, information or hint.
//
// severity
// --------
//      0   N/A
//      1   error
//      2   warning
//      3   information
//      4   hint
//
// https://microsoft.github.io/language-server-protocol/specification#diagnostic

namespace diagnostic_severity
{

    constexpr int error = 1;
    constexpr int warning = 2;
    constexpr int information = 3;
    constexpr int hint = 4;
}

struct diagnostic
{
    lsp::range range;
    int severity = 0;
    int code = 0;
    std::optional<std::string> source;
    std::string message;
    std::vector<diagnostic_related_information> related_information;
};

SERIALIZABLE_STRUCT_BEGIN(diagnostic)
SERIALIZABLE_STRUCT_MEMBER_RENAMED(range, "range")
SERIALIZABLE_STRUCT_MEMBER_RENAMED(severity, "severity")
SERIALIZABLE_STRUCT_MEMBER_RENAMED(code, "code")
SERIALIZABLE_STRUCT_MEMBER_RENAMED(source, "source")
SERIALIZABLE_STRUCT_MEMBER_RENAMED(message, "message")
SERIALIZABLE_STRUCT_MEMBER_RENAMED(related_information, "relatedInformation")
SERIALIZABLE_STRUCT_END()

// represent a command
//
// https://microsoft.github.io/language-server-protocol/specification#command
struct command
{
    std::string title;
    std::string command;
    std::optional<json::string> arguments;
};

SERIALIZABLE_STRUCT_BEGIN(command)
SERIALIZABLE_STRUCT_MEMBER(title)
SERIALIZABLE_STRUCT_MEMBER(command)
SERIALIZABLE_STRUCT_MEMBER(arguments)
SERIALIZABLE_STRUCT_END()

// represent a textual edit
//
// https://microsoft.github.io/language-server-protocol/specification#textedit
struct text_edit
{
    lsp::range range;
    std::string new_text;
};

SERIALIZABLE_STRUCT_BEGIN(text_edit)
SERIALIZABLE_STRUCT_MEMBER_RENAMED(range, "range")
SERIALIZABLE_STRUCT_MEMBER_RENAMED(new_text, "newText")
SERIALIZABLE_STRUCT_END()

// identify a text document
//
// https://microsoft.github.io/language-server-protocol/specification#textdocumentidentifier
struct text_document_identifier
{
    document_uri uri;
};

SERIALIZABLE_STRUCT_BEGIN(text_document_identifier)
SERIALIZABLE_STRUCT_MEMBER(uri)
SERIALIZABLE_STRUCT_END()

// transfer a text document from the client to the server
//
// https://microsoft.github.io/language-server-protocol/specification#textdocumentitem
struct text_document_item
{
    document_uri uri;
    std::string language_id;
    int version;
    std::string text;
};

SERIALIZABLE_STRUCT_BEGIN(text_document_item)
SERIALIZABLE_STRUCT_MEMBER(uri)
SERIALIZABLE_STRUCT_MEMBER_RENAMED(language_id, "languageId")
SERIALIZABLE_STRUCT_MEMBER(version)
SERIALIZABLE_STRUCT_MEMBER(text)
SERIALIZABLE_STRUCT_END()

// identifier denoting a specific version of a text document
//
// https://microsoft.github.io/language-server-protocol/specification#versionedtextdocumentidentifier
struct versioned_text_document_identifier
{
    document_uri uri;
    json::nullable<int> version; // The version number of this documen
};

SERIALIZABLE_STRUCT_BEGIN(versioned_text_document_identifier)
SERIALIZABLE_STRUCT_MEMBER(uri)
SERIALIZABLE_STRUCT_MEMBER(version)
SERIALIZABLE_STRUCT_END()

// describe a textual change on a single text document
//
// https://microsoft.github.io/language-server-protocol/specification#textdocumentedit
struct text_document_edit
{
  versioned_text_document_identifier text_document;
  std::vector<text_edit> edits;
};

SERIALIZABLE_STRUCT_BEGIN(text_document_edit)
SERIALIZABLE_STRUCT_MEMBER_RENAMED(text_document, "textDocument")
SERIALIZABLE_STRUCT_MEMBER(edits)
SERIALIZABLE_STRUCT_END()

// workspace edit represents changes to many resources managed in the workspace.
// The edit provides a vector document_changes
//
// https://microsoft.github.io/language-server-protocol/specification#workspaceedit
struct workspace_edit
{
  std::vector<text_document_edit> document_changes;
};

SERIALIZABLE_STRUCT_BEGIN(workspace_edit)
SERIALIZABLE_STRUCT_MEMBER_RENAMED(document_changes, "documentChanges");
SERIALIZABLE_STRUCT_END()


// text document position represents a text document and a position inside that
// document and is used during goto declarations, goto definitions, or hover
// hover request among others
//
// https://microsoft.github.io//language-server-protocol/specification#textdocumentpositionparams
struct text_document_position_params
{
    text_document_identifier text_document;
    lsp::position position;
};

SERIALIZABLE_STRUCT_BEGIN(text_document_position_params)
SERIALIZABLE_STRUCT_MEMBER_RENAMED(text_document, "textDocument");
SERIALIZABLE_STRUCT_MEMBER(position);
SERIALIZABLE_STRUCT_END()


// denote a document through properties like language, scheme or pattern
//
// https://microsoft.github.io//language-server-protocol/specification#documentfilter
struct document_filter
{
    std::optional<std::string> language;
    std::optional<std::string> scheme;
    std::optional<std::string> pattern;
};

SERIALIZABLE_STRUCT_BEGIN(document_filter)
SERIALIZABLE_STRUCT_MEMBER(language)
SERIALIZABLE_STRUCT_MEMBER(scheme)
SERIALIZABLE_STRUCT_MEMBER(pattern)
SERIALIZABLE_STRUCT_END()


// markup_contents represent a string which can be represented in different
// formats. Currently plaintext and markdown are supported formats
//
// https://microsoft.github.io//language-server-protocol/specification#markupcontent
enum markup_kind { plaintext, markdown };

inline void execute(serialize::json_reader& reader, markup_kind& value)
{
    if (!reader.is_string())
        value = plaintext;
    else if (reader.get_string() == "markdown")
        value = markdown;
    else
        value = plaintext;
}

inline void execute(serialize::json_writer& writer, markup_kind& value)
{
    std::string result;
    if (value == markdown)
        result = "markdown";
    else
        result = "plaintext";
    execute(writer, result);
}

struct markup_content
{
    markup_kind kind = markup_kind::plaintext;
    std::string value;
};

SERIALIZABLE_STRUCT_BEGIN(markup_content)
SERIALIZABLE_STRUCT_MEMBER(kind)
SERIALIZABLE_STRUCT_MEMBER(value)
SERIALIZABLE_STRUCT_END()


enum message_type { error = 1, warning = 2, info = 3, log = 4 };
SERIALIZABLE_ENUM(message_type)

//
// Window
// ======

//
// window/showMessage
// ------------------
// ask the client to display a particular message in the user interface. This is
// used by the window/showMessage notification
//
// https://microsoft.github.io/language-server-protocol/specification#window_showMessage
struct show_message_params
{
    message_type type = message_type::log;
    std::string message;
};

SERIALIZABLE_STRUCT_BEGIN(show_message_params)
SERIALIZABLE_STRUCT_MEMBER(type)
SERIALIZABLE_STRUCT_MEMBER(message)
SERIALIZABLE_STRUCT_END()

//
// window/logMessage
// Log messages
//
// https://microsoft.github.io/language-server-protocol/specification#window_logMessage
struct log_message_params
{
  message_type type = message_type::log;
  std::string message;
};

SERIALIZABLE_STRUCT_BEGIN(log_message_params)
SERIALIZABLE_STRUCT_MEMBER(type)
SERIALIZABLE_STRUCT_MEMBER(message)
SERIALIZABLE_STRUCT_END()

struct workspace_folder
{
  document_uri uri;
  std::string name;
};

SERIALIZABLE_STRUCT_BEGIN(workspace_folder)
SERIALIZABLE_STRUCT_MEMBER(uri)
SERIALIZABLE_STRUCT_MEMBER(name)
SERIALIZABLE_STRUCT_END()

//
// textDocument/publishDiagnostics
// -------------------------------
// diagnostics notification are sent from the server to the client to signal
// results of validation runs
//
// https://microsoft.github.io/language-server-protocol/specification#textDocument_publishDiagnostics
struct publish_diagnostics_params
{
  document_uri uri;
  std::vector<diagnostic> diagnostics;
};

SERIALIZABLE_STRUCT_BEGIN(publish_diagnostics_params)
SERIALIZABLE_STRUCT_MEMBER(uri)
SERIALIZABLE_STRUCT_MEMBER(diagnostics)
SERIALIZABLE_STRUCT_END()

struct workdone_progress_create_params
{
    std::string token;
};

SERIALIZABLE_STRUCT_BEGIN(workdone_progress_create_params)
SERIALIZABLE_STRUCT_MEMBER(token)
SERIALIZABLE_STRUCT_END()

struct workdone_progress_begin_params
{
    std::string token;
    std::string title;
    std::optional<std::string> message;
    int percentage;
};

void inline execute(serialize::json_writer& writer, lsp::workdone_progress_begin_params& value)
{
    writer.start_object();
        writer.key("token"); writer.string(value.token);
        writer.key("value");
        writer.start_object();
            writer.key("kind"); writer.string("begin");
            writer.key("title"); writer.string(value.title);

            if (value.message) {
                writer.key("message");
                writer.string(*value.message);
            }
            writer.key("percentage");
            writer.integer(value.percentage == 0? 1 : value.percentage);
        writer.end_object();
    writer.end_object();
}

struct workdone_progress_report_params
{
    std::string token;
    std::optional<std::string> message;
    int percentage;
};

void inline execute(serialize::json_writer& writer, lsp::workdone_progress_report_params& value)
{
    writer.start_object();
        writer.key("token"); writer.string(value.token);
        writer.key("value");
        writer.start_object();
            writer.key("kind"); writer.string("report");

            if (value.message) {
                writer.key("message");
                writer.string(*value.message);
            }
            writer.key("percentage");
            writer.integer(value.percentage == 0? 1 : value.percentage);
        writer.end_object();
    writer.end_object();
}

struct workdone_progress_end_params
{
    std::string token;
    std::optional<std::string> message;
};

void inline execute(serialize::json_writer& writer, lsp::workdone_progress_end_params& value)
{
    writer.start_object();
        writer.key("token"); writer.string(value.token);
        writer.key("value");
        writer.start_object();
            writer.key("kind"); writer.string("end");

            if (value.message) {
                writer.key("message");
                writer.string(*value.message);
            }
        writer.end_object();
    writer.end_object();
}


struct text_document_did_open_save_close_params
{
    lsp::text_document_item text_document;
};

SERIALIZABLE_STRUCT_BEGIN(text_document_did_open_save_close_params)
SERIALIZABLE_STRUCT_MEMBER_RENAMED(text_document, "textDocument")
SERIALIZABLE_STRUCT_END()

struct text_document_hover_params
{
    lsp::text_document_item text_document;
    lsp::position position;
};

SERIALIZABLE_STRUCT_BEGIN(text_document_hover_params)
SERIALIZABLE_STRUCT_MEMBER_RENAMED(text_document, "textDocument")
SERIALIZABLE_STRUCT_MEMBER(position)
SERIALIZABLE_STRUCT_END()

struct folding_range_params
{
    lsp::text_document_item text_document;
    lsp::position position;
};

SERIALIZABLE_STRUCT_BEGIN(folding_range_params)
SERIALIZABLE_STRUCT_MEMBER_RENAMED(text_document, "textDocument")
SERIALIZABLE_STRUCT_MEMBER(position)
SERIALIZABLE_STRUCT_END()

struct document_symbols_params
{
    lsp::text_document_item text_document;
    lsp::position position;
};

SERIALIZABLE_STRUCT_BEGIN(document_symbols_params)
SERIALIZABLE_STRUCT_MEMBER_RENAMED(text_document, "textDocument")
SERIALIZABLE_STRUCT_MEMBER(position)
SERIALIZABLE_STRUCT_END()


}



#endif
