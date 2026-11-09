#pragma once

#include "../symbol/symbol.hxx"

#include <unordered_map>
#include <vector>
#include <string>

class smbTable {

private:

    using scope =
        std::unordered_map<std::string, symbol>;

    std::vector<scope> scopes;

public:

    smbTable();

    void enterscope();
    void exitscope();

    bool declare(const symbol& symbol);

    smbptr lookup(const std::string& name);

    smbptr lookupcurrent(const std::string& name);

};