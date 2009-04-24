#include <iostream>

#include "b.h"

BigTop::BigTop( Actor *a) {
    forActor = a;
}

void BigTop::show() {
    std::cout << forActor->toString() << std::endl;
}
