#include "smbtable.hxx"

smbTable::smbTable() {
    scopes.emplace_back();
}

void smbTable::enterscope() {
    scopes.emplace_back();
}

void smbTable::exitscope() {
    if (scopes.size() > 1) {
        scopes.pop_back();
    }
}

bool smbTable::declare(const symbol& sym) {
    if (scopes.empty()) {
        return false;
    }

    auto& currentScope = scopes.back();

    auto [it, inserted] =
        currentScope.emplace(sym.name, sym);

    return inserted;
}

symbol* smbTable::lookupcurrent(const std::string& name) {
    if (scopes.empty()) {
        return nullptr;
    }

    auto& currentScope = scopes.back();

    auto it = currentScope.find(name);

    if (it == currentScope.end()) {
        return nullptr;
    }

    return &it->second;
}

symbol* smbTable::lookup(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);

        if (found != it->end()) {
            return &found->second;
        }
    }

    return nullptr;
}