
#ifndef VHDL_PARSER_H
#define VHDL_PARSER_H

#include <array>
#include <bitset>
#include <memory>
#include <tuple>
#include <vector>

#include "common/diagnostics.h"

#include "lexer.h"
#include "token.h"

#include "vhdl_syntax.h"

namespace vhdl
{

class parser
{

    using tk = vhdl::token::kind_t;

    // Parse_name parses the occurence of a vhdl name and tries, based on
    // context, to guess its meaning. Thanks to the eloquent nature of the vhdl
    // language, a name can denote one of these:
    // - a slice                    slice name
    // - an array index             indexed name
    // - an attribute               attribute name
    // - an explicit dereference  }
    // - a record reference       } selected name
    // - an expanded name         }
    // - an object reference     -| selected name and
    // - a non-object reference  -| simple names
    //
    // name_options is a structure that stores the parameters to customize the
    // run time behaviour of vhdl::parser::parse_name().
    struct name_options
    {
        // a set of parenthesis following a name can have several different
        // meaning depending on the context such as:
        // - a slice name
        // - an indexed name
        // - a function call
        // - a type conversion
        // - a qualified expression (not true, but lets do as if it is)
        //
        // Some occurence of names do not allow parenthesis.
        // For example, type marks, names that denote types.
        // Type marks are not normally followed by ( ).
        // However, in an expression a type mark followed by ( ) can mean a
        // type conversion.
        // In a type indication, a type mark followed by ( ) means a constraint
        // TLDR, it is complicated.
        //
        // Same for function names followed by ( ).
        //
        // This flag controls whether parenthesis are allowed during parsing.
        // For example, parse_name(allow_qifts = false) will return as soon as
        // it encounters a (.
        // True:  Parenthesis are allowed (by default)
        // False: Parenthesis are not allowed
        bool allow_qifts;

        // Some occurence of names only allow simple names.
        // For example in an attribute specification.
        //
        // This flag controls whether complex names are allowed during parsing.
        // For example, parse_name(allow_complex_names = false) will return as
        // soon as it finishes parsing the simple name.
        //
        // Of course, we still do overload resolution.
        bool allow_complex_names;

        // A signature following a name is used to resolve the name's overload
        // meaning and appears at these places:
        // - alias declarations
        // - entity designators
        // - and attribute names (but lets leave this off the list for the sake
        //   of this discussion)
        // This flag controls whereby signatures are allowed after a name.
        // True:  signatures are allowed
        // False: signatures are not allowed (by default). But if one appears
        //        as part of an attribute name, do parse the signature!
        bool allow_signature;

        // name_options is a structure that stores the parameters to customize
        // the run time behaviour of vhdl::parser::parse_name().
        name_options(bool q = true, bool c = true, bool s = false)
            : allow_qifts(q), allow_complex_names(c), allow_signature(s)
        {
        }
    };

    public:
    //
    // Construct a vhdl parser with content to parse and optional file name
    //
    parser(common::stringtable*, vhdl::syntax::design_file*, version = vhdl93);
    ~parser();

    // ------------------------------------------------------------------------
    // Parser methods
    // ------------------------------------------------------------------------

    //
    // Parse the initialized file
    //
    std::tuple<bool, std::vector<common::diagnostic>>
    operator()();

    // ------------------------------------------------------------------------
    // Design entities and configurations
    // ------------------------------------------------------------------------
    // Implements these chapters from the VHDL LRM:
    // LRM93 1.0
    // ------------------------------------------------------------------------

    // LRM93 1.1 Entity declarations
    //
    // entity_declaration ::=
    //     ENTITY identifier IS
    //         entiy_header
    //         entity_declarative_part
    //     [ BEGIN entity_statement_part ]
    //     END [ ENTITY ] [ ENTITY_simple_name ]
    //
    // entity_header ::=
    //     [ FORMAL_generic_clause ]
    //     [ FORMAL_port_clause ]
    //
    vhdl::syntax::design_unit* parse_entity_declaration();

    // LRM93 1.1.1 Generics
    //
    // generic_clause ::=
    //     generic ( generic_list ) ;
    //
    // generic_list ::= GENERIC_interface_list
    std::tuple<common::location, common::location,
               std::vector<vhdl::syntax::declarative_item*>, common::location>
    parse_generic_clause();

    // LRM93 1.1.1 Ports
    //
    // port_clause ::=
    //     port ( port_list ) ;
    //
    // port_list ::= PORT_interface_list
    std::tuple<common::location, common::location,
               std::vector<vhdl::syntax::declarative_item*>, common::location>
    parse_port_clause();

    // LRM93 1.1.2
    //
    // entity_declarative_part ::=
    //    { entity_declarative_item }
    std::vector<vhdl::syntax::declarative_item*>
    parse_entity_declarative_part();

    // LRM93 1.1.2
    //
    // entity_declarative_item ::=
    //     subprogram_declaration
    //     | subprogram_body
    //     | type_declaration
    //     | subtype_declaration
    //     | constant_declaration
    //     | signal_declaration
    //     | shared_variable_declaration
    //     | file_declaration
    //     | alias_declaration
    //     | attribute_declaration
    //     | attribute_specification
    //     | disconnection_specification
    //     | use_clause
    //     | group_template_declaration
    //     | group_declaration
    vhdl::syntax::declarative_item* parse_entity_declarative_item();

    // LRM93 1.1.3
    //
    // entity_statement_part ::=
    //     { entity_statement }
    std::vector<vhdl::syntax::concurrent_statement*>
    parse_entity_statement_part();

    // LRM93 1.1.3
    //
    // entity_statement ::=
    //     concurrent_assertion_statement
    //     | passive_concurrent_procedure_call
    //     | passive_process_statement
    vhdl::syntax::concurrent_statement* parse_entity_statement();

    // LRM93 1.2
    //
    // An architecture body defines the body of a design entity. It specifies
    // the relationships between the inputs and outputs of a design entity and
    // may be expressed in terms of structure, dataflow, or behavior.
    //
    // architecture_body ::=
    //     ARCHITECTURE identifier OF ENTITY_name IS
    //         architecture_declarative_part
    //     BEGIN
    //         architecture_statement_part
    //     END [ ARCHITECTURE ] [ ARCHITECTURE_simple_name ] ;
    vhdl::syntax::design_unit* parse_architecture_body();

    // LRM93 1.2.1
    //
    // architecture_declarative_part ::=
    //      { block_declarative_item }
    std::vector<vhdl::syntax::declarative_item*>
    parse_architecture_declarative_part();

    // LRM93 1.2.1
    //
    // block_declarative_item ::=
    //      subprogram_declaration
    //      | subprogram_body
    //      | type_declaration
    //      | subtype_declaration
    //      | constant_declaration
    //      | signal_declaration
    //      | shared_variable_declaration
    //      | file_declaration
    //      | alias_declaration
    //      | component_declaration
    //      | attribute_declaration
    //      | attribute_specification
    //      | configuration_specification
    //      | disconnection_specification
    //      | use_clause
    //      | group_template_declaration
    //      | group_declaration
    vhdl::syntax::declarative_item* parse_block_declarative_item();

    // LRM93 1.2.2
    //
    // The architecture statement part contains statements that describe the
    // internal organization and/or operation of the block. The various kind of
    //
    // architecture_statement_part ::=
    //      { concurrent_statement }
    std::vector<vhdl::syntax::concurrent_statement*>
    parse_architecture_statement_part();

    // LRM93 1.3
    //
    // configuration_declaration ::=
    //     CONFIGURATION identifier OF ENTITY_name IS
    //         configuration_declarative_part
    //         block_configuration
    //     END [ CONFIGURATION ] [ CONFIGURATION_simple_name ] ;
    //
    vhdl::syntax::design_unit* parse_configuration_declaration();

    // LRM93 1.3
    //
    // configuration_declarative_part ::= { configuration_declarative_item }
    //
    // configuration_declarative_item ::= use_clause
    //                                  | attribute_specification
    //                                  | group_declaration
    vhdl::syntax::declarative_item* parse_configuration_declarative_item();

    // LRM93 1.3.1 Block configuration
    //
    // block_configuration ::=
    //          FOR block_specification
    //              { use_clause }
    //              { configuration_item }
    //          END FOR ;
    //
    vhdl::syntax::block_configuration* parse_block_configuration();

    // LRM93 1.3.1 Block configuration
    //
    // block_specification ::=
    //            ARCHITECTURE_name
    //          | BLOCK_STATEMENT_label
    //          | GENERATE_STATEMENT_label [ ( index_specification ) ]
    //
    void parse_block_specification(
        std::unique_ptr<vhdl::syntax::block_configuration>&);

    // LRM93 1.3.1 Block configuration
    //
    //  index_specification ::=
    //            discrete_range
    //          | static_expression
    //
    void parse_index_specification(
        std::unique_ptr<vhdl::syntax::block_configuration>&);

    // LRM93 1.3.1 Block configuration
    //
    // configuration_item ::=
    //            block_configuration
    //          | component_configuration
    vhdl::syntax::configuration_item* parse_configuration_item();

    // LRM93 1.3.2 Component configuration
    //
    // component_configuration ::=
    //          FOR component_specification
    //              [ binding_indication ; ]
    //              [ block_configuration ]
    //          END FOR ;
    vhdl::syntax::component_configuration* parse_component_configuration();

    // ------------------------------------------------------------------------
    // Subprograms and packages
    // ------------------------------------------------------------------------
    // Implements these chapters from the VHDL LRM:
    // LRM93 2.0
    // ------------------------------------------------------------------------

    // LRM93 2.1
    //
    // Subprograms define algorithms for computing values or for exhibiting
    // behavior.
    //
    // subprogram_declaration ::= subprogram_specification ;
    //
    //
    // Note a function declaration must have the syntax:
    //
    // function_declaration ::=
    //      [ PURE | IMPURE ] FUNCTION designator [ ( formal_parameter_list ) ]
    //      RETURN type_mark
    //
    // Note a procedure declaration must have the following syntax:
    //
    // procedure_declaration ::=
    //      PROCEDURE designator [ ( formal_parameter_list ) ] |
    //
    // designator ::= identifier
    //
    // formal_parameter_list ::= PARAMETER_interface_list
    std::tuple<std::optional<common::location>, vhdl::syntax::subprogram*,
               std::optional<common::location>>
    parse_subprogram_specification();

    // LRM93 2.2
    //
    // subprogram_body ::=
    //     function_specification IS subprogram_declarative_part
    //     BEGIN
    //         subprogram_statement_part
    //     END [ subprogram_kind ] [ designator ] ;
    //
    // subprogram_declarative_part ::=
    //     { subprogram_declarative_item }
    //
    //
    // Note, a function body specification must have the following syntax:
    //
    // function_specification ::=
    //      [ PURE | IMPURE ] FUNCTION designator [ ( formal_parameter_list ) ]
    //      RETURN type_mark
    //
    // designator ::= identifier | operator_symbol
    vhdl::syntax::declarative_item* parse_function_body(
        std::tuple<std::optional<common::location>, vhdl::syntax::subprogram*,
                   std::optional<common::location>>);

    // LRM93 2.2
    //
    // Function accepts a preparsed subprogram specification
    //
    // subprogram_body ::=
    //     procedure_specification IS subprogram_declarative_part
    //     BEGIN
    //         subprogram_statement_part
    //     END [ subprogram_kind ] [ designator ] ;
    //
    // subprogram_declarative_part ::=
    //     { subprogram_declarative_item }
    //
    //
    // Note a procedure body specification must have the following syntax:
    //
    // procedure_specification ::=
    //      PROCEDURE designator [ ( formal_parameter_list ) ] |
    //
    // designator ::= identifier
    vhdl::syntax::declarative_item* parse_procedure_body(
        std::tuple<std::optional<common::location>, vhdl::syntax::subprogram*,
                   std::optional<common::location>>);

    // LRM93 2.2
    //
    // subprogram_declarative_item ::=
    //     subprogram_declaration
    //     | subprogram_body
    //     | type_declaration
    //     | subtype_declaration
    //     | constant_declaration
    //     | variable_declaration
    //     | file_declaration
    //     | alias_declaration
    //     | attribute_declaration
    //     | attribute_specification
    //     | use_clause
    //     | group_template_declaration
    //     | group_declaration
    vhdl::syntax::declarative_item* parse_subprogram_declarative_item();

    // LRM93 2.3.2 Signatures
    // signature ::= [ [ type_mark { , type_mark } ] [ RETURN type_mark ] ]
    vhdl::syntax::signature* parse_signature();

    // LRM93 2.5
    //
    // package_declaration ::=
    //     PACKAGE identifier IS
    //         package_declarative_part
    //     END [ PACKAGE ] [ PACKAGE_simple_name ] ;
    //
    // package_declarative_part ::=
    //     { package_declarative_item }
    vhdl::syntax::design_unit* parse_package_declaration();

    // LRM93 2.5
    //
    // package_declarative_item ::=
    //     subprogram_declaration
    //     | type_declaration
    //     | subtype_declaration
    //     | constant_declaration
    //     | signal_declaration
    //     | shared_variable_declaration
    //     | file_declaration
    //     | alias_declaration
    //     | component_declaration
    //     | attribute_declaration
    //     | attribute_specification
    //     | disconnection_specification
    //     | use_clause
    //     | group_template_declaration
    //     | group_declaration
    vhdl::syntax::declarative_item* parse_package_declarative_item();

    // LRM93 2.6 Package bodies
    //
    // package_body ::=
    //     PACKAGE BODY PACKAGE_simple_name IS
    //         package_body_declarative_part
    //     END [ PACKAGE BODY ] [ PACKAGE_simple_name ] ;
    vhdl::syntax::design_unit* parse_package_body();

    // LRM93 2.6 Package bodies
    //
    // package_body_declarative_item ::=
    //     subprogram_declaration
    //     | subprogram_body
    //     | type_declaration
    //     | subtype_declaration
    //     | constant_declaration
    //     | shared_variable_declaration
    //     | file_declaration
    //     | alias_declaration
    //     | use_clause
    //     | group_template_declaration
    //     | group_declaration
    vhdl::syntax::declarative_item* parse_package_body_declarative_item();

    // ------------------------------------------------------------------------
    // Types
    // ------------------------------------------------------------------------
    // Implements these chapters from the VHDL LRM:
    // LRM93 3.0
    // ------------------------------------------------------------------------

    // LRM93 3.1 Scalar types
    //
    // There are five classes of types. Scalar types are integers, floating
    // point types, physical types, and types defined by an enumeration of
    // their values; values of these types have no elements
    //
    // scalar_type_definition ::=
    //                      enumeration_type_definition
    //                      | integer_type_definition
    //                      | floating_type_definition
    //                      | physical_type_definition
    vhdl::syntax::type_definition* parse_scalar_type_definition();
    vhdl::syntax::type_definition* parse_anonymous_type_definition(
        vhdl::syntax::range*);
    vhdl::syntax::type_definition* parse_physical_type_definition(
        vhdl::syntax::range*);

    // LRM93 3.1 Scalar types
    //
    // constrain an integer, float, enum or physical type. If value of the
    // constrined data goes out of range, this is an error
    //
    // range_constraint ::= RANGE range
    vhdl::syntax::constraint* parse_range_constraint();

    // LRM93 3.1 Scalar types
    //
    // specify a range of values that belong to an int, float, enum or physical
    // type. static range: bounds can be calculated during parse/analysis
    //
    // range ::=
    //        RANGE_attribute_name
    //        | simple_expression direction simple_expression
    //
    // direction ::= TO | DOWNTO
    //
    vhdl::syntax::range* parse_range();

    // LRM93 3.2 Composite types
    //
    // Composite types are array and record types
    //
    // composite_type_definition ::=
    //                      array_type_definition
    //                      | record_type_definition
    // array_type_definition ::=
    //                      unconstrained_array_definition
    //                      | constrained_array_definition
    //
    // unconstrained_array_definition ::=
    //      ARRAY ( index_subtype_definition { , index_subtype_definition } )
    //      OF ELEMENT_subtype_indication
    //
    // constrained_array_definition ::=
    //      ARRAY index_constraint OF ELEMENT_subtype_indication
    //
    // record_type_definition ::=
    //      RECORD
    //          element_declaration
    //          { element_declaration }
    //      END RECORD [ RECORD_TYPE_simple_name ]
    //
    vhdl::syntax::type_definition* parse_composite_type_definition();

    // LRM93 3.2 Composite types
    //
    // index_subtype ::= type_mark RANGE <>
    vhdl::syntax::index_subtype* parse_index_subtype();

    // LRM93 3.2.1 Array types
    //
    // constraint for a constrained array data type. If value of the data goes
    // of range, this is an error
    //
    // number of discrete range specifies dimensionality of constrained array
    //
    // index_constraint ::=
    //             ( discrete_range { , discrete_range } )
    vhdl::syntax::constraint* parse_index_constraint();

    // LRM93 3.2.1 Array types
    //
    // discrete_range ::=
    //             DISCRETE_subtype_indication | range
    vhdl::syntax::range* parse_discrete_range();

    // LRM93 3.2.2 Record types
    //
    // element of a record
    //
    // element_declaration ::=
    //          identifier_list : element_subtype_definition ;
    vhdl::syntax::element_declaration* parse_element_declaration();

    // LRM93 3.2.2 Record types
    //
    // identifier_list ::= identifier { , identifier }
    std::vector<vhdl::token> parse_identifier_list();

    // LRM93 3.3 Access types
    //
    // Access types provide access to objects of a given type
    //
    // access_type_definition ::=
    //                      ACCESS subtype_indication
    vhdl::syntax::type_definition* parse_access_type_definition();

    // LRM93 3.4 File types
    //
    // File types provide access to objects that contain a sequence of values
    // of a given type
    //
    // file_type_definition ::=
    //                      FILE OF type_mark
    vhdl::syntax::type_definition* parse_file_type_definition();

    // ------------------------------------------------------------------------
    // Declarations
    // ------------------------------------------------------------------------
    // Implements these chapters from the VHDL LRM:
    // LRM93 4.0
    // ------------------------------------------------------------------------

    enum class declarative_part
    {
        architecture,
        block,
        configuration,
        entity,
        package_body,
        package,
        process,
        protected_body,
        protected_declaration,
        subprogram
    };

    vhdl::syntax::declarative_item* parse_declarative_item(declarative_part);

    // LRM93 4.1 Type declarations
    // type_declaration::=
    //                  full_type_declaration
    //                  | incomplete_type_declaration
    //
    // full_type_declaration::= TYPE identifier IS type_definition ;
    // incomplete_type_declaration::= TYPE identifier ;
    //
    // type_definition ::=
    //                  scalar_type_definition
    //                  | composite_type_definition
    //                  | access_type_definition
    //                  | file_type_definition
    //                  | protected_type_definition
    //
    // Parses a type declaration and return it
    vhdl::syntax::declarative_item* parse_type_declaration();

    // LRM93 4.2 Subtype declarations
    //
    // subtype_declaration ::=
    //                  SUBTYPE identifier IS subtype_indication ;
    //
    vhdl::syntax::declarative_item* parse_subtype_declaration();

    // LRM93 4.2 Subtype declarations
    //
    // subtype_indication ::=
    //                  [ RESOLUTION_FUNCTION_name ] type_mark [ constraint ]
    vhdl::syntax::subtype* parse_subtype_indication();

    // LRM93 4.2 Subtype declarations
    //
    // type_mark ::= type_name
    //             | subtype_name
    vhdl::syntax::name* parse_type_mark();

    // LRM93 4.3 Objects
    //
    // LRM93 4.3.1 Object declarations
    // object_declaration ::=
    //              constant_declaration
    //              | signal_declaration
    //              | variable_declaration
    //              | file_declaration

    // LRM93 4.3.1.1 Constant declarations
    //
    // constant_declaration ::=
    //     CONSTANT identifier_list : subtype_indication [ := expression ]
    vhdl::syntax::declarative_item* parse_constant_declaration();

    // LRM93 4.3.1.2 Signal declarations
    //
    // signal_declaration ::=
    //     SIGNAL identifier_list : subtype_information [ signal_kind ]
    //         [ := expression ]
    //
    // signal_kind ::= REGISTER | BUS
    vhdl::syntax::declarative_item* parse_signal_declaration();

    // LRM93 4.3.1.3 Variable declarations
    //
    // variable_declaration ::=
    //     [ SHARED ] VARIABLE identifier_list : subtype_indication
    //         [ := expression ]
    vhdl::syntax::declarative_item* parse_variable_declaration();

    //
    // file_declaration ::=
    //     FILE identifier : subtype_indication IS [ mode ] file_logical_name
    //
    // file_logical_name ::= STRING_expression
    //
    // LRM93 4.3.1.3 File declarations
    //
    // file_declaration ::=
    //     FILE identifier_list : subtype_indication [ file_open_information ]
    //
    // file_open_information ::=
    //     [ OPEN FILE_OPEN_KIND_expression ] IS file_logical_name
    //
    // file_logical_name ::= STRING_expression
    //
    vhdl::syntax::declarative_item* parse_file_declaration();

    enum class interface { function, procedure, port, generic };

    // LRM93 4.3.2 Interface declaration
    //
    // interface_list ::=
    //     interface_element { ; interface_element }
    std::vector<vhdl::syntax::declarative_item*> parse_interface_list(
        interface kind);

    // LRM93 4.3.2 Interface declaration
    //
    // interface_element ::= interface_declaration
    vhdl::syntax::declarative_item* parse_interface_element(interface);

    // LRM93 4.3.2 Interface declaration
    // interface_declaration ::=
    //                  interface_constant_declaration
    //                  | interface_signal_declaration
    //                  | interface_variable_declaration
    //                  | interface_file_declaration

    // LRM93 4.3.2 Interface declaration
    //
    // interface_constant_declaration ::=
    //      [ CONSTANT ] identifier_list : [ IN ] subtype_indication [ := static_expression ]
    vhdl::syntax::declarative_item* parse_interface_constant_declaration();

    // LRM93 4.3.2 Interface declaration
    //
    // interface_signal_declaration ::=
    //      [ SIGNAL ] identifier_list : [ mode ] subtype_indication [ BUS ] [ := static_expression ]
    vhdl::syntax::declarative_item* parse_interface_signal_declaration();

    // LRM93 4.3.2 Interface declaration
    //
    // mode ::= IN | OUT | INOUT | BUFFER | LINKAGE
    vhdl::syntax::mode parse_mode();

    // LRM93 4.3.2 Interface declaration
    //
    // interface_variable_declaration ::=
    //      [ VARIABLE ] identifier_list : [ mode ] subtype_indication [ := static_expression ]
    vhdl::syntax::declarative_item* parse_interface_variable_declaration();

    // LRM93 4.3.2 Interface declaration
    //
    // interface_file_declaration ::=
    //      FILE identifier_list : subtype_indication
    vhdl::syntax::declarative_item* parse_interface_file_declaration();

    // LRM93 4.3.2.2 Association lists
    //
    // association_list ::= association_element { , association_element }
    //
    // association_element ::= [ formal_part => ] actual_part
    //
    // formal_part ::=
    //          formal_designator
    //          | function_name ( formal_designator )
    //          | type_mark ( formal_designator )
    //
    // formal_designator ::=
    //          generic_name
    //          | port_name
    //          | parameter_name
    //
    // actual_part ::=
    //          actual_designator
    //          | function_name ( actual_designator )
    //          | type_mark ( actual_designator )
    //
    // actual_designator ::=
    //          expression
    //          | signal_name
    //          | variable_name
    //          | file_name
    //          | open
    //
    std::vector<vhdl::syntax::association_element*> parse_association_list();

    // LRM93 4.3.3 Alias declarations
    // alias_declaration ::=
    //               ALIAS alias_designator [ : subtype_indication ]
    //               IS name [ signature ] ;
    //
    // alias_designator ::= identifier | character_literal | operator_symbol
    vhdl::syntax::declarative_item* parse_alias_declaration();

    // LRM93 4.4 Attributes declarations
    // attribute_declaration ::=
    //              ATTRIBUTE identifier : type_mark ;
    //
    // Pass the attribute identifier as a parameter to this function to directly
    // skip to parsing the attribute type mark
    vhdl::syntax::declarative_item* parse_attribute_declaration();

    // LRM93 4.5 Component declarations
    // component_declaration ::=
    //              COMPONENT identifier [ IS ]
    //                  [ LOCAL_generic_clause ]
    //                  [ LOCAL_port_clause ]
    //              END COMPONENT [ COMPONENT_simple_name ] ;
    vhdl::syntax::declarative_item* parse_component_declaration();


    // ------------------------------------------------------------------------
    // Specification
    // ------------------------------------------------------------------------
    // Implements these chapters from the VHDL LRM:
    // LRM93 5.0
    // ------------------------------------------------------------------------

    // LRM93 5.1
    //
    // attribute_specification ::=
    //   ATTRIBUTE attribute_designator OF entity_specification IS expression ;
    //
    vhdl::syntax::declarative_item* parse_attribute_specification();

    // LRM93 5.1
    //
    // entity_specification ::= entity_name_list : entity_class
    //
    // entity_name_list ::= entity_designator { , entity_designator }
    //                    | others
    //                    | all
    vhdl::syntax::entity_specification* parse_entity_specification();

    // LRM93 5.1
    //
    // entity_designator ::= entity_tag [ signature ]
    //
    // Pass the entity designator of an attribute
    vhdl::syntax::name* parse_entity_designator();

    // LRM93 5.1
    //
    // entity_class ::= ENTITY    | ARCHITECTURE | CONFIGURATION
    //                | PROCEDURE | FUNCTION     | PACKAGE
    //                | TYPE      | SUBTYPE      | CONSTANT
    //                | SIGNAL    | VARIABLE     | COMPONENT
    //                | LABEL     | LITERAL      | UNITS
    //                | GROUP     | FILE
    //
    bool parse_entity_class(vhdl::syntax::entity_class&);

    // LRM93 5.2
    //
    // configuration_specification ::=
    //             FOR component_specification binding_indication;
    //
    vhdl::syntax::declarative_item* parse_configuration_specification();

    // LRM93 5.2
    //
    // component_specification ::=
    //             instantiation_list : COMPONENT_name
    //
    // instantiation_list ::=
    //             instantiation_label { , instantiation_label }
    //             | OTHERS
    //             | ALL
    vhdl::syntax::component_specification* parse_component_specification();

    // LRM93 5.2
    //
    // binding_indication ::=
    //              [ USE entity_aspect ]
    //              [ generic_map_aspect ]
    //              [ port_map_aspect ]
    //
    vhdl::syntax::binding_indication* parse_binding_indication();

    // LRM93 5.2
    //
    // entity_aspect ::=
    //              ENTITY ENTITY_name [ ( ARCHITECTURE_identifier) ]
    //              | CONFIGURATION CONFIGURATION_name
    //              | OPEN
    void parse_entity_aspect(vhdl::syntax::binding_indication*);

    std::tuple<common::location, common::location,
               std::vector<vhdl::syntax::association_element*>,
               common::location>
    parse_generic_map_aspect();
    std::tuple<common::location, common::location,
               std::vector<vhdl::syntax::association_element*>,
               common::location>
    parse_port_map_aspect();

    // ------------------------------------------------------------------------
    // Names
    // ------------------------------------------------------------------------
    // Implements these chapters from the VHDL LRM:
    // LRM93 6.0
    // ------------------------------------------------------------------------

    vhdl::syntax::name* parse_name(name_options = {});

    vhdl::syntax::name* parse_simple_name(name_options&);
    vhdl::syntax::name* parse_selected_name(name_options&, vhdl::syntax::name*);
    vhdl::syntax::name* parse_parenthesis_name(name_options&,
                                               vhdl::syntax::name*);
    vhdl::syntax::name* parse_signed_name(name_options&, vhdl::syntax::name*);
    vhdl::syntax::name* parse_attribute_name(name_options&,
                                             vhdl::syntax::name*);

    // ------------------------------------------------------------------------
    // Expressions
    // ------------------------------------------------------------------------
    // Implements these chapters from the VHDL LRM:
    // LRM93 7.0
    // ------------------------------------------------------------------------

    vhdl::syntax::op parse_operation();

    vhdl::syntax::expression* parse_expression();

    // LRM93 7.1
    //
    // parse a vhdl primary and return either a type literal, a name, aggregate
    // or new
    // NOTE: only names allowed as primaries are attributes that yield values,
    // and names denoting objects or values
    vhdl::syntax::expression* parse_primary();

    // LRM93 7.2.1
    //
    // expression is a `formula` defining a computed value
    // Valid logical operators are:
    // - and
    // - or
    // - nand
    // - nor
    // - xor
    // - xnor
    // - not
    vhdl::syntax::expression* parse_boolean_expression();

    // LRM93 7.2.2 Relational operators
    //
    // relation operators are:
    // - test for equality
    // - test for inequality
    // - ordering of operands (<, >, <=, >=)
    vhdl::syntax::expression* parse_relation();

    // LRM93 7.2.3 Shift operators
    //
    // expression is a `formula` defining a computed value
    // Valid logical operators are:
    // - sll
    // - srl
    // - sla
    // - sra
    // - rol
    // - ror
    vhdl::syntax::expression* parse_shift_expression();

    vhdl::syntax::expression* parse_simple_expression();

    vhdl::syntax::expression* parse_term();

    vhdl::syntax::expression* parse_factor();

    // LRM93 7.3.2 Aggregate
    //
    // an aggregate is a basic operation that combines one or more values into
    // a composite value of a record or array type
    //
    // aggregate::= (element_association { , element_association })
    vhdl::syntax::expression* parse_aggregate();

    // LRM93 7.3.2 Aggregate
    //
    // element_association ::= [ choices => ] expression
    vhdl::syntax::element_association* parse_element_association();

    // LRM93 7.3.2 Aggregate
    //
    // choice ::= choice { | choice }
    //
    // Parse vhdl choices. For example, 14 | 7 downto 0 => '0' consists of 2
    // choices:
    // 1. bit 14                                    } will take value '0'
    // 2. bit 7 downto 0                            }
    // while others => '1' consists of 1 choice     } every bits will be '1'
    std::vector<vhdl::syntax::choice*> parse_choices();

    // LRM93 7.3.2 Aggregate
    //
    // choice ::=
    //          simple_expression
    //          | discrete_range
    //          | ELEMENT_simple_name
    //          | others
    //
    vhdl::syntax::choice* parse_choice();

    // LRM93 7.3.3
    //
    // function_call ::=
    //      function_name [ ( actual_parameter_part ) ]
    //
    // Prefix is probably overloaded, therefore, check prefix is appropriate
    // for a function call.
    vhdl::syntax::name* parse_function_call(name_options&, vhdl::syntax::name*);

    // LRM93 7.3.3
    //
    // actual_parameter_part ::= association_list
    std::vector<vhdl::syntax::association_element*>
    parse_actual_parameter_part();

    // LRM93 7.3.4
    //
    // qualified_expression ::=
    //      type_mark ' ( expression )
    //      | type_mark ' aggregate
    vhdl::syntax::name* parse_qualified_expression(name_options& options,
                                                   vhdl::syntax::name*);

    // LRM93 7.3.6 Allocators
    //
    // allocator ::= NEW subtype_indication
    //             | NEW qualified_expression
    vhdl::syntax::expression* parse_allocator();

    // ------------------------------------------------------------------------
    // Sequential statements
    // ------------------------------------------------------------------------
    // Implements these chapters from the VHDL LRM:
    // LRM93 8.0
    // ------------------------------------------------------------------------

    // LRM93 8.0
    //
    // sequence_of_statements ::=
    //          { sequential_statement }
    std::vector<vhdl::syntax::sequential_statement*>
    parse_sequence_of_statements();

    // LRM93 8.0
    //
    // sequential_statement ::=
    //          wait_statement
    //          | assertion_statement
    //          | report_statement
    //          | signal_assignment_statement
    //          | variable_assignment_statement
    //          | procedure_call_statement
    //          | if_statement
    //          | case_statement
    //          | loop_statement
    //          | next_statement
    //          | exit_statement
    //          | return_statement
    //          | null_statement
    vhdl::syntax::sequential_statement* parse_sequential_statement();

    // LRM93 8.1
    //
    // wait_statement ::=
    //     [ label : ] WAIT [ sensitivity_clause ] [ condition_clause ]
    //         [ timeout_clause ] ;
    //
    // sensitivity_clause ::= ON sensitivity_list
    //
    // condition_clause ::= UNTIL conditiion
    //
    // timeout_clause ::= FOR TIME_expression
    vhdl::syntax::sequential_statement* parse_wait_statement(
        std::optional<vhdl::token>);

    // LRM93 8.1
    //
    // sensitivity_list ::= SIGNAL_name { , SIGNAL_name }
    std::vector<vhdl::syntax::name*> parse_sensitivity_list();

    // LRM93 8.2
    //
    // assertion_statement ::= [ label : ] assertion ;
    vhdl::syntax::sequential_statement* parse_assertion_statement(
        std::optional<vhdl::token>);

    // LRM93 8.2
    //
    // assertion ::=
    //      ASSERT condition [ REPORT expression ] [ SEVERITY expression ] ;
    std::tuple<vhdl::syntax::expression*, vhdl::syntax::expression*,
               vhdl::syntax::expression*>
    parse_assertion();

    // LRM93 8.3
    // report_statement ::= [ label : ] REPORT expression [ SEVERITY expression ] ;
    vhdl::syntax::sequential_statement* parse_report_statement(
        std::optional<vhdl::token>);

    // LRM93 8.4
    //
    // signal_assignment_statement ::=
    //          [ label : ] target <= [ delay_mechanism ] waveform ;
    vhdl::syntax::sequential_statement* parse_signal_assignment_statement(
        std::optional<vhdl::token>, vhdl::syntax::target*);

    // LRM93 8.4
    //
    // delay_mechanism ::= TRANSPORT | [ REJECT time_expression ] INERTIAL
    std::tuple<bool, bool, vhdl::syntax::expression*, bool>
    parse_delay_mechanism();

    // LRM93 8.4
    //
    // target ::= name | aggregate
    vhdl::syntax::target* parse_target(vhdl::syntax::name* name = nullptr);

    // LRM93 8.4
    //
    // waveform ::= waveform_element { , waveform_element } | UNAFFECTED
    std::vector<vhdl::syntax::waveform_element*> parse_waveform();

    // LRM93 8.4
    //
    // waveform_element ::= VALUE_expression [ AFTER TIME_expression ]
    //                    | NULL [ AFTER TIME_expression ]
    vhdl::syntax::waveform_element* parse_waveform_element();

    // LRM93 8.5
    //
    // variable_assignment_statement ::=
    //             [ label : ] target := expression ;
    vhdl::syntax::sequential_statement* parse_variable_assignment_statement(
        std::optional<vhdl::token>, vhdl::syntax::target*);

    // LRM93 8.6
    //
    // procedure_call_statement ::= [ label : ] procedure_call ;
    // procedure_call ::= procedure_NAME [ ( actual_parameter_part ) ]
    vhdl::syntax::sequential_statement* parse_procedure_call_statement(
        std::optional<vhdl::token>, vhdl::syntax::name*);

    // LRM93 8.7
    //
    // if_statement ::=
    // [ if_label : ] IF condition THEN sequence_of_statements
    //                { ELSIF condition THEN sequence_of_statements }
    //                [ ELSE sequence_of_statements ]
    //                END IF [ if_label ] ;
    vhdl::syntax::sequential_statement* parse_if_statement(
        std::optional<vhdl::token>);

    //
    // LRM93 8.8
    //
    // case_statement ::=
    // [ CASE_label : ] CASE expression IS
    //                  case_statement_alternative
    //                  { case_statement_alternative }
    //                  END CASE [ CASE_label ] ;
    //
    // case_statement_alternative ::= WHEN choices => sequence_of_statements
    vhdl::syntax::sequential_statement* parse_case_statement(
        std::optional<vhdl::token>);

    // LRM93 8.9
    //
    // loop_statement ::=
    // [ LOOP_label : ] [ iteration_scheme ] LOOP
    //                  sequence_of_statements
    //                  END LOOP [ LOOP_label ] ;
    //
    // iteration_scheme ::= WHILE condition | FOR LOOP_parameter_specification
    vhdl::syntax::sequential_statement* parse_loop_statement(
        std::optional<vhdl::token>);

    // LRM93 8.9
    //
    // parameter_specification ::= identifier IN discrete_range
    std::tuple<vhdl::syntax::param_spec*, bool> parse_parameter_specification();

    // LRM93 8.9
    //
    // See parse_loop_statement() with iterator_scheme = FOR parameter_specification
    vhdl::syntax::sequential_statement* parse_for_loop_statement(
        std::optional<vhdl::token>);

    // LRM93 8.9
    //
    // See parse_loop_statement() with iterator_scheme = WHILE condition
    vhdl::syntax::sequential_statement* parse_while_loop_statement(
        std::optional<vhdl::token>);

    // LRM93 8.10
    //
    // next_statement ::= [ label : ] NEXT [ LOOP_label ] [ WHEN condition ] ;
    vhdl::syntax::sequential_statement* parse_next_statement(
        std::optional<vhdl::token>);

    // LRM93 8.11
    //
    // exit_statement ::= [ label : ] EXIT [ LOOP_label ] [ WHEN condition ] ;
    vhdl::syntax::sequential_statement* parse_exit_statement(
        std::optional<vhdl::token>);

    // LRM93 8.12
    //
    // return_statement ::= [ label : ] RETURN [ expression ] ;
    vhdl::syntax::sequential_statement* parse_return_statement(
        std::optional<vhdl::token>);

    // LRM93 8.13
    //
    // null_statement ::= [ label : ] NULL ;
    vhdl::syntax::sequential_statement* parse_null_statement(
        std::optional<vhdl::token>);

    // ------------------------------------------------------------------------
    //  Concurrent statements
    // ------------------------------------------------------------------------
    // Implements these chapters from the VHDL LRM:
    // LRM93 9.0
    // ------------------------------------------------------------------------

    std::vector<vhdl::syntax::concurrent_statement*>
    parse_concurrent_statements();

    // LRM93 9.0
    //
    // concurrent_statement ::=
    //     block_statement
    //     | process_statement
    //     | concurrent_procedure_call_statement
    //     | concurrent_assertion_statement
    //     | concurrent_signal_assignment_statement
    //     | component_instantiation_statement
    //     | generate_statement
    vhdl::syntax::concurrent_statement* parse_concurrent_statement();

    // LRM93 9.1
    //
    // block_statement ::=
    //  BLOCK_label :
    //  BLOCK [ ( GUARD_expression ) ] [ IS ]
    //      block_header
    //      block_declarative_part
    //  BEGIN
    //      block_statement_part
    //  END BLOCK [ BLOCK_label ] ;
    //
    // block_header ::=
    //      [ generic_clause [ generic_map_aspect ; ] ]
    //      [ port_clause [ port_map_aspect ; ] ]
    //
    vhdl::syntax::concurrent_statement* parse_block_statement();

    //
    // LRM93 9.1
    //
    // block_declarative_part ::= { block_declarative_item }
    std::vector<vhdl::syntax::declarative_item*> parse_block_declarative_part();

    //
    // LRM93 9.1
    //
    // block_statement_part ::= { concurrent_statement }
    std::vector<vhdl::syntax::concurrent_statement*>
    parse_block_statement_part();

    // LRM93 9.2
    //
    // process_statement ::=
    //  [ label : ]
    //  [ POSTPONED ] PROCESS [ ( process_sensitivity_list ) ] [ IS ]
    //      process_declarative_part
    //  BEGIN
    //      process_statement_part
    //  END [ POSTPONED ] PROCESS [ label ] ;
    //
    // process_sensitivity_list ::= ALL | sensitivity_list
    vhdl::syntax::concurrent_statement* parse_process_statement();

    // LRM93 9.2
    //
    // process_declarative_item ::=
    //                 subprogram_declaration
    //                 | subprogram_body
    //                 | type_declaration
    //                 | subtype_declaration
    //                 | constant_declaration
    //                 | variable_declaration
    //                 | file_declaration
    //                 | alias_declaration
    //                 | attribute_declaration
    //                 | attribute_specification
    //                 | use_clause
    //                 | group_template_declaration
    //                 | group_declaration
    //
    vhdl::syntax::declarative_item* parse_process_declarative_item();

    // LRM93 9.3
    //
    // concurrent_procedure_call_statement ::=
    //      [ label : ] [ POSTPONED ] procedure_call ;
    vhdl::syntax::concurrent_statement*
    parse_concurrent_procedure_call_statement();

    // LRM93 9.4
    //
    // concurrent_assertion_statement ::=
    //     [ label : ] [ POSTPONED ] assertion ;
    vhdl::syntax::concurrent_statement* parse_concurrent_assertion_statement();

    // LRM93 9.5
    //
    // concurrent_signal_assignment_statement ::=
    //     [ label : ] [ POSTPONED ] conditional_signal_assignment
    //   | [ label : ] [ POSTPONED ] selected_signal_assignment
    vhdl::syntax::concurrent_statement*
    parse_concurrent_signal_assignment_statement();

    // LRM93 9.5
    //
    // options ::= [ guarded ] [ delay_mechanism ]
    std::tuple<bool, bool, bool, vhdl::syntax::expression*, bool>
    parse_option();

    // LRM93 9.5.1
    //
    // concurrent_conditional_signal_assignment ::=
    //     target <= [ GUARDED ] [ delay_mechanism ] conditional_waveforms ;
    vhdl::syntax::concurrent_statement*
    parse_concurrent_conditional_signal_assignment_statement(
        std::optional<vhdl::token>, bool);

    // LRM93 9.5.1
    //
    // conditional_waveforms ::=
    //     { waveform WHEN condition ELSE } waveform [ WHEN condition ]
    void parse_conditional_waveforms(std::vector<vhdl::syntax::con_waveform*>&);

    // Not part of any lrm. But it makes parsing conditional waveforms easier
    //
    // conditional_waveform:
    //      waveform [ WHEN condition ]
    vhdl::syntax::con_waveform* parse_conditional_waveform();

    // LRM93 9.5.2
    //
    // selected_signal_assignment ::=
    //     WITH expresion SELECT target <= options selected_waveforms ;
    vhdl::syntax::concurrent_statement*
    parse_concurrent_selected_signal_assignment_statement(
        std::optional<vhdl::token>, bool);

    // LRM93 9.5.2
    //
    // selected_waveforms ::=
    //     { waveform WHEN choices , } waveform WHEN choices
    void parse_selected_waveforms(std::vector<vhdl::syntax::sel_waveform*>&);

    // Not part of any lrm. But it makes parsing selected waveforms easier
    //
    // selected_waveform:
    //      waveform WHEN choices
    vhdl::syntax::sel_waveform* parse_selected_waveform();

    // LRM93 9.6
    //
    // component_instantiation_statement ::=
    //   label : instantiated_unit [ generic_map_aspect ] [ port_map_aspect ] ;
    vhdl::syntax::concurrent_statement*
    parse_component_instantiation_statement();

    // LRM93 9.6
    //
    // instantiated_unit ::=
    //       [ COMPONENT ] COMPONENT_name
    //     | ENTITY ENTITY_name [ ( ARCHITECTURE_identifier ) ]
    //     | CONFIGURATION CONFIGURATION_name
    vhdl::syntax::instantiated_unit* parse_instantiated_unit();

    // LRM93 9.7
    //
    // generate_statement ::=
    //      label : generation_scheme GENERATE
    //          [ { block_declarative_item }
    //      BEGIN ]
    //          { concurrent_statement }
    //      END GENERATE [ label ] ;
    //
    // generation_scheme ::=
    //      FOR parameter_specification
    //    | IF condition
    vhdl::syntax::concurrent_statement* parse_generate_statement();

    // Not part of any lrm. But it makes parsing generate if statements
    //
    // selected_waveform:
    //      waveform WHEN choices
    vhdl::syntax::concurrent_statement* parse_generate_if_statement();

    // Not part of any lrm. But it makes parsing generate for statements
    //
    // selected_waveform:
    //      waveform WHEN choices
    vhdl::syntax::concurrent_statement* parse_generate_for_statement();

    // ------------------------------------------------------------------------
    //  Scope and visibility
    // ------------------------------------------------------------------------
    // Implements these chapters from the VHDL LRM:
    // LRM93 10.0
    // ------------------------------------------------------------------------

    // LRM93 10.4
    // use_clause ::= USE selected_name { , selected_name }
    vhdl::syntax::context_item* parse_use_clause();

    // ------------------------------------------------------------------------
    //  Design units and their analysis
    // ------------------------------------------------------------------------
    // Implements these chapters from the VHDL LRM:
    // LRM93 11.0
    // ------------------------------------------------------------------------

    // LRM93 11.1
    // design_file ::= design_unit { design_unit }
    bool parse_design_file();

    // LRM93 11.1
    // design_unit ::= context_clause library_unit
    vhdl::syntax::design_unit* parse_design_unit();

    // LRM93 11.2
    // library_clause ::= LIBRARY logical_name_list
    vhdl::syntax::context_item* parse_library_clause();

    // LRM93 11.3 Context clauses
    // context_clause ::= { context_item }
    //
    // context_item ::= library_clause | use_clause
    //
    // Parse the current context clause into the design unit provided
    std::vector<vhdl::syntax::context_item*> parse_context_clause();

    // LRM93 13.3 Identifiers
    // identifier ::= basic_identifier | extended_identifier
    //
    // extended_identifier ::=
    //      \ graphic_character { graphic_character } \   <---
    //
    std::tuple<vhdl::token, bool> parse_identifier(std::string_view message);

    //
    std::optional<vhdl::token> parse_optional_label();

    //
    std::tuple<vhdl::token, bool> parse_non_optional_label(
        std::string_view message);

    //
    std::tuple<vhdl::token, bool> parse_designator(std::string_view message);

    //
    std::tuple<vhdl::token, bool> parse_alias_designator();

    //
    // check for end name
    //
    void parse_optional_closing_label(vhdl::token);
    void parse_optional_closing_designator(vhdl::syntax::subprogram*);

    private:
    // ------------------------------------------------------------------------
    // Miscellaneous functions used to assist parsing
    // ------------------------------------------------------------------------

    int temporary_count_that_prevents_forever_loops = 0;

    //
    // scan the next token and discard the current token
    //
    void skip();

    //
    // peek the next token and discard the current token
    //
    vhdl::token::kind_t peek(unsigned nth = 1);

    //
    // get the current token
    //
    vhdl::token::kind_t current_token();

    //
    // check whether current token is equal to `token`
    //
    // If current token is `token`: return true
    // If current token is not `token`: send a diagnosis and return false
    //
    bool expecting(token::kind_t token);

    //
    // check whether current token is equal to `token`
    //
    // If current token is `token`: return true
    // If current token is not `token`: send a diagnosis and return false
    //
    bool expecting(token::kind_t token, std::string_view message);

    //
    // eat current token iff token is expected
    //
    // If current token is `token`:     consume it and return its location
    // If current token is not `token`: send a diagnostic, do NOT consume the
    //                                  token and return the current location
    //
    common::location eat(token::kind_t token);

    //
    // consume current token iff token is expected
    //
    // If current token is `token`:     consume it
    // If current token is not `token`: send a diagnostic, do NOT consume the
    //                                  token and
    //
    void consume(token::kind_t token);

    //
    // consume current token iff token is expected
    //
    // If current token is `token`:     consume it
    // If current token is not `token`: send the diagnostic, do NOT consume the
    //                                  token
    //
    void consume(token::kind_t token, std::string_view message);

    //
    // check whether current token equals to `token`
    //
    // If current token is `token`:     consume it and return true
    // If current token is not `token`: do NOT consume the token and return
    //                                  false
    //
    bool optional(token::kind_t token);

    common::diagnostic& diag(const std::string_view msg);

    common::diagnostic& diag(const std::string_view msg, common::location loc);

    void resync_to_next_semicolon(bool consume_the_semicolon = true);

    // helper to skip the whatever is in between ( and ).
    // Argument nest specifies the number of ( already passed. Default 0, in
    // which case, expects current token to be (.
    void resync_to_end_of_rightpar(unsigned nests = 0);

    // parsing state.
    //
    // Stores the state of mind of the parser. This is used extensively by the
    // `parse_many` function.
    enum class state
    {
        design_unit_in_design_file,
        interface_lists,
        declarative_part_begin_end,
        declarative_part_begin,
        declarative_part_end,
        declarative_part_for,
        entity_statement_part,
        concurrent_statements,
        sequential_statements,
    };
    using ps = vhdl::parser::state;

    const std::array<ps, 9> array_of_states = {
        ps::design_unit_in_design_file, ps::interface_lists,
        ps::declarative_part_begin_end, ps::declarative_part_begin,
        ps::declarative_part_end,       ps::declarative_part_for,
        ps::entity_statement_part,      ps::concurrent_statements,
        ps::sequential_statements,
    };
    std::bitset<9> parsing_states;

    // Return true if parser, when in the given state, knows how to parse vhdl
    // token.
    // True means there is more to parse in this state. Stay in this state.
    bool is_begin_of_element_in_state(tk, ps);

    // Return true if parser, when in the given state, recognises vhdl token as
    // the end of the current state.
    // True means there is nothing to parse in this state. Exit the state.
    bool is_end_of_state(tk, ps);

    // Return true if parser knows how to parse vhdl token.
    // True means that parser knows how to parse the vhdl token. Dont skip vhdl
    // token, and instead, exit this state.
    bool is_valid_in_parsing_states(tk);

    //
    bool is_begin_element_of_design_unit_in_design_file(tk);
    bool is_begin_element_of_interface_lists(tk);
    bool is_begin_element_of_declarative_part_begin_end(tk);
    bool is_begin_element_of_declarative_part_begin(tk);
    bool is_begin_element_of_declarative_part_end(tk);
    bool is_begin_element_of_declarative_part_for(tk);
    bool is_begin_element_of_entity_statement_part(tk);
    bool is_begin_element_of_concurrent_statements(tk);
    bool is_begin_element_of_sequential_statements(tk);

    bool is_end_of_design_unit_in_design_file(tk);
    bool is_end_of_interface_lists(tk);
    bool is_end_of_declarative_part_begin_end(tk);
    bool is_end_of_declarative_part_begin(tk);
    bool is_end_of_declarative_part_end(tk);
    bool is_end_of_declarative_part_for(tk);
    bool is_end_of_entity_statement_part(tk);
    bool is_end_of_concurrent_statements(tk);
    bool is_end_of_sequential_statements(tk);

    template <typename T, typename F>
    std::vector<T> parse_many(ps state, F&& fn)
    {
        auto saved = parsing_states;
        parsing_states.set(static_cast<size_t>(state), true);

        std::vector<T> result;
        while (!is_end_of_state(current_token(), state))
        {
            if (is_begin_of_element_in_state(current_token(), state))
            {
                if (auto element = fn())
                {
                    result.push_back(element);
                }
                continue;
            }
            // handle errors
            if (is_valid_in_parsing_states(current_token()))
                break;

            constexpr std::string_view msg = "Skipping '{}'";
            diag(msg) << lexer_.current_token().to_string();

            skip();
        }

        parsing_states = saved;

        return result;
    }

    template <typename T, typename F>
    std::vector<T> parse_many_sep(ps state, vhdl::token::kind_t sep, F&& fn)
    {
        auto saved = parsing_states;
        parsing_states.set(static_cast<size_t>(state), true);

        std::vector<T> result;
        while (!is_end_of_state(current_token(), state))
        {
            if (is_begin_of_element_in_state(current_token(), state))
            {
                if (auto element = fn())
                {
                    result.push_back(element);
                }

                if (current_token() == sep && is_end_of_state(peek(), state))
                {
                    constexpr std::string_view msg = "Ignoring {}";
                    diag(msg) << lexer_.current_token().to_string();
                    skip();
                    break;
                }

                if (is_begin_of_element_in_state(current_token(), state))
                {
                    expecting(sep);
                }

                optional(sep);
                continue;
            }
            // handle errors
            if (is_valid_in_parsing_states(current_token()))
                break;

            constexpr std::string_view msg = "Skipping '{}'";
            diag(msg) << lexer_.current_token().to_string();

            skip();
        }

        parsing_states = saved;

        return result;
    }

    vhdl::syntax::design_file* file_;
    vhdl::lexer lexer_;
    std::vector<common::diagnostic> diagnostics;

    version version_ = vhdl93;
};

}

#endif
