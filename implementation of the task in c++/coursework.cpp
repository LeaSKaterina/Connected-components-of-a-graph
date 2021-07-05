#include <iostream>
#include <vector>
#include "cpp/sc_addr.hpp"
#include "cpp/sc_memory.hpp"
#include "cpp/sc_iterator.hpp"
#include "utils.h"

using namespace std;

ScAddr graph, rrel_arcs, rrel_nodes, father;


void get_edge_vertexes(const std::unique_ptr<ScMemoryContext> &context, ScAddr edge, ScAddr &v1, ScAddr &v2) {
    v1 = context->GetEdgeSource(edge);
    v2 = context->GetEdgeTarget(edge);
}

bool find_vertex_in_set(const std::unique_ptr<ScMemoryContext> &context, ScAddr element, ScAddr set) {
    bool find = false;
    ScIterator3Ptr location = context->Iterator3(set, ScType::EdgeAccessConstPosPerm, ScType(0));

    while (location->Next()) {
        ScAddr loc = location->Get(2);

        if (loc != element) {
            find = false;
            continue;
        } else {
            find = true;
            break;
        }
    }
    return find;
}

void print_graph(const std::unique_ptr<ScMemoryContext> &context) {

    ScAddr arcs, nodes, v1, v2, printed_vertex;
    bool find;
    printed_vertex = context->CreateNode(ScType::Const);

    printEl(context, graph);
    std::cout << std::endl << "----------------------" << std::endl;

    ScIterator5Ptr it = context->Iterator5(graph, ScType::EdgeAccessConstPosPerm, ScType(0),
                                           ScType::EdgeAccessConstPosPerm, rrel_arcs);

    if (it->Next()) {
        arcs = it->Get(2);

        ScIterator3Ptr arcs_it = context->Iterator3(arcs, ScType::EdgeAccessConstPosPerm, ScType(0));

        while (arcs_it->Next()) {

            ScAddr t_arc = arcs_it->Get(2);

            get_edge_vertexes(context, t_arc, v1, v2);

            printEl(context, v1);
            std::cout << " -- ";
            printEl(context, v2);
            std::cout << std::endl;

            if (!find_vertex_in_set(context, v1, printed_vertex))
                context->CreateEdge(ScType::EdgeAccessConstPosPerm, printed_vertex, v1);
            if (!find_vertex_in_set(context, v2, printed_vertex))
                context->CreateEdge(ScType::EdgeAccessConstPosPerm, printed_vertex, v2);
        }
    }

    it = context->Iterator5(graph, ScType::EdgeAccessConstPosPerm, ScType(0), ScType::EdgeAccessConstPosPerm,
                            rrel_nodes);

    if (it->Next()) {
        nodes = it->Get(2);

        ScIterator3Ptr nodes_it = context->Iterator3(nodes, ScType::EdgeAccessConstPosPerm, ScType(0));


        while (nodes_it->Next()) {

            ScAddr t_node = nodes_it->Get(2);

            find = find_vertex_in_set(context, t_node, printed_vertex);

            if (!find) {
                printEl(context, t_node);
                std::cout << std::endl;
            }
        }
    }
}

void printComponent(const std::unique_ptr<ScMemoryContext> &context, ScAddr &checked_vertexes){
    cout<<"---"<<endl;
    ScIterator3Ptr it = context->Iterator3(checked_vertexes, ScType::EdgeAccessConstPosPerm, ScType(0));
    ScIterator3Ptr it2 = context->Iterator3(checked_vertexes, ScType::EdgeAccessConstPosPerm, ScType(0));
    it2->Next();
    if(!it2->Next()) {
        it->Next();
        cout << context->HelperGetSystemIdtf(it->Get(2))<<endl;
        return;
    }
    while(it->Next()){
        ScIterator3Ptr it_buffer = context->Iterator3(checked_vertexes, ScType::EdgeAccessConstPosPerm, ScType(0));
        while ((it_buffer->Next())){
            if (context->HelperCheckEdge(it->Get(2),it_buffer->Get(2),ScType::EdgeDCommon)){
                std::cout<<context->HelperGetSystemIdtf(it->Get(2))<<"--"<<context->HelperGetSystemIdtf(it_buffer->Get(2))<<endl;
            }
        }
    }
    ScIterator3Ptr to_delete = context->Iterator3(checked_vertexes, ScType::EdgeAccessConstPosPerm, ScType(0));
    while (to_delete->Next()){
        context->EraseElement(to_delete->Get(2));
    }
}


void DFS(const std::unique_ptr<ScMemoryContext> &context, ScAddr &begin){
    ScAddr checked_vertexes = context->CreateNode(ScType::Const);
    ScAddrList list;
    list.push_back(begin);
    while (!list.empty()){
        ScAddr current = list.back();
        if (context->HelperCheckEdge(checked_vertexes,current,ScType::EdgeAccessConstPosPerm)){
            list.pop_back();
            continue;
        }
        context->CreateEdge(ScType::EdgeAccessConstPosPerm, checked_vertexes, current);
        ScIterator3Ptr it = context->Iterator3(current, ScType::EdgeDCommon, ScType(0));
        while(it->Next()){
            list.push_back(it->Get(2));
        }
        list.pop_back();
    }
    printComponent(context,checked_vertexes);
}


void find_connected_components(const std::unique_ptr<ScMemoryContext> &context) {
    ScIterator5Ptr it = context->Iterator5(graph,
                                           ScType::EdgeAccessConstPosPerm,
                                           ScType(0),
                                           ScType::EdgeAccessConstPosPerm,
                                           rrel_nodes
    );
    it->Next();
    ScAddr nodes = it->Get(2);
    ScIterator3Ptr it_nodes = context->Iterator3(nodes, ScType::EdgeAccessConstPosPerm, ScType(0));

    while (it_nodes->Next()){
        ScAddr begin = it_nodes->Get(2);
        DFS(context,begin);
    }
}

void run_test(const std::unique_ptr<ScMemoryContext> &context, std::string number_test) {
    father = context->CreateNode(ScType::Const);

    std::string gr = "Gx";
    gr[1] = number_test[0];
    graph = context->HelperResolveSystemIdtf(gr);
    rrel_arcs = context->HelperResolveSystemIdtf("rrel_arcs");
    rrel_nodes = context->HelperResolveSystemIdtf("rrel_nodes");
    std::cout << "Graph: ";

    print_graph(context);

    std::cout << "Find connected components. " << std::endl;
    find_connected_components(context);

    std::cout << std::endl;
    context->EraseElement(father);
}

int main() {
    sc_memory_params params;

    sc_memory_params_clear(&params);
    params.repo_path = "/home/katsiaryna/ostis-web-platform/kb.bin";
    params.config_file = "/home/katsiaryna/ostis-web-platform/config/sc-web.ini";
    params.ext_path = "/home/katsiaryna/ostis-web-platform/sc-machine/bin/extensions";
    params.clear = SC_FALSE;

    ScMemory::Initialize(params);

    const std::unique_ptr<ScMemoryContext> context(new ScMemoryContext(sc_access_lvl_make_max, "coursework"));

    run_test(context, "0");
    run_test(context, "1");
    run_test(context, "2");
    run_test(context, "3");
    run_test(context, "4");

    cout << "The end." << endl;

    ScMemory::Shutdown(true);
    return 0;
}