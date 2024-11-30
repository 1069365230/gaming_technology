//#pragma once
#ifndef MYCUBE_H_
#define MYCUBE_H_

#include <iostream>

using namespace ve;
using namespace std;

class Mycube {

private:
	VESceneNode * node;
	VESceneNode * parentnode;

	//string name;
	//my own class would be better
	float mass;
	float settime;
	glm::vec3 impulse;
	float timeforimpulse;
	glm::vec3 velocity = glm::vec3(0, 0, 0);

	glm::vec3 angularMomentum = glm::vec3(0, 0, 0);

	glm::mat3 inertia;

	
public:
	//store the parent 
	Mycube(VESceneNode *node, VESceneNode *parent, float mass) {
		this->parentnode = parent;
		this->node = node;
		this->mass = mass;

	};

	VESceneNode* getNode() {
		return this->node;
	};
	VESceneNode* getParentNode() {
		return this->parentnode;
	};


	void setAngularMomentum(glm::vec3 angularM) {
		this->angularMomentum = angularM;
	}

	void setImpulse(glm::vec3 impulse, float time) {
		this->timeforimpulse = time;
		this->impulse = impulse;
	}
	void setTime(float time) {
		this->settime = time;
	}
	float getTime() {
		return this->settime;
	}

	void setInertia(float mass, glm::vec3 initialPosition) {
		//does not change
		//should be precomputed
		//no need to have variable to store initial position, just pass it when the cube is created
		glm::mat3 initial = glm::mat3(1.0f);
		//Ixx
		initial[0][0] = mass * (pow(initialPosition.y, 2) + pow(initialPosition.z, 2));
		//Iyy
		initial[1][1] = mass * (pow(initialPosition.x, 2) + pow(initialPosition.z, 2));
		//Izz
		initial[2][2] = mass * (pow(initialPosition.x, 2) + pow(initialPosition.y, 2));
		this->inertia = initial;
		
	}

	glm::mat3 buildSkew(float x, float y, float z) {
		glm::mat3 skewmatrix = glm::mat3(0.0f);
		skewmatrix[0][0] = 0;
		skewmatrix[0][1] = -z;
		skewmatrix[0][2] = y;

		skewmatrix[1][0] = z;
		skewmatrix[1][1] = 0;
		skewmatrix[1][2] = -x;

		skewmatrix[2][0] = -y;
		skewmatrix[2][1] = x;
		skewmatrix[2][2] = 0;

		return skewmatrix;
	}

	glm::vec3 getVelocity(float dt, float timepassed) {
		if (timepassed == this->timeforimpulse || timepassed > this->timeforimpulse) {
			
			//this->velocity = (glm::vec3(0, -9.81, 0)/ this->mass);
			//return glm::vec3(0, -9.81, 0) * time / this->mass;
			//return glm::vec3(0, -9.81, 0) * dt / this->mass;
			
			//here needs to add up the velocity, can not explain why exactly, but the formula in slides will do
			this->velocity = this->velocity + glm::vec3(0, -9.81, 0) * dt / this->mass;
			return this->velocity;

		}
		else {
			
			//this->velocity = this->velocity + (this->impulse + glm::vec3(0, -9.81, 0)) * dt / this->mass;
			//this also works, does not look very accurate
			//return (this->impulse + glm::vec3(0, -9.81, 0)) * dt / this->mass;
			
			//most accurate
			//impulse already considered in the if statement
			//so the velocity does not add up here
			this->velocity = (this->impulse + glm::vec3(0, -9.81, 0) * timepassed) / this->mass;
			return this->velocity;
		}
	}

	glm::mat3 getAngularVelocity(glm::vec3 position, glm::mat3 rotation, float dt, glm::vec3 torque) {
		//dt here is the time passed, and not event.dt!
		//not exactly know how, but it works

		glm::mat3 R = rotation;
		glm::mat3 reversedI = R * glm::inverse(this->inertia) * glm::transpose(R);
		//this->angularMomentum = torque * dt if speed increases;
		this->angularMomentum = torque;
		
		//angular velocity
		glm::vec3 w = reversedI * this->angularMomentum;
		glm::mat3 changeofOrientation = buildSkew(w.x, w.y, w.z);
		return changeofOrientation * R;
		
	}

	~Mycube() {
		//delete node;
		getSceneManagerPointer()->deleteSceneNodeAndChildren(parentnode->getName());

		
	};
};

//store pointers rather than 
extern vector<Mycube*> allmycubes;

extern int pressednumber;

#endif
