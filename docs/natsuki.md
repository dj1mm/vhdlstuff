
# Preface

Project: *vhdlstuff*

vhdlstuff is a vhdl parser/analyser

vhdlstuff currently represents vhdl as a tree of generic ast objects. Each
generic ast object consists of a map of strings, integers and other ast objects.

Issue: i am currently writing vhdlstuff's vhdl syntax analyser, and the current
tree/nest of ast objects is not scalable. It is also not typesafe and is a
mess to work with.

This warrants the need to create a c++ object for each node of the ast.

I will not manually write each and every ast node. This is too repetitive and
will not be easy to maintain.

Instead, I will describe each ast nodes in a dsl, and a generator will then be
used to automatically generate ast objects in valid c++.

## Natsuki

Natsuki is the generator.

This document will confusingly refer everythign related to the generator
including the dsl syntax as natsuki.

## Inspiration

Natsuki has been greatly inspired from:

[scottmcpeak's elkhound ast](https://scottmcpeak.com/elkhound/sources/ast/)

Natsuki syntax is a c++ dsl.

Example

```cpp
#include <unordered_map>
#include <vector>
#include <optional>
#include <string>
#include "a_header_file.h"

// To write a comment, one can use the double forward slash same as in C++ and
// this comment will not appear in any of the generated .h or .cpp.

// options
// -------
[[namespace=my_namespace]];
[[visitable]];

// custom types
// ------------
#define integer int
#define string  std::string
#define boolean bool
#define pointer car_brand*
#define decimal double

class transport_method(riders_sorted_by_age: person<integer>);
-> class car(colour: colours, owner&: person, [[cowned=the_owner_bought_this]] gps: gps, [[notvisitable]] brand: pointer);
-> class bicycle(properties: string<string>);

class two_wheeled(price: decimal, [[istrait]] year_purchased: integer);
-> class bicycle(bought_for_exercise: boolean);
-> class skateboard();

class gps(home_location?: location)
private: { /* this will appear in the generated .h */ }
public:  { /* this will appear in the generated .cpp */ };

class person(v: person_is_either_boy_or_girl, name: string, friends: person[]);
union person_is_either_boy_or_girl {
    class boy (has_short_hair: boolean);
    class girl(has_long_hairs: boolean);
};

union person_is_either_boy_or_girl boy (is_a_gamer:     boolean);
union person_is_either_boy_or_girl girl(has_many_dolls: boolean);

// note that enums and classes and even unions can be declared in any order
enum colours { red, blue, yellow, green };
class location(lat, lon: integer);
```

This assumes there is already a file called `a_header_file.h` in the include
path, in which, the class `car_brand` is defined. A possible example follows:
```cpp
#include<iostream>
class car_brand {};

template <typename T>
std::basic_ostream<T>& operator<<(std::basic_ostream<T>& os, const car_brand& t) { return os; }

```

This generates the followign header file and c++ source code:

```cpp
#ifndef NATSUKI_GENERATED_VHDL_NODES_20210923155143_H
#define NATSUKI_GENERATED_VHDL_NODES_20210923155143_H

#include <cassert>

#include <unordered_map>
#include <vector>
#include <optional>
#include <string>
#include "a_header_file.h"

namespace my_namespace
{

enum class kind {
    transport_method,
    car,
    two_wheeled,
    bicycle,
    skateboard,
    gps,
    person,
    location,
};

// forward declarations
class transport_method;
class car;
class two_wheeled;
class bicycle;
class skateboard;
class gps;
class person;
class location;
class visitor;

enum class colours {
    red,
    blue,
    yellow,
    green,
};

class transport_method
{
    public:
    std::unordered_map<int, my_namespace::person*> riders_sorted_by_age;
    transport_method();
    transport_method(const transport_method&) = delete;
    transport_method(transport_method&&) = delete;
    transport_method& operator=(const transport_method&) = delete;
    transport_method& operator=(transport_method&&) = delete;

    virtual ~transport_method();
    virtual enum kind get_kind() const = 0;

    bool is_car();
    bool is_bicycle();

    class my_namespace::car* as_car();
    class my_namespace::bicycle* as_bicycle();

    void traverse(visitor& v);
};

class car: public transport_method
{
    public:
    my_namespace::colours colour;
    my_namespace::person* owner;
    my_namespace::gps* gps;
    car_brand* brand;
    car();
    car(const car&) = delete;
    car(car&&) = delete;
    car& operator=(const car&) = delete;
    car& operator=(car&&) = delete;

    virtual ~car();
    virtual enum kind get_kind() const;
    bool is_transport_method();

    class my_namespace::transport_method* as_transport_method();

    // guard: the_owner_bought_this
    //
    // if true, class car owns field: gps
    // fields owned by this class will be destroyed by its destructor
    // default value is false
    bool the_owner_bought_this = false;

    void traverse(visitor& v);
};

class two_wheeled
{
    public:
    double price;
    two_wheeled();
    two_wheeled(const two_wheeled&) = delete;
    two_wheeled(two_wheeled&&) = delete;
    two_wheeled& operator=(const two_wheeled&) = delete;
    two_wheeled& operator=(two_wheeled&&) = delete;

    virtual ~two_wheeled();
    virtual enum kind get_kind() const = 0;

    bool is_bicycle();
    bool is_skateboard();

    class my_namespace::bicycle* as_bicycle();
    class my_namespace::skateboard* as_skateboard();

    void traverse(visitor& v);
};

class bicycle: public transport_method, public two_wheeled
{
    public:
    std::unordered_map<std::string, std::string> properties;
    bool bought_for_exercise;
    int year_purchased;
    bicycle();
    bicycle(const bicycle&) = delete;
    bicycle(bicycle&&) = delete;
    bicycle& operator=(const bicycle&) = delete;
    bicycle& operator=(bicycle&&) = delete;

    virtual ~bicycle();
    virtual enum kind get_kind() const;
    bool is_transport_method();
    bool is_two_wheeled();

    class my_namespace::transport_method* as_transport_method();
    class my_namespace::two_wheeled* as_two_wheeled();

    void traverse(visitor& v);
};

class skateboard: public two_wheeled
{
    public:
    int year_purchased;
    skateboard();
    skateboard(const skateboard&) = delete;
    skateboard(skateboard&&) = delete;
    skateboard& operator=(const skateboard&) = delete;
    skateboard& operator=(skateboard&&) = delete;

    virtual ~skateboard();
    virtual enum kind get_kind() const;
    bool is_two_wheeled();

    class my_namespace::two_wheeled* as_two_wheeled();

    void traverse(visitor& v);
};

class gps
{
    public:
    my_namespace::location* home_location;
    gps();
    gps(const gps&) = delete;
    gps(gps&&) = delete;
    gps& operator=(const gps&) = delete;
    gps& operator=(gps&&) = delete;
    ~gps();

 /* this will appear in the generated .h */

    void traverse(visitor& v);
};

class person
{
    // deliberate private structs and types not for the eyes of the developer
    struct person_boy {
        ~person_boy();
        bool has_short_hair;
        bool is_a_gamer;
    };

    struct person_girl {
        ~person_girl();
        bool has_long_hairs;
        bool has_many_dolls;
    };

    union person_person_is_either_boy_or_girl_v {
        person_person_is_either_boy_or_girl_v() {};
        ~person_person_is_either_boy_or_girl_v() {};
        person_boy boy;
        person_girl girl;
    };

    public:
    enum class v_ {
        none,
        boy,
        girl,
    };
    v_ v_kind = v_::none;
    person_person_is_either_boy_or_girl_v v;

    std::string name;
    std::vector<my_namespace::person*> friends;
    person();
    person(const person&) = delete;
    person(person&&) = delete;
    person& operator=(const person&) = delete;
    person& operator=(person&&) = delete;
    ~person();
    void set_v_kind(v_ kind);

    void traverse(visitor& v);
};

class location
{
    public:
    int lat;
    int lon;
    location();
    location(const location&) = delete;
    location(location&&) = delete;
    location& operator=(const location&) = delete;
    location& operator=(location&&) = delete;
    ~location();

    void traverse(visitor& v);
};

class visitor
{
    public:
    visitor();
    virtual ~visitor();

    virtual bool visit(my_namespace::transport_method* t);

    virtual bool visit(my_namespace::car* c);
    virtual void post_visit(my_namespace::car* c);

    virtual bool visit(my_namespace::two_wheeled* t);

    virtual bool visit(my_namespace::bicycle* b);
    virtual void post_visit(my_namespace::bicycle* b);

    virtual bool visit(my_namespace::skateboard* s);
    virtual void post_visit(my_namespace::skateboard* s);

    virtual bool visit(my_namespace::gps* g);
    virtual void post_visit(my_namespace::gps* g);

    virtual bool visit(my_namespace::person* p);
    virtual void post_visit(my_namespace::person* p);

    virtual bool visit(my_namespace::location* l);
    virtual void post_visit(my_namespace::location* l);

};

}

#endif

```

```cpp
#include "example.h"

using namespace my_namespace;

my_namespace::transport_method::transport_method()
{
}

my_namespace::transport_method::~transport_method()
{
    for (auto& it: riders_sorted_by_age) if (it.second) delete it.second;
}

bool my_namespace::transport_method::is_car()
{
    return get_kind() == kind::car;
}

bool my_namespace::transport_method::is_bicycle()
{
    return get_kind() == kind::bicycle;
}

class car* my_namespace::transport_method::as_car()
{
    assert(get_kind() == kind::car);
    return static_cast<class car*>(this);
}

class bicycle* my_namespace::transport_method::as_bicycle()
{
    assert(get_kind() == kind::bicycle);
    return static_cast<class bicycle*>(this);
}

void transport_method::traverse(visitor& __v__)
{
    for (auto& _it: riders_sorted_by_age) if (_it.second) _it.second->traverse(__v__);
}

my_namespace::car::car()
: colour(colours::red), owner(nullptr), gps(nullptr), brand(nullptr)
{
}

my_namespace::car::~car()
{
    if (the_owner_bought_this && gps) delete gps;
    if (brand) delete brand;
}

kind car::get_kind() const
{
    return kind::car;
}

bool car::is_transport_method()
{
    return true;
}

class my_namespace::transport_method* car::as_transport_method()
{
    return static_cast<class my_namespace::transport_method*>(this);
}

void car::traverse(visitor& __v__)
{
    if (!__v__.visit(this)) return;
    if (gps) gps->traverse(__v__);
    transport_method::traverse(__v__);
    __v__.post_visit(this);
}

my_namespace::two_wheeled::two_wheeled()
{
}

my_namespace::two_wheeled::~two_wheeled()
{
}

bool my_namespace::two_wheeled::is_bicycle()
{
    return get_kind() == kind::bicycle;
}

bool my_namespace::two_wheeled::is_skateboard()
{
    return get_kind() == kind::skateboard;
}

class bicycle* my_namespace::two_wheeled::as_bicycle()
{
    assert(get_kind() == kind::bicycle);
    return static_cast<class bicycle*>(this);
}

class skateboard* my_namespace::two_wheeled::as_skateboard()
{
    assert(get_kind() == kind::skateboard);
    return static_cast<class skateboard*>(this);
}

void two_wheeled::traverse(visitor& __v__)
{
}

my_namespace::bicycle::bicycle()
{
}

my_namespace::bicycle::~bicycle()
{
}

kind bicycle::get_kind() const
{
    return kind::bicycle;
}

bool bicycle::is_transport_method()
{
    return true;
}

bool bicycle::is_two_wheeled()
{
    return true;
}

class my_namespace::transport_method* bicycle::as_transport_method()
{
    return static_cast<class my_namespace::transport_method*>(this);
}

class my_namespace::two_wheeled* bicycle::as_two_wheeled()
{
    return static_cast<class my_namespace::two_wheeled*>(this);
}

void bicycle::traverse(visitor& __v__)
{
    if (!__v__.visit(this)) return;
    transport_method::traverse(__v__);
    two_wheeled::traverse(__v__);
    __v__.post_visit(this);
}

my_namespace::skateboard::skateboard()
{
}

my_namespace::skateboard::~skateboard()
{
}

kind skateboard::get_kind() const
{
    return kind::skateboard;
}

bool skateboard::is_two_wheeled()
{
    return true;
}

class my_namespace::two_wheeled* skateboard::as_two_wheeled()
{
    return static_cast<class my_namespace::two_wheeled*>(this);
}

void skateboard::traverse(visitor& __v__)
{
    if (!__v__.visit(this)) return;
    two_wheeled::traverse(__v__);
    __v__.post_visit(this);
}

my_namespace::gps::gps()
: home_location(nullptr)
{
}

my_namespace::gps::~gps()
{
    if (home_location) delete home_location;
}


 /* this will appear in the generated .cpp */
void gps::traverse(visitor& __v__)
{
    if (!__v__.visit(this)) return;
    if (home_location) home_location->traverse(__v__);
    __v__.post_visit(this);
}

my_namespace::person::person_boy::~person_boy()
{
}

my_namespace::person::person_girl::~person_girl()
{
}

my_namespace::person::person()
{
}

my_namespace::person::~person()
{
    switch (v_kind) {
    case v_::none: break;
    case v_::boy: v.boy.~person_boy(); break;
    case v_::girl: v.girl.~person_girl(); break;
    }
    for (auto& it: friends) if (it) delete it;
}

void my_namespace::person::set_v_kind(v_ kind)
{
    switch (v_kind) {
    case v_::none: break;
    case v_::boy: v.boy.~person_boy(); break;
    case v_::girl: v.girl.~person_girl(); break;
    }
    v_kind = kind;
    switch (v_kind) {
    case v_::none: break;
    case v_::boy: new (&v.boy) person_boy(); break;
    case v_::girl: new (&v.girl) person_girl(); break;
    }
}

void person::traverse(visitor& __v__)
{
    if (!__v__.visit(this)) return;
    switch(v_kind) {
    case v_::boy:
        break;
    case v_::girl:
        break;
    default:
        break;
    }
    for (auto& _it: friends) if (_it) _it->traverse(__v__);
    __v__.post_visit(this);
}

my_namespace::location::location()
{
}

my_namespace::location::~location()
{
}

void location::traverse(visitor& __v__)
{
    if (!__v__.visit(this)) return;
    __v__.post_visit(this);
}


visitor::visitor()
{
}

visitor::~visitor()
{
}

bool visitor::visit(my_namespace::transport_method* t)
{
    return true;
}

bool visitor::visit(my_namespace::car* c)
{
    return true;
}

void visitor::post_visit(my_namespace::car* c)
{
}

bool visitor::visit(my_namespace::two_wheeled* t)
{
    return true;
}

bool visitor::visit(my_namespace::bicycle* b)
{
    return true;
}

void visitor::post_visit(my_namespace::bicycle* b)
{
}

bool visitor::visit(my_namespace::skateboard* s)
{
    return true;
}

void visitor::post_visit(my_namespace::skateboard* s)
{
}

bool visitor::visit(my_namespace::gps* g)
{
    return true;
}

void visitor::post_visit(my_namespace::gps* g)
{
}

bool visitor::visit(my_namespace::person* p)
{
    return true;
}

void visitor::post_visit(my_namespace::person* p)
{
}

bool visitor::visit(my_namespace::location* l)
{
    return true;
}

void visitor::post_visit(my_namespace::location* l)
{
}
```

# Syntax

Natsuki reads a dsl written in natsuki syntax, and generates c++ code.

```bnf
natsuki_file ::= declaration { declaration }
declaration ::= include | attributes | type
```

## Types

A *natsuki type* is either a *node*, a *subnode*, an *enumeration*, a *custom
type* or a *union*.

```bnf
type ::= node | subnode | enumeration | custom_type | union
```

*Nodes* are implemented as c++ classes.  
*Subnodes* are implemented as c++ classes that extend other classes.  
*Enumerations* are implemented as c++ enum classes.  
*Custom types* are implemented as aliases to c++ types.  
*Unions* are implemented as c++ unions.

Note that the generated c++ code may not appear in the order it was specified
in the dsl. Natsuki will always generate forward declaration as a preemptive
action against compilation errors.

### Nodes

A *node* has a name, a bunch of *fields*, a bunch of *verbatims* and a bunch of
*subnodes*.

A *node* starts with the `class` keyword, followed by its name, optional
*fields* within an optional `()`, and optional `{}` that contains custom c++
code. The *node* declaration is finally complete with a `;`.

```bnf
node ::= [ attributes ] CLASS name [ fields ] [ custom_code ] ; { subnodes }
subnodes ::= subnode { subnode }
```

For example, to generate three c++ classes each named: *A*, *X* and *Y*, do:

```cpp
class A(b: B, c?: C, d&: D, e: E[]); // Cpp class has fields named b,c,d,e
class B() private: {};
class C public: { /* put valid c++ code here. Can be multilined */ };
class D;
class E();
```

Note that a *node* need not define *fields* even if `()` is specified.

### Subnodes

A *subnode* is specified by appending `->` to a *node* or to a *subnode*.

```bnf
subnode ::= -> CLASS name [ fields ] [ custom_code ] ;
```

The *parent* of the subnode is the last non subnode declared in the dsl.
There is a limit of 1 to the hierarchy of subnodes. ie. subnodes of subnodes is
illegal.
The *subnode* is said to be the *child* of the *parent* node.

There is no limit to the number of *parents* a *subnode* can have. This is left
to the programmer's discretion.

*Parent nodes* and *subnodes* are respectively implemented in c++ as abstract/
virtual classes and classes that extend the abstract/virtual classes.

The c++ generated for a *parent* node has these following additional virtual
methods defined:

```cpp
virtual kind get_kind();
bool is_##subnode_name##();           // true if child is specified kind
##subnode##* as_##subnode_name##();   // returns child class if child is of
                                      // specified kind. Else throw except
```

The generated c++ for a *subnode* has the following additional methods:

```cpp
kind get_kind() { return kind_t::name; };
##node##* as_##node_name##();         // returns as parent class
```

### Enumerations

An *enumeration* is useful to specify a *field* which can only be one of a
known discrete value.

An *enumeration* starts with the `enum` keyword followed by its name, `{`.
Specify the possible literals of the enumeration. `}` and `;` completes the
declaration of the *enumeration*.

```bnf
enumeration ::= ENUM name { enumeration_literal { , enumeration_literal } } ;
```

*Name* is limited to characters [a-zA-Z_]+  
*enumeration_literal* is limited to characters [a-zA-Z_]+

### Custom types

By default *natsuki types* are *nodes*, *enumerations* or *unions*. Custom
types can be specified by the `#define` keyword followed by the name of the
custom type and the custom type. The declaration is completed with a `;`.

There is no limit to the number *custom types* that is defined.

```bnf
custom_type ::= #DEFINE name type;
```

*Name* is limited to characters [a-zA-Z_]+  
*Type* is limited to characters [a-zA-Z_:<>*]+?

By default, custom types are owned, not optional, not visitable fields. If the
*type* of the custom type ends with a `*`, natsuki assumes that this is a ptr.

For example, to define three custom types integer, token and an owned pointer
to a declarative region - and use these to define fields of a class *abcd*:

```cpp
#define integer int;
#define token vhdl::token;
#define dclrgn std::vector<vhdl::analyser::declarative_region*>;
class abcd(xd: integer?, operation?: token, region: dclrgn);
```

This is not the case with custom types.

### Unions

A *natsuki union* is helpful when there is a need to model variants in c++
without the use of c++ inheritance.

A *union* begins with the `union` keyword followed by its name, `{`.
Specify the possible types of the union and complete it with `}` and `;`.

```bnf
union ::= UNION name { union_node { union_node } } ;
        | UNION name union_node_name [ fields ] ;
union_node ::= CLASS union_node_name [ fields ] ;
```

*Name* is limited to characters [a-zA-Z_]+  
*union_node_name* is limited to characters [a-zA-Z_]+

## Fields

A *natsuki field* has a *name* and a *value* of *type* spcified.

The syntax defines a field with the name of the field, followed by `:` and its
*type*.

```bnf
fields ::= ( [ field { , field } ] )
field ::= [ attributes ] field_names [?] [&] : field_type [ [] { [] } ]
field_names ::= field_name { field_name }
```

*Field_name* is limited to characters [a-zA-Z_]+  
*Field_type* is limited to characters [a-zA-Z_]+

By default natsuki generates a *regular field*.

A *regular field* is a field which should contain a value.
The value is owned by the node and is visitable.

A *regular field* is implemented as a c++ pointer.

Note that due to c++ implementation details, a *regular field* is kind of
allowed to not contain a value.
This is not a feature, and may be changed without notice.

### Decorated fields

Natsuki can generate fields with modified properties. These properties are said
to decorate a field.

Properties are such that a field can be:

- optional

  An *optional field* is a *field* which contains a value or not.
  The default *value* is always null.
  Therefore, the user is advised to check whether an optional field has a
  value.

  The *value* is owned by the c++ class.

  Use the `?` shortcut following the field name.  
  Alternatively attach the `optional` attribute to the field. See the chapter
  **Attributes** for more info.

- a reference

  A *reference field* is a field whose value is not owned by a node.

  Use the `&` shortcut after the field name.  
  Alternatively attach the `reference` attribute to the field. See the section
  **Attributes** for more information.

  By default, a *reference field* has *optional* value and is not visitable.
  The default *value* is always null.
  Therefore, the user is advised to check whether the reference refers to an
  actual value.

  **Note:** the property *optional* is automatically inferred when specifying a
  *reference*. It is an error to specify both properties at the same time.

  A *reference* is not visitable by default.

- an object

  The *value* of this field is a c++ *object* (by default, the value of a field
  is a *pointer*).

  **Note:** we dont recommend *objects* of *natsuki types*. Do it at ur own 
  risks

  To represent optional objects, such field is implemented as a std::optional.
  To represent references to objects, such field is implemented as a non owning
  pointer.

  Attach the `object` attribute to the field. See **Attributes** for more info.

- a map

  The field will contain a std::unordered_map of key,values of specified type.
  The size of the map needs not be defined.

  Use the `<key_type>` shortcut following the type of the value.
  Alternatively attach the `map` attribute to the field. See **Attributes**
  for more information.

  Note this property is ***mutually exclusive*** to *optional*, *darray* and
  *array*. Therefore, you can neither define a map of optional values nor an
  optional map of values.
  For the first case, jz dont include the value in the map.
  For the second case, can you do with a map of size zero?
  For the third case, y?

  Map of arrays are also not allowed. Ditto for maps of arrays of arrays.

  Note 1: the key of a map is always owned by its node. Reference keys are not
  allowed  
  Note 2: the key of a map must be a custom type which is comparable to be used
  as a key to an unordered map.  
  Note 3: Values of a map can be references.

- an array

  The field will contain a std::vector of values of the specified type.
  The size of the vector needs not be defined.

  Use the `[]` shortcut following the type.  
  Alternatively attach the `array` attribute to the field. See **Attributes**
  for more information.

  Note this property is ***mutually exclusive*** to *optional*, *darray* and
  *map*. Therefore, you can neither define an array of optional values nor an
  optional array of values.
  For the first case, jz dont include the value in the array.
  For the second case, can you do with an array of size zero?
  For the third case, y?

  Ditto for arrays of arrays and maps.

- an array of array

  The field will contain a std::vector of std::vector of values of the type
  specified.  The size of the vector needs not be defined.

  Use the `[][]` shortcut following the type.  
  Alternatively attach the `darray` attribute to the field. See **Attributes**
  for more information.

  Note this property is ***mutually exclusive*** to *optional*, *array* and
  *map*.  Therefore, you can neither define an array of array of maps, arrays
  or optional values.

- conditionally owned

  The ownership of the value can be decided at runtime by changing a *guard*.

  This was added to the dsl to model *nodes* whose ownership is shared between
  multiple *nodes*.

  This is an ***exclusive property*** to *reference* and *object*.
  Therefore you cannot define a conditionally owned references or objects.

  Attach the `cowned` attribute to the field. See **Attributes** for more
  detail.

  Note that thinking of this now, we could get away with std::shared_ptr, shall
  we?

Examples with shortcuts:

```cpp
name1&: type    // name1 is referencing an object from somewhere else
name2: type<k>  // name2 is an map of type type whose key is of type k.
name3&: type[]  // name3 is an array of references of type
name4?: type    // name4 is optional
```

The above can be specified using attributes and will have the same meaning:

```cpp
[[reference]]       name1: type
[[map=k]]           name2: type
[[reference,array]] name3: type
[[optional]]        name4: type
```

You can even mix and match shortcuts with attributes:

```cpp
[[cowned]] name5?: type[]
```

Jz dont do something stupid:

```cpp
[[object,cowned]] name6: type; // invalid
```

Note that these decorations can only be applied to fields whose type is either
a node or a subnode.

Therefore, you cannot decorate enumerations or custom types.

## Custom c++ code

Natsuki dsl can describe custom c++ values/functions/you name it that must be
present in the generated c++.
This is done in the dsl. Append the specifier, `{`, the custom c++ code and `}`
to the class.

```bnf
custom_code ::= NOTHING
                | custom_code private_custom_code
                | custom_code public_custom_code

private_custom_code ::= { .* } | PRIVATE { .* }
public_custom_code ::= PUBLIC { .* }
```

If no specifier is specified, `private` is assumed by default.
Note the `{}` must preceed the `;` corresponding to the class.

There are two specifiers

- public

  Cpp code present within the `{}` preceding the public: keyowrd is copied to
  the generated .c++ file

- private

  Cpp code present within the `{}` preceding the private: keyword is copied to
  the gernerated .h file

Natsuki does not care about the order of custom c++ codes.
Natsuki always write custom c++ codes at the end of the c++ object.

Natsuki does not imposes a limit of *custom codes* per class.
That is, you can have either a *public custom code* or a *private custom code*,
or none, or even both at the same time. You can also have more than 1 public or
1 private *custom code* per class.

Note the c++ code within `{}` is *verbatim* and copied word for word. Beware of
syntax errors.

For example,

```cpp
class X()                                                    // notice no ;
public: { /* put valid c++ code here. Can be multilined */ } // notice no ;
private: { /* put valid c++ code there. Can be multilined */ };

class Y()
private: { /* code */ }     // note private before public is allowed too
public: { /* code */ };     // also note the ; is at the end
```

## Inheritance of subnodes

Natsuki has the ability to create a hierarchy c++ *parent* classes and *child*
classes. This is useful when the programmer wants to model complex asts.

Consider the case of vhdl.

A design file consists of one or more design units. A design unit is either of
an entity, an architecture, configuration, package body or package.

Natsuki models this in this way

```cpp
class design_file(units: design_unit[]);
class design_unit;
-> class entity(ports: port[]) private: {/* private custom code */};
-> class architecture;
-> class configuration;
-> class package_body;
-> class package;       // subclass of design_unit
```

This generates the following classes in c++ (*parent* and *children*)

```cpp
class deisgn_unit {};
class entity: public design_unit
{
    std::vector<port> port;
    /* private custom code */
};
class architecture: public design_unit {};
// etc ...

class another_class {};
```

Natsuki is able to generate classes that inherit from one or more classes.
Consider the previous example, natsuki generates an entity, an architecture, a
configuration ... class that inherits one parent (design unit).

However, in vhdl, an entity is also a declaration, as well as a design unit.

To assist this, natsuki allows entity to inherit from multiple classes:

```cpp
class design_unit;              // we saw that
-> class entity(ports: port[]);

class declaration;
-> class entity private: {/* private custom code */};

class declarative_region;
-> class entity(decls: declarations[])

// note no need to redefine ports of entity the second time
// the public custom code is defined here
```

This generates the followign c++ code:

```cpp
class deisgn_unit {};
class declaration {};
class entity: public design_unit, public declaration
{
    std::vector<port> port;
    std::vector<declarations> decls;
    /* private custom code */
};
// The generated c++ code contains both the port[] as well as the private
// custom code
```

*Fields*, public *custom code*, private *custom code* and other properties of a
*child* class can be defined at more than one place. However, you cannot
redefine a *field* once defined.

Again, there is a limit of 1 to the hierarchy of children classes. ie. *child*
of a *child* class is illegal.

## Attributes

*Attributes* customises the behaviour of natsuki and is used to give it more
info on how to generate the c++.

```bnf
attributes ::= [[ attribute { , attribute } ]]
attribute ::= name [ = value ]
```

*Name* is limited to characters [a-zA-Z_]+  
*Value* is limited to characters [a-zA-Z_/<>"]+

Some attributes are ***mutually exclusive*** as described in this table:

|              | optional | reference | pointer | object | map | array | cowned | istrait | visitable | notvisitable |
|--------------|----------|-----------|---------|--------|-----| ------|--------|---------|-----------|--------------|
| optional     |          |    NO     |   ok    |   ok   | NO  |  NO   |   ok   |   ok    |    ok     |     ok       |
| reference    |  INFERS  |           |   ok    |   ok   | ok  |  ok   |   NO   |   ok    |    ok     |   INFERS     |
| pointer      |    ok    |    ok     |         |   NO   | ok  |  ok   |   ok   |   ok    |    ok     |     ok       |
| object       |    ok    |    ok     |   NO    |        | ok  |  ok   |   NO   |   ok    |    ok     |     ok       |
| map          |    NO    |    ok     |   ok    |   ok   |     |  NO   |   ok   |   ok    |    ok     |     ok       |
| array        |    NO    |    ok     |   ok    |   ok   | NO  |       |   ok   |   ok    |    ok     |     ok       |
| cowned       |    ok    |    NO     |   ok    |   NO   | ok  |  ok   |        |   ok    |    ok     |     ok       |
| istrait      |    ok    |    ok     |   ok    |   ok   | ok  |  ok   |   ok   |         |    ok     |     ok       |
| visitable    |    ok    |    ok     |   ok    |   ok   | ok  |  ok   |   ok   |   ok    |           |     NO       |
| notvisitable |    ok    |    NO     |   ok    |   ok   | ok  |  ok   |   ok   |   ok    |    NO     |              |

Combinations reading NO are not allowed. Eg: optional arrays are not allowed.

Combinations reading INFERS indicates that one attribute automatically infers
the other. Eg: reference fields are automatically optional.

### Valid file attributes

- namespace

  By default natsuki generates all c++ classes in the global namespace. This
  can be overwritten with the `namespace` attribute.

  Create nested namespaces by seperating the subnamespace(s) from the parent
  namespace(s) with `::`.

  For example, to create a namespace ***abcd***, which consists of namespace
  ***efgh***, which consists of namespace ***ijkl***, which consists of
  whatever generated classes, do:

  ```cpp
  [[namespace=abcd::efgh::ijkl]];
  ```

  Note that only one namespace attribute is valid per file.

- no_forward_declaration

  [what is this](https://www.learncpp.com/c++-tutorial/forward-declarations/)

  By default, natsuki forward declares generated c++ classes automatically.
  Disable this by specifying the `no_forward_declaration` attribute.

  Note that only one `no_forward_declaration` attribute is valid per file.

- json_serialisable

  By default, natsuki generated c++ cannot be serialised/deserialised to json.
  Enable this by specifying the `json_serialisation` attribute.

  Json serialisation follows common serialisation interface.
  See common/serialisation.h

  Note that natsuki assumes custom types follow the same common serialisation
  interface too.

  Note that only one `json_serialisation` attribute is valid per file.

- visitable

  If the `visitable` attribute is present, natsuki will generate a visitor for
  the generated c++ class.

  Note that only one `visitable` attribute is valid per file.

  - dumpable

  If the `dumpable` attribute is present, natsuki will generate a class that is
  able to dump data stored in the generated c++ class.

  Useful for debugging.
  Note that only one `dumpable` attribute is valid per file.

### Valid class attributes

By default natsuki always generates these when generating nodes:

- a constructor. This is important to initialise the class and its parent(s).
- a default copy constructor
- a default move constructor
- a default copy assignment operator
- a default copy assignment operator
- a destructor. This is important to destroy pointers owned by a class.

This can be toggled off by specifing attributes as follows.

| Attribute name | Description                                          |
|----------------|------------------------------------------------------|
| noctor         | Does not generate a default constructor              |
| nocpct         | Does not generate a default copy constructor         |
| nomvct         | Does not generate a default move constructor         |
| nocpas         | Does not generate a default copy assignment operator |
| nomvas         | Does not generate a default copy assignment operator |
| nodtor         | Does not generate a destructor                       |

### Valid custom type attributes

By default custom types specified to natsuki are *non-visitable* *non-optional*
*owning* object. This can be customised by specifying *attributes* as follows.
See the section **Custom fields** for more info.

- optional
- reference
- pointer
- visitable

### Valid field attributes

By default natsuki *fields* are *visitable* *owning* *non-optional* *pointers*.
This can be customised by specifing *attributes* as follows. See the section
**Custom fields** for more info on some of these *attributes*.

- optional
- reference
- object
- map
- array
- cowned
- istrait

  *Fields* in a *parent* natsuki node, that are decorated with the `istrait`
  *attribute*, are automatically replicated to the child node.

  Note *trait* for natsuki does not mean c++ traits. It is rather like c#'s
  interface or php's trait system.

- visitable

  *Reference fields* are not *visitable* by default. Toggle this behaviour with
  the `visitable` attribute.

- notvisitable

  *Regular fields*, *optional* fields, *object* fields, *array* fields or 
  *conditionally owned fields* are *visitable* by default. Toggle this
  behaviour by specifying the `notvisitable` attribute.

## Include

Include c++ header files.

```bnf
include ::= #include file
```

*File* is limited to characters [a-zA-Z_/<>."]+

## Comments

Comments follows a subset of the c++ standard namely `//`

Verbatim code contains c++ standard code. So comments there are `/* */` and
`//`.

## Keywords

Keywords are not case sensitive.
Keywords include:

- `class`
- `for`
- `#include`
- `#define`
- `enum`
- `public`
- `private`

# Usage

Help menu:

```
usage: natsuki.py [-h] [-o OUTPUT] [-b] [-c] ASTFILE

generator of vhdl ast class for use in vhdlstuff

positional arguments:
  ASTFILE               input ast file(s)

optional arguments:
  -h, --help            show this help message and exit
  -o OUTPUT, --output OUTPUT
                        output directory/file without .h or .cpp extension
  -b, --generate_h      generate .h file corresponding to ASTFILE
  -c, --generate_cpp    generate .cpp file corresponding to ASTFILE
```
Do:

```
python natsuki.py --generate_h vhdl.ast -o outputfile
python natsuki.py --generate_cpp vhdl.ast -o outputfile
```

# Misc

[ ] We should really use c++ concepts - when available - to represent concepts
[x] Subclass to represent inheritence.
[ ] Currently subclass do office both of concepts and inheritence.

