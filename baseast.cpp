#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "baseast.hpp"
#include "consts.hpp"

void AST::FunctionDefinition::print()
{
    std::cout << std::endl
              << "-------------------" << std::endl;
    std::cout << "FnName: " << proto->fnName << std::endl;
    std::cout << "FnType: " << static_cast<int>(proto->fnType) << std::endl;
    std::cout << "Args :" << std::endl;
    for (auto arg : proto->fnParams)
    {
        std::cout << arg.name << " " << static_cast<int>(arg.type);
        std::cout << std::endl;
    }
}

AST::PrintStmt::PrintStmt(std::vector<std::unique_ptr<BaseExpr>> &vec)
{
    args.clear();
    for (int i = 0; i < vec.size(); i++)
    {
        args.push_back(std::move(vec[i]));
    }
}

AST::FnCallExpr::FnCallExpr(std::string name,
                            std::vector<std::unique_ptr<BaseExpr>> &fnArgs1)
                            : name(name)
{
    fnArgs.clear();
    for (int i = 0; i < fnArgs1.size(); i++)
    {
        fnArgs.push_back(std::move(fnArgs1[i]));
    }
}