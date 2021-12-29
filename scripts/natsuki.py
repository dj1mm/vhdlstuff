
#
# Project: vhdlstuff
#
# vhdlstuff is a vhdl parser/analyser
#
# Natsuki is a generator that takes in dsl and outputs C++ code.
# dsl describes the structure of vhdlstuff's asts.
#
# See tools/natsuki.md for a better description of the dsl
#

from typing import Union, Dict, List, Optional

import argparse
import copy
from enum import Enum
from datetime import datetime

# =============================
#   ERRORS AND HELPER CLASSES
# =============================

class Error(Enum):
    # parse errors
    AstNotVisitable = "Unable to do action because ast is not visitable"
    Attribute_redefinition = "Redefinition of {}"
    CType_redefinition = "Redefinition of {}"
    Expecting_this_got_that = "Expected {}"
    Field_redefined = "Redefinition of field {}.{}"
    Union_of_union_not_allowed = "Union of union not allowed {}"
    Field_unknown_type = "Field {} has unknown type {}"
    Field_union_cannot_be_decorated = "Field {} cannot be decorated with {}"
    GenerateNotImplemented = "Generate function not implemented"
    Map_key_shud_b_ctype = "Key {} of map {} must be a custom type"
    Name_ad_ctype = "{} already defines a custom type"
    Name_ad_enum = "{} already defines an enum"
    Name_ad_node = "{} already defines a node"
    Name_ad_aggr = "{} already defines a union"
    No_map_array_field = "Field {} cannot be array and map"
    No_optional_array_field = "Field {} cannot be optional and array"
    No_optional_cowned_field = "Field {} cannot be optionally conditionally owned"
    No_optional_map_field = "Field {} cannot be optional and map"
    No_array_mmap_field = "Field {} cannot be an array of multimaps"
    No_map_mmap_field = "Field {} cannot be map of multimaps"
    No_optional_mmap_field = "Field {} cannot be a multimap of optionals"
    No_optional_reference_field = "Field {} cannot be optional and reference"
    Subnode_of_subnode = "Subnode {} of subnodes {} not ok"
    Subnode_parent_of_subnode = "Subnode {} cannot be a parent to other subnode {}"
    Trait_redefines_field = "Trait {} of {} redefines a field of node {}"
    Unexpected_token = "Unexpected token"
    Unknown_option = "Unknown option"
    UnsupportedFeature = "Feature '{}' is not yet supported"

class LexerError(Exception):
    def __init__(self, message="unexpected error"):
        self.message = f'{self.__class__.__name__}: {message}'

class ParserError(Exception):
    def __init__(self, message="unexpected error"):
        self.message = f'{self.__class__.__name__}: {message}'

class GeneratorError(Exception):
    def __init__(self, message="unexpected error"):
        self.message = f'{self.__class__.__name__}: {message}'


class TokenType(Enum):
    # single-character token types
    LCARET          = '<'
    RCARET          = '>'
    LPAREN          = '('
    RPAREN          = ')'
    LBRACKET        = '{'
    RBRACKET        = '}'
    LSQUARE         = '['
    RSQUARE         = ']'
    HASH            = '#'
    COLON           = ':'
    AND             = '&'
    TIMES           = '*'
    COMMA           = ','
    SEMICOLON       = ';'
    QUESTION        = '?'
    SINGLEQUOTE     = '\''
    DOUBLEQUOTE     = '"'
    EQUALS          = '='
    ARROW           = '->'
    ATTRIBUTE_BEGIN = '[['
    ATTRIBUTE_END   = ']]'
    # block of reserved words
    CLASS           = 'CLASS'
    ENUM            = 'ENUM'
    UNION           = 'UNION'
    USING           = 'USING'
    PUBLIC          = 'PUBLIC'
    PRIVATE         = 'PRIVATE'
    INCLUDE         = 'INCLUDE'
    DEFINE          = 'DEFINE'
    # misc
    VERBATIM        = 'VERBATIM'
    ID              = 'ID'
    FOR             = 'FOR'
    EOF             = 'EOF'
    INVALID         = 'INVALID'


class Token:
    def __init__(self, type: TokenType, value: str, lineno: int, column: int):
        self.type = type
        self.value = value
        self.lineno = lineno
        self.column = column

    def __str__(self):
        """
        String representation of the Token instance.
        Example:
            >>> Token(TokenType.INTEGER, 7, lineno=5, column=10)
            Token(TokenType.INTEGER, 7, position=5:10)
        """
        return 'Token({type}, {value}, position={lineno}:{column})'.format(
            type=self.type,
            value=repr(self.value),
            lineno=self.lineno,
            column=self.column,
        )

    def __repr__(self):
        return self.__str__()


def _build_reserved_keywords():
    """
    Build a dictionary of reserved keywords.
    """
    tt_list = list(TokenType)
    start_index = tt_list.index(TokenType.CLASS)
    end_index = tt_list.index(TokenType.DEFINE)
    reserved_keywords = {
        token_type.value: token_type
        for token_type in tt_list[start_index:end_index + 1]
    }
    return reserved_keywords


RESERVED_KEYWORDS = _build_reserved_keywords()

# =========
#   LEXER
# =========

class Lexer:
    text: str = ""         # client string input
    pos: int = 0           # self.pos is an index into self.text
    current_char: str = "" # none represents eof
    lineno: int = 1        # token line number and column number
    column: int = 1

    def __init__(self, text):
        self.text = text
        self.current_char = self.text[self.pos]

    def error(self):
        s = "Lexer error on '{lexeme}' line: {lineno} column: {column}".format(
            lexeme=self.current_char,
            lineno=self.lineno,
            column=self.column,
        )
        raise LexerError(message=s)

    def advance(self):
        """Advance the `pos` pointer and set the `current_char` variable."""
        if self.current_char == '\n':
            self.lineno += 1
            self.column = 0

        self.pos += 1
        if self.pos > len(self.text) - 1:
            self.current_char = ""  # Indicates end of input
        else:
            self.current_char = self.text[self.pos]
            self.column += 1

    def peek(self):
        pos = self.pos + 1
        if pos > len(self.text) - 1:
            return ""
        else:
            return self.text[pos]

    def skip_whitespace(self):
        while self.current_char != "" and self.current_char.isspace():
            self.advance()

    def _id(self):
        """Handle identifiers and reserved keywords"""

        # Create a new token with current line and column number
        token = Token(TokenType.INVALID, "", self.lineno, self.column)

        value = ''
        while self.current_char != "":
            if not self.current_char.isalnum() and self.current_char not in ['_']:
                break
            value += self.current_char
            self.advance()

        token_type = RESERVED_KEYWORDS.get(value.upper())
        if token_type is None:
            token.type = TokenType.ID
            token.value = value
        else:
            # reserved keyword
            token.type = token_type
            token.value = value.upper()

        return token

    def next_token(self,
                   expected: Optional[TokenType] = None,
                   nestin: Optional[List[str]] = None,
                   nestout: Optional[List[str]] = None,
                   until: Optional[List[str]] = None):
        """
        get the next token. it takes no arguments, or 4 arguments
        In the latter case, next_token() scans the input stream until `until`
        is met. In that case, it returns the `expected` as token.

        THis is used to parse verbatim sections. We know the next token should
        be verbatim. So tell the lexer to keep parsing until the next `}` is
        found. Also does nest in and out. Check the code.
        """
        while self.current_char != "":

            if expected is not None:
                nestin = [] if nestin is None else nestin
                nestout = [] if nestout is None else nestout
                assert(until is not None)

                nest = 0

                text = ""
                while self.current_char not in until or nest != 0:
                    if self.current_char in nestin:
                        nest = nest + 1
                    elif self.current_char in nestout:
                        nest = nest - 1
                    text += self.current_char
                    self.advance()

                return Token(expected, text, self.lineno, self.column)

            if self.current_char == '/' and self.peek() == '/':
                while self.current_char not in ['\r', '\n']:
                    self.advance()

            if self.current_char.isspace():
                self.skip_whitespace()
                continue

            if self.current_char.isalpha() or self.current_char in ['_']:
                return self._id()

            if self.current_char == '-' and self.peek() =='>':
                token_type = TokenType.ARROW
                token = Token(token_type, '->', self.lineno, self.column)
                self.advance()
                self.advance()
                return token

            if self.current_char == '[' and self.peek() =='[':
                token_type = TokenType.ATTRIBUTE_BEGIN
                token = Token(token_type, '[[', self.lineno, self.column)
                self.advance()
                self.advance()
                return token

            if self.current_char == ']' and self.peek() ==']':
                token_type = TokenType.ATTRIBUTE_END
                token = Token(token_type, ']]', self.lineno, self.column)
                self.advance()
                self.advance()
                return token

            try:
                # get enum member by value
                token_type = TokenType(self.current_char)
            except ValueError:
                # no enum member with value equal to self.current_char
                self.error()
            else:
                # create a token with a single-character lexeme as its value
                token = Token(token_type, token_type.value, self.lineno, self.column)
                self.advance()
                return token

        # if we r here, its an eof
        return Token(TokenType.EOF, "", self.lineno, self.column)

# ==========
#   PARSER
# ==========

class AST:
    pass


class Field(AST):
    class Access(Enum):
        Pointer = 0
        Object = 1
    class Ownership(Enum):
        Full = 0
        Reference = 1
        Conditional = 2
    class Model(Enum):
        Node = 0
        Enum = 1
        Type = 2
        Aggr = 3
    class Container(Enum):
        Value = 0
        Optional = 1
        Map = 2
        Array = 3
        DoubleArray = 4
        MultiMap = 5

    """
    A natsuki field has a name and a value of type spcified.
    """
    def __init__(self, name: str, tipe: str, attributes: Dict[str, str], default: Optional[str]):
        self.name = name
        self.type = tipe
        self.attributes = attributes
        self.default = default

        self.ownership: Optional[Ownership] = None
        self.access: Optional[Access] = None
        self.container: Optional[Container] = None
        self.model: Optional[Model] = None

        self.is_visitable: bool = False
        self.resolved_type: Union[Node,Enumeration,CustomType,None] = None
        self.node: Optional[Node] = None

        # the display name is the name the Debug_Dot_Cpp_Generator uses to
        # field names.
        #
        # Usually this is not needed as field.name is not changed when
        # generating c++ code. However, Debug_Dot_Cpp_Generator has to change
        # the name of union fields. Display name is here so that natsuki knows
        # the original name of the field
        self.display_name = name

class Enumeration(AST):
    """
    An enumeration is useful to specify a field which can only be one of a
    known discrete value.
    """
    def __init__(self, name: str, values: List[str]):
        self.name = name
        self.values = values

        # the fully qualified name of the generated code. It should be of the
        # format namespace::name
        self.fully_qualified_name = name

class CustomType(AST):
    """
    By default natsuki types are nodes, subnodes or enumerations. Custom types
    all the user to use non-natsuki types in the dsl
    """
    def __init__(self, name: str, tipe: str, default: Optional[str]):
        self.name = name
        self.type = tipe
        self.default = default

class Aggr(AST):
    """
    A union is helpful when there is a need to specify multiple variants of a
    node without the use of c++ inheritance.
    """

    def __init__(self, name: str):
        self.name = name
        self.nodes: List[Node] = []

class Node(AST):
    """
    A node has a name, a bunch of fields, a bunch of verbatims and a bunch of
    subnodes.
    """

    def __init__(self, name: str, attributes: Dict[str, str]):
        self.name = name
        self.attributes = attributes

        self.guards: Dict[str,List[str]] = {}
        self.public: List[Verbatim] = []
        self.private: List[Verbatim] = []
        self.parents: List[str] = []
        self.children: List[str] = []
        self.fields: List[Field] = []

        self.resolved_parents: List[Node] = []
        self.resolved_children: List[Node] = []

        # the fully qualified name of the generated code. It should be of the
        # format namespace::name
        self.fully_qualified_name = name

class Verbatim(AST):
    def __init__(self, content: str, attributes: Dict[str, str]) -> None:
        self.content = content
        self.attributes = attributes

class File(AST):
    def __init__(self):
        self.includes: List[str] = []
        self.options: Dict[str, str] = {}
        self.declarations: List[Union[Node,Enumeration,CustomType]] = []
        self.public: List[str] = []
        self.private: List[str] = []

class Parser:

    def __init__(self, lexer):
        self.lexer: Lexer = lexer
        self.current_token: Token = self.lexer.next_token()

        # place holder for natsuki types while parsing
        self.nodes: Dict[str, Node] = {}
        self.enums: Dict[str, Enumeration] = {}
        self.types: Dict[str, CustomType] = {}
        self.aggrs: Dict[str, Aggr] = {}

        self.file: File = File()

    def error(self, error: Error,
              args: Union[List[str], Dict[str,str]]=[],
              token: Optional[Token] = None
              ):
        """
        raise a parse error.
        Error takes the error code & args which is either a str[], or str<str>
        """
        token = self.current_token if token is None else token
        if isinstance(args, list):
            message = error.value.format(*args)
        else:
            message = error.value.format(**args)

        raise ParserError(f'{token} -> {message}')

    def consume(self, token_type = None,
                expected: Optional[TokenType] = None,
                nestin: Optional[List[str]] = None,
                nestout: Optional[List[str]] = None,
                until: Optional[List[str]] = None):
        """
        consume the current token.
        Update the parser.current_token with the next token.

        Function takes no arguments, or 4 arguments. In the latter case,consume
        scans the input stream until `until` is met. In that case, it returns
        the `expected` as token.

        THis is used to parse verbatim sections. We know the next token should
        be verbatim. So tell the lexer to keep parsing until the next `}` is
        found. Also does nest in and out. Check the code.
        """
        if token_type is not None and self.current_token.type != token_type:
            self.error(Error.Expecting_this_got_that, [token_type])
        self.current_token = self.lexer.next_token(expected, nestin, nestout, until)

    def parse_natsuki_file(self):
        """ natsuki_file ::= declaration { declaration } """
        while self.current_token.type != TokenType.EOF:
            self.parse_declaration()

    def parse_declaration(self):
        """ declaration ::= include | attributes | type """
        if self.current_token.type == TokenType.HASH:
            self.parse_include_or_custom_type()
        elif self.current_token.type == TokenType.ATTRIBUTE_BEGIN:
            self.parse_file_attributes_or_node_attributes()
        elif self.current_token.type == TokenType.CLASS:
            self.parse_node()
        elif self.current_token.type == TokenType.ENUM:
            self.parse_enumeration()
        elif self.current_token.type == TokenType.UNION:
            self.parse_union()
        elif self.current_token.type in [TokenType.PUBLIC, TokenType.PRIVATE]:
            public, private = self.parse_custom_code()
            self.file.public += public
            self.file.private += private
        else:
            self.error(Error.Unexpected_token)

    def parse_include_or_custom_type(self):
        self.consume(TokenType.HASH)

        if self.current_token.type == TokenType.DEFINE:
            self.parse_custom_type()
        else:
            self.parse_include()

    def parse_custom_type(self):
        self.consume(TokenType.DEFINE)
        name = self.current_token.value

        self.consume(TokenType.ID, TokenType.ID, [], [], ['\n', '='])
        if name in self.enums:
            self.error(Error.Name_ad_enum, [name])
        if name in self.types:
            self.error(Error.CType_redefinition, [name])
        if name in self.aggrs:
            self.error(Error.Name_ad_aggr, [name])

        tipe = self.current_token.value.strip()
        self.consume(TokenType.ID)

        default = None
        if self.current_token.type == TokenType.EQUALS:
            self.consume(TokenType.EQUALS, TokenType.ID, [], [], ['\n'])
            default = self.current_token.value.strip()
            self.consume(TokenType.ID)

        self.types[name] = CustomType(name, tipe, default)

    def parse_include(self):
        self.consume(TokenType.INCLUDE, TokenType.ID, [], [], ['\n'])

        include = self.current_token.value.strip()
        self.file.includes += [include]

        self.consume(TokenType.ID)

    def parse_file_attributes_or_node_attributes(self):
        attributes = self.parse_attributes()

        if self.current_token.type == TokenType.SEMICOLON:
            self.consume(TokenType.SEMICOLON)
            self.file.options.update(attributes)

        elif self.current_token.type == TokenType.CLASS:
            self.parse_node(attributes)

        else:
            self.error(Error.Unexpected_token)

    def parse_union(self) -> str:
        """
        union ::= UNION name { union_type { union_type } } ;
        union_type ::= CLASS union_node_name [ fields ] ;

        Parse union.
        """

        self.consume(TokenType.UNION)

        name = self.current_token.value
        self.consume(TokenType.ID)

        expect_multiple_classes = self.current_token.type == TokenType.LBRACKET
        if expect_multiple_classes:
            self.consume(TokenType.LBRACKET)

        if name in self.enums:
            self.error(Error.Name_ad_enum, [name])
        if name in self.types:
            self.error(Error.Name_ad_ctype, [name])
        if name in self.nodes:
            self.error(Error.Name_ad_node, [name])
        if name not in self.aggrs:
            self.aggrs[name] = Aggr(name)

        union_node_names = []

        while self.current_token.type == TokenType.CLASS or not expect_multiple_classes:

            # parse union_type
            if expect_multiple_classes:
                self.consume(TokenType.CLASS)

            union_node_name = self.current_token.value
            self.consume(TokenType.ID)

            if union_node_name in self.enums:
                self.error(Error.Name_ad_enum, [union_node_name])
            if union_node_name in self.types:
                self.error(Error.Name_ad_ctype, [union_node_name])
            if union_node_name in self.aggrs:
                self.error(Error.Name_ad_aggr, [union_node_name])
            if union_node_name in union_node_names and expect_multiple_classes:
                self.error(Error.Name_ad_union_node, [union_node_name])

            union_node_names += [union_node_name]

            node = None
            for n in self.aggrs[name].nodes:
                if n.name == union_node_name:
                    node = n
                    break

            if node is None:
                node = Node(union_node_name, {})
                self.aggrs[name].nodes.append(node)

            fields = self.parse_fields()
            for field in node.fields:
                for f in fields:
                    if field.name == f.name:
                        self.error(Error.Field_redefined, [name, f.name])
            node.fields += fields

            if expect_multiple_classes:
                self.consume(TokenType.SEMICOLON)

            if not expect_multiple_classes:
                break

        if expect_multiple_classes:
            self.consume(TokenType.RBRACKET)

        self.consume(TokenType.SEMICOLON)

        return name

    def parse_node(self, attributes: Dict[str,str] = {}, parent: str = "") -> str:
        """
        class ::= [ attributes ] 
                  CLASS name [ fields ] [ custom_code ] ; { subnodees }

        Parse node assumes attributes have already been parsed, and are given
        as an argument.
        If parsing a subnode, pass the name of the parent as an arg
        """

        self.consume(TokenType.CLASS)

        name = self.current_token.value
        self.consume(TokenType.ID)

        if name in self.enums:
            self.error(Error.Name_ad_enum, [name])
        if name in self.types:
            self.error(Error.Name_ad_ctype, [name])
        if name in self.aggrs:
            self.error(Error.Name_ad_aggr, [name])
        if name not in self.nodes:
            self.nodes[name] = Node(name, attributes)

        fields = self.parse_fields()
        for field in self.nodes[name].fields:
            for f in fields:
                if field.name == f.name:
                    self.error(Error.Field_redefined, [name, f.name])
        self.nodes[name].fields += fields

        public, private = self.parse_custom_code()

        self.nodes[name].public += public
        self.nodes[name].private += private

        self.consume(TokenType.SEMICOLON)

        if parent == "":
            while self.current_token.type == TokenType.ARROW:
                self.parse_subnode(name)

        return name

    def parse_fields(self):
        """ fields ::= ( [ field { , field } ] ) """
        if self.current_token.type != TokenType.LPAREN:
            return []

        self.consume(TokenType.LPAREN)

        fields = []
        if self.current_token.type in [TokenType.ID,TokenType.ATTRIBUTE_BEGIN]:
            fields += self.parse_field()

            while self.current_token.type == TokenType.COMMA:
                self.consume(TokenType.COMMA)
                fields += self.parse_field()

        self.consume(TokenType.RPAREN)

        return fields

    def parse_field(self):
        """
        field ::= [ attributes ] names [?] [&] : type [ [] { [] }] [ = default ]
        names ::= name { name }
        name ::= [a-zA-Z_]+
        type ::= [a-zA-Z_]+
        default ::= any
        """

        attributes: Dict[str,str] = {}
        if self.current_token.type == TokenType.ATTRIBUTE_BEGIN:
            attributes = self.parse_attributes()

        names = []
        names += [self.current_token.value]
        self.consume(TokenType.ID)
        while self.current_token.type == TokenType.COMMA:
            self.consume(TokenType.COMMA)
            names += [self.current_token.value]
            self.consume(TokenType.ID)

        if self.current_token.type == TokenType.QUESTION:
            attributes["optional"] = ""
            self.consume(TokenType.QUESTION)
        if self.current_token.type == TokenType.AND:
            attributes["reference"] = ""
            self.consume(TokenType.AND)

        self.consume(TokenType.COLON)

        tipe = self.current_token.value
        self.consume(TokenType.ID)

        if self.current_token.type == TokenType.LSQUARE:
            self.consume(TokenType.LSQUARE)
            self.consume(TokenType.RSQUARE)
            if self.current_token.type == TokenType.LSQUARE:
                self.consume(TokenType.LSQUARE)
                self.consume(TokenType.RSQUARE)
                attributes["darray"] = ""
            else:
                attributes["array"] = ""

        if self.current_token.type == TokenType.LCARET:
            self.consume(TokenType.LCARET)
            attributes["map"] = self.current_token.value
            self.consume(TokenType.ID)
            self.consume(TokenType.RCARET)

        if self.current_token.type == TokenType.LBRACKET:
            self.consume(TokenType.LBRACKET)
            attributes["mmap"] = self.current_token.value
            self.consume(TokenType.ID)
            self.consume(TokenType.RBRACKET)

        default = None
        if self.current_token.type == TokenType.EQUALS:
            self.consume(TokenType.EQUALS, TokenType.ID, [], [], [',', '\n', ')'])
            default = self.current_token.value.strip()
            self.consume(TokenType.ID)

        return [ Field(name, tipe, attributes, default) for name in names ]

    def parse_custom_code(self):
        """
        custom_code ::= NOTHING
                        | custom_code private_custom_code
                        | custom_code public_custom_code

        private_custom_code ::= { .* } | PRIVATE { .* }
        public_custom_code ::= PUBLIC { .* }
        """
        public = []
        private = []

        while self.current_token.type in [TokenType.PUBLIC, TokenType.PRIVATE, TokenType.ATTRIBUTE_BEGIN]:
            attributes: Dict[str,str] = {}
            if self.current_token.type == TokenType.ATTRIBUTE_BEGIN:
                attributes = self.parse_attributes()

            if self.current_token.type == TokenType.PUBLIC:
                self.consume(TokenType.PUBLIC)
                self.consume(TokenType.COLON)
                public += [self._parse_custom_code(attributes)]

            if self.current_token.type == TokenType.PRIVATE:
                if self.current_token.type == TokenType.PRIVATE:
                    self.consume(TokenType.PRIVATE)
                    self.consume(TokenType.COLON)
                private += [self._parse_custom_code(attributes)]

        return public, private

    def _parse_custom_code(self, attributes: Dict[str,str] = {}):
        self.consume(TokenType.LBRACKET,TokenType.VERBATIM,['{'],['}'],['}'])
        code = self.current_token.value
        self.consume(TokenType.VERBATIM)
        self.consume(TokenType.RBRACKET)
        return Verbatim(code, attributes)

    def parse_subnode(self, parent) -> str:
        """ subnodees ::= subnode { subnode }
        
        Helper function to parse subnode. Accepts the name of its parent.
        This function consumes the -> before simply re-calling parse_node
        """
        self.consume(TokenType.ARROW)

        attributes: Dict[str,str] = {}
        if self.current_token.type == TokenType.ATTRIBUTE_BEGIN:
            attributes = self.parse_attributes()

        name = self.parse_node(attributes, parent)

        self.nodes[parent].children += [ name ]
        self.nodes[name].parents += [ parent ]

        if self.nodes[parent].parents != []:
            self.error(Error.Subnode_of_subnode, [name, parent])

        return name

    def parse_attributes(self) -> Dict[str, str]:
        """ attributes ::= [[ attribute { , attribute } ]] """
        self.consume(TokenType.ATTRIBUTE_BEGIN)

        attributes: Dict[str, str] = {}

        # parse attribute
        attributes = self.parse_attribute(attributes)

        while self.current_token.type == TokenType.COMMA:
            self.consume(TokenType.COMMA)

            # parse other attributes
            attributes = self.parse_attribute(attributes)

        self.consume(TokenType.ATTRIBUTE_END)

        return attributes

    def parse_attribute(self, attributes: Dict[str, str]) -> Dict[str, str]:
        """
        Current attributes as an argument. Return it after parsing attributes
        """
        key = self.current_token.value
        self.consume(TokenType.ID)

        value = ""
        if self.current_token.type == TokenType.EQUALS:
            self.consume(TokenType.EQUALS, TokenType.ID, None, None, [',',']'])
            value = self.current_token.value
            self.consume(TokenType.ID)
        attributes[key] = value

        return attributes

    def parse_enumeration(self):
        """
        enumeration ::= ENUM name
                        { enumeration_literal { , enumeration_literal } } ;
        """
        self.consume(TokenType.ENUM)

        name = self.current_token.value
        self.consume(TokenType.ID)

        if name in self.nodes:
            self.error(Error.Name_ad_node, [name])
        if name in self.types:
            self.error(Error.Name_ad_ctype, [name])
        if name in self.enums:
            self.error(Error.Name_ad_enum, [name])
        if name in self.aggrs:
            self.error(Error.Name_ad_aggr, [name])


        literals: List[str] = []
        self.consume(TokenType.LBRACKET)
        while self.current_token.type == TokenType.ID:
            literals += [self.current_token.value]
            self.consume(TokenType.ID)

            if self.current_token.type != TokenType.COMMA:
                break

            self.consume(TokenType.COMMA)

        self.consume(TokenType.RBRACKET)
        self.consume(TokenType.SEMICOLON)

        self.enums[name] = Enumeration(name, literals)

    def sanity_check(self):
        """
        Do sanity checks
        Raise a parse error if something went wrong

        - check that nodes/subnodes are not both parents and children atst
        - handle trait fields in parents. Copy the field into all its children
        - add the guards of each node's conditionally onwed fields
        - check field type is a valid one
        - update the internal representation of fields. Optional, pointer? etc
        """

        # Update the fully qualified name if a namespace was provided
        if 'namespace' in self.file.options:
            for _, node in self.nodes.items():
                node.fully_qualified_name = '{}::{}'.format(self.file.options['namespace'], node.name)

            for _, enum in self.enums.items():
                enum.fully_qualified_name = '{}::{}'.format(self.file.options['namespace'], enum.name)

        for _, node in self.nodes.items():

            if node.parents != [] and node.children != []:
                self.error(Error.Subnode_parent_of_subnode, [node.name, str(node.children)])

            # handle parent traits/interfaces - remember, traits here isnt the
            # same thing as cpp type traits. traits for natsuki is rather like
            # the php trait, or c# interface (a way to enable code reuse)
            for parent in node.parents:
                # no need to check null, parent shud exist if it was in nodes
                parent = self.nodes[parent]
                node.resolved_parents += [parent]
                for field in parent.fields:
                    if 'istrait' not in field.attributes:
                        continue
                    for f in node.fields:
                        if field.name == f.name:
                            self.error(Error.Trait_redefines_field, [field.name, parent.name, node.name])
                    field = copy.deepcopy(field)
                    del field.attributes['istrait']
                    field.attributes['wastrait'] = ""
                    node.fields += [field]
                for private in parent.private:
                    if 'istrait' not in private.attributes:
                        continue
                    private = copy.deepcopy(private)
                    del private.attributes['istrait']
                    private.attributes['wastrait'] = ""
                    node.private += [private]
                for public in parent.public:
                    if 'istrait' not in private.attributes:
                        continue
                    public = copy.deepcopy(public)
                    del public.attributes['istrait']
                    public.attributes['wastrait'] = ""
                    node.public += [public]

            for child in node.children:
                # no need to check null, child shud exist if it was in nodes
                child = self.nodes[child]
                node.resolved_children += [child]

            # handle conditionally owned fields
            for field in node.fields:
                if 'cowned' not in field.attributes:
                    continue
                if field.attributes['cowned'] == '':
                    field.attributes['cowned'] = '_owns_fields'
                guard = field.attributes['cowned']
                if guard in node.guards:
                    continue
                fields: List[str] = [f.name for f in node.fields if 'cowned' in f.attributes]
                node.guards[guard] = fields

            # check every fields of the node, reporting errors if found. Also
            # check fields imported from traits
            for field in node.fields:

                field.node = node

                # fix field's memory model
                if field.type in self.nodes:
                    field.model = Field.Model.Node
                    field.resolved_type = self.nodes[field.type]
                elif field.type in self.enums:
                    field.model = Field.Model.Enum
                    field.resolved_type = self.enums[field.type]
                elif field.type in self.types:
                    field.model = Field.Model.Type
                    field.resolved_type = self.types[field.type]
                elif field.type in self.aggrs:
                    field.model = Field.Model.Aggr
                    field.resolved_type = self.aggrs[field.type]
                else:
                    self.error(Error.Field_unknown_type, [field.name, field.type])

                # custom types - specified by #define custom_type
                # custom pointers - specified by #define custom_type. but are *
                # natsuki types - specified by natsuki type definitions
                if field.model is Field.Model.Type and field.resolved_type.type[-1] == "*":
                    field.ownership              = Field.Ownership.Full
                    field.access                 = Field.Access.Pointer
                    field.container              = Field.Container.Value
                    field.is_visitable           = False
                elif field.model is Field.Model.Type and field.resolved_type.type[-1] != "*":
                    field.ownership              = Field.Ownership.Full
                    field.access                 = Field.Access.Object
                    field.container              = Field.Container.Value
                    field.is_visitable           = False
                elif field.model is Field.Model.Node:
                    field.ownership              = Field.Ownership.Full
                    field.access                 = Field.Access.Pointer
                    field.container              = Field.Container.Value
                    field.is_visitable           = True
                elif field.model is Field.Model.Enum:
                    field.ownership              = Field.Ownership.Full
                    field.container              = Field.Container.Value
                    field.access                 = Field.Access.Object
                    field.is_visitable           = False
                elif field.model is Field.Model.Aggr:
                    field.ownership              = Field.Ownership.Full
                    field.container              = Field.Container.Value
                    field.access                 = Field.Access.Object
                    field.is_visitable           = True
                else:
                    self.error(Error.Field_unknown_type, [field.name, field.type])

                for k in ['optional','reference','array','darray','map','mmap','cowned']:
                    if k in field.attributes and field.model is Field.Model.Aggr:
                        self.error(Error.Field_union_cannot_be_decorated, [field.name, k])

                if {'optional','reference'} <= field.attributes.keys():
                    self.error(Error.No_optional_reference_field, [field.name])

                if {'optional','array'} <= field.attributes.keys():
                    self.error(Error.No_optional_array_field, [field.name])

                if {'optional','darray'} <= field.attributes.keys():
                    self.error(Error.No_optional_array_field, [field.name])

                if {'map','array'} <= field.attributes.keys():
                    self.error(Error.No_map_array_field, [field.name])

                if {'map','darray'} <= field.attributes.keys():
                    self.error(Error.No_map_array_field, [field.name])

                if {'optional','map'} <= field.attributes.keys():
                    self.error(Error.No_optional_map_field, [field.name])

                if {'optional','mmap'} <= field.attributes.keys():
                    self.error(Error.No_optional_mmap_field, [field.name])

                if {'map','mmap'} <= field.attributes.keys():
                    self.error(Error.No_map_mmap_field, [field.name])

                if {'array','mmap'} <= field.attributes.keys():
                    self.error(Error.No_array_mmap_field, [field.name])

                if {'cowned','reference'} <= field.attributes.keys():
                    self.error(Error.No_optional_cowned_field, [field.name])

                if 'map' in field.attributes and field.attributes['map'] not in self.types:
                    self.error(Error.Map_key_shud_b_ctype, [field.attributes['map'], field.name])

                # update the key of the map with the correct type
                if 'map' in field.attributes:
                    field.attributes['map'] = self.types[field.attributes['map']].type

                # update the key of the map with the correct type
                if 'mmap' in field.attributes:
                    field.attributes['mmap'] = self.types[field.attributes['mmap']].type

                # fix field's container property
                if 'map' in field.attributes:
                    field.container = Field.Container.Map
                elif 'mmap' in field.attributes:
                    field.container = Field.Container.MultiMap
                elif 'array' in field.attributes:
                    field.container = Field.Container.Array
                elif 'darray' in field.attributes:
                    field.container = Field.Container.DoubleArray
                elif 'optional' in field.attributes:
                    field.container = Field.Container.Optional
                    field.ownership = Field.Ownership.Full
                else:
                    field.container = Field.Container.Value

                # fix field's ownership property if override specified
                if 'reference' in field.attributes:
                    field.ownership = Field.Ownership.Reference
                    field.is_visitable = False

                if 'cowned' in field.attributes:
                    field.ownership = Field.Ownership.Conditional

                if 'visitable' in field.attributes:
                    field.is_visitable = True

                if 'notvisitable' in field.attributes:
                    field.is_visitable = False

        # clear trait fields and public/private code from nodes
        for _, node in self.nodes.items():
            node.fields = [f for f in node.fields if 'istrait' not in f.attributes]
            node.private = [p for p in node.private if 'istrait' not in p.attributes]
            node.public = [p for p in node.public if 'istrait' not in p.attributes]

        for _, aggr in self.aggrs.items():
            for node in aggr.nodes:
                for field in node.fields:
                    if 'cowned' not in field.attributes:
                        continue
                    if field.attributes['cowned'] == '':
                        field.attributes['cowned'] = '_owns_fields'
                    guard = field.attributes['cowned']
                    if guard in node.guards:
                        continue
                    fields: List[str] = [f.name for f in node.fields if 'cowned' in f.attributes]
                    node.guards[guard] = fields

                # check every fields of the node, reporting errors if found. Also
                # check fields imported from traits
                for field in node.fields:

                    field.node = node

                    # fix field's memory model
                    if field.type in self.nodes:
                        field.model = Field.Model.Node
                        field.resolved_type = self.nodes[field.type]
                    elif field.type in self.enums:
                        field.model = Field.Model.Enum
                        field.resolved_type = self.enums[field.type]
                    elif field.type in self.types:
                        field.model = Field.Model.Type
                        field.resolved_type = self.types[field.type]
                    elif field.type in self.aggrs:
                        self.error(Error.Union_of_union_not_allowed, [field.name])
                    else:
                        self.error(Error.Field_unknown_type, [field.name, field.type])

                    # custom types - specified by #define custom_type
                    # custom pointers - specified by #define custom_type. but are *
                    # natsuki types - specified by natsuki type definitions
                    if field.model is Field.Model.Type and field.resolved_type.type[-1] == "*":
                        field.ownership              = Field.Ownership.Full
                        field.access                 = Field.Access.Pointer
                        field.container              = Field.Container.Value
                        field.is_visitable           = False
                    elif field.model is Field.Model.Type and field.resolved_type.type[-1] != "*":
                        field.ownership              = Field.Ownership.Full
                        field.access                 = Field.Access.Object
                        field.container              = Field.Container.Value
                        field.is_visitable           = False
                    elif field.model is Field.Model.Node:
                        field.ownership              = Field.Ownership.Full
                        field.access                 = Field.Access.Pointer
                        field.container              = Field.Container.Value
                        field.is_visitable           = True
                    elif field.model is Field.Model.Enum:
                        field.ownership              = Field.Ownership.Full
                        field.container              = Field.Container.Value
                        field.access                 = Field.Access.Object
                        field.is_visitable           = False
                    elif field.model is Field.Model.Aggr:
                        self.error(Error.Union_of_union_not_allowed, [field.name])
                    else:
                        self.error(Error.Field_unknown_type, [field.name, field.type])

                    if {'optional','reference'} <= field.attributes.keys():
                        self.error(Error.No_optional_reference_field, [field.name])

                    if {'optional','array'} <= field.attributes.keys():
                        self.error(Error.No_optional_array_field, [field.name])

                    if {'optional','darray'} <= field.attributes.keys():
                        self.error(Error.No_optional_array_field, [field.name])

                    if {'map','array'} <= field.attributes.keys():
                        self.error(Error.No_map_array_field, [field.name])

                    if {'map','darray'} <= field.attributes.keys():
                        self.error(Error.No_map_array_field, [field.name])

                    if {'optional','map'} <= field.attributes.keys():
                        self.error(Error.No_optional_map_field, [field.name])

                    if {'optional','mmap'} <= field.attributes.keys():
                        self.error(Error.No_optional_mmap_field, [field.name])

                    if {'map','mmap'} <= field.attributes.keys():
                        self.error(Error.No_map_mmap_field, [field.name])

                    if {'array','mmap'} <= field.attributes.keys():
                        self.error(Error.No_array_mmap_field, [field.name])

                    if {'cowned','reference'} <= field.attributes.keys():
                        self.error(Error.No_optional_cowned_field, [field.name])

                    if 'map' in field.attributes and field.attributes['map'] not in self.types:
                        self.error(Error.Map_key_shud_b_ctype, [field.attributes['map'], field.name])

                    # update the key of the map with the correct type
                    if 'map' in field.attributes:
                        field.attributes['map'] = self.types[field.attributes['map']].type

                    # update the key of the map with the correct type
                    if 'mmap' in field.attributes:
                        field.attributes['mmap'] = self.types[field.attributes['mmap']].type

                    # fix field's container property
                    if 'map' in field.attributes:
                        field.container = Field.Container.Map
                    elif 'mmap' in field.attributes:
                        field.container = Field.Container.MultiMap
                    elif 'array' in field.attributes:
                        field.container = Field.Container.Array
                    elif 'darray' in field.attributes:
                        field.container = Field.Container.DoubleArray
                    elif 'optional' in field.attributes:
                        field.container = Field.Container.Optional
                        field.ownership = Field.Ownership.Full
                    else:
                        field.container = Field.Container.Value

                    # fix field's ownership property if override specified
                    if 'reference' in field.attributes:
                        field.ownership = Field.Ownership.Reference
                        field.is_visitable = False

                    if 'cowned' in field.attributes:
                        field.ownership = Field.Ownership.Conditional

                    if 'visitable' in field.attributes:
                        field.is_visitable = True

                    if 'notvisitable' in field.attributes:
                        field.is_visitable = False

    def commit_to_file(self):
        """
        Takes all the enumerations, nodes, subnodes. Reorder their declarations
        to minimise need of forward decls. Put em in a file.declaration
        """
        processed: List[str] = []

        for name, enum in self.enums.items():
            if name in processed:
                continue
            self.file.declarations += [enum]
            processed += [name]

        for name, node in self.nodes.items():
            if name in processed:
                continue
            for parent in node.parents:
                if parent in processed:
                    continue
                self.file.declarations += [self.nodes[parent]]
                processed += [parent]

            self.file.declarations += [node]
            processed += [name]

    def parse(self):
        """
        natsuki_file ::= declaration { declaration }
        declaration ::= option | class

        class ::= CLASS name [ fields ] [ custom_code ] ; { subnodees }
        subnode ::= -> CLASS name [ fields ] [ custom_code ] ;

        fields ::= ( [ field { , field } ] )
        field ::= name [?] [&] : type [ [] ]
        name ::= [a-zA-Z_]+
        type ::= [a-zA-Z_]+

        custom_code ::= private_custom_code [ public_custom_code ]
                        | public_custom_code [ private_custom_code ]

        private_custom_code ::= { .* } | PRIVATE { .* }
        public_custom_code ::= PUBLIC { .* }

        subnodees ::= subnode { subnode }
        """
        self.parse_natsuki_file()
        if self.current_token.type != TokenType.EOF:
            self.error(Error.Unexpected_token)

        self.sanity_check()
        self.commit_to_file()

        return self.file

# ============
#   GENERATOR
# ============

class Generator:

    def __init__(self, file, output):
        self.file = file
        self.name = "ast" if output is None else output.rsplit('/')[-1]
        self.ns = "" if 'namespace' not in self.file.options else f"{self.file.options['namespace']}::"

    def error(self, error: Error, args: Union[List[str], Dict[str,str]]=[]):
        if isinstance(args, list):
            message = error.value.format(*args)
        else:
            message = error.value.format(**args)

        raise GeneratorError(f'{type(self).__name__} -> {message}')

    def generate(self):
        self.error(Error.GenerateNotImplemented)

class Debug_Dot_H_Generator(Generator):

    def generate(self):

        # debug is possible only if this ast is visitable
        if 'visitable' not in self.file.options:
            self.error(Error.AstNotVisitable)

        random_guard_name = "NATSUKI_GENERATED_{}_DEBUG_{}_H".format(self.name.upper(),datetime.now().strftime("%Y%m%d%H%M%S"))

        result = [""]
        result += ["#ifndef {}".format(random_guard_name)]
        result += ["#define {}".format(random_guard_name)]
        result += [""]
        result += ["#include <stack>"]
        result += ["#include <iostream>"] # fixme; this should be sstream
        result += [""]

        if 'namespace' in self.file.options:
            result += ["namespace {}".format(self.file.options['namespace'])]
            result += ["{"]
            result += [""]

        result += ["// forward declarations"]
        for decl in self.file.declarations:
            if not isinstance(decl, Node):
                continue
            result += ["class {};".format(decl.name)]
        result += [""]

        result += ["class {}_debug: public visitor".format(self.name)]
        result += ["{"]
        result += ["    struct _ctx { int indent; bool array; bool reference; };"]
        result += ["    class new_context {"]
        result += [f"        {self.name}_debug* visitor;"]
        result += ["        public:"]
        result += [f"        new_context({self.name}_debug* v, bool a = false, bool r = false);"]
        result += ["        ~new_context();"]
        result += ["    };"]
        result += [f"    friend class {self.name}_debug::new_context;"]
        result += ["    std::stack<_ctx> current_context;"]
        result += [""]
        result += ["    std::ostream& mOutput;"]
        result += ["    int mOffset;"]
        result += ["    public:"]
        result += ["    {}_debug(std::ostream& output, int offset = 2);".format(self.name)]
        result += ["    ~{}_debug() {{}};".format(self.name)]
        result += ["    std::ostream& title();"]
        result += ["    std::ostream& indent();"]
        result += ["    std::ostream& output();"]
        result += ["    std::ostream& nl();"]
        result += [""]

        for decl in self.file.declarations:
            if isinstance(decl, Node):
                result += self.generate_node(decl)
        result += ["};"]
        result += [""]

        # almost done. if there was a custom namespace, close it
        if 'namespace' in self.file.options:
            result += ["}"]

        result += [""]
        result += ["#endif"]
        result += [""]
        return '\n'.join(result)
        return result

    def generate_node(self, node):
        result = []

        if node.children != []:
            return result

        result += ["    virtual bool visit({}* {});".format(node.fully_qualified_name, node.name[0])]
        return result

class Debug_Dot_Cpp_Generator(Generator):

    def generate(self):

        # add the traverse method if this the ast is visitable
        if 'visitable' not in self.file.options:
            self.error(Error.AstNotVisitable)

        result = [""]
        result += ["#include \"{}.h\"".format(self.name)]
        result += ["#include \"{}_debug.h\"".format(self.name)]
        result += [""]

        if 'namespace' in self.file.options:
            result += ["using namespace {};".format(self.file.options['namespace'])]
            result += [""]

        result += [f"{self.name}_debug::new_context::new_context({self.name}_debug* v, bool a, bool r)"]
        result += [": visitor(v)"]
        result += ["{"]
        result += ["    visitor->current_context.push({visitor->current_context.top().indent + visitor->mOffset, a, r});"]
        result += ["}"]
        result += [""]
        result += [f"{self.name}_debug::new_context::~new_context()"]
        result += ["{"]
        result += ["    visitor->current_context.pop();"]
        result += ["}"]
        result += [""]
        result += ["{0}_debug::{0}_debug(std::ostream& output, int offset)".format(self.name)]
        result += [": mOutput(output), mOffset(offset)"]
        result += ["{"]
        result += ["    current_context.push({0, false});"]
        result += ["}"]
        result += [""]
        result += ["std::ostream& {}_debug::output()".format(self.name)]
        result += ["{"]
        result += ["    return mOutput;"]
        result += ["}"]
        result += [""]
        result += ["std::ostream& {}_debug::nl()".format(self.name)]
        result += ["{"]
        result += ["    mOutput << \"\\n\";"]
        result += ["    return mOutput;"]
        result += ["}"]
        result += [""]
        result += ["std::ostream& {}_debug::indent()".format(self.name)]
        result += ["{"]
        result += ["    int i = current_context.top().indent;"]
        result += ["    while(i--)"]
        result += ["        mOutput << \" \";"]
        result += ["    return mOutput;"]
        result += ["}"]
        result += [""]
        result += ["std::ostream& {}_debug::title()".format(self.name)]
        result += ["{"]
        result += ["    int i = current_context.top().indent;"]
        result += ["    if (i >= 2 && current_context.top().array) i -= 2;"]
        result += ["    while(i--)"]
        result += ["        mOutput << \" \";"]
        result += ["    if (current_context.top().array) mOutput << \"- \";"]
        result += ["    return mOutput;"]
        result += ["}"]
        result += [""]

        for decl in self.file.declarations:
            if isinstance(decl, Node):
                result += self.generate_node(decl)

        
        return '\n'.join(result)

    def generate_node(self, node):
        result = []
        header = []
        body = []

        if node.children != []:
            return result

        for field in node.fields:
            temp,in_header = self.generate_field(field.node, field)
            if in_header:
                header += temp
            else:
                body += temp

        # todo: can we merge this for loop with the one above it
        for parent in node.resolved_parents:
            for field in parent.fields:
                temp,in_header = self.generate_field(node, field)
                if in_header:
                    header += temp
                else:
                    body += temp

        for guard in node.guards.keys():
            header += [f"    output() << \" {guard}: \" << {node.name[0]}->{guard};"]

        # todo: can we merge this for loop with the one above it
        for parent in node.resolved_parents:
            for guard in parent.guards.keys():
                header += [f"    output() << \" {guard}: \" << {node.name[0]}->{guard};"]

        result += ["bool {}_debug::visit({}* {})".format(self.name, node.fully_qualified_name, node.name[0])]
        result += ["{"]
        result += ["    if (current_context.top().reference && !current_context.top().array) {"]
        result += [f"        output() << \"{node.name.title()}\";"]
        result += [f"        output() << \" \" << {node.name[0]};"]
        result += ["        nl();"]
        result += ["        return false;"]
        result += ["    } else if (current_context.top().reference && current_context.top().array) {"]
        result += [f"        title() << \"{node.name.title()}\";"]
        result += [f"        output() << \" \" << {node.name[0]};"]
        result += ["        nl();"]
        result += ["        return false;"]
        result += ["    }"]
        result += [""]
        result += ["    title() << \"{}\";".format(node.name.title())]
        result += [""]
        result += ["    // print pointer"]
        result += ["    output() << \" \" << {};".format(node.name[0])]
        result += [""]
        result += ["    // user defined types and references to ast types"]
        if len(header) == 0:
            result += ["    // none"]
        else:
            result += header

        result += [""]
        result += ["    output() << \"\\n\";"]
        result += [""]

        result += ["    // owned ast types"]
        if len(body) == 0:
            result += ["    // none"]
        else:
            result += body
        result += [""]

        result += ["    return false;"]
        result += ["}"]
        result += [""]
        result += [""]
        return result

    def generate_field(self, node, field):
        ownership = str(field.ownership).rsplit('.',1)[-1].lower()
        access    = str(field.access).rsplit('.',1)[-1].lower()
        container = str(field.container).rsplit('.',1)[-1].lower()
        model     = str(field.model).rsplit('.',1)[-1].lower()
        fcall     = f'generate_field_{model}_{ownership}_{access}_{container}'

        # Either this (will throw exception when method not found)
        return getattr(self, fcall, lambda x,y: self.error(Error.UnsupportedFeature, [fcall]))(node,field)

        # Or that (will not even complain. Just returns nothing when no there's
        # no function to handle the generation)
        # return getattr(self, fcall, lambda x: ([], False))(field)

    # Various function to generate code for each use case.
    #
    # generate_{model}_{ownership}_{access}_{container}
    # where
    #  model:     node, enum, type, aggr
    #  ownership: conditional, full, reference
    #  access:    pointer, object
    #  container: map, array, optional, value
    def generate_field_aggr_full_object_value(self, node, field):
        result = []
        result += [f"    // debug printer for union `{node.name[0]}->{field.name}` follows"]
        result += [f"    indent() << \"{field.display_name}:\";"]
        result += [f"    switch ({node.name[0]}->{field.name}_kind) {{"]
        for n in field.resolved_type.nodes:
            # initialise these because they are used down below
            header = []
            body = []

            for f in n.fields:
                # we need to temporarily append the accessor to f.name and
                # f.attributes['cowned']. So we save the old name before
                # overwriting it
                f_name = f.name
                f.name = f"{field.name}.{n.name}.{f_name}"

                f_cowned = None
                if 'cowned' in f.attributes:
                    f_cowned = f.attributes['cowned']
                    f.attributes['cowned'] = f"{field.name}.{n.name}.{f_cowned}"

                # we generate the line(s) that debug print things
                temp,in_header = self.generate_field(node, f)

                # we then restore the correct name
                f.name = f_name
                if f_cowned is not None:
                    f.attributes['cowned'] = f_name

                if in_header:
                    header += temp
                else:
                    body += temp
            for guard in n.guards.keys():
                header += [f"        output() << \" {guard}: \" << {n.name[0]}->{guard};"]

            result += [f"    case {node.name}::{field.name}_::{n.name}: {{ nl(); new_context nc(this);"]
            result += [f"        title() << \"{n.name.title()}\";"]
            result += [f"        output() << \" \" << &{node.name[0]}->{field.name}.{n.name};"]
            if len(header) != 0:
                result += [f"    {x}" for x in header]
            result += ["        nl();"]

            if len(body) != 0:
                result += [f"    {x}" for x in body]
            result += [f"    }}   break;"]
        result += [f"    case {node.name}::{field.name}_::none: [[fallthrough]];"]
        result += [ "    default:"]
        result += [f"        output() << \" nada\"; nl();"]
        result += [ "        break;"]
        result += [f"    }}"]
        result += [f"    // debug printer for union `{node.name[0]}->{field.name}` done"]
        result += [""]
        #TODO
        return result, False

    def generate_field_node_full_pointer_value(self, node, field):
        result = []
        result += [f"    indent() << \"{field.display_name}:\";"]
        result += [f"    if (!{node.name[0]}->{field.name}) {{ output() << \" nullptr\"; nl(); }}"]
        result += [f"    else {{ nl(); new_context nc(this);"]
        for child in field.resolved_type.children:
            result += [f"        if ({node.name[0]}->{field.name}->is_{child}()) {node.name[0]}->{field.name}->as_{child}()->traverse(*this);"]
        if len(field.resolved_type.children) == 0:
            result += [f"        {node.name[0]}->{field.name}->traverse(*this);"]
        result += [f"    }}"]
        return result, False

    def generate_field_node_full_pointer_optional(self, node, field):
        result = []
        result += [f"    indent() << \"{field.display_name}:\";"]
        result += [f"    if (!{node.name[0]}->{field.name}) {{ output() << \" nullptr\"; nl(); }}"]
        result += [f"    else {{ nl(); new_context nc(this);"]
        for child in field.resolved_type.children:
            result += [f"        if ({node.name[0]}->{field.name}->is_{child}()) {node.name[0]}->{field.name}->as_{child}()->traverse(*this);"]
        if len(field.resolved_type.children) == 0:
            result += [f"        {node.name[0]}->{field.name}->traverse(*this);"]
        result += [f"    }}"]
        return result, False

    def generate_field_node_full_pointer_array(self, node, field):
        result = []
        result += [f"    indent() << \"{field.display_name}:\";"]
        result += [f"    if ({node.name[0]}->{field.name}.size() == 0) {{ output() << \" []\"; nl(); }}"]
        result += [f"    else {{ nl(); for (auto& {field.name[0]}_: {node.name[0]}->{field.name}) {{ new_context nc(this, true);"]
        result += [f"        if (!{field.name[0]}_) {{ title() << \"nullptr\"; nl(); continue; }}"]
        for child in field.resolved_type.children:
            result += [f"        if ({field.name[0]}_->is_{child}()) {field.name[0]}_->as_{child}()->traverse(*this);"]
        if len(field.resolved_type.children) == 0:
            result += [f"        {field.name[0]}_->traverse(*this);"]
        result += [f"    }} }}"]
        return result, False

    def generate_field_type_full_object_array(self, node, field):
        result = []
        result += [f"    indent() << \"{field.display_name}: [\";"]
        result += [f"    {{ bool first = true; for (auto& {field.name[0]}_: {node.name[0]}->{field.name}) {{"]
        result += [f"        if (first) first = false; else output() << \", \";"]
        result += [f"        output() << {field.name[0]}_;"]
        result += [f"    }}"]
        result += [f"    output() << \"]\"; nl(); }}"]
        return result, False

    def generate_field_type_full_object_value(self, node, field):
        result = []
        result += [f"    output() << \" {field.display_name}: \" << {node.name[0]}->{field.name};"]
        return result, True

    def generate_field_type_reference_pointer_value(self, node, field):
        result = []
        result += [f"    output() << \" {field.display_name}: \" << {node.name[0]}->{field.name};"]
        return result, True

    def generate_field_type_reference_pointer_array(self, node, field):
        result = []
        result += [f"    indent() << \"{field.display_name}: [\";"]
        result += [f"    {{ bool first = true; for (auto& {field.name[0]}_: {node.name[0]}->{field.name}) {{"]
        result += [f"        if (first) first = false; else output() << \", \";"]
        result += [f"        output() << {field.name[0]}_;"]
        result += [f"    }}"]
        result += [f"    output() << \"]\"; nl(); }}"]
        return result, True

    def generate_field_enum_full_object_value(self, node, field):
        result = []
        result += [f"    output() << \" {field.display_name}:\";"]
        for value in field.resolved_type.values:
            result += [f"    if ({node.name[0]}->{field.name} == {field.resolved_type.name}::{value}) {{ output() << \" {value}\"; }}"]
        return result, True

    def generate_field_node_reference_pointer_value(self, node, field):
        result = []
        result += [f"    indent() << \"{field.display_name}:\";"]
        result += [f"    if (!{node.name[0]}->{field.name}) {{ output() << \" nullref\"; nl(); }}"]
        result += [f"    else {{ output() << \" \"; new_context nc(this, false, true);"]
        for child in field.resolved_type.children:
            result += [f"        if ({node.name[0]}->{field.name}->is_{child}()) {node.name[0]}->{field.name}->as_{child}()->traverse(*this);"]
        if len(field.resolved_type.children) == 0:
            result += [f"        {node.name[0]}->{field.name}->traverse(*this);"]
        result += [f"    }}"]
        return result, False

    def generate_field_node_conditional_pointer_array(self, node, field):
        guard = field.attributes['cowned']
        result = []
        result += [f"    indent() << \"{field.display_name}:\";"]
        result += [f"    if ({node.name[0]}->{field.name}.size() == 0) {{ output() << \" []\"; nl(); }}"]

        # array contains data and we own the data
        result += [f"    else if ({node.name[0]}->{guard}) {{ nl(); for (auto& {field.name[0]}_: {node.name[0]}->{field.name}) {{ new_context nc(this, true);"]
        result += [f"        if (!{field.name[0]}_) {{ title() << \"nullptr\"; nl(); continue; }}"]
        for child in field.resolved_type.children:
            result += [f"        if ({field.name[0]}_->is_{child}()) {field.name[0]}_->as_{child}()->traverse(*this);"]
        if len(field.resolved_type.children) == 0:
            result += [f"        {field.name[0]}_->traverse(*this);"]
        result += [f"    }} }}"]

        # array contains data but we dont own the data
        result += [f"    else if (!{node.name[0]}->{guard}) {{ nl(); for (auto& {field.name[0]}_: {node.name[0]}->{field.name}) {{ new_context nc(this, true, true);"]
        result += [f"        if (!{field.name[0]}_) {{ title() << \"nullref\"; nl(); continue; }}"]
        for child in field.resolved_type.children:
            result += [f"        if ({field.name[0]}_->is_{child}()) {field.name[0]}_->as_{child}()->traverse(*this);"]
        if len(field.resolved_type.children) == 0:
            result += [f"        {field.name[0]}_->traverse(*this);"]
        result += [f"    }} }}"]

        return result, False

    def generate_field_node_conditional_pointer_value(self, node, field):
        guard = field.attributes['cowned']
        result = []
        result += [f"    indent() << \"{field.display_name}:\";"]
        result += [f"    if ({node.name[0]}->{guard} && !{node.name[0]}->{field.name}) {{ output() << \" nullptr\"; nl(); }}"]
        result += [f"    else if (!{node.name[0]}->{guard} && !{node.name[0]}->{field.name}) {{ output() << \" nullref\"; nl(); }}"]
        result += [f"    else {{ new_context nc(this, false, !{node.name[0]}->{guard});"]
        result += [f"        if (!{node.name[0]}->{guard}) output() << \" \";"]
        result += [f"        else nl();"]
        for child in field.resolved_type.children:
            result += [f"        if ({node.name[0]}->{field.name}->is_{child}()) {node.name[0]}->{field.name}->as_{child}()->traverse(*this);"]
        if len(field.resolved_type.children) == 0:
            result += [f"        {node.name[0]}->{field.name}->traverse(*this);"]
        result += [f"    }}"]
        return result, False

    def generate_field_node_reference_pointer_array(self, node, field):
        result = []
        result += [f"    indent() << \"{field.display_name}:\";"]
        result += [f"    if ({node.name[0]}->{field.name}.size() == 0) {{ output() << \" []\"; nl(); }}"]
        result += [f"    else {{ nl(); for (auto& {field.name[0]}_: {node.name[0]}->{field.name}) {{ new_context nc(this, true, true);"]
        result += [f"        if (!{field.name[0]}_) {{ title() << \"nullref\"; nl(); continue; }}"]
        for child in field.resolved_type.children:
            result += [f"        if ({field.name[0]}_->is_{child}()) {field.name[0]}_->as_{child}()->traverse(*this);"]
        if len(field.resolved_type.children) == 0:
            result += [f"        {field.name[0]}_->traverse(*this);"]
        result += [f"    }} }}"]
        return result, False

    def generate_field_node_conditional_pointer_optional(self, node, field):
        guard = field.attributes['cowned']
        result = []
        result += [f"    indent() << \"{field.display_name}:\";"]
        result += [f"    if (!{node.name[0]}->{field.name}) {{ output() << \" nullptr\"; nl(); }}"]
        result += [f"    else {{ new_context nc(this, false, !{node.name[0]}->{guard});"]
        result += [f"        if (!{node.name[0]}->{guard}) output() << \" \";"]
        result += [f"        else nl();"]
        for child in field.resolved_type.children:
            result += [f"        if ({node.name[0]}->{field.name}->is_{child}()) {node.name[0]}->{field.name}->as_{child}()->traverse(*this);"]
        if len(field.resolved_type.children) == 0:
            result += [f"        {node.name[0]}->{field.name}->traverse(*this);"]
        result += [f"    }}"]
        return result, False

    def generate_field_type_full_object_optional(self, node, field):
        result = []
        result += [f"    indent() << \"{field.display_name}:\";"]
        result += [f"    if (!{node.name[0]}->{field.name}) {{ output() << \" nullopt\"; nl(); }}"]
        result += [f"    else {{ output() << \" \" << *{node.name[0]}->{field.name}; nl(); }}"]
        return result, False

    def generate_field_type_full_object_map(self, node, field):
        return [], False

    def generate_field_enum_full_object_optional(self, node, field):
        result = []
        result += [f"    indent() << \"{field.display_name}:\";"]
        result += [f"    if (!{node.name[0]}->{field.name}) {{ output() << \" nullopt\"; nl(); }}"]
        for value in field.resolved_type.values:
            result += [f"    else if ({node.name[0]}->{field.name} == {field.resolved_type.name}::{value}) {{ output() << \" {value}\"; nl(); }}"]
        return result, False

    def generate_field_type_full_pointer_array(self, node, field):
        result = []
        result += [f"    indent() << \"{field.display_name}: [\";"]
        result += [f"    {{ bool first = true; for (auto& {field.name[0]}_: {node.name[0]}->{field.name}) {{"]
        result += [f"        if (first) first = false; else output() << \", \";"]
        result += [f"        if ({field.name[0]}_) {{ output() << {'*' if 'showptr' not in field.attributes else ''}{field.name[0]}_; }}"]
        result += [f"        else {{ output() << \"nullptr\"; }}"]
        result += [f"    }}"]
        result += [f"    output() << \"]\"; nl(); }}"]
        return result, False

    def generate_field_enum_full_object_array(self, node, field):
        result = []
        result += [f"    indent() << \"{field.display_name}: [\";"]
        result += [f"    {{ bool first = true; for (auto& {field.name[0]}_: {node.name[0]}->{field.name}) {{"]
        result += [f"        if (first) first = false; else output() << \", \";"]
        for value in field.resolved_type.values:
            result += [f"        if ({field.name[0]}_ == {field.resolved_type.name}::{value}) {{ output() << \"{value}\"; }}"]
        result += [f"    }}"]
        result += [f"    output() << \"]\"; nl(); }}"]
        return result, False

    def generate_field_type_full_pointer_map(self, node, field):
        return [], False

    def generate_field_type_full_pointer_optional(self, node, field):
        result = []
        result += [f"    indent() << \"{field.display_name}:\";"]
        result += [f"    if (!{node.name[0]}->{field.name}) {{ output() << \" nullptr\"; nl(); }}"]
        result += [f"    else {{ output() << \" \" << {'*' if 'showptr' not in field.attributes else ''}{node.name[0]}->{field.name}; nl(); }}"]
        return result, False

    def generate_field_enum_full_object_map(self, node, field):
        return [], False

    def generate_field_node_full_pointer_map(self, node, field):
        return [], False

    def generate_field_type_full_pointer_value(self, node, field):
        result = []
        result += [f"    output() << \" {field.display_name}:\";"]
        result += [f"    if (!{node.name[0]}->{field.name}) {{ output() << \" nullptr\"; }}"]
        result += [f"    else {{ output() << \" \" << {'*' if 'showptr' not in field.attributes else ''}{node.name[0]}->{field.name}; }}"]
        return result, True

    def generate_field_node_reference_pointer_multimap(self, node, field):
        result = []
        result += [f"    indent() << \"{field.display_name}:\";"]
        result += [f"    if ({node.name[0]}->{field.name}.size() == 0) {{ output() << \" {{}}\"; nl(); }}"]
        result += [f"    else {{ nl(); new_context nc(this, false, true);"]
        result += [f"       for (auto it = {node.name[0]}->{field.name}.begin(); it != {node.name[0]}->{field.name}.end();) {{"]
        result += [f"           auto curr = it; title() << curr->first; nl();"]
        result += [f"           while (it != {node.name[0]}->{field.name}.end() && it->first == curr->first) {{ new_context nc(this, true, true);"]
        for child in field.resolved_type.children:
            result += [f"               if (it->second->is_{child}()) it->second->as_{child}()->traverse(*this);"]
        if len(field.resolved_type.children) == 0:
            result += [f"               it->second->traverse(*this);"]
        result += [f"               ++it;"]
        result += [f"    }} }} }}"]
        return result, False

class Nodes_Dot_H_Generator(Generator):

    def generate(self):

        random_guard_name = "NATSUKI_GENERATED_{}_{}_H".format(self.name.upper(),datetime.now().strftime("%Y%m%d%H%M%S"))

        result = [""]
        result += ["#ifndef {}".format(random_guard_name)]
        result += ["#define {}".format(random_guard_name)]
        result += [""]
        result += ["#include <cassert>"]
        result += [""]

        for include in self.file.includes:
            result += ["#include {}".format(include)]
        result += [""]

        if 'namespace' in self.file.options:
            result += ["namespace {}".format(self.file.options['namespace'])]
            result += ["{"]
            result += [""]

        # write the enum class kind
        result += ["enum class kind {"]
        for decl in self.file.declarations:
            if not isinstance(decl, Node):
                continue
            result += ["    {},".format(decl.name)]
        result += ["};"]
        result += [""]

        result += self.generate_forward_declarations()

        for decl in self.file.declarations:
            if isinstance(decl, Node):
                result += self.generate_node(decl)
            elif isinstance(decl, Enumeration):
                result += self.generate_enumeration(decl)

        # add the traverse method if this the ast is visitable
        if 'visitable' in self.file.options:
            result += self.generate_visitor()

        # display any verbatim custom codes if any
        for private in self.file.private:
            result += [""]
            result += [ private.content.rstrip() ]

        # almost done. if there was a custom namespace, close it
        if 'namespace' in self.file.options:
            result += ["}"]

        result += [""]
        result += ["#endif"]
        result += [""]
        return '\n'.join(result)
        return result

    def generate_visitor(self):

        result = ["class visitor"]
        result += ["{"]
        result += ["    public:"]
        result += ["    visitor();"]
        result += ["    virtual ~visitor();"]
        result += [""]

        for decl in self.file.declarations:
            if isinstance(decl, Node):
                pass
            elif isinstance(decl, Enumeration):
                continue
            elif isinstance(decl, CustomType):
                continue

            # got children? this is an abstract node, only provide a visit
            # function, no post visit
            if decl.children != []:
                result += ["    virtual bool visit({}* {});".format(decl.fully_qualified_name, decl.name[0])]
                result += [""]
            # no children? this is a child node or a normal node, provide a
            # visit and a post visit function
            else:
                result += ["    virtual bool visit({}* {});".format(decl.fully_qualified_name, decl.name[0])]
                result += ["    virtual void post_visit({}* {});".format(decl.fully_qualified_name, decl.name[0])]
                result += [""]
        result += ["};"]
        result += [""]

        return result

    def generate_forward_declarations(self):
        fwdecls = []
        for node in self.file.declarations:
            if not isinstance(node, Node):
                continue
            fwdecls += ["class {};".format(node.name)]
        if 'visitable' in self.file.options:
            fwdecls += ["class visitor;"]
        fwdecls += [""]
        return ["// forward declarations"] + fwdecls

    def generate_field(self, field):
        ownership = str(field.ownership).rsplit('.',1)[-1].lower()
        access    = str(field.access).rsplit('.',1)[-1].lower()
        container = str(field.container).rsplit('.',1)[-1].lower()
        model     = str(field.model).rsplit('.',1)[-1].lower()
        fcall     = f'generate_field_{model}_{ownership}_{access}_{container}'

        # Either this (will throw exception when method not found)
        return getattr(self, fcall, lambda x: self.error(Error.UnsupportedFeature, [fcall]))(field)

        # Or that (will not even complain. Just returns nothing when no there's
        # no function to handle the generation)
        # return getattr(self, fcall, lambda x: ([], False))(field)

    def generate_field_aggr_full_object_value(self, field: Field):
        result = []
        result += [f"    enum class {field.name}_ {{"]
        result += [f"        none,"]
        for node in field.resolved_type.nodes:
            result += [f"        {node.name},"]
        result += [f"    }};"]
        result += [f"    {field.name}_ {field.name}_kind = {field.name}_::none;"]
        result += [f"    {field.node.name}_{field.resolved_type.name}_{field.name} {field.name};"]
        result += [""]
        return result

    def generate_field_node_reference_pointer_value(self, field):
        return [f"    {field.resolved_type.fully_qualified_name}* {field.name};"]

    def generate_field_node_full_pointer_value(self, field):
        return [f"    {field.resolved_type.fully_qualified_name}* {field.name};"]

    def generate_field_node_full_pointer_optional(self, field):
        return [f"    {field.resolved_type.fully_qualified_name}* {field.name};"]

    def generate_field_node_full_pointer_array(self, field):
        return [f"    std::vector<{field.resolved_type.fully_qualified_name}*> {field.name};"]

    def generate_field_type_full_object_array(self, field):
        return [f"    std::vector<{field.resolved_type.type}> {field.name};"]

    def generate_field_type_full_object_value(self, field):
        return [f"    {field.resolved_type.type} {field.name};"]

    def generate_field_type_reference_pointer_value(self, field):
        return [f"    {field.resolved_type.type} {field.name};"]

    def generate_field_type_reference_pointer_array(self, field):
        return [f"    std::vector<{field.resolved_type.type}> {field.name};"]

    def generate_field_enum_full_object_value(self, field):
        return [f"    {field.resolved_type.fully_qualified_name} {field.name};"]

    def generate_field_node_conditional_pointer_value(self, field):
        return [f"    {field.resolved_type.fully_qualified_name}* {field.name};"]

    def generate_field_node_conditional_pointer_array(self, field):
        return [f"    std::vector<{field.resolved_type.fully_qualified_name}*> {field.name};"]

    def generate_field_node_reference_pointer_array(self, field):
        return [f"    std::vector<{field.resolved_type.fully_qualified_name}*> {field.name};"]

    def generate_field_node_conditional_pointer_optional(self, field):
        return [f"    {field.resolved_type.fully_qualified_name}* {field.name};"]

    def generate_field_type_full_object_optional(self, field):
        return [f"    std::optional<{field.resolved_type.type}> {field.name};"]

    def generate_field_type_full_object_map(self, field):
        return [f"    std::unordered_map<{field.attributes['map']}, {field.resolved_type.type}> {field.name};"]

    def generate_field_enum_full_object_optional(self, field):
        return [f"    std::optional<{field.resolved_type.fully_qualified_name}> {field.name};"]

    def generate_field_node_reference_pointer_multimap(self, field):
        return [f"    std::unordered_multimap<{field.attributes['mmap']}, {field.resolved_type.fully_qualified_name}*> {field.name};"]

    def generate_field_enum_full_object_map(self, field):
        return [f"    std::unordered_map<{field.attributes['map']}, {field.resolved_type.fully_qualified_name}> {field.name};"]

    def generate_field_type_full_pointer_map(self, field):
        return [f"    std::unordered_map<{field.attributes['map']}, {field.resolved_type.type}> {field.name};"]

    def generate_field_type_full_pointer_array(self, field):
        return [f"    std::vector<{field.resolved_type.type}> {field.name};"]

    def generate_field_enum_full_object_array(self, field):
        return [f"    std::vector<{field.resolved_type.fully_qualified_name}> {field.name};"]

    def generate_field_type_full_pointer_optional(self, field):
        return [f"    {field.resolved_type.type} {field.name};"]

    def generate_field_node_full_pointer_map(self, field):
        return [f"    std::unordered_map<{field.attributes['map']}, {field.resolved_type.fully_qualified_name}*> {field.name};"]

    def generate_field_type_full_pointer_value(self, field):
        return [f"    {field.resolved_type.type} {field.name};"]

    def generate_node(self, node):

        result = ["class {}".format(node.name)]
        if node.parents != []:
            result[-1] += ": public " + ', public '.join(node.parents)
        result += ["{"]

        all_fields_that_are_unions = []
        for field in node.fields:
            if field.model == Field.Model.Aggr:
                all_fields_that_are_unions.append(field)
        if len(all_fields_that_are_unions) != 0:
            result += ["    // deliberate private structs and types not for the eyes of the developer"]
        for f in all_fields_that_are_unions:
            for n in f.resolved_type.nodes:
                result += [f"    struct {node.name}_{n.name} {{"]
                result += [f"        ~{node.name}_{n.name}();"]
                for f in n.fields:
                    result += [f"    {x}" for x in self.generate_field(f)]
                
                # handle conditioally owned fields
                for guard, fields in n.guards.items():
                    result += [""]
                    result += ["        // guard: {}".format(guard)]
                    result += ["        //"]
                    result += ["        // if true, struct {} owns field{}: {}".format(n.name, 's' if len(fields) > 1 else '', ', '.join(fields))]
                    result += ["        // fields owned by this struct will be destroyed by its destructor"]
                    result += ["        // default value is false"]
                    result += ["        bool {} = false;".format(guard)]
                result += [f"    }};"]
                result += [""]

        for f in all_fields_that_are_unions:
            result += [f"    union {node.name}_{f.resolved_type.name}_{f.name} {{"]
            result += [f"        {node.name}_{f.resolved_type.name}_{f.name}() {{}};"]
            result += [f"        ~{node.name}_{f.resolved_type.name}_{f.name}() {{}};"]
            for n in f.resolved_type.nodes:
                result += [f"        {node.name}_{n.name} {n.name};"]
            result += [f"    }};"]
            result += [""]

        result += ["    public:"]

        for field in node.fields:
            result += self.generate_field(field)

        # generate default constructor and the rule of 5 constructors

        if 'noctor' not in node.attributes:
            result += ["    {}();".format (node.name)]
            result += ["    {0}(const {0}&) = delete;".format (node.name)]
            result += ["    {0}({0}&&) = delete;".format (node.name)]
            result += ["    {0}& operator=(const {0}&) = delete;".format (node.name)]
            result += ["    {0}& operator=({0}&&) = delete;".format (node.name)]

        # so we want a destructor
        if 'nodtor' not in node.attributes:
            if node.children != []:
                result += [""]
                result += ["    virtual ~{}();".format (node.name)]
                result += ["    virtual enum kind get_kind() const = 0;".format (node.name)]
                result += [""]
            elif node.parents != []:
                result += [""]
                result += ["    virtual ~{}();".format(node.name)]
            else:
                result += ["    ~{}();".format(node.name)]

        if node.children != []:
            for child in node.children:
                result += ["    bool is_{0}();".format(child)]
            result += [""]
            for child in node.resolved_children:
                result += [f"    class {child.fully_qualified_name}* as_{child.name}();"]

        # if has children, this is an abstract node. create virtual methods
        # and other stuffs
        # if has parents, this is a subnode. create the virtual get_kind that
        # will implement its abstract node' get_kind
        elif node.parents != []:
            result += ["    virtual enum kind get_kind() const;"]
            for parent in node.parents:
                result += ["    bool is_{0}();".format(parent)]
            result += [""]
            for parent in node.resolved_parents:
                result += [f"    class {parent.fully_qualified_name}* as_{parent.name}();"]

        # else, create a simple desctuctor
        else:
            pass

        # handle conditioally owned fields
        for guard, fields in node.guards.items():
            result += [""]
            result += ["    // guard: {}".format(guard)]
            result += ["    //"]
            result += ["    // if true, class {} owns field{}: {}".format(node.name, 's' if len(fields) > 1 else '', ', '.join(fields))]
            result += ["    // fields owned by this class will be destroyed by its destructor"]
            result += ["    // default value is false"]

            for parent in node.resolved_parents:
                if guard in parent.guards:
                    result += ["    //"]
                    result += ["    // Note this class extends {} which also has the same guard".format(parent.name)]
                    result += ["    // When changing {}, you advised to do:".format(guard)]
                    result += ["    //    {}->{} = true or false;".format(node.name[0], guard)]
                    result += ["    //    {}->as_{}()->{} = true or false;".format(node.name[0], parent.name, guard)]
            result += ["    bool {} = false;".format(guard)]

        for f in all_fields_that_are_unions:
            result += [f"    void set_{f.name}_kind({f.name}_ kind);"]

        # display any verbatim custom codes if any
        for private in node.private:
            result += [""]
            result += [ private.content.rstrip() ]

        # add the traverse method if this the ast is visitable
        if 'visitable' in self.file.options:
            result += [""]
            result += ["    void traverse(visitor& v);"]

        # we are done
        result += ["};"]
        result += [""]
        return result

    def generate_enumeration(self, enum):
        # write the enumeration as an enum class
        result = ["enum class {} {{".format(enum.name)]
        for literal in enum.values:
            result += ["    {},".format(literal)]
        result += ["};"]
        result += [""]
        return result

class Nodes_Dot_Cpp_Generator(Generator):

    def generate(self):
        result = [""]
        result += ["#include \"{}.h\"".format(self.name)]
        result += [""]

        if 'namespace' in self.file.options:
            result += ["using namespace {};".format(self.file.options['namespace'])]
            result += [""]

        for decl in self.file.declarations:
            if isinstance(decl, Node):
                result += self.generate_node(decl)
            elif isinstance(decl, Enumeration):
                continue

        if 'visitable' in self.file.options:
            result += self.generate_visitor()

        # display any verbatim custom codes if any
        for public in self.file.public:
            result += [""]
            result += [ public.content.rstrip() ]

        return '\n'.join(result)

    def generate_node(self, node):
        result = []

        all_fields_that_are_unions = []
        for field in node.fields:
            if field.model == Field.Model.Aggr:
                all_fields_that_are_unions.append(field)
        for f in all_fields_that_are_unions:
            for n in f.resolved_type.nodes:
                result += [f"{node.fully_qualified_name}::{node.name}_{n.name}::~{node.name}_{n.name}()"]
                result += ["{"]
                for f in n.fields:
                    result += self.generate_destroyer(f)
                result += ["}"]
                result += [""]

        if 'noctor' not in node.attributes:
            result += ["{}::{}()".format(node.fully_qualified_name, node.name)]

            initialiser = []
            for field in node.fields:
                default_value = field.default
                if default_value is None:
                    default_value = self.generate_initialiser(field)

                if default_value is not None:
                    initialiser += [f"{field.name}({default_value})"]
            
            if len(initialiser) != 0:
                result += [": " + ', '.join(initialiser)]

            result += ["{"]
            result += ["}"]
            result += [""]

        if 'nodtor' not in node.attributes:
            result += ["{}::~{}()".format(node.fully_qualified_name, node.name)]
            result += ["{"]
            for field in node.fields:
                result += self.generate_destroyer(field)

            result += ["}"]
            result += [""]

        if len(all_fields_that_are_unions) != 0:
            for f in all_fields_that_are_unions:
                result += [f"void {node.fully_qualified_name}::set_{f.name}_kind({f.name}_ kind)"]
                result += ["{"]
                result += [f"    switch ({f.name}_kind) {{"]
                result += [f"    case {f.name}_::none: break;"]
                for n in f.resolved_type.nodes:
                    result += [f"    case {f.name}_::{n.name}: {f.name}.{n.name}.~{node.name}_{n.name}(); break;"]
                result += ["    }"]
                result += [f"    {f.name}_kind = kind;"]
                result += [f"    switch ({f.name}_kind) {{"]
                result += [f"    case {f.name}_::none: break;"]
                for n in f.resolved_type.nodes:
                    result += [f"    case {f.name}_::{n.name}: new (&{f.name}.{n.name}) {node.name}_{n.name}(); break;"]
                result += ["    }"]
                result += ["}"]
                result += [""]

        for child in node.children:
            result += ["bool {1}::is_{0}()".format(child, node.fully_qualified_name)]
            result += ["{"]
            result += ["    return get_kind() == kind::{};".format(child)]
            result += ["}"]
            result += [""]

        for child in node.children:
            result += ["class {0}* {1}::as_{0}()".format(child, node.fully_qualified_name)]
            result += ["{"]
            result += ["    assert(get_kind() == kind::{});".format(child)]
            result += ["    return static_cast<class {}*>(this);".format(child)]
            result += ["}"]
            result += [""]

        if node.parents != []:
            result += ["kind {}::get_kind() const".format(node.name)]
            result += ["{"]
            result += ["    return kind::{};".format(node.name)]
            result += ["}"]
            result += [""]

        for parent in node.parents:
            result += ["bool {1}::is_{0}()".format(parent, node.name)]
            result += ["{"]
            result += ["    return true;"]
            result += ["}"]
            result += [""]

        for parent in node.resolved_parents:
            result += ["class {}* {}::as_{}()".format(parent.fully_qualified_name, node.name, parent.name)]
            result += ["{"]
            result += ["    return static_cast<class {}*>(this);".format(parent.fully_qualified_name)]
            result += ["}"]
            result += [""]

        for public in node.public:
            result += [""]
            result += [ public.content.rstrip() ]

        if 'visitable' in self.file.options:
            result += ["void {}::traverse(visitor& __v__)".format(node.name)]
            result += ["{"]

            # if has children, this is a parent node. Do not call the
            # visitor.visit() function - this will already have been done by
            # the child node
            if node.children != []:
                pass
            else:
                result += ["    if (!__v__.visit(this)) return;"]

            for field in node.fields:
                # dont even bother to traverse a notvisitable field
                if not field.is_visitable:
                    continue
                result += self.generate_traveller(field)

            # if has parents, this is a child node. Call the corresponding
            # parent::traverse() functions
            for parent in node.parents:
                result += ["    {}::traverse(__v__);".format(parent)]

            # again, if has children, this is a parent node. Dont call the
            # visitor.post_visit() function - this will already have been done
            # by the child node
            if node.children != []:
                pass
            else:
                result += ["    __v__.post_visit(this);"]

            result += ["}"]
            result += [""]

        return result

    def generate_initialiser(self, field):
        ownership = str(field.ownership).rsplit('.',1)[-1].lower()
        access    = str(field.access).rsplit('.',1)[-1].lower()
        container = str(field.container).rsplit('.',1)[-1].lower()
        model     = str(field.model).rsplit('.',1)[-1].lower()
        fcall     = f'generate_initialiser_{model}_{ownership}_{access}_{container}'

        # Either this (will throw exception when method not found)
        return getattr(self, fcall, lambda x: self.error(Error.UnsupportedFeature, [fcall]))(field)

        # Or that (will not even complain. Just returns nothing when no there's
        # no function to handle the generation)
        # return getattr(self, fcall, lambda x: None)(field)

    def generate_initialiser_aggr_full_object_value(self, field):
        # Because this expands as a c++ union, there is nothing to initialise
        return None

    def generate_initialiser_node_reference_pointer_value(self, field):
        return "nullptr"

    def generate_initialiser_node_full_pointer_value(self, field):
        return "nullptr"

    def generate_initialiser_node_full_pointer_optional(self, field):
        return "nullptr"

    def generate_initialiser_node_full_pointer_array(self, field):
        return None

    def generate_initialiser_type_full_object_array(self, field):
        return None

    def generate_initialiser_type_full_object_value(self, field):
        return field.resolved_type.default

    def generate_initialiser_type_reference_pointer_value(self, field):
        return None

    def generate_initialiser_type_reference_pointer_array(self, field):
        return None

    def generate_initialiser_enum_full_object_value(self, field):
        return f"{field.type}::{field.resolved_type.values[0]}"

    def generate_initialiser_node_conditional_pointer_value(self, field):
        return "nullptr"

    def generate_initialiser_node_conditional_pointer_array(self, field):
        return None

    def generate_initialiser_node_reference_pointer_array(self, field):
        return None

    def generate_initialiser_node_conditional_pointer_optional(self, field):
        return "nullptr"

    def generate_initialiser_type_full_object_optional(self, field):
        return "std::nullopt"

    def generate_initialiser_type_full_object_map(self, field):
        return None

    def generate_initialiser_enum_full_object_optional(self, field):
        return "std::nullopt"

    def generate_initialiser_node_reference_pointer_multimap(self, field):
        return None

    def generate_initialiser_enum_full_object_map(self, field):
        return None

    def generate_initialiser_type_full_pointer_map(self, field):
        return None

    def generate_initialiser_type_full_pointer_array(self, field):
        return None

    def generate_initialiser_enum_full_object_array(self, field):
        return None

    def generate_initialiser_type_full_pointer_optional(self, field):
        return "nullptr"

    def generate_initialiser_node_full_pointer_map(self, field):
        return None

    def generate_initialiser_type_full_pointer_value(self, field):
        if field.resolved_type.default is not None:
            return field.resolved_type.default

        return "nullptr"

    def generate_destroyer(self, field):
        ownership = str(field.ownership).rsplit('.',1)[-1].lower()
        access    = str(field.access).rsplit('.',1)[-1].lower()
        container = str(field.container).rsplit('.',1)[-1].lower()
        model     = str(field.model).rsplit('.',1)[-1].lower()
        fcall     = f'generate_destroyer_{model}_{ownership}_{access}_{container}'

        # Either this (will throw exception when method not found)
        return getattr(self, fcall, lambda x: self.error(Error.UnsupportedFeature, [fcall]))(field)

        # Or that (will not even complain. Just returns nothing when no there's
        # no function to handle the generation)
        # return getattr(self, fcall, lambda x: [])(field)

    def generate_destroyer_aggr_full_object_value(self, field):
        result = []
        result += [f"    switch ({field.name}_kind) {{"]
        result += [f"    case {field.name}_::none: break;"]
        for node in field.resolved_type.nodes:
            result += [f"    case {field.name}_::{node.name}: {field.name}.{node.name}.~{field.node.name}_{node.name}(); break;"]
        result += ["    }"]

        return result

    def generate_destroyer_node_reference_pointer_value(self, field):
        return []

    def generate_destroyer_node_full_pointer_value(self, field):
        return [f"    if ({field.name}) delete {field.name};"]

    def generate_destroyer_node_full_pointer_optional(self, field):
        return [f"    if ({field.name}) delete {field.name};"]

    def generate_destroyer_node_full_pointer_array(self, field):
        return [f"    for (auto& it: {field.name}) if (it) delete it;"]

    def generate_destroyer_type_full_object_array(self, field):
        return []

    def generate_destroyer_type_full_object_value(self, field):
        return []

    def generate_destroyer_type_reference_pointer_value(self, field):
        return []

    def generate_destroyer_type_reference_pointer_array(self, field):
        return []

    def generate_destroyer_enum_full_object_value(self, field):
        return []

    def generate_destroyer_node_conditional_pointer_value(self, field):
        return [f"    if ({field.attributes['cowned']} && {field.name}) delete {field.name};"]
        return []

    def generate_destroyer_node_conditional_pointer_array(self, field):
        return [f"    if ({field.attributes['cowned']}) for (auto& it: {field.name}) if (it) delete it;"]

    def generate_destroyer_node_reference_pointer_array(self, field):
        return []

    def generate_destroyer_node_conditional_pointer_optional(self, field):
        return [f"    if ({field.attributes['cowned']} && {field.name}) delete {field.name};"]

    def generate_destroyer_type_full_object_optional(self, field):
        return []

    def generate_destroyer_type_full_object_map(self, field):
        return []

    def generate_destroyer_enum_full_object_optional(self, field):
        return []

    def generate_destroyer_node_reference_pointer_multimap(self, field):
        return []

    def generate_destroyer_enum_full_object_map(self, field):
        return []

    def generate_destroyer_type_full_pointer_map(self, field):
        return [f"    for (auto& it: {field.name}) if (it.second) delete it.second;"]

    def generate_destroyer_type_full_pointer_array(self, field):
        return [f"    for (auto& it: {field.name}) if (it) delete it;"]

    def generate_destroyer_enum_full_object_array(self, field):
        return []

    def generate_destroyer_type_full_pointer_optional(self, field):
        return [f"    if ({field.name}) delete {field.name};"]

    def generate_destroyer_node_full_pointer_map(self, field):
        return [f"    for (auto& it: {field.name}) if (it.second) delete it.second;"]

    def generate_destroyer_type_full_pointer_value(self, field):
        return [f"    if ({field.name}) delete {field.name};"]

    def generate_traveller(self, field):
        access    = str(field.access).rsplit('.',1)[-1].lower()
        container = str(field.container).rsplit('.',1)[-1].lower()
        model     = str(field.model).rsplit('.',1)[-1].lower()
        fcall     = f'generate_traveller_{model}_{access}_{container}'

        # Either this (will throw exception when method not found)
        return getattr(self, fcall, lambda x: self.error(Error.UnsupportedFeature, [fcall]))(field)

        # Or that (will not even complain. Just returns nothing when no there's
        # no function to handle the generation)
        # return getattr(self, fcall, lambda x: [])(field)

    def generate_traveller_aggr_object_value(self, field):
        result = []
        result += [f"    switch({field.name}_kind) {{"]
        for node in field.resolved_type.nodes:
            result += [f"    case {field.name}_::{node.name}:"]
            for f in node.fields:
                if not f.is_visitable:
                    continue

                # we need to temporarily append the accessor to f.name and
                # f.attributes['cowned']. So we save the old name before
                # overwriting it
                f_name = f.name
                f.name = f"{field.name}.{node.name}.{f.name}"

                f_cowned = None
                if 'cowned' in f.attributes:
                    f_cowned = f.attributes['cowned']
                    f.attributes['cowned'] = f"{field.name}.{node.name}.{f_cowned}"

                # we generate the line that actually destroys things
                result += [f"    {x}" for x in self.generate_traveller(f)]

                # we then restore the correct name
                f.name = f_name
                if f_cowned is not None:
                    f.attributes['cowned'] = f_name
            result += ["        break;"]
        result += [f"    default:"]
        result += ["        break;"]
        result += [f"    }}"]
        return result

    def generate_traveller_node_pointer_value(self, field):
        if field.resolved_type.children == []:
            return [f"    if ({field.name}) {field.name}->traverse(__v__);"]

        result = []
        result += [f"    if ({field.name} && __v__.visit({field.name})) {{"]
        for child in field.resolved_type.children:
            result += [f"        if({field.name}->is_{child}()) {field.name}->as_{child}()->traverse(__v__);"]
        result += ["    }"]

        return result

    def generate_traveller_node_pointer_optional(self, field):
        if field.resolved_type.children == []:
            return [f"    if ({field.name}) {field.name}->traverse(__v__);"]

        result = []
        result += [f"    if ({field.name} && __v__.visit({field.name})) {{"]
        for child in field.resolved_type.children:
            result += [f"        if({field.name}->is_{child}()) {field.name}->as_{child}()->traverse(__v__);"]
        result += ["    }"]

        return result

    def generate_traveller_node_pointer_array(self, field):
        if field.resolved_type.children == []:
            return [f"    for (auto& _it: {field.name}) if (_it) _it->traverse(__v__);"]

        result = []
        result += [f"    for (auto& _it: {field.name}) {{ if (!_it) continue;"]
        result += ["        if (!__v__.visit(_it)) continue;"]
        for child in field.resolved_type.children:
            result += [f"        if(_it->is_{child}()) _it->as_{child}()->traverse(__v__);"]
        result += ["    }"]

        return result

    def generate_traveller_node_pointer_map(self, field):
        if field.resolved_type.children == []:
            return [f"    for (auto& _it: {field.name}) if (_it.second) _it.second->traverse(__v__);"]

        result = []
        result += [f"    for (auto& _it: {field.name}) {{ if (!_it.second) continue;"]
        result += ["        if (!__v__.visit(_it.second)) continue;"]
        for child in field.resolved_type.children:
            result += [f"        if(_it.second->is_{child}()) _it.second->as_{child}()->traverse(__v__);"]
        result += ["    }"]

        return result

    def generate_visitor(self):

        result = [""]
        result += [f"{self.ns}visitor::visitor()"]
        result += ["{"]
        result += ["}"]
        result += [""]
        result += [f"{self.ns}visitor::~visitor()"]
        result += ["{"]
        result += ["}"]
        result += [""]

        for decl in self.file.declarations:
            if isinstance(decl, Node):
                pass
            elif isinstance(decl, Enumeration):
                continue
            elif isinstance(decl, CustomType):
                continue

            # got children? this is a parent node, only provide a visit
            # function, no post visit
            if decl.children != []:
                result += [f"bool {self.ns}visitor::visit({decl.fully_qualified_name}* {decl.name[0]})"]
                result += ["{"]
                result += ["    return true;"]
                result += ["}"]
                result += [""]
            # no children? this is a child node or a normal node, provide a
            # visit and a post visit function
            else:
                result += [f"bool {self.ns}visitor::visit({decl.fully_qualified_name}* {decl.name[0]})"]
                result += ["{"]
                result += ["    return true;"]
                result += ["}"]
                result += [""]
                result += [f"void {self.ns}visitor::post_visit({decl.fully_qualified_name}* {decl.name[0]})"]
                result += ["{"]
                result += ["}"]
                result += [""]
        return result

def main():
    parser = argparse.ArgumentParser(description='generator of vhdl ast class for use in vhdlstuff')
    parser.add_argument('-o', '--output',                              help='output directory/file without .h or .cpp extension')
    parser.add_argument('-b', '--generate_h',     action='store_true', help="generate .h file corresponding to ASTFILE")
    parser.add_argument('-c', '--generate_cpp',   action='store_true', help="generate .cpp file corresponding to ASTFILE")
    parser.add_argument('-d', '--generate_debug', action='store_true', help="generate _debug.h/_debug.cpp files for ASTFILE")
    parser.add_argument('-n', '--generate_nodes', action='store_true', help="generate .h/.cpp files for ASTFILE")
    parser.add_argument('file', metavar='ASTFILE', type=str, help='input ast file(s)')

    args = parser.parse_args()
    text = open(args.file, 'r').read()
    try:
        lexer = Lexer(text)
        parser = Parser(lexer)
        file = parser.parse()

        if args.generate_h and args.generate_nodes:
            generator = Nodes_Dot_H_Generator(file, args.output)
            dot_h = generator.generate()
            if args.output == None:
                print (dot_h)
            else:
                open(args.output + '.h', 'w').write(dot_h)
        if args.generate_cpp and args.generate_nodes:
            generator = Nodes_Dot_Cpp_Generator(file, args.output)
            dot_cpp = generator.generate()
            if args.output == None:
                print (dot_cpp)
            else:
                open(args.output + '.cpp', 'w').write(dot_cpp)

        if args.generate_h and args.generate_debug:
            generator = Debug_Dot_H_Generator(file, args.output)
            dot_h = generator.generate()
            if args.output == None:
                print (dot_h)
            else:
                open(args.output + '_debug.h', 'w').write(dot_h)
        if args.generate_cpp and args.generate_debug:
            generator = Debug_Dot_Cpp_Generator(file, args.output)
            dot_cpp = generator.generate()
            if args.output == None:
                print (dot_cpp)
            else:
                open(args.output + '_debug.cpp', 'w').write(dot_cpp)

    except (LexerError, ParserError, GeneratorError) as e:
        print(e.message)
        exit(1)


if __name__ == "__main__":
    main()
