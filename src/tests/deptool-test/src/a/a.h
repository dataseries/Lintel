#ifndef ACTOR__H
#define ACTOR__H
#include <string>

class Actor {
    std::string name;

public:
    Actor( std::string _name);
    std::string toString() const;
};

#endif
