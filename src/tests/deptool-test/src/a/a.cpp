#include "a.h"
Actor::Actor( std::string _name) {
    name = _name;
}

std::string Actor::toString() const {
    return name;
}
