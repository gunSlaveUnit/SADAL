//
// Created by gunSlaveUnit on 2/18/22.
//

#ifndef SHATTEREDSOULS_TRANSFORMATION_H
#define SHATTEREDSOULS_TRANSFORMATION_H

#include <glm/glm.hpp>


struct Transformation {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};


#endif //SHATTEREDSOULS_TRANSFORMATION_H
