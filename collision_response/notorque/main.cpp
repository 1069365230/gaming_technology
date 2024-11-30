/**
* The Vienna Vulkan Engine
*
* (c) bei Helmut Hlavacs, University of Vienna
*
*/


#include "VEInclude.h"
#include "collider.h"
#include "gjk_epa.h"
#include "contact.h"

vector<vpe::Box*> planes;

bool moved = false;

using namespace std;
using namespace ve;
ostream& operator<< (ostream& o, glm::mat3 matrix) {
	o << endl;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			o.width(9);
			o << matrix[i][j] << " ";
		}
		o << endl;
	}
	o << endl;
	return o;
}
ostream& operator<< (ostream& o, glm::mat4 matrix) {
	o << endl;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			o.width(9);
			o << matrix[i][j] << " ";
		}
		o << endl;
	}
	o << endl;
	return o;
}
ostream& operator<< (ostream& o, glm::vec3 vector) {
	o << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")";
	return o;
}

ostream& operator<< (ostream& o, glm::vec4 vector) {
	o << "(" << vector.x << ", " << vector.y << ", " << vector.z << ", " << vector.w << ")";
	return o;
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

glm::mat3 buildRodrigues(glm::vec3 axis, float angle) {
	glm::vec3 n = glm::normalize(axis);
	glm::mat3 skewMatrix = buildSkew(n.x, n.y, n.z);

	double radian = glm::radians(angle);
	glm::mat3 identity = glm::mat3(1.0f);
	glm::mat3 rodrigues = (float)(round(cos(radian) * 1000000) / 1000000) * identity + (float)(round(sin(radian) * 1000000) / 1000000) * skewMatrix + (1 - (float)(round(cos(radian) * 1000000) / 1000000)) * glm::outerProduct(n, n);

	return rodrigues;
}


glm::mat4 buildTransform(glm::mat3 rotation, glm::vec3 translation) {
	glm::mat4 base = glm::mat4(rotation);
	base[0][3] = translation.x;
	base[1][3] = translation.y;
	base[2][3] = translation.z;
	return base;

}

glm::mat3 buildMatrixK(float mb, glm::vec3 rb, glm::mat3 inverseIb, float ma, glm::vec3 ra, glm::mat3 inverseIa) {
	glm::mat3 identity1 = glm::mat3(1.0f);
	glm::mat3 identity2 = glm::mat3(1.0f);
	return (-1 / mb) * identity1 + buildSkew(rb.x, rb.y, rb.z) * inverseIb * buildSkew(rb.x, rb.y, rb.z) - (1 / ma) * identity2 + buildSkew(ra.x, ra.y, ra.z) * inverseIa * buildSkew(ra.x, ra.y, ra.z);
}

//glm::vec3 findfirstcontact
//Prel(t-)
glm::vec3 collisionVelocity(glm::vec3 linearVa, glm::mat3 angularVa, glm::vec3 ra, glm::vec3 linearVb, glm::mat3 angularVb, glm::vec3 rb) {
	glm::vec3 pa = linearVa + angularVa * ra;
	glm::vec3 pb = linearVb + angularVb * rb;
	glm::vec3 prel = pb - pa;
	return prel;
}
//d(t-)
glm::vec3 closingVelocity(glm::vec3 normal, glm::vec3 collisionV) {
	//vec3?
	//glm::vec3 dt = normal * collisionV
	
	return normal * collisionV;
}
glm::mat3 getInertia(float mass, glm::vec3 initialPosition) {
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
	return initial;

}



//global impulse
//glm::vec3 impulse(0.0, 0.0, 0.0);
glm::vec3 torque = glm::vec3(6.0f, 0.0f, 0.0f);
namespace ve {


	uint32_t g_score = 0;				//derzeitiger Punktestand
	double g_time = 30.0;				//zeit die noch übrig ist
	bool g_gameLost = false;			//true... das Spiel wurde verloren
	bool g_restart = false;			//true...das Spiel soll neu gestartet werden

	
	//
	//Zeichne das GUI
	//
	
	class EventListenerGUI : public VEEventListener {
	protected:
		
		virtual void onDrawOverlay(veEvent event) {
			VESubrenderFW_Nuklear * pSubrender = (VESubrenderFW_Nuklear*)getRendererPointer()->getOverlay();
			if (pSubrender == nullptr) return;

			struct nk_context * ctx = pSubrender->getContext();

			if (!g_gameLost) {
				if (nk_begin(ctx, "", nk_rect(0, 0, 200, 170), NK_WINDOW_BORDER )) {
					char outbuffer[100];
					nk_layout_row_dynamic(ctx, 45, 1);
					sprintf(outbuffer, "Score: %03d", g_score);
					nk_label(ctx, outbuffer, NK_TEXT_LEFT);

					nk_layout_row_dynamic(ctx, 45, 1);
					sprintf(outbuffer, "Time: %004.1lf", g_time);
					nk_label(ctx, outbuffer, NK_TEXT_LEFT);
				}
			}
			else {
				if (nk_begin(ctx, "", nk_rect(500, 500, 200, 170), NK_WINDOW_BORDER )) {
					nk_layout_row_dynamic(ctx, 45, 1);
					nk_label(ctx, "Game Over", NK_TEXT_LEFT);
					if (nk_button_label(ctx, "Restart")) {
						g_restart = true;
					}
				}

			};

			nk_end(ctx);
		}

	public:
		///Constructor of class EventListenerGUI
		EventListenerGUI(std::string name) : VEEventListener(name) { };

		///Destructor of class EventListenerGUI
		virtual ~EventListenerGUI() {};
	};
	

	static std::default_random_engine e{ 12345 };					//Für Zufallszahlen
	static std::uniform_real_distribution<> d{ -10.0f, 10.0f };		//Für Zufallszahlen

	//
	// Überprüfen, ob die Kamera die Kiste berührt
	//


	class EventListenerCollision : public VEEventListener {
	protected:
		virtual void onFrameStarted(veEvent event) {

			static uint32_t cubeid = 0;
			//algorith + update scenenodes
			//rotation -> tranformation matrix -> 
			//scene node -> multiplyTransform(param: transformation matrix)//update scenenode
			//gloabal list container (uses for passing forces)

			if (!allmycubes.empty()) {

				//allmycubes[pressednumber]->setTime(event.dt);
				float passedTime = allmycubes[pressednumber]->getTime();
				
				//add the event.dt every frame so you can get the total passed time
				allmycubes[pressednumber]->setTime(event.dt + passedTime);
				//impulse  = momentum change 
				//impulse  = force * time = N * s (newton for how many seconds)
				//mass * velocity = change of momentum
				
				//impulse = glm::vec3(0.0f, 20.0f, 0.0f);
				//glm::vec3 torque = glm::vec3(6.0f, 0.0f, 0.0f);
				//allmycubes[pressednumber]->setImpulse(impulse, 0);

				cout << "----------------------------------" << endl;
				cout << "pasedTime: " << passedTime <<endl;
				glm::vec3 v = allmycubes[pressednumber]->getVelocity(event.dt, passedTime);
				cout << "++first velocity++: " << v << endl;
				glm::vec3 translation = v * (float)event.dt;
				//glm::vec3 translation = allmycubes[pressednumber]->getVelocity(event.dt, passedTime) * (float)event.dt;
				glm::vec3 currentposition = allmycubes[pressednumber]->getNode()->getPosition();
				glm::vec3 newposition = currentposition + translation;
				
				glm::mat3 currentRotation = glm::mat3(allmycubes[pressednumber]->getNode()->getRotation());
				//torque passes through or not?
				//torque = glm::vec3(6.0f, 0.0f, 0.0f);
				allmycubes[pressednumber]->setAngularMomentum(torque);
				glm::mat3 rotation = allmycubes[pressednumber]->getAngularVelocity(newposition, currentRotation) * (float)event.dt;
				glm::mat4 newRotation = glm::mat4(currentRotation + rotation);
				cout << "new rotation: " << newRotation << endl;
				
				//if collosion dectected returns 
				
				
				cout << currentposition << endl;
				vpe::Box box{ {(float)currentposition.x, (float)currentposition.y, (float)currentposition.z} };
				vpe::Box ground = *(planes[0]);

				//collision part
				glm::vec3 mtv(0.0f, 1.0f, 0.0f);
				auto hit = gjk(box, ground, mtv, true);
				cout << "hit:---" << hit << endl;
				
				if (!hit) {

					//allmycubes[pressednumber]->getNode()->setTransform(newRotation);
					allmycubes[pressednumber]->getNode()->setPosition(newposition);
					return;
				}

				if (isnan(mtv.x)) {
					mtv.x = 0;
				}

				if (isnan(mtv.y)) {
					mtv.y = 0;
				}

				if (isnan(mtv.z)) {
					mtv.z = 0;
				}
				cout << "MTV: " << mtv << endl;
					
				//if (!moved) {
				std::set<vpe::contact> ct{};
				cout << "1" << endl;
				cout << "box pos: " << box.pos() << endl;
				currentposition += mtv;
				box.pos() += mtv;
				vpe::contacts(box, ground, mtv, ct);

					
				//when it hits and the contacts points are ready
				glm::vec3 normal(0.0f, 1.0f, 0.0f);
				//collisionVelocity(glm::vec3 linearVa, glm::mat3 angularVa, glm::vec3 ra, glm::vec3 linearVb, glm::mat3 angularVb, glm::vec3 rb)
				glm::vec3 linearVa = v;

				//changes
				//glm::mat3 angularVa = allmycubes[pressednumber]->getAngularVelocity(box.pos(), currentRotation, torque);
				glm::mat3 angularVa = allmycubes[pressednumber]->getAngularVelocity(box.pos(), currentRotation);
				cout << "linearVa: " << linearVa << endl;
				cout << "angularVa: " << angularVa << endl;
				cout << "ct size: " << ct.size() << endl;
				//cube went inside the ground in another frame, still detected collision but no contact points
				if (ct.size() != 0 || box.pos().y < 1) {


					for (auto it : ct) {

						std::cout << "(" << it.pos.x << " " << it.pos.y << " " << it.pos.z << ")" << std::endl;

					}
					auto it = *(ct.begin());
					//cout << "it: " << it->pos << endl;
					glm::vec3 contactPoint(it.pos.x, it.pos.y, it.pos.z);
					glm::vec3 ratemp = box.pos() - contactPoint;
					glm::vec3 ra = ratemp / ratemp.length();
					glm::vec3 linearVb(0.0, 0.0, 0.0);

					//angular matrix = 0 or identity matrix?
					glm::mat3 angularVb = glm::mat3(0.0f);
					glm::vec3 rbtemp = box.pos() - contactPoint;
					glm::vec3 rb = rbtemp / rbtemp.length();
					
					//Prel(t-) 
					glm::vec3 Prelminus = collisionVelocity(linearVa, angularVa, ra, linearVb, angularVb, rb);
					cout << "Prel(t-): " << Prelminus << endl;

					//d(t-) = closingVelocity(normal, collisionV)
					glm::vec3 dtminus = closingVelocity(normal, Prelminus);
					cout << "d(t-): " << dtminus << endl;
					//Prel(t+)
					//e is the elastic not the fucking e from math
					glm::vec3 Prelplus = -0.5 * dtminus * normal;
					cout << "Prel(t+): " << Prelplus << endl;
					//glm::vec3 temp = (Prelplus - collisionV) ;

					//way to get matrix K now
					float ma = allmycubes[pressednumber]->getMass();
					float mb = 99.0;
					glm::mat inverseIa = allmycubes[pressednumber]->getInverseInertia();
					//box.matRS instead of identity 
					cout <<"Gound POS: " << ground.pos() << endl;
					//inverse interia at pos 000 is undefined
					cout << "intertia Ground: " << glm::inverse(getInertia(mb, ground.pos())) << endl;
					//glm::mat inverseIb = glm::mat3(1.0f) * glm::inverse(getInertia(mb, ground.pos())) * glm::transpose(glm::mat3(1.0f));
					glm::mat inverseIb = glm::mat3(1.0f);
					cout << "Ib-: "<< inverseIb << endl;
					cout << "IA-: " << inverseIa << endl;
					glm::mat3 K = buildMatrixK(mb, rb, inverseIb, ma, ra, inverseIa);
					//glm::mat3 buildMatrixK(float mb, glm::vec3 rb, glm::mat3 inverseIb, float ma, glm::vec3 ra, glm::mat3 inverseIa)
					cout << "MatrixK: " << K << endl;
					//force
					glm::vec3 F = glm::inverse(K) * (Prelplus - Prelminus);
					cout << "P+ - P-: " << Prelplus - Prelminus << endl;
					cout << "MatrixK: " << inverse(K) << endl;
					//impulse = F;
					cout << "FORCE: " << F << endl;
					glm::vec3 fn = (F * normal) * normal;
					cout << "Fn: " << fn << endl;

					cout << "Ft: " << F - fn << endl;

					allmycubes[pressednumber]->setTime(event.dt);

					float passedTime = allmycubes[pressednumber]->getTime();
					allmycubes[pressednumber]->setTime(event.dt + passedTime);

					
					allmycubes[pressednumber]->setVelocity(v + fn);
					glm::vec3 translation = (v + fn)* (float)event.dt;

					if ((v + fn).y < 0.5 && (v + fn).y > -0.5) {
						allmycubes[pressednumber]->setVelocity(glm::vec3(0.0, 0.0, 0.0));
						translation = glm::vec3(0.0, 0.0, 0.0) * (float)event.dt;

					}

					cout << "new velocity: " << v + F << endl;
					cout << "box po: " << box.pos()<< endl;
					cout << "Translation: " << translation << endl;
					cout << "new box pos after hitting ground: " << box.pos() + translation << endl;
					glm::vec3 newposition = box.pos() + translation;
					
					glm::vec3 angularM = allmycubes[pressednumber]->getAugularMomentum();
					cout << "Angular Momentum: " << angularM << endl;
					glm::vec3 collisionTorque = ra * F;
					torque = torque + collisionTorque;
					//allmycubes[pressednumber]->setAngularMomentum(torque);
					//used to work
					//glm::mat3 rotation = allmycubes[pressednumber]->getAngularVelocity(newposition, currentRotation, torque + collisionTorque) * (float)event.dt;

					glm::mat3 rotation = allmycubes[pressednumber]->getAngularVelocity(newposition, currentRotation) * (float)event.dt;
					glm::mat4 newRotation = glm::mat4(currentRotation + rotation);
					 
					//allmycubes[pressednumber]->getNode()->setTransform(newRotation);
					allmycubes[pressednumber]->getNode()->setPosition(newposition);
					return;
				}
				//stop rotating when the roation matrix x is 0?
				//when to stop moving?
				//mutrualize the position and rotation ?
				//mutrualize the y and z component of the rotation matrix idea 1
				allmycubes[pressednumber]->getNode()->setPosition(box.pos());

				cout << "2" << endl;
				//see the contact points x
				std::cout << "------------------------------" << std::endl;
				std::cout << "ct size: " << ct.size() << std::endl;
				cout << "3" << endl;
				/*for (auto it : ct) {
							
					std::cout << "(" << it.pos.x << " " << it.pos.y << " " << it.pos.z << ")" << std::endl;

				}*/
				cout << "4" << endl;
				//allmycubes[pressednumber]->getNode()->setPosition(box.pos());
					//moved = true;
					//}
				
				


			}


			if (g_restart) {
				g_gameLost = false;
				g_restart = false;
				g_time = 30;
				g_score = 0;
				getSceneManagerPointer()->getSceneNode("The Cube Parent")->setPosition(glm::vec3(d(e), 2.0f, d(e)));
				//getEnginePointer()->m_irrklangEngine->play2D("media/sounds/ophelia.wav", true);
				return;
			}
			if (g_gameLost) return;
			/*
			glm::vec3 positionCube   = getSceneManagerPointer()->getSceneNode("The Cube Parent")->getPosition();
			glm::vec3 positionCamera = getSceneManagerPointer()->getSceneNode("StandardCameraParent")->getPosition();
			
			
			float distance = glm::length(positionCube - positionCamera);
			if (distance < 1) {
				g_score++;
				getEnginePointer()->m_irrklangEngine->play2D("media/sounds/explosion.wav", false);
				if (g_score % 10 == 0) {
					g_time = 30;
					getEnginePointer()->m_irrklangEngine->play2D("media/sounds/bell.wav", false);
				}

				VESceneNode *eParent = getSceneManagerPointer()->getSceneNode("The Cube Parent");
				eParent->setPosition(glm::vec3(d(e), 1.0f, d(e)));

				getSceneManagerPointer()->deleteSceneNodeAndChildren("The Cube"+ std::to_string(cubeid));
				VECHECKPOINTER(getSceneManagerPointer()->loadModel("The Cube"+ std::to_string(++cubeid)  , "media/models/test/crate0", "cube.obj", 0, eParent) );
			}
			*/

			//do not want to have a time limit right now
			/*g_time -= event.dt;
			if (g_time <= 0) {
				g_gameLost = true;
				getEnginePointer()->m_irrklangEngine->removeAllSoundSources();
				getEnginePointer()->m_irrklangEngine->play2D("media/sounds/gameover.wav", false);
			}*/
		};

	public:
		///Constructor of class EventListenerCollision
		EventListenerCollision(std::string name) : VEEventListener(name) { };

		///Destructor of class EventListenerCollision
		virtual ~EventListenerCollision() {};
	};

	

	///user defined manager class, derived from VEEngine
	class MyVulkanEngine : public VEEngine {
	public:

		MyVulkanEngine( bool debug=false) : VEEngine(debug) {};
		~MyVulkanEngine() {};


		///Register an event listener to interact with the user
		
		virtual void registerEventListeners() {
			VEEngine::registerEventListeners();

			registerEventListener(new EventListenerCollision("Collision"), { veEvent::VE_EVENT_FRAME_STARTED });
			//registerEventListener(new EventListenerGUI("GUI"), { veEvent::VE_EVENT_DRAW_OVERLAY});
		};
		

		///Load the first level into the game engine
		///The engine uses Y-UP, Left-handed
		virtual void loadLevel( uint32_t numLevel=1) {

			VEEngine::loadLevel(numLevel );			//create standard cameras and lights

			VESceneNode *pScene;
			VECHECKPOINTER( pScene = getSceneManagerPointer()->createSceneNode("Level 1", getRoot()) );
	
			//scene models
			//change createskybox
			VESceneNode *sp1;
			VECHECKPOINTER( sp1 = getSceneManagerPointer()->createSkybox("The Sky", "media/models/test/sky/cloudy",
										{	"bluecloud_ft.jpg", "bluecloud_bk.jpg", "bluecloud_up.jpg", 
											"bluecloud_dn.jpg", "bluecloud_rt.jpg", "bluecloud_lf.jpg" }, pScene)  );

			VESceneNode *e4;
			VECHECKPOINTER( e4 = getSceneManagerPointer()->loadModel("The Plane", "media/models/test", "plane_t_n_s.obj",0, pScene) );
			e4->setTransform(glm::scale(glm::mat4(1.0f), glm::vec3(1000.0f, 1.0f, 1000.0f)));
			
			//remove the plane
			//added bottom sky box in VESceneManager class
			VEEntity *pE4;
			VECHECKPOINTER( pE4 = (VEEntity*)getSceneManagerPointer()->getSceneNode("The Plane/plane_t_n_s.obj/plane/Entity_0") );
			pE4->setParam( glm::vec4(1000.0f, 1000.0f, 0.0f, 0.0f) );
			
			vec3 pos = e4->getPosition();
			cout << "plane pos:" << pos << endl;
			mat4 temp = e4->getTransform();
			cout << "plane trans:" << temp << endl;
			vpe::Box* ground = new vpe::Box({ (float)pos.x, (float)pos.y - 499.5, (float)pos.z }, scale(mat4(1.0f), vec3(1000.0f, 1000.0f, 1000.0f)));
			
			planes.push_back(ground);

			//add new cube under new parent 
			VESceneNode *e1,*eParent;
			//eParent = getSceneManagerPointer()->createSceneNode("The Cube Parent", pScene, glm::mat4(1.0));
			//VECHECKPOINTER(e1 = getSceneManagerPointer()->loadModel("The Cube0", "media/models/test/crate0", "cube.obj"));
			
			//eParent->multiplyTransform(glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 1.0f, 10.0f)));
			//eParent->addChild(e1);

			//eParent->addChild(e2);

			//m_irrklangEngine->play2D("media/sounds/ophelia.wav", true);
		};
	};


}

#include <typeinfo>


int main() {
	
	bool debug = true;
	MyVulkanEngine mve(debug);	//enable or disable debugging (=callback, validation layers)
	
	
	glm::vec3 axis(0.0f,1.0f,0.0f);
	const float angle = 90;
	glm::mat3 r = buildRodrigues(axis, angle);
	glm::mat4 transformation = glm::mat4(r);
	cout << "----------------------------------" << endl;
	
	cout << "angle: " << angle << endl;
	cout << "axis: " << axis << endl;
	cout << "rotation matrix: " << r << endl;
	cout << "transformation matrix: " << transformation << endl;

	//4 vectors
	glm::vec4 xa = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec4 xb = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	glm::vec4 xc = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
	glm::vec4 xd = glm::vec4(1.0f, 1.0f, 0.0f, 0.0f);
	glm::vec4 vectors[4] = {xa, xb, xc, xd};
	for (int i = 0; i < 4; i++) {
		glm::vec4 output = vectors[i] * transformation;
		cout << "vector x: " << vectors[i] << " rotated vector: " << output << endl;
	
	}

	cout << "---------------------------" << endl;





	mve.initEngine();
	mve.loadLevel(1);
	mve.run();

	return 0;
}

