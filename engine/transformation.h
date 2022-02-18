//
// Created by gunSlaveUnit on 2/18/22.
//

#ifndef SHATTEREDSOULS_TRANSFORMATION_H
#define SHATTEREDSOULS_TRANSFORMATION_H

#include <glm/glm.hpp>


struct Transformation {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
};


#endif //SHATTEREDSOULS_TRANSFORMATION_H
