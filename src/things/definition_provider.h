
#ifndef THINGS_DEFINITION_PROVIDER_H
#define THINGS_DEFINITION_PROVIDER_H

#include "fmt/format.h"
#include "vhdl_syntax.h"

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace things
{

class vhdl_definition_provider: public vhdl::syntax::visitor
{
    void definition_found(common::location target, common::position selection_begin, common::position selection_end);
    void identifier_denotes_that_entity(vhdl::syntax::design_unit*);
    void identifier_denotes_that_architecture(vhdl::syntax::design_unit*);
    void identifier_denotes_that_configuration(vhdl::syntax::design_unit*);
    void identifier_denotes_that_package(vhdl::syntax::design_unit*);
    void identifier_denotes_that_package_body(vhdl::syntax::design_unit*);
    void identifier_denotes_that_typedecl(vhdl::syntax::declarative_item*);
    void identifier_denotes_that_subtype(vhdl::syntax::declarative_item*);
    void identifier_denotes_that_constant(vhdl::syntax::declarative_item*,int);
    void identifier_denotes_that_signal(vhdl::syntax::declarative_item*,int);
    void identifier_denotes_that_variable(vhdl::syntax::declarative_item*,int);
    void identifier_denotes_that_file(vhdl::syntax::declarative_item*,int);
    void identifier_denotes_that_alias(vhdl::syntax::declarative_item*);
    void identifier_denotes_that_function(vhdl::syntax::declarative_item*);
    void identifier_denotes_that_procedure(vhdl::syntax::declarative_item*);
    void identifier_denotes_that_component(vhdl::syntax::declarative_item*);
    void identifier_denotes_that_element(vhdl::syntax::element_declaration*,int);
    void identifier_denotes_that_library(vhdl::syntax::context_item*,int);
    void identifier_denotes_that_literal(vhdl::syntax::type_definition*,int);
    void identifier_denotes_these_entities(std::string_view, std::vector<vhdl::node::named_entity*>);

    public:
    vhdl_definition_provider(rapidjson::Writer<rapidjson::StringBuffer>*, bool&, common::position);
    ~vhdl_definition_provider();

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
    bool found;
    bool& json_populated;
    common::position position;
};

}

#endif
