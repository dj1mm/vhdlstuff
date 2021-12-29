
#ifndef THINGS_DOCUMENT_SYMBOL_PROVIDER_H
#define THINGS_DOCUMENT_SYMBOL_PROVIDER_H

#include "vhdl_syntax.h"
#include "vhdl_nodes.h"

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace things
{

enum class symbol_kind
{
	file = 1,
	module = 2,
	ns = 3,
	package = 4,
	clazz = 5,
	method = 6,
	property = 7,
	field = 8,
	constructor = 9,
	enumerator = 10,
	interface = 11,
	function = 12,
	variable = 13,
	constant = 14,
	str = 15,
	number = 16,
	boolean = 17,
	array = 18,
	object = 19,
	key = 20,
	null = 21,
	enummember = 22,
	structure = 23,
	event = 24,
	op = 25,
	typeparameter = 26,
};

class vhdl_document_symbol_provider: public vhdl::syntax::visitor
{
    void symbol(vhdl::token, common::location, common::location, symbol_kind, std::optional<std::string> = std::nullopt);
    void symbol(std::string, common::location, common::location, common::location, symbol_kind, std::optional<std::string> = std::nullopt);
    void close_symbol();

    public:
    vhdl_document_symbol_provider(rapidjson::Writer<rapidjson::StringBuffer>*);
    ~vhdl_document_symbol_provider();

    virtual bool visit(vhdl::syntax::component_specification* c);
    virtual bool visit(vhdl::syntax::configuration_item* c);

    virtual bool visit(vhdl::syntax::block_configuration* b);
    virtual bool visit(vhdl::syntax::component_configuration* c);
    virtual bool visit(vhdl::syntax::subprogram* s);
    virtual bool visit(vhdl::syntax::signature* s);
    virtual bool visit(vhdl::syntax::type_definition* t);
    virtual bool visit(vhdl::syntax::subtype* s);
    virtual bool visit(vhdl::syntax::index_subtype* i);
    virtual bool visit(vhdl::syntax::range* r);
    virtual bool visit(vhdl::syntax::constraint* c);
    virtual bool visit(vhdl::syntax::unit_declaration* u);
    virtual bool visit(vhdl::syntax::physical_literal* p);
    virtual bool visit(vhdl::syntax::element_declaration* e);
    virtual bool visit(vhdl::syntax::declarative_item* d);
    virtual bool visit(vhdl::syntax::object* i);
    virtual bool visit(vhdl::syntax::interface* i);
    virtual bool visit(vhdl::syntax::association_element* a);
    virtual bool visit(vhdl::syntax::entity_specification* e);
    virtual bool visit(vhdl::syntax::configuration_specification* c);
    virtual bool visit(vhdl::syntax::binding_indication* b);
    virtual bool visit(vhdl::syntax::name* n);
    virtual bool visit(vhdl::syntax::expression* e);
    virtual bool visit(vhdl::syntax::element_association* e);
    virtual bool visit(vhdl::syntax::choice* c);
    virtual bool visit(vhdl::syntax::sequential_statement* s);
    virtual bool visit(vhdl::syntax::target* t);
    virtual bool visit(vhdl::syntax::waveform_element* w);
    virtual bool visit(vhdl::syntax::case_statement_alternative* c);
    virtual bool visit(vhdl::syntax::concurrent_statement* c);
    virtual bool visit(vhdl::syntax::con_waveform* c);
    virtual bool visit(vhdl::syntax::sel_waveform* s);
    virtual bool visit(vhdl::syntax::instantiated_unit* i);
    virtual bool visit(vhdl::syntax::design_file* d);
    virtual bool visit(vhdl::syntax::design_unit* d);
    virtual bool visit(vhdl::syntax::context_item* c);


    private:
    rapidjson::Writer<rapidjson::StringBuffer>* w;
};

}

#endif
