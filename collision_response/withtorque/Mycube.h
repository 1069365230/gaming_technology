//#pragma once
#ifndef MYCUBE_H_
#define MYCUBE_H_

#include <iostream>

using namespace ve;
using namespace std;

class Mycube {

private:
	VESceneNode* node;
	VESceneNode* parentnode;

	//string name;
	//my own class would be better
	float mass;
	float settime;
	glm::vec3 impulse;
	//vector of impulses?
	vector<glm::vec3> impulses;
	float timeforimpulse;
	glm::vec3 velocity = glm::vec3(0, 0, 0);

	glm::vec3 angularMomentum = glm::vec3(0, 0, 0);

	glm::mat3 inertia;
	glm::mat3 inverseInertia;
	glm::vec3 gravity = glm::vec3(0.0, -9.81, 0.0);
	bool impulseLastFrame = false;
	glm::vec3 w;
	glm::vec3 torque;


public:
	//store the parent 
	Mycube(VESceneNode* node, VESceneNode* parent, float mass) {
		this->parentnode = parent;
		this->node = node;
		this->mass = mass;

	};
	void setTorque(glm::vec3 torque) {
		this->torque = torque;
	}
	glm::mat3 getIntertia() {
		return this->inertia;
	}
	VESceneNode* getNode() {
		return this->node;
	};
	VESceneNode* getParentNode() {
		return this->parentnode;
	};

	bool getimpulseLastFrame() {
		return this->impulseLastFrame;

	};

	void setimpulseLastFrame(bool b) {
		this->impulseLastFrame = b;
	};
	void setAngularMomentum(glm::vec3 angularM) {
		this->angularMomentum = angularM;
	}

	glm::vec3 getAugularMomentum() {
		return this->angularMomentum;
	}

	void setImpulse(glm::vec3 impulse, float time) {
		this->timeforimpulse = time;
		this->impulse = impulse;
	}

	void setStaticImpulse(glm::vec3 impulse) {
		this->impulse = impulse;
	}

	void setTime(float time) {
		this->settime = time;
	}
	float getTime() {
		return this->settime;
	}
	void setGravity(glm::vec3 g) {
		this->gravity = g;
	}
	glm::vec3 getW() {
		return this->w;
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

				//this->velocity = (glm::vec3(0, -9.81, 0) * timepassed/ this->mass);
				//return glm::vec3(0, -9.81, 0) * time / this->mass;
				//return glm::vec3(0, -9.81, 0) * dt / this->mass;

				//here needs to add up the velocity, can not explain why exactly, but the formula in slides will do

				//not adding up?
				this->velocity = this->velocity + this->gravity * dt / this->mass;
				return this->velocity;

			}
			else {

				//this->velocity = this->velocity + (this->impulse + glm::vec3(0, -9.81, 0)) * dt / this->mass;
				//this also works, does not look very accurate
				//return (this->impulse + glm::vec3(0, -9.81, 0)) * dt / this->mass;

				//most accurate
				//impulse already considered in the if statement
				//so the velocity does not add up here
				/*glm::vec3 sum(0.0, 0.0, 0.0);
				for (int i = 0; i < this->impulses.size(); i++) {
					sum += this->impulses[0];
				}*/
				//this->velocity = (sum + glm::vec3(0, -9.81, 0) * timepassed) / this->mass;
				//before
				this->velocity = (this->impulse + this->gravity * timepassed) / this->mass;
				return this->velocity;
			}



	}
	void setVelocity(glm::vec3 velocity) {
		this->velocity = velocity;
	}
	glm::mat3 getAngularVelocity(glm::vec3 position, glm::mat3 rotation) {
		//dt here is the time passed, and not event.dt!
		//not exactly know how, but it works

		glm::mat3 R = rotation;
		glm::mat3 reversedI = R * glm::inverse(this->inertia) * glm::transpose(R);
		this->inverseInertia = reversedI;
		//if speed increases;
		//this->angularMomentum = (this->angularMomentum + this->torque) * dt;
		//this->angularMomentum = torque;
		
		//angular velocity
		// //before
		//momentum is velocity
		//torque is the acceleration
		glm::vec3 w = reversedI * this->angularMomentum;
		this->w = w;
		//glm::vec3 w = w + reversedI * this->angularMomentum;
		//store the private w
		//w = .. + this->angulamomentum
		//w should be smaller in the end (when it hits) 

		glm::mat3 changeofOrientation = buildSkew(this->w.x, this->w.y, this->w.z);
		
		return changeofOrientation * R;
		
	}

	
	glm::vec3 setW(glm::vec3 w) {
		return this->w = w;
	}
	glm::mat3 getInverseInertia() {
		return this->inverseInertia;
	}

	float getMass() {
		return this->mass;
	}

	void addImpuleses(glm::vec3 impulse){
		//glm::vec3 *tmp(impulse);
		//tmp = impulse;
		this->impulses.push_back(impulse);
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
