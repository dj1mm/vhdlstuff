
#ifndef VHDL_COMMON_H
#define VHDL_COMMON_H

// forward declarations
namespace vhdl::node
{
class named_entity;
class entity;
class architecture;
class configuration;
class package;
class package_body;
class declarative_region;
class visitor;
}

namespace vhdl::syntax
{
class component_specification;
class configuration_item;
class block_configuration;
class component_configuration;
class subprogram;
class signature;
class type_definition;
class subtype;
class index_subtype;
class range;
class constraint;
class unit_declaration;
class physical_literal;
class element_declaration;
class declarative_item;
class object;
class interface;
class association_element;
class entity_specification;
class configuration_specification;
class binding_indication;
class name;
class expression;
class element_association;
class choice;
class sequential_statement;
class target;
class waveform_element;
class case_statement_alternative;
class concurrent_statement;
class con_waveform;
class sel_waveform;
class instantiated_unit;
class design_file;
class design_unit;
class context_item;
class a_declarative_region;
class visitor;
}

namespace vhdl
{
class ast;
}

#endif
