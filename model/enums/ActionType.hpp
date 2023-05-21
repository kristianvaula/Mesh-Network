#ifndef MESH_NETWORK_ACTION_TYPE_H
#define MESH_NETWORK_ACTION_TYPE_H
#include <map>
#include <string>

enum class ActionType {
    REMOVE_NODE,
    MOVETO,
    HELLO,
    REPLACE,
    NONE
};

std::map<ActionType, std::string> actionTypeToString{
    {ActionType::REMOVE_NODE, "REMOVE_NODE"},
    {ActionType::MOVETO, "MOVETO"},
    {ActionType::HELLO, "HELLO"},
    {ActionType::REPLACE, "REPLACE"},
    {ActionType::NONE, "NONE"}
};

ActionType actionFromString (const std::string& str) {
    for (const auto& pair : actionTypeToString) {
        if (pair.second == str) {
            return pair.first; 
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

#endif //MESH_NETWORK_ACTION_TYPE_H