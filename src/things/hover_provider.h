
#ifndef THINGS_HOVER_PROVIDER_H
#define THINGS_HOVER_PROVIDER_H

#include "fmt/format.h"
#include "vhdl_syntax.h"

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace things
{

class vhdl_hover_provider: public vhdl::syntax::visitor
{
    template <typename... T>
    void hover(std::string fmt, T&&... args)
    {
        if (found)
            return;

        auto f = fmt::vformat(fmt, fmt::make_format_args(args...));
        w->StartObject();
        w->Key("kind"); w->String("markdown");
        w->Key("contents"); w->String(fmt::format("```\n{}\n```", f));
        w->EndObject();
        found = true;
    }

    void hover_identifier_denotes_entity(std::string_view name, std::vector<vhdl::node::named_entity*> denotes)
    {
        if (found)
            return;

        std::string str;
        switch (denotes.size()) {
        case 0:
            str = fmt::format("`{}` not found", name);
            break;
        case 1:
            hover(denotes[0]);
            return;
        default:
            str = fmt::format("{} overloads for `{}`", denotes.size(), name);
            break;
        }

        w->StartObject();
        w->Key("kind"); w->String("markdown");
        w->Key("contents"); w->String(str);
        w->EndObject();
        found = true;
    }

    public:
    vhdl_hover_provider(rapidjson::Writer<rapidjson::StringBuffer>*, bool&, common::position);
    ~vhdl_hover_provider();

    void hover(vhdl::node::named_entity*);
    void hover(vhdl::syntax::type_definition* t, int i);
    void hover(vhdl::syntax::element_declaration* e, int);
    void hover(vhdl::syntax::declarative_item* d);
    void hover(vhdl::syntax::object* i, int);
    void hover(vhdl::syntax::interface* i, int);
    void hover(vhdl::syntax::design_unit* d);
    void hover(vhdl::syntax::context_item* c, int);

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
    bool& found;
    common::position position;
};

}

#endif
