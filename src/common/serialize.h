#ifndef COMMON_SERIALIZE_H
#define COMMON_SERIALIZE_H

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "json.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

// directly copied from cquery

namespace serialize {

class json_reader
{
    rapidjson::GenericValue<rapidjson::UTF8<>>* reader_;

    public:
    json_reader(json::string& json);
    json_reader(rapidjson::GenericValue<rapidjson::UTF8<>>* reader);

    bool is_json()
    {
        return true;
    }

    bool is_bool()
    {
        return reader_->IsBool();
    }

    bool is_null()
    {
        return reader_->IsNull();
    }

    bool is_array()
    {
        return reader_->IsArray();
    }

    bool is_int()
    {
        return reader_->IsInt();
    }

    bool is_double()
    {
        return reader_->IsDouble();
    }

    bool is_string()
    {
        return reader_->IsString();
    }

    json::string get_json()
    {
        rapidjson::StringBuffer s;
        rapidjson::Writer<rapidjson::StringBuffer> writer(s);
        reader_->Accept(writer);
        return s.GetString();
    }

    void get_null()
    {
       
    }

    bool get_bool()
    {
        return reader_->GetBool();
    }

    int get_int()
    {
        return reader_->GetInt();
    }

    double get_double()
    {
        return reader_->GetDouble();
    }

    std::string get_string()
    {
        return reader_->GetString();
    }

    bool has_member(std::string x)
    {
        return reader_->HasMember(x);
    }

    std::unique_ptr<serialize::json_reader> operator[](std::string key)
    {
        auto& value = (*reader_)[key];
        return std::make_unique<serialize::json_reader>(&value);
    }

    void for_each(std::function<void(std::string, serialize::json_reader&)> fn)
    {
        if (!reader_->IsObject())
            throw std::invalid_argument("object");

        for (auto& entry : reader_->GetObject())
        {
            auto saved = reader_;
            auto key = entry.name.GetString();
            reader_ = &entry.value;
            fn(key, *this);
            reader_ = saved;
        }
    }

    void for_each(std::function<void(serialize::json_reader&)> fn)
    {
        if (!reader_->IsArray())
            throw std::invalid_argument("array");

        for (auto& entry : reader_->GetArray())
        {
            auto saved = reader_;
            reader_ = &entry;
            fn(*this);
            reader_ = saved;
        }
    }

    void for_each(const char* name, std::function<void(serialize::json_reader&)> fn)
    {
        auto it = reader_->FindMember(name);
        if (it != reader_->MemberEnd())
        {
            auto saved = reader_;
            reader_ = &it->value;
            fn(*this);
            reader_ = saved;
        }
    }
};

class json_writer
{
    rapidjson::Writer<rapidjson::StringBuffer>* writer_;

    public:
    json_writer(json::string& json);
    json_writer(rapidjson::Writer<rapidjson::StringBuffer>* writer);

    void json(json::string& x)
    {
        auto data = x->c_str();
        auto length = x->length();
        rapidjson::Type type;
        switch (x->at(0)) {
        case '{': type = rapidjson::kObjectType; break;
        case '"': type = rapidjson::kStringType; break;
        case '[': type = rapidjson::kArrayType;  break;
        case 'n':
        case 'N': type = rapidjson::kNullType;   break;
        case 't':
        case 'T': type = rapidjson::kTrueType;   break;
        case 'f':
        case 'F': type = rapidjson::kFalseType;  break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': type = rapidjson::kNumberType; break;
        default:  type = rapidjson::kNullType;   break;
        }
        writer_->RawValue(data, length, type);
    }

    void null()
    {
        writer_->Null();
    }

    void boolean(bool x)
    {
        writer_->Bool(x);
    }

    void integer(int x)
    {
        writer_->Int(x);
    }

    void decimal(double x)
    {
        writer_->Double(x);
    }

    void string(const std::string x)
    {
        writer_->String(x.c_str());
    }

    void string(const char* x, size_t len)
    {
        writer_->String(x, len);
    }

    void start_array(size_t)
    {
        writer_->StartArray();
    }

    void end_array()
    {
        writer_->EndArray();
    }

    void start_object()
    {
        writer_->StartObject();
    }

    void end_object()
    {
        writer_->EndObject();
    }

    void key(const char* name)
    {
        writer_->Key(name);
    }

    void key(const std::string name)
    {
        writer_->Key(name);
    }

};


//
// Elementary data types
//
// ----------------------------------------------------------------------------
// CPP                 JSON
// ----------------------------------------------------------------------------
// uint8_t          -> number
// short            -> number
// unsigned short   -> number
// int              -> number
// unsigned int     -> number
// long             -> number
// unsigned long    -> number
// long long        -> number
// unsigned longlong-> number
// double           -> number
// bool             -> boolean
// std::string      -> string
// std::string_view -> string
// std::vector<T>   -> array
// json::null       -> null
// cpp object       -> object
// std::optional<T> -> no json member element if optional is std::nullopt
// json::string     -> can represent anything
// json::nullable   -> represents the datatype: anything | null
// std::variant<A..>-> represents the datatype: A | B | C | ...
// ----------------------------------------------------------------------------
//
void execute(serialize::json_reader& reader, uint8_t& value);
void execute(serialize::json_writer& writer, uint8_t& value);

void execute(serialize::json_reader& reader, short& value);
void execute(serialize::json_writer& writer, short& value);

void execute(serialize::json_reader& reader, unsigned short& value);
void execute(serialize::json_writer& writer, unsigned short& value);

void execute(serialize::json_reader& reader, int& value);
void execute(serialize::json_writer& writer, int& value);

void execute(serialize::json_reader& reader, unsigned int& value);
void execute(serialize::json_writer& writer, unsigned int& value);

void execute(serialize::json_reader& reader, long& value);
void execute(serialize::json_writer& writer, long& value);

void execute(serialize::json_reader& reader, unsigned long& value);
void execute(serialize::json_writer& writer, unsigned long& value);

void execute(serialize::json_reader& reader, long long& value);
void execute(serialize::json_writer& writer, long long& value);

void execute(serialize::json_reader& reader, unsigned long long& value);
void execute(serialize::json_writer& writer, unsigned long long& value);

void execute(serialize::json_reader& reader, double& value);
void execute(serialize::json_writer& writer, double& value);

void execute(serialize::json_reader& reader, bool& value);
void execute(serialize::json_writer& writer, bool& value);

void execute(serialize::json_reader& reader, std::string& value);
void execute(serialize::json_writer& writer, std::string& value);

void execute(serialize::json_reader& reader, std::string_view& view);
void execute(serialize::json_writer& writer, std::string_view& view);

void execute(serialize::json_reader& reader, json::null& value);
void execute(serialize::json_writer& writer, json::null& value);

void execute(serialize::json_reader& reader, json::string& value);
void execute(serialize::json_writer& writer, json::string& value);

// native cpp class types includes the std::optional and the std::vector
template <typename T>
void execute(serialize::json_reader& reader, std::optional<T>& value)
{
    T real_value;
    execute(reader, real_value);
    value = std::make_optional<T>(real_value);
}

template <typename T>
void execute(serialize::json_writer& writer, std::optional<T>& value)
{
    if (value)
        execute(writer, *value);
    else
        writer.null();
}

// std::unordered_map
template <typename T>
void execute(serialize::json_reader& reader, std::unordered_map<std::string, T>& values)
{
    reader.for_each([&] (std::string key, serialize::json_reader& entry)
    {
        T entry_value;
        execute(entry, entry_value);
        values[key] = std::move(entry_value);
    });
}
template <typename T>
void execute(serialize::json_writer& writer, std::unordered_map<std::string, T>& values)
{
    writer.start_object();
    for (auto& it : values)
    {
        writer.key(it.first);
        execute(writer, it.second);
    }
    writer.end_object();
}

// std::map
template <typename T>
void execute(serialize::json_reader& reader, std::map<std::string, T>& values)
{
    reader.for_each([&] (std::string key, serialize::json_reader& entry)
    {
        T entry_value;
        execute(entry, entry_value);
        values[key] = std::move(entry_value);
    });
}
template <typename T>
void execute(serialize::json_writer& writer, std::map<std::string, T>& values)
{
    writer.start_object();
    for (auto& it : values)
    {
        writer.key(it.first);
        execute(writer, it.second);
    }
    writer.end_object();
}

// std::vector
template <typename T>
void execute(serialize::json_reader& reader, std::vector<T>& values)
{
    reader.for_each([&] (serialize::json_reader& entry)
    {
        T entry_value;
        execute(entry, entry_value);
        values.push_back(std::move(entry_value));
    });
}
template <typename T>
void execute(serialize::json_writer& writer, std::vector<T>& values)
{
    writer.start_array(values.size());
    for (auto& value : values)
        execute(writer, value);
    writer.end_array();
}

// std::list
template <typename T>
void execute(serialize::json_reader& reader, std::list<T>& values)
{
    reader.for_each([&] (serialize::json_reader& entry)
    {
        T entry_value;
        execute(entry, entry_value);
        values.push_back(std::move(entry_value));
    });
}
template <typename T>
void execute(serialize::json_writer& writer, std::list<T>& values)
{
    writer.start_array(values.size());
    for (auto& value : values)
        execute(writer, value);
    writer.end_array();
}

// json::nullable
template <typename T>
void execute(serialize::json_reader& reader, json::nullable<T>& value)
{
    if (reader.is_null())
    {
        reader.get_null();
        value = json::null_value;
        return;
    }
    T real_value;
    execute(reader, real_value);
    value = std::move(real_value);
}

template <typename T>
void execute(serialize::json_writer& writer, json::nullable<T>& value)
{
    if (!value.is_null())
        execute(writer, *value);
    else
        writer.null();
}

template <typename T>
static bool __common_serializable_struct_begin(serialize::json_reader&, T&)
{
    return false;
}

template <typename T>
static bool __common_serializable_struct_begin(serialize::json_writer& writer, T&)
{
    writer.start_object();
    return true;
}

template <typename T>
void __common_serializable_member(serialize::json_writer& writer, const char* name, std::optional<T>& value)
{
    if (value.has_value())
    {
        writer.key(name);
        execute(writer, value);
    }
}

template <typename T>
void __common_serializable_member(serialize::json_reader& reader, const char* name, T& value)
{
    reader.for_each(name, [&] (serialize::json_reader& child)
    {
        execute(child, value);
    });
}

template <typename T>
void __common_serializable_member(serialize::json_writer& writer, const char* name, T& value)
{
    writer.key(name);
    execute(writer, value);
}

template <typename T>
void __common_serializable_member_end(serialize::json_reader&, T&)
{

}

template <typename T>
void __common_serializable_member_end(serialize::json_writer& writer, T&)
{
    writer.end_object();
}

//
// This is the serialize.h api
// ---------------------------
//
// We provide the ability to serialize structs and enums and other data types to
// and from json
//
// Consider the following structure in cpp;
//
// ```cpp
//  struct A
//  {
//      int member1;
//      std::string member2;
//  };
// ```
//
// We expose the data structure (in this example it is struct A and its members)
// to the serialize api by doing:
//
// ```cpp
//  SERIALIZABLE_STRUCT_BEGIN(A);
//  SERIALIZABLE_STRUCT_MEMBER(member1);
//  SERIALIZABLE_STRUCT_MEMBER_RENAMED(member2, "renamed_member2");
//  SERIALIZABLE_STRUCT_END();
// ```
//
// We are then able to serialize instantiations of sturct A into json by using
// the serialize::to_json<>() template:
//
// ```cpp
//  A inst0{.member1 = 123, .member2 = "hello world"};
//  serialized_json = serialize::to_json(inst0);
// ```
//
// The serialize::to_json<>() template outputs the following json:
//  {"member1":123, "renamed_member2":"hello world"}
//
// Notice how 'member2' (in cpp) is serialized as 'renamed_member2' (in json).
// Member renaming is fully supported during serialization as well as during
// deserialization.
//
// Finally we can deserialize json into inst1. inst1 will take values (int) 123
// and (std::string) "hello world" as expected
//
// ```cpp
//  A inst1;
//  inst1 = serialize::from_json(serialized_json);
// ```

//
// Serialize a struct data type
//
#define SERIALIZABLE_STRUCT_BEGIN(type)     \
    template <typename T>                   \
    void execute(T& json, type& value)      \
    {                                       \
        __common_serializable_struct_begin(json, value);

#define SERIALIZABLE_STRUCT_MEMBER(member) __common_serializable_member(json, #member, value.member);
#define SERIALIZABLE_STRUCT_MEMBER_RENAMED(member, rename) __common_serializable_member(json, rename, value.member);
#define SERIALIZABLE_STRUCT_END() __common_serializable_member_end(json, value); }

//
// Serialize an enum
//
#define SERIALIZABLE_ENUM(name)                                             \
    inline void execute(serialize::json_reader& reader, name& value)        \
    {                                                                       \
        std::underlying_type<name>::type value0;                            \
        execute(reader, value0);                                            \
        value = static_cast<name>(value0);                                  \
    }                                                                       \
    inline void execute(serialize::json_writer& writer, name& value)        \
    {                                                                       \
        auto value0 = static_cast<std::underlying_type<name>::type>(value); \
        execute(writer, value0);                                            \
    }

//
// Convert a json::string into a cpp data type
//
template <typename T>
inline T from_json(const json::string& json)
{
    T object;

    rapidjson::Document r;
    r.Parse(json.str.c_str());
    serialize::json_reader reader{&r};
    execute(reader, object);

    return std::move(object);
}

//
// Convert a cpp data type into a json::string
//
template <typename T>
inline json::string to_json(T& object)
{
    rapidjson::StringBuffer output;
    rapidjson::Writer<rapidjson::StringBuffer> w(output);
    serialize::json_writer writer(&w);
    execute(writer, object);

    return json::string(output.GetString());
}

}

#endif
