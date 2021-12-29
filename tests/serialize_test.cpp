
#include <catch2/catch.hpp>
#include <optional>
#include "common/serialize.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace rapidjson;

struct simple_struct
{
    std::string text;
    int integer;
    bool boolean;
    double decimal;
    json::null nullable;
    std::vector<std::string> array;
};

SERIALIZABLE_STRUCT_BEGIN(simple_struct)
SERIALIZABLE_STRUCT_MEMBER(text)
SERIALIZABLE_STRUCT_MEMBER(integer)
SERIALIZABLE_STRUCT_MEMBER(boolean)
SERIALIZABLE_STRUCT_MEMBER(decimal)
SERIALIZABLE_STRUCT_MEMBER(nullable)
SERIALIZABLE_STRUCT_MEMBER(array)
SERIALIZABLE_STRUCT_END()

TEST_CASE("simple structure test", "[serialize]")
{
    rapidjson::StringBuffer output;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(output);
    writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
    serialize::json_writer to_json(&writer);
    simple_struct first;
    first.text = "helloworldddd";
    first.integer = 123;
    first.boolean = true;
    first.decimal = 1.5;
    first.array = {"abc", "def", "ghi", "jkl"};

    execute(to_json, first);

    std::string parsed = output.GetString();

    rapidjson::Document reader;
    reader.Parse(parsed.c_str());
    serialize::json_reader from_json{&reader};

    simple_struct second;
    execute(from_json, second);

    REQUIRE(first.text == second.text);
    REQUIRE(first.integer == second.integer);
    REQUIRE(first.boolean == second.boolean);
    REQUIRE(first.decimal == second.decimal);
    REQUIRE(first.nullable == second.nullable);
    REQUIRE(first.array == second.array);
    REQUIRE(first.array.size() == second.array.size());
}
TEST_CASE("use the json reader/writer interface", "[serialize]")
{
    simple_struct first;
    first.text = "helloworldddd";
    first.integer = 123;
    first.boolean = true;
    first.decimal = 1.5;
    first.array = {"abc", "def", "ghi", "jkl"};

    json::string parsed = serialize::to_json<simple_struct>(first);
    simple_struct second;
    second = serialize::from_json<simple_struct>(parsed);

    REQUIRE(first.text == second.text);
    REQUIRE(first.integer == second.integer);
    REQUIRE(first.boolean == second.boolean);
    REQUIRE(first.decimal == second.decimal);
    REQUIRE(first.nullable == second.nullable);
    REQUIRE(first.array == second.array);
    REQUIRE(first.array.size() == second.array.size());
}

struct nestable
{
    std::vector<nestable> nest;
    std::string name;

    friend bool operator==(nestable const& lhs, nestable const& rhs)
    {
        return lhs.name == rhs.name and lhs.nest == rhs.nest;
    }
};

SERIALIZABLE_STRUCT_BEGIN(nestable)
SERIALIZABLE_STRUCT_MEMBER(nest)
SERIALIZABLE_STRUCT_MEMBER(name)
SERIALIZABLE_STRUCT_END()

TEST_CASE("nestable test", "[serialize]")
{
    rapidjson::StringBuffer output;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(output);
    writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
    serialize::json_writer to_json(&writer);
    nestable first;
    first.nest = { {{{{},"grandkid"}},"one"}, {{},"two"}, {{},"three"}, {{{{},"grandkid2"}},"four"} };
    first.name = "big dad";

    execute(to_json, first);

    std::string parsed = output.GetString();

    rapidjson::Document reader;
    reader.Parse(parsed.c_str());
    serialize::json_reader from_json{&reader};

    nestable second;
    execute(from_json, second);

    REQUIRE(first == second);
    REQUIRE(first.name == second.name);
    REQUIRE(first.nest.size() == second.nest.size());
}

enum number { zero = 0, waan = 1, toou = 2, tree = 3, foar = 4000 };
SERIALIZABLE_ENUM(number)

struct identifier
{
    int id;
    number num = number::zero;
};

SERIALIZABLE_STRUCT_BEGIN(identifier)
SERIALIZABLE_STRUCT_MEMBER(id)
SERIALIZABLE_STRUCT_MEMBER(num)
SERIALIZABLE_STRUCT_END()

TEST_CASE("enum test", "[serialize]")
{
    rapidjson::StringBuffer output;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(output);
    writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
    serialize::json_writer to_json(&writer);
    identifier first = {1,number::foar};

    execute(to_json, first);

    std::string parsed = output.GetString();

    rapidjson::Document reader;
    reader.Parse(parsed.c_str());
    serialize::json_reader from_json{&reader};

    identifier second;
    execute(from_json, second);

    REQUIRE(first.id == second.id);
    REQUIRE(first.num == second.num);
}

struct arrayable_struct
{
    std::string name = "1AHWD !5";
    double decimal = 123.5;

    friend bool operator==(arrayable_struct const& lhs, arrayable_struct const& rhs)
    {
        return lhs.name == rhs.name and lhs.decimal == rhs.decimal;
    }
};

SERIALIZABLE_STRUCT_BEGIN(arrayable_struct)
SERIALIZABLE_STRUCT_MEMBER(name)
SERIALIZABLE_STRUCT_MEMBER(decimal)
SERIALIZABLE_STRUCT_END()

struct container_of_arrayable_struct
{
    std::vector<arrayable_struct> array;

    friend bool operator==(container_of_arrayable_struct const& lhs, container_of_arrayable_struct const& rhs)
    {
        return lhs.array == rhs.array;
    }
};

SERIALIZABLE_STRUCT_BEGIN(container_of_arrayable_struct)
SERIALIZABLE_STRUCT_MEMBER(array)
SERIALIZABLE_STRUCT_END()

TEST_CASE("arrayable test", "[serialize]")
{
    rapidjson::StringBuffer output;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(output);
    writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
    serialize::json_writer to_json(&writer);
    container_of_arrayable_struct first;
    first.array = {{"n", 1.5}, {"o", 2.5}};

    execute(to_json, first);

    std::string parsed = output.GetString();

    rapidjson::Document reader;
    reader.Parse(parsed.c_str());
    serialize::json_reader from_json{&reader};

    container_of_arrayable_struct second;
    execute(from_json, second);

    REQUIRE(first == second);
}

TEST_CASE("direct serialisation of array of arrayable_struct", "[serialize]")
{
    rapidjson::StringBuffer output;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(output);
    writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
    serialize::json_writer to_json(&writer);
    std::vector<arrayable_struct> first;
    first.push_back({"n", 1.5});
    first.push_back({"n", 2.5});
    first.push_back({"n", 1});
    first.push_back({"n", 1.6});

    execute(to_json, first);

    std::string parsed = output.GetString();

    rapidjson::Document reader;
    reader.Parse(parsed.c_str());
    serialize::json_reader from_json{&reader};

    std::vector<arrayable_struct> second;
    execute(from_json, second);

    REQUIRE(first == second);
    REQUIRE(first.size() == second.size());
}

struct struct_with_optional_members
{
    std::optional<std::string> id = std::nullopt;
    std::string name = "HELLO";
};

SERIALIZABLE_STRUCT_BEGIN(struct_with_optional_members)
SERIALIZABLE_STRUCT_MEMBER(id)
SERIALIZABLE_STRUCT_MEMBER(name)
SERIALIZABLE_STRUCT_END()

TEST_CASE("optional test", "[serialize]")
{
    rapidjson::StringBuffer output;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(output);
    writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
    serialize::json_writer to_json(&writer);
    struct_with_optional_members first;
    first.id = "identifier";
    first.name = "yo";

    execute(to_json, first);

    std::string parsed = output.GetString();

    rapidjson::Document reader;
    reader.Parse(parsed.c_str());
    serialize::json_reader from_json{&reader};

    struct_with_optional_members second;
    execute(from_json, second);

    REQUIRE(reader.HasMember("id"));
    REQUIRE(first.id == second.id);
    REQUIRE(first.name == second.name);
}

TEST_CASE("optional test. when nullopt, dont even output it to json", "[serialize]")
{
    rapidjson::StringBuffer output;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(output);
    writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
    serialize::json_writer to_json(&writer);
    struct_with_optional_members first;
    first.name = "yo";

    execute(to_json, first);

    std::string parsed = output.GetString();

    rapidjson::Document reader;
    reader.Parse(parsed.c_str());
    serialize::json_reader from_json{&reader};

    struct_with_optional_members second;
    execute(from_json, second);

    REQUIRE(!reader.HasMember("id"));
    REQUIRE(first.id == second.id);
    REQUIRE(first.name == second.name);
}

struct serialize_json
{
    int id;
    json::string params;
};

SERIALIZABLE_STRUCT_BEGIN(serialize_json)
SERIALIZABLE_STRUCT_MEMBER(id)
SERIALIZABLE_STRUCT_MEMBER(params)
SERIALIZABLE_STRUCT_END()

TEST_CASE("serdes json", "[serialize]")
{
    rapidjson::StringBuffer output;
    rapidjson::Writer<rapidjson::StringBuffer> writer(output);
    serialize::json_writer to_json(&writer);
    serialize_json first;
    first.id = 123;
    first.params = "{\"hello\":\"world\",\"num\":1}";

    execute(to_json, first);

    std::string parsed = output.GetString();

    rapidjson::Document reader;
    reader.Parse(parsed.c_str());
    serialize::json_reader from_json{&reader};

    serialize_json second;
    execute(from_json, second);

    REQUIRE(first.id == second.id);
    REQUIRE(first.params == second.params);
}

//
// Can this be done:
//
// serialize_nullable {
//     id: int | null
// }
//
struct serialize_nullable
{
    json::nullable<int> id;
};

SERIALIZABLE_STRUCT_BEGIN(serialize_nullable)
SERIALIZABLE_STRUCT_MEMBER(id)
SERIALIZABLE_STRUCT_END()

SCENARIO("json::nullable", "[serialize]")
{
    GIVEN("serialize_nullable")
    {
        rapidjson::StringBuffer output;
        rapidjson::Writer<rapidjson::StringBuffer> writer(output);
        serialize::json_writer to_json(&writer);

        rapidjson::Document reader;
        serialize::json_reader from_json{&reader};

        WHEN("serialize_nullable id is not null")
        {
            serialize_nullable first;
            first.id = 123;

            execute(to_json, first);

            std::string parsed = output.GetString();

            THEN("the deserialised struct should contain a value")
            {
                reader.Parse(parsed.c_str());

                serialize_nullable second;
                execute(from_json, second);

                REQUIRE(!first.id.is_null());
                REQUIRE(!second.id.is_null());
                REQUIRE(reader.HasMember("id"));
                REQUIRE(reader["id"].IsInt());
                REQUIRE(first.id == second.id);
            }
        }

        WHEN("serialize_nullable id is null")
        {
            serialize_nullable first;
            first.id = json::null_value;
            execute(to_json, first);

            std::string parsed = output.GetString();

            THEN("the deserialized struct should contain a null")
            {
                reader.Parse(parsed.c_str());

                serialize_nullable second;
                execute(from_json, second);

                REQUIRE(reader.HasMember("id"));
                REQUIRE(reader["id"].IsNull());
                REQUIRE(first.id == second.id);
                REQUIRE(first.id.is_null());
                REQUIRE(second.id.is_null());
            }
        }

        WHEN("serialize_nullable id not equal?")
        {
            serialize_nullable first;
            first.id = 123;
            serialize_nullable second;
            second.id = 4;

            THEN("we expect that")
            {
                REQUIRE(first.id != second.id);
                REQUIRE(!first.id.is_null());
                REQUIRE(!second.id.is_null());
            }
        }

        WHEN("serialize_nullable id definitely not equal?")
        {
            serialize_nullable first;
            first.id = 123;
            serialize_nullable second;
            second.id = json::null_value;

            THEN("expect that to happen")
            {
                REQUIRE(first.id != second.id);
                REQUIRE(!first.id.is_null());
                REQUIRE(second.id.is_null());
            }
        }

        WHEN("serialize_nullable id is sorta equal!!?")
        {
            serialize_nullable first;
            first.id = json::null_value;
            serialize_nullable second;
            second.id = json::null_value;

            THEN("expect that to happen")
            {
                REQUIRE(first.id == second.id);
                REQUIRE(first.id.is_null());
                REQUIRE(second.id.is_null());
            }
        }
    }
}

//
// Can this be done:
//
// serialize_optional_nullable {
//     id?: int | null
// }
//
struct serialize_optional_nullable
{
    std::optional<json::nullable<int>> id;
};

SERIALIZABLE_STRUCT_BEGIN(serialize_optional_nullable)
SERIALIZABLE_STRUCT_MEMBER(id)
SERIALIZABLE_STRUCT_END()

SCENARIO("optional json::nullable", "[serialize]")
{
    GIVEN("serialize_optional_nullable")
    {
        rapidjson::StringBuffer output;
        rapidjson::Writer<rapidjson::StringBuffer> writer(output);
        serialize::json_writer to_json(&writer);

        rapidjson::Document reader;
        serialize::json_reader from_json{&reader};

        WHEN("serialize_optional_nullable id is not defined (nullopt)")
        {
            serialize_optional_nullable first;
            first.id = std::nullopt;

            execute(to_json, first);

            std::string parsed = output.GetString();

            THEN("the deserialised struct should not even contain an id")
            {
                reader.Parse(parsed.c_str());

                serialize_optional_nullable second;
                execute(from_json, second);

                REQUIRE(first.id == std::nullopt);
                REQUIRE(second.id == std::nullopt);

                REQUIRE(!first.id.has_value());
                REQUIRE(!second.id.has_value());

                REQUIRE(first.id == second.id);
            }
        }

        WHEN("serialize_optional_nullable id is defined and is null (json::null")
        {
            serialize_optional_nullable first;
            first.id = std::optional<json::nullable<int>>(json::null_value);

            execute(to_json, first);

            std::string parsed = output.GetString();

            THEN("the deserialised struct should contain an id = null")
            {
                reader.Parse(parsed.c_str());

                serialize_optional_nullable second;
                execute(from_json, second);

                // We are comparing: json::nullable<int> to json::null
                REQUIRE(first.id.value() == json::null_value);
                REQUIRE(second.id.value() == json::null_value);

                // Returns int, but since json::nullable is null, it throws an
                // exception
                REQUIRE_THROWS(first.id->value());
                REQUIRE_THROWS(second.id->value());

                REQUIRE(first.id.has_value());
                REQUIRE(second.id.has_value());

                REQUIRE(first.id.value().is_null());
                REQUIRE(second.id.value().is_null());

                REQUIRE(first.id == second.id);
            }
        }

        WHEN("serialize_optional_nullable id is defined and is not null")
        {
            serialize_optional_nullable first;
            first.id = std::optional<json::nullable<int>>(13);

            execute(to_json, first);

            std::string parsed = output.GetString();

            THEN("the deserialised struct should not even contain an id")
            {
                reader.Parse(parsed.c_str());

                serialize_optional_nullable second;
                execute(from_json, second);

                // We are comparing: json::nullable<int> to int
                REQUIRE(first.id.value() == 13);
                REQUIRE(second.id.value() == 13);

                // We are comparing: int to int
                REQUIRE(first.id->value() == 13);
                REQUIRE(second.id->value() == 13);

                REQUIRE(first.id.has_value());
                REQUIRE(second.id.has_value());

                REQUIRE(!first.id.value().is_null());
                REQUIRE(!second.id.value().is_null());

                REQUIRE(first.id == second.id);
            }
        }
    }
}

struct response
{
    int id;
};

SERIALIZABLE_STRUCT_BEGIN(response)
SERIALIZABLE_STRUCT_MEMBER_RENAMED(id, "IDENT")
SERIALIZABLE_STRUCT_END()

struct struct_under_test
{
    int value = -1;
    std::string str = "14";
    double awd = 123.15;
    std::optional<response> resp;
    std::optional<response> not_defined_resp;
    json::nullable<json::string> jsn;
    json::nullable<json::string> jsn2;
};

SERIALIZABLE_STRUCT_BEGIN(struct_under_test)
SERIALIZABLE_STRUCT_MEMBER_RENAMED(value, "value_is_been_renamed")
SERIALIZABLE_STRUCT_MEMBER_RENAMED(str, "string_renamed")
SERIALIZABLE_STRUCT_MEMBER_RENAMED(awd, "doubs")
SERIALIZABLE_STRUCT_MEMBER_RENAMED(resp, "defined")
SERIALIZABLE_STRUCT_MEMBER_RENAMED(not_defined_resp, "undefined")
SERIALIZABLE_STRUCT_MEMBER_RENAMED(jsn, "nullable")
SERIALIZABLE_STRUCT_MEMBER_RENAMED(jsn2, "with_content")
SERIALIZABLE_STRUCT_END()

SCENARIO("Using the json serializer renamer", "[serialize]")
{
    GIVEN("A struct")
    {
        rapidjson::StringBuffer output;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(output);
        writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
        serialize::json_writer to_json(&writer);

        WHEN("serializing and deserializing")
        {
            response ddd{1};
            struct_under_test first;
            first.value = 10123;
            first.str = "helloworldddd";
            first.awd = 0.01923;
            first.resp = ddd;
            first.jsn = json::null_value;
            first.jsn2 = "{}";

            execute(to_json, first);

            std::string parsed = output.GetString();
            INFO(parsed);

            rapidjson::Document reader;
            reader.Parse(parsed.c_str());
            serialize::json_reader from_json{&reader};

            struct_under_test second;
            execute(from_json, second);

            THEN("we ensure that the deserialized struct contains the correct data")
            {
                REQUIRE(second.value == 10123);
                REQUIRE(second.str == "helloworldddd");
                REQUIRE(second.awd == 0.01923);
                REQUIRE(first.resp.has_value());
            }

            THEN("there should be no not_defined_resp in the json")
            {
                REQUIRE(!first.not_defined_resp.has_value());
                REQUIRE(!second.not_defined_resp.has_value());
            }

            THEN("check that renamed values exists")
            {
                REQUIRE(!reader.HasMember("value"));
                REQUIRE(reader.HasMember("value_is_been_renamed"));
                REQUIRE(!reader.HasMember("str"));
                REQUIRE(reader.HasMember("string_renamed"));
                REQUIRE(reader.HasMember("doubs"));
                REQUIRE(reader.HasMember("defined"));
                REQUIRE(!reader["defined"].HasMember("id"));
                REQUIRE(reader["defined"].HasMember("IDENT"));
                REQUIRE(reader["defined"]["IDENT"].GetInt() == 1);
                REQUIRE(second.resp->id == 1);

                REQUIRE(!reader.HasMember("undefined"));

                REQUIRE(reader.HasMember("nullable"));
                REQUIRE(reader["nullable"].IsNull());
                REQUIRE(second.jsn == json::null_value);

                REQUIRE(reader.HasMember("with_content"));
                REQUIRE(!reader["with_content"].IsNull());
                REQUIRE(second.jsn2 == json::string("{}"));
            }
        }
    }
}

struct aloha
{
    std::string xd;

    friend bool operator==(aloha const& lhs, aloha const& rhs)
    {
        return lhs.xd == rhs.xd;
    }
};

SERIALIZABLE_STRUCT_BEGIN(aloha)
SERIALIZABLE_STRUCT_MEMBER(xd)
SERIALIZABLE_STRUCT_END()

struct maptest
{
    std::map<std::string, aloha> children;
    std::string name;

    friend bool operator==(maptest const& lhs, maptest const& rhs)
    {
        return lhs.name == rhs.name and lhs.children == rhs.children;
    }
};

SERIALIZABLE_STRUCT_BEGIN(maptest)
SERIALIZABLE_STRUCT_MEMBER(children)
SERIALIZABLE_STRUCT_MEMBER(name)
SERIALIZABLE_STRUCT_END()

TEST_CASE("mapable alohaas", "[serialize]")
{
    rapidjson::StringBuffer output;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(output);
    writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
    serialize::json_writer to_json(&writer);
    maptest first;
    first.name = "big dad";
    first.children["A"] = { "a" };
    first.children["B"] = { "b" };
    first.children["C"] = { "c" };
    first.children["D"] = { "d" };
    first.children["E"] = { "e" };

    execute(to_json, first);

    std::string parsed = output.GetString();

    rapidjson::Document reader;
    reader.Parse(parsed.c_str());
    serialize::json_reader from_json{&reader};

    maptest second;
    execute(from_json, second);

    REQUIRE(first == second);
}

struct unorderedly_maptest
{
    std::unordered_map<std::string, aloha> children;
    std::string name;

    friend bool operator==(unorderedly_maptest const& lhs, unorderedly_maptest const& rhs)
    {
        return lhs.name == rhs.name and lhs.children == rhs.children;
    }
};

SERIALIZABLE_STRUCT_BEGIN(unorderedly_maptest)
SERIALIZABLE_STRUCT_MEMBER(children)
SERIALIZABLE_STRUCT_MEMBER(name)
SERIALIZABLE_STRUCT_END()

TEST_CASE("unorderedly mapable alohaas", "[serialize]")
{
    rapidjson::StringBuffer output;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(output);
    writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
    serialize::json_writer to_json(&writer);
    unorderedly_maptest first;
    first.name = "big dad";
    first.children["F"] = { "f" };
    first.children["G"] = { "g" };
    first.children["H"] = { "h" };
    first.children["I"] = { "i" };
    first.children["J"] = { "j" };

    execute(to_json, first);

    std::string parsed = output.GetString();

    rapidjson::Document reader;
    reader.Parse(parsed.c_str());
    serialize::json_reader from_json{&reader};

    unorderedly_maptest second;
    execute(from_json, second);

    REQUIRE(first == second);
}

struct listtest
{
    std::list<std::string> abcd_list;

    friend bool operator==(listtest const& lhs, listtest const& rhs)
    {
        return lhs.abcd_list == rhs.abcd_list;
    }
};

SERIALIZABLE_STRUCT_BEGIN(listtest)
SERIALIZABLE_STRUCT_MEMBER(abcd_list)
SERIALIZABLE_STRUCT_END()

TEST_CASE("listable texts", "[serialize]")
{
    rapidjson::StringBuffer output;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(output);
    writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
    serialize::json_writer to_json(&writer);
    listtest first;
    first.abcd_list.push_back("Aaà");
    first.abcd_list.push_back("Bbb");
    first.abcd_list.push_back("Ccç");
    first.abcd_list.push_front("Ddd");
    first.abcd_list.push_back("Eeè");

    execute(to_json, first);

    std::string parsed = output.GetString();

    rapidjson::Document reader;
    reader.Parse(parsed.c_str());
    serialize::json_reader from_json{&reader};

    listtest second;
    execute(from_json, second);

    REQUIRE(first == second);
}
