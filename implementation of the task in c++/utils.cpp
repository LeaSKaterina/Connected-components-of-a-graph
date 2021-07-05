#include "utils.h"
#include <iostream>

void printEl (const std::unique_ptr<ScMemoryContext>& context, ScAddr element)
{
    ScType type;
    type = context->GetElementType(element);

    if (type.IsNode() == ScType::Node)
    {
        std::string data;
        data = context->HelperGetSystemIdtf(element);
        std::cout << data;
    }
    else
    {
        ScAddr elem1, elem2;
        elem1 = context->GetEdgeSource(element);
        elem2 = context->GetEdgeTarget(element);
        std::cout << "(";
        printEl(context, elem1);
        std::cout << " -- ";
        printEl(context, elem2);
        std::cout << ")";
    }
}
