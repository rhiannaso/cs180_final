#pragma once
#ifndef _SHAPEI_H_
#define _SHAPEI_H_

#include "Shape.h"
#include <string>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_obj_loader/tiny_obj_loader.h>

class Program;

class ShapeInst : public Shape {
public:
	ShapeInst(int howMany) : Shape{} {
		amount = howMany;
	}
	~ShapeInst() {}
	void init(glm::mat4* modelMatrices);
	void draw(const std::shared_ptr<Program> prog) const;
    void update(glm::mat4* modelMatrices);
private:
	int amount;
	//glm::mat4 *modelMatrices;
};

#endif
