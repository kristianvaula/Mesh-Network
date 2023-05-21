#ifndef MESH_NETWORK_ACTION_TYPE_H
#define MESH_NETWORK_ACTION_TYPE_H

#include <map>
#include <string>

enum class ActionType {
    OK,
    REMOVE_NODE,
    MOVETO,
    HELLO,
    REPLACE,
    NONE
};

extern std::map<ActionType, std::string> actionTypeToString;

ActionType actionFromString(const std::string& str);
std::string actionToString(const ActionType& action);

#endif // MESH_NETWORK_ACTION_TYPE_H
