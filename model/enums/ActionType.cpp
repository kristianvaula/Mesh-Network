#include "ActionType.hpp"

std::map<ActionType, std::string> actionTypeToString{
    {ActionType::OK, "OK"},
    {ActionType::REMOVE_NODE, "REMOVE_NODE"},
    {ActionType::MOVETO, "MOVETO"},
    {ActionType::HELLO, "HELLO"},
    {ActionType::REPLACE, "REPLACE"},
    {ActionType::NONE, "NONE"}
};

ActionType actionFromString(const std::string& str) {
    for (const auto& pair : actionTypeToString) {
        if (pair.second == str) {
            return pair.first;
        }
    }

    size_t underscorePos = str.find('_');
    if (underscorePos != std::string::npos) {
        std::string substring = str.substr(0, underscorePos);
        for (const auto& pair : actionTypeToString) {
            if (pair.second == substring) {
                return pair.first;
            }
        }
    }

    return ActionType::NONE;
}

std::string actionToString(const ActionType& action) {
    auto it = actionTypeToString.find(action);
    if (it != actionTypeToString.end()) {
        return it->second;
    }

    return "NONE";
}