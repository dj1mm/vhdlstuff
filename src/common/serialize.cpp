
#include "serialize.h"

// serialize
// Elementary types

serialize::json_reader::json_reader(json::string& json)
{
    auto doc = new rapidjson::Document();
    doc->Parse(json->c_str());
    reader_ = doc;
}

serialize::json_reader::json_reader(rapidjson::GenericValue<rapidjson::UTF8<>>* reader) : reader_(reader)
{

}

serialize::json_writer::json_writer(json::string& json)
{
    rapidjson::StringBuffer output;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(output);
    writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
    writer_ = &writer;
}

serialize::json_writer::json_writer(rapidjson::Writer<rapidjson::StringBuffer>* writer) : writer_(writer)
{

}

void serialize::execute(serialize::json_reader& reader, uint8_t& value)
{
    if (!reader.is_int())
        throw std::invalid_argument("uint8_t");
    value = (uint8_t) reader.get_int();
}

void serialize::execute(serialize::json_writer& writer, uint8_t& value)
{
    writer.integer(value);
}

void serialize::execute(serialize::json_reader& reader, short& value)
{
    if (!reader.is_int())
        throw std::invalid_argument("short");
    value = (short) reader.get_int();
}

void serialize::execute(serialize::json_writer& writer, short& value)
{
    writer.integer(value);
}

void serialize::execute(serialize::json_reader& reader, unsigned short& value)
{
    if (!reader.is_int())
        throw std::invalid_argument("unsigned short");
    value = (unsigned short) reader.get_int();
}

void serialize::execute(serialize::json_writer& writer, unsigned short& value)
{
    writer.integer(value);
}

void serialize::execute(serialize::json_reader& reader, int& value)
{
    if (!reader.is_int())
        throw std::invalid_argument("int");
    value = reader.get_int();
}

void serialize::execute(serialize::json_writer& writer, int& value)
{
    writer.integer(value);
}

void serialize::execute(serialize::json_reader& reader, unsigned int& value)
{
    if (!reader.is_int())
        throw std::invalid_argument("int");
    value = reader.get_int();
}

void serialize::execute(serialize::json_writer& writer, unsigned int& value)
{
    writer.integer(value);
}

void serialize::execute(serialize::json_reader& reader, long& value)
{
    if (!reader.is_int())
        throw std::invalid_argument("long");
    value = reader.get_int();
}

void serialize::execute(serialize::json_writer& writer, long& value)
{
    writer.integer(value);
}

void serialize::execute(serialize::json_reader& reader, unsigned long& value)
{
    if (!reader.is_int())
        throw std::invalid_argument("long");
    value = reader.get_int();
}

void serialize::execute(serialize::json_writer& writer, unsigned long& value)
{
    writer.integer(value);
}

void serialize::execute(serialize::json_reader& reader, long long& value)
{
    if (!reader.is_int())
        throw std::invalid_argument("long");
    value = reader.get_int();
}

void serialize::execute(serialize::json_writer& writer, long long& value)
{
    writer.integer(value);
}

void serialize::execute(serialize::json_reader& reader, unsigned long long& value)
{
    if (!reader.is_int())
        throw std::invalid_argument("long");
    value = reader.get_int();
}

void serialize::execute(serialize::json_writer& writer, unsigned long long& value)
{
    writer.integer(value);
}

void serialize::execute(serialize::json_reader& reader, double& value)
{
    if (!reader.is_double())
        throw std::invalid_argument("double");
    value = reader.get_double();
}

void serialize::execute(serialize::json_writer& writer, double& value)
{
    writer.decimal(value);
}

void serialize::execute(serialize::json_reader& reader, bool& value)
{
    if (!reader.is_bool())
        throw std::invalid_argument("bool");
    value = reader.get_bool();
}

void serialize::execute(serialize::json_writer& writer, bool& value)
{
    writer.boolean(value);
}

void serialize::execute(serialize::json_reader& reader, std::string& value)
{
    if (!reader.is_string())
        throw std::invalid_argument("std::string");
    value = reader.get_string();
}

void serialize::execute(serialize::json_writer& writer, std::string& value)
{
    writer.string(value.c_str(), (rapidjson::SizeType) value.size());
}

/* void serialize::execute(serialize::json_reader&, std::string_view&)
{
    throw std::invalid_argument("std::string_view is not supported");
}

void serialize::execute(serialize::json_writer& writer, std::string_view& data)
{
    if (data.empty())
        writer.string("");
    else
        writer.string(&data[0], (rapidjson::SizeType) data.size());
}*/

void serialize::execute(serialize::json_reader& reader, json::null&)
{
    reader.get_null();
}

void serialize::execute(serialize::json_writer& writer, json::null&)
{
    writer.null();
}

void serialize::execute(serialize::json_reader& reader, json::string& value)
{
    if (!reader.is_json())
        throw std::invalid_argument("json");
    value = reader.get_json();
}

void serialize::execute(serialize::json_writer& writer, json::string& value)
{
    writer.json(value);
}

