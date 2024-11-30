/**
* The Vienna Vulkan Engine
*
* (c) bei Helmut Hlavacs, University of Vienna
*
*/


#include "VEInclude.h"
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
/*glm::vec3 functionOfTime(glm::vec3 force, float time) {
	
}*/

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
				//cout << "not empty" << endl;
				//cout << allmycubes.size() << endl;
				//cout << "++++++++++++++++++++++" << position << endl;
				float passedTime = allmycubes[pressednumber]->getTime();
				
				
				cout<<"lifetime: " << event.dt + passedTime << endl;
				allmycubes[pressednumber]->setTime(event.dt + passedTime);
				//impulse  = momentum change 
				//impulse  = force * time = N * s (newton for how many seconds)
				//mass * velocity = change of momentum
				
				glm::vec3 impulse = glm::vec3(0, 20, 10);
				glm::vec3 torque = glm::vec3(3, 0, 0);
				allmycubes[pressednumber]->setImpulse(impulse, 2);

				glm::vec3 translation = allmycubes[pressednumber]->getVelocity(event.dt, passedTime) * (float)event.dt;
				glm::vec3 currentposition = allmycubes[pressednumber]->getNode()->getPosition();
				glm::vec3 newposition = currentposition + translation;

				glm::mat3 currentRotation = glm::mat3(allmycubes[pressednumber]->getNode()->getRotation());
				glm::mat3 rotation = allmycubes[pressednumber]->getAngularVelocity(newposition, currentRotation, passedTime, torque) * (float)event.dt;
				glm::mat4 newRotation = glm::mat4(currentRotation + rotation);
				
				//glm::mat4 transform = buildTransform(newRotation, newposition);
				//cout << transform << endl;
				//cout << newRotation << endl;
				
				//allmycubes[pressednumber]->getNode()->multiplyTransform(glm::translate(glm::mat4(1.0f), translation));
				//allmycubes[pressednumber]->getNode()->multiplyTransform(glm::mat4(rotation));
				
				
				//100% right
				allmycubes[pressednumber]->getNode()->setTransform(newRotation);
				
				//100% right now 
				allmycubes[pressednumber]->getNode()->setPosition(newposition);

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

			//VESceneNode *e4;
			//VECHECKPOINTER( e4 = getSceneManagerPointer()->loadModel("The Plane", "media/models/test", "plane_t_n_s.obj",0, pScene) );
			//e4->setTransform(glm::scale(glm::mat4(1.0f), glm::vec3(1000.0f, 1.0f, 1000.0f)));

			//remove the plane
			//added bottom sky box in VESceneManager class
			//VEEntity *pE4;
			//VECHECKPOINTER( pE4 = (VEEntity*)getSceneManagerPointer()->getSceneNode("The Plane/plane_t_n_s.obj/plane/Entity_0") );
			//pE4->setParam( glm::vec4(1000.0f, 1000.0f, 0.0f, 0.0f) );

			//add new cube under new parent 
			//VESceneNode *e1,*eParent;
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

