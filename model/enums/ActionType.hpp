#ifndef MESH_NETWORK_ACTION_TYPE_HPP
#define MESH_NETWORK_ACTION_TYPE_HPP

#include <map>
#include <string>

enum class ActionType {
    OK,
    REMOVE_NODE,
    MOVETO,
    HELLO,
    REPLACE,
    NONE, 
    REPLACE_SELF
};

extern std::map<ActionType, std::string> actionTypeToString;

ActionType actionFromString(const std::string& str);
std::string actionToString(const ActionType& action);

#endif // MESH_NETWORK_ACTION_TYPE_HPP
