Task 1:
It's clear that we need to do the iterative search in the Collider.h inside the support() function, which is been done already.
So my job now is to test the brute force method and iteratively neighbor search separately.

My approach: 
modify the support() to only do the brute force:
here is the function:

vec3 support(vec3 dirW) {
            if (m_vertices.empty()) return { 0,0,0 };

            auto dirL = dirW2L(dirW);
            vec3 furthest_point;
            float max_dot;

            furthest_point = m_vertices[0].pointL();
            max_dot = dot(furthest_point, dirL);
            std::for_each(std::begin(m_vertices), std::end(m_vertices),
                [&](auto& vertex) {
                float d = dot(vertex.pointL(), dirL);
                if (d > max_dot) {
                    max_dot = d;
                    furthest_point = vertex.pointL();
                }
            });

            return posL2W(furthest_point);
        }

then I will go in test.h and surround the unit_test_box_box() with the high resolution clock which is used to do the timing which looks like this:

inline bool unit_tests() {
		using std::chrono::high_resolution_clock;
		using std::chrono::duration;
		using std::chrono::milliseconds;


		unit_sub_test( unit_test_pluecker() );
		unit_sub_test( unit_test_normals() );

		auto begin = high_resolution_clock::now();
		unit_sub_test( unit_test_box_box() );

		duration<double, std::milli> totaltime = high_resolution_clock::now() - begin;
		std::cout << "time passed: " << totaltime << std::endl;
		return true;
    }

the screenshot can be seen in "BRUTE_FORCE_TIMING"

Then for testing the iteratively neighbor search I modify the support() again (idea: basically separate the "else" in the given support() into a new function):
here is the function:
vec3 support(vec3 dirW) {
            auto dirL = dirW2L(dirW);
            vec3 furthest_point;
            float max_dot;

            if (m_vertices.empty()) return { 0,0,0 };

            m_supporting_point = m_supporting_point >= 0 ? m_supporting_point : 0;
            furthest_point = m_vertices[m_supporting_point].pointL();
            max_dot = dot(furthest_point, dirL);
            int maxi = m_supporting_point;

            do {
                m_supporting_point = maxi;
                for (auto& nei : m_vertices[m_supporting_point].neighbors()) {
                    float d = dot(m_vertices[nei].pointL(), dirL);
                    if (d > max_dot) {
                        max_dot = d;
                        furthest_point = m_vertices[nei].pointL();
                        maxi = nei;
                    }
                }
            } while (maxi != m_supporting_point);


            return posL2W(furthest_point);

        }

the screenshot can be seen in "ITERATIVE_NEIGHBOUR_TIMING"

Conculsion: iterative neighbour is faster than the brute force, however it is not very significantly here because the polytopes are very simple. 
It will have a significant difference if we have two or one complex polytopes collide each other.


Task 2:

I have decided to intergrate the physic engine into the vulkan engine, to give me a visual interpretation. So I import all the header files.
It is based on my previous lab, that to throw a cube. But this time with no impulse(which means the force will only be gravity), and a different position.

I will also upload a video if you want to see it:) and also the main.cpp, VEEventListenerGLFW.cpp, which are files that I changed.

https://youtu.be/8nF3OzUZcEo

so everything basically happens in onFrame, when the box collides the ground, it will first use the mtv to translate back to the state where the cube just touches the ground.
Then I use the contact() to get the contact points and output it on console. which can be seen on the screenshot "CONTACT_POINTS".

4 contact points are:
(0.5 0.5 3.5)
(0.5 0.5 2.5)
(-0.5 0.5 3.5)
(-0.5 0.5 2.5)









