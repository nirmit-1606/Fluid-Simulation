#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <vector>
#include <unordered_map>

// #include <omp.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef F_PI
#define F_PI ((float)(M_PI))
#define F_2_PI ((float)(2.f * F_PI))
#define F_PI_2 ((float)(F_PI / 2.f))
#endif

#include "glew.h"
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include "glut.h"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author for loading the window:			Joe Graphics
//  Author for implementing simulation: 	Nirmit Patel

// title of these windows:

const char *WINDOWTITLE = "OpenGL / GLUT Simulation -- Nirmit Patel";
const char *GLUITITLE = "User Interface Window";

// what the glui package defines as true and false:

const int GLUITRUE = true;
const int GLUIFALSE = false;

// the escape key:

const int ESCAPE = 0x1b;

// initial window size:

const int INIT_WINDOW_SIZE = 600;

// size of the 3d box to be drawn:

const float BOXSIZE = 2.f;

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// minimum allowable scale factor:

const float MINSCALE = 0.05f;

// scroll wheel button values:

const int SCROLL_WHEEL_UP = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = 5.f;

// active mouse buttons (or them together):

const int LEFT = 4;
const int MIDDLE = 2;
const int RIGHT = 1;

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

const GLfloat BACKCOLOR[] = {0., 0., 0., 1.};

// line width for the axes:

const GLfloat AXES_WIDTH = 3.;

// the color numbers:
// this order must match the radio button order, which must match the order of the color names,
// 	which must match the order of the color RGB values

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA
};

char *ColorNames[] =
	{
		(char *)"Red",
		(char *)"Yellow",
		(char *)"Green",
		(char *)"Cyan",
		(char *)"Blue",
		(char *)"Magenta"};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[][3] =
	{
		{1., 0., 0.}, // red
		{1., 1., 0.}, // yellow
		{0., 1., 0.}, // green
		{0., 1., 1.}, // cyan
		{0., 0., 1.}, // blue
		{1., 0., 1.}, // magenta
};

// fog parameters:

const GLfloat FOGCOLOR[4] = {.0f, .0f, .0f, 1.f};
const GLenum FOGMODE = GL_LINEAR;
const GLfloat FOGDENSITY = 0.30f;
const GLfloat FOGSTART = 1.5f;
const GLfloat FOGEND = 4.f;

// for lighting:

const float WHITE[] = {1., 1., 1., 1.};

// for animation:

const int MS_PER_CYCLE = 30000; // 10000 milliseconds = 10 seconds

// --------------------------------------------------------------------
// A structure for holding two neighboring particles and their weighted distances
struct Particle;
struct Neighbor
{
	Particle *j;
	float q, q2;
};

// --------------------------------------------------------------------
// The Particle structure holding all of the relevant information.
struct Particle
{
	glm::vec3 pos;
	float r, g, b;

	glm::vec3 pos_old;
	glm::vec3 vel;
	glm::vec3 force;
	float mass;
	float rho;
	float rho_near;
	float press;
	float press_near;
	float sigma;
	float beta;
	std::vector<Neighbor> neighbors;
};

// Our collection of particles
std::vector<Particle> particles;

// --------------------------------------------------------------------
// Some constants for the relevant simulation.
const float p_size = .07;		   // particle size
const float G = .001f * .25;		   // Gravitational Constant for our simulation
const float spacing = .22f;		   // Spacing of particles
const float k = spacing / 1000.0f; // Far pressure weight
const float k_near = k * 10.;	   // Near pressure weight
const float r = spacing * 1.25f;   // Radius of Support
const float rsq = r * r;		   // ... squared for performance stuff
const float SIM_W = 1.5;		   // The size of the world
const float bottom = 0;			   // The floor of the world
const float i_girth = 2.f;		   // initial parameters

int N = 200;
float rest_density = 3.;	   // Rest Density

// #define DEMO_Z_FIGHTING
// #define DEMO_DEPTH_BUFFER

// non-constant global variables:

int ActiveButton;	 // current button that is down
GLuint AxesList;	 // list to hold the axes
int AxesOn;			 // != 0 means to draw the axes
GLuint ParticleList;		 // object display list
int DebugOn;		 // != 0 means to print debugging info
int DepthCueOn;		 // != 0 means to use intensity depth cueing
int DepthBufferOn;	 // != 0 means to use the z-buffer
int DepthFightingOn; // != 0 means to force the creation of z-fighting
int MainWindow;		 // window id for main graphics window
int NowColor;		 // index into Colors[ ]
int NowProjection;	 // ORTHO or PERSP
float Scale;		 // scaling factor
int ShadowsOn;		 // != 0 means to turn shadows on
float Time;			 // used for animation, this has a value between 0. and 1.
int Xmouse, Ymouse;	 // mouse values
float Xrot, Yrot;	 // rotation angles in degrees

bool doSimulation;
bool useGravity;

// function prototypes:

void Animate();
void Display();
void DoAxesMenu(int);
void DoColorMenu(int);
void DoDepthBufferMenu(int);
void DoDepthFightingMenu(int);
void DoDepthMenu(int);
void DoDebugMenu(int);
void DoMainMenu(int);
void DoProjectMenu(int);
void DoRasterString(float, float, float, char *);
void DoStrokeString(float, float, float, float, char *);
float ElapsedSeconds();
void InitGraphics();
void InitLists();
void InitMenus();
void Keyboard(unsigned char, int, int);
void MouseButton(int, int, int, int);
void MouseMotion(int, int);
void Reset();
void Resize(int, int);
void Visibility(int);

void Axes(float);
void HsvRgb(float[3], float[3]);
void Cross(float[3], float[3], float[3]);
float Dot(float[3], float[3]);
float Unit(float[3], float[3]);
float Unit(float[3]);

void initParticles(const unsigned int);
void addParticleLayers(const unsigned int);
void step();

// utility to create an array from 3 separate values:

float *
Array3(float a, float b, float c)
{
	static float array[4];

	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}

// utility to create an array from a multiplier and an array:

float *
MulArray3(float factor, float array0[])
{
	static float array[4];

	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}

float *
MulArray3(float factor, float a, float b, float c)
{
	static float array[4];

	float *abc = Array3(a, b, c);
	array[0] = factor * abc[0];
	array[1] = factor * abc[1];
	array[2] = factor * abc[2];
	array[3] = 1.;
	return array;
}

// --------------------------------------------------------------------
// Between [0,1]
float rand01()
{
	return (float)rand() * (1.f / RAND_MAX);
}

// --------------------------------------------------------------------
// Between [a,b]
float randab(float a, float b)
{
	return a + (b - a) * rand01();
}

// these are here for when you need them -- just uncomment the ones you need:

#include "setmaterial.cpp"
#include "setlight.cpp"
#include "osusphere.cpp"
// #include "osucone.cpp"
// #include "osutorus.cpp"
// #include "bmptotexture.cpp"
// #include "loadobjfile.cpp"
// #include "keytime.cpp"
// #include "glslprogram.cpp"'

// --------------------------------------------------------------------
template <typename T>
class SpatialIndex
{
public:
	typedef std::vector<T *> NeighborList;

	SpatialIndex(
		const unsigned int numBuckets, // number of hash buckets
		const float cellSize		   // grid cell size
		// const bool twoDeeNeighborhood  // true == 3x3 neighborhood, false == 3x3x3
		)
		: mHashMap(numBuckets), mInvCellSize(1.0f / cellSize)
	{
		// initialize neighbor offsets
		for (int i = -1; i <= 1; i++)
			for (int j = -1; j <= 1; j++)
				// if (twoDeeNeighborhood)
				// 	mOffsets.push_back(glm::ivec3(i, j, 0));
				// else
					for (int k = -1; k <= 1; k++)
						mOffsets.push_back(glm::ivec3(i, j, k));
	}

	void Insert(const glm::vec3 &pos, T *thing)
	{
		mHashMap[Discretize(pos, mInvCellSize)].push_back(thing);
	}

	void Neighbors(const glm::vec3 &pos, NeighborList &ret) const
	{
		const glm::ivec3 ipos = Discretize(pos, mInvCellSize);
		for (const auto &offset : mOffsets)
		{
			typename HashMap::const_iterator it = mHashMap.find(offset + ipos);
			if (it != mHashMap.end())
			{
				ret.insert(ret.end(), it->second.begin(), it->second.end());
			}
		}
	}

	void Clear()
	{
		mHashMap.clear();
	}

private:
	// "Optimized Spatial Hashing for Collision Detection of Deformable Objects"
	// Teschner, Heidelberger, et al.
	// returns a hash between 0 and 2^32-1
	struct TeschnerHash : std::unary_function<glm::ivec3, std::size_t>
	{
		std::size_t operator()(glm::ivec3 const &pos) const
		{
			const unsigned int p1 = 73856093;
			const unsigned int p2 = 19349663;
			const unsigned int p3 = 83492791;
			return size_t((pos.x * p1) ^ (pos.y * p2) ^ (pos.z * p3));
		};
	};

	// returns the indexes of the cell pos is in, assuming a cellSize grid
	// invCellSize is the inverse of the desired cell size
	static inline glm::ivec3 Discretize(const glm::vec3 &pos, const float invCellSize)
	{
		return glm::ivec3(glm::floor(pos * invCellSize));
	}

	typedef std::unordered_map<glm::ivec3, NeighborList, TeschnerHash> HashMap;
	HashMap mHashMap;

	std::vector<glm::ivec3> mOffsets;

	const float mInvCellSize;
};

// template<typename T>
// class SpatialIndex {
// public:
//     typedef std::vector<T*> NeighborList;

//     SpatialIndex(const unsigned int numBuckets, const float cellSize)
//         : mHashMap(numBuckets), mInvCellSize(1.0f / cellSize) {}

//     void Insert(const glm::vec3& pos, T* thing) {
//         mHashMap[Discretize(pos, mInvCellSize)].push_back(thing);
//     }

//     void Neighbors(const glm::vec3& pos, NeighborList& ret) const {
//         const glm::ivec3 ipos = Discretize(pos, mInvCellSize);
//         for (const auto& offset : mOffsets) {
//             typename HashMap::const_iterator it = mHashMap.find(offset + ipos);
//             if (it != mHashMap.end()) {
//                 ret.insert(ret.end(), it->second.begin(), it->second.end());
//             }
//         }
//     }

//     void Clear() {
//         mHashMap.clear();
//     }

// private:
//     struct TeschnerHash : std::unary_function<glm::ivec3, std::size_t> {
//         std::size_t operator()(glm::ivec3 const& pos) const {
//             const unsigned int p1 = 73856093;
//             const unsigned int p2 = 19349663;
//             const unsigned int p3 = 83492791;
//             return size_t((pos.x * p1) ^ (pos.y * p2) ^ (pos.z * p3));
//         };
//     };

//     static inline glm::ivec3 Discretize(const glm::vec3& pos, const float invCellSize) {
//         return glm::ivec3(glm::floor(pos * invCellSize));
//     }

//     typedef std::unordered_map<glm::ivec3, NeighborList, TeschnerHash> HashMap;
//     HashMap mHashMap;

//     const float mInvCellSize;
//     std::vector<glm::ivec3> mOffsets {
//         {-1, -1, -1}, {-1, -1, 0}, {-1, -1, 1},
//         {-1, 0, -1}, {-1, 0, 0}, {-1, 0, 1},
//         {-1, 1, -1}, {-1, 1, 0}, {-1, 1, 1},
//         {0, -1, -1}, {0, -1, 0}, {0, -1, 1},
//         {0, 0, -1}, {0, 0, 0}, {0, 0, 1},
//         {0, 1, -1}, {0, 1, 0}, {0, 1, 1},
//         {1, -1, -1}, {1, -1, 0}, {1, -1, 1},
//         {1, 0, -1}, {1, 0, 0}, {1, 0, 1},
//         {1, 1, -1}, {1, 1, 0}, {1, 1, 1}
//     };
// };

typedef SpatialIndex<Particle> IndexType;
IndexType indexsp(4093, r*2);

// --------------------------------------------------------------------
void initParticles(const unsigned int pN)
{
	float w = p_size;
	float layer_W = i_girth / 2.;
	for (float y = bottom + 0.1; y <= 5.; y += r * 0.5f)
	{
		for (float x = -layer_W / 2.; x <= layer_W / 2.; x += r * 0.5f)
		{
			for (float z = -layer_W * cos((x)*F_PI / layer_W) / 2.; z <= layer_W * cos((x)*F_PI / layer_W) / 2.; z += r * 0.5f)
			{
				if (particles.size() > pN)
				{
					break;
				}

				Particle p;
				p.pos = glm::vec3(x, y, z);
				p.pos_old = p.pos; // + 0.001f * glm::vec3(rand01(), rand01(), rand01());
				p.force = glm::vec3(0, 0, 0);
				p.sigma = 3.f;
				p.beta = 4.f;
				particles.push_back(p);
			}
		}
	}
}

// float add_new = 0;
void addParticleLayers(const unsigned int pL)
{
	float w = p_size;
	int layer = 0;
	float layer_W = i_girth * 0.4;
	float startLayer = bottom + 5.5;
	// if(add_new > 0. && Time - add_new < .1){
	// 	startLayer += (Time - add_new)*100 + pL*r*0.5;
	// }
	// fprintf(stderr, "Time: %f \t LayerStart:%f\n", Time, startLayer);
	// add_new = Time;
	for (float y = startLayer; y <= 8.; y += r * 0.5f)
	{
		for (float x = -layer_W / 2.; x <= layer_W / 2.; x += r * 0.5f)
		{
			for (float z = -layer_W * cos((x)*F_PI / layer_W) / 2.; z <= layer_W * cos((x)*F_PI / layer_W) / 2.; z += r * 0.5f)
			{
				// if (particles.size() > pN)
				// {
				// 	break;
				// }

				Particle p;
				p.pos = glm::vec3(x, y, z);
				p.pos_old = p.pos + 0.001f * glm::vec3(rand01(), rand01(), rand01());
				p.force = glm::vec3(0, -::G * 2, 0);
				p.sigma = 3.f;
				p.beta = 4.f;
				particles.push_back(p);
			}
		}
		layer++;
		if (layer >= pL)
		{
			break;
		}
	}
}

// --------------------------------------------------------------------
// Update particle positions
void step()
{
	//
	for (int i = 0; i < (int)particles.size(); ++i)
	{
		// Apply the currently accumulated forces
		particles[i].pos += particles[i].force;

		// Restart the forces with gravity only. We'll add the rest later.
		if (useGravity)
		{
			particles[i].force = glm::vec3(0.f, -::G, 0.f);
		}
		else
		{
			particles[i].force = glm::vec3(0.f, 0.f, 0.f);
		}

		// Calculate the velocity for later.
		particles[i].vel = particles[i].pos - particles[i].pos_old;

		// A small hack
		const float max_vel = 2.0f;
		const float vel_mag = glm::dot(particles[i].vel, particles[i].vel);
		// If the velocity is greater than the max velocity, then cut it in half.
		if (vel_mag > max_vel * max_vel)
		{
			particles[i].vel *= .1f;
		}

		// Normal verlet stuff
		particles[i].pos_old = particles[i].pos;
		particles[i].pos += particles[i].vel;

		// If the Particle is outside the bounds of the world, then
		// Make a little spring force to push it back in.
		if (useGravity)
		{
			if (particles[i].pos.x < -SIM_W)
				particles[i].force.x -= (particles[i].pos.x - -SIM_W) / 8;
			if (particles[i].pos.x > SIM_W)
				particles[i].force.x -= (particles[i].pos.x - SIM_W) / 8;

			if (particles[i].pos.z < -SIM_W)
				particles[i].force.z -= (particles[i].pos.z - -SIM_W) / 8;
			if (particles[i].pos.z > SIM_W)
				particles[i].force.z -= (particles[i].pos.z - SIM_W) / 8;

			if (particles[i].pos.y < bottom)
				particles[i].force.y -= (particles[i].pos.y - bottom) / 8;
			if (particles[i].pos.y > bottom+10)
				particles[i].force.y -= (particles[i].pos.y - (bottom+10)) / 8;
		}

		// Reset the nessecary items.
		// particles[i].rho = 0;
		// particles[i].rho_near = 0;
		particles[i].neighbors.clear();
	}

	// update spatial index
	indexsp.Clear();
	for (auto &particle : particles)
	{
		indexsp.Insert(glm::vec3(particle.pos), &particle);
	}

	// DENSITY
	// Calculate the density by basically making a weighted sum
	// of the distances of neighboring particles within the radius of support (r)
	for (int i = 0; i < (int)particles.size(); ++i)
	{
		particles[i].rho = 0;
		particles[i].rho_near = 0;

		// We will sum up the 'near' and 'far' densities.
		float d = 0;
		float dn = 0;

		IndexType::NeighborList neigh;
		neigh.reserve(64);
		indexsp.Neighbors(glm::vec3(particles[i].pos), neigh);
		for (int j = 0; j < (int)neigh.size(); ++j)
		{
			if (neigh[j] == &particles[i])
			{
				// do not calculate an interaction for a Particle with itself!
				continue;
			}

			// The vector seperating the two particles
			const glm::vec3 rij = neigh[j]->pos - particles[i].pos;

			// Along with the squared distance between
			const float rij_len2 = glm::dot(rij, rij);

			// If they're within the radius of support ...
			if (rij_len2 < rsq)
			{
				// Get the actual distance from the squared distance.
				float rij_len = sqrt(rij_len2);

				// And calculated the weighted distance values
				const float q = 1 - (rij_len / r);
				const float q2 = q * q;
				const float q3 = q2 * q;

				d += q2;
				dn += q3;

				// Set up the Neighbor list for faster access later.
				Neighbor n;
				n.j = neigh[j];
				n.q = q;
				n.q2 = q2;
				particles[i].neighbors.push_back(n);
			}
		}

		particles[i].rho += d;
		particles[i].rho_near += dn;
	}

	// PRESSURE
	// Make the simple pressure calculation from the equation of state.
	for (int i = 0; i < (int)particles.size(); ++i)
	{
		particles[i].press = k * (particles[i].rho - rest_density);
		particles[i].press_near = k_near * particles[i].rho_near;
	}

	// PRESSURE FORCE
	// We will force particles in or out from their neighbors
	// based on their difference from the rest density.
	for (int i = 0; i < (int)particles.size(); ++i)
	{
		// For each of the neighbors
		glm::vec3 dX(0);
		for (const Neighbor &n : particles[i].neighbors)
		{
			// The vector from Particle i to Particle j
			const glm::vec3 rij = (*n.j).pos - particles[i].pos;

			// calculate the force from the pressures calculated above
			const float dm = n.q * (particles[i].press + (*n.j).press) + n.q2 * (particles[i].press_near + (*n.j).press_near);

			// Get the direction of the force
			const glm::vec3 D = glm::normalize(rij) * dm;
			dX += D;
		}

		particles[i].force -= dX;
	}

	// Viscosity
	for (int i = 0; i < (int)particles.size(); ++i)
	{
		// We'll let the color be determined by
		// ... x-velocity for the red component
		// ... y-velocity for the green-component
		// ... pressure for the blue component
		particles[i].r = 0.3f + (20 * fabs(glm::dot(particles[i].vel.x, particles[i].vel.z)) );
		particles[i].g = 0.3f + (20 * fabs(particles[i].vel.y) );
		particles[i].b = 0.3f + (.1f * particles[i].rho );

		// For each of that particles neighbors
		for (const Neighbor &n : particles[i].neighbors)
		{
			const glm::vec3 rij = (*n.j).pos - particles[i].pos;
			const float l = glm::length(rij);
			const float q = l / r;

			const glm::vec3 rijn = (rij / l);
			// Get the projection of the velocities onto the vector between them.
			const float u = glm::dot(particles[i].vel - (*n.j).vel, rijn);
			if (u > 0)
			{
				// Calculate the viscosity impulse between the two particles
				// based on the quadratic function of projected length.
				const glm::vec3 I = (1 - q) * ((*n.j).sigma * u + (*n.j).beta * u * u) * rijn;

				// Apply the impulses on the current particle
				particles[i].vel -= I * 0.5f;
			}
		}
	}
}

// main program:

int main(int argc, char *argv[])
{
	// turn on the glut package:
	// (do this before checking argc and argv since glutInit might
	// pull some command line arguments out)

	glutInit(&argc, argv);

	// setup all the graphics stuff:

	InitGraphics();

	// create the display lists that **will not change**:

	InitLists();

	// Initialize initial number of particles
	initParticles(N);

	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset();

	// setup all the user interface stuff:

	InitMenus();

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow(MainWindow);
	glutMainLoop();

	// glutMainLoop( ) never actually returns
	// the following line is here to make the compiler happy:

	return 0;
}

// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutPostRedisplay( ) do it

void Animate()
{
	// put animation stuff in here -- change some global variables for Display( ) to find:

	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MS_PER_CYCLE;						// makes the value of ms between 0 and MS_PER_CYCLE-1
	Time = (float)ms / (float)MS_PER_CYCLE; // makes the value of Time between 0. and slightly less than 1.
	// fprintf(stderr, "%f\n", Time);

	// for example, if you wanted to spin an object in Display( ), you might call: glRotatef( 360.f*Time,   0., 1., 0. );

	// force a call to Display( ) next time it is convenient:

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// draw the complete scene:

void Display()
{
	// initial timer
	int ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MS_PER_CYCLE;						// makes the value of ms between 0 and MS_PER_CYCLE-1
	float i_time = (float)ms / (float)MS_PER_CYCLE; // makes the value of Time between 0. and slightly less than 1.
	if (DebugOn != 0)
		fprintf(stderr, "Starting Display.\n");

	// set which window we want to do the graphics into:
	glutSetWindow(MainWindow);

	// erase the background:
	glDrawBuffer(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
#ifdef DEMO_DEPTH_BUFFER
	if (DepthBufferOn == 0)
		glDisable(GL_DEPTH_TEST);
#endif

	// specify shading to be flat:

	glShadeModel(GL_SMOOTH);

	// set the viewport to be a square centered in the window:

	GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
	GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
	GLsizei v = vx < vy ? vx : vy; // minimum dimension
	GLint xl = (vx - v) / 2;
	GLint yb = (vy - v) / 2;
	glViewport(xl, yb, v, v);

	// set the viewing volume:
	// remember that the Z clipping  values are given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (NowProjection == ORTHO)
		glOrtho(-2.f, 2.f, -2.f, 2.f, 0.1f, 1000.f);
	else
		gluPerspective(70.f, 1.f, 0.1f, 1000.f);

	// place the objects into the scene:

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// set the eye position, look-at position, and up-vector:

	gluLookAt(0.f, 5.f, 5.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);

	// rotate the scene:

	glRotatef((GLfloat)Yrot, 0.f, 1.f, 0.f);
	glRotatef((GLfloat)Xrot, 1.f, 0.f, 0.f);

	// uniformly scale the scene:

	if (Scale < MINSCALE)
		Scale = MINSCALE;
	glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);

	// set the fog parameters:

	if (DepthCueOn != 0)
	{
		glFogi(GL_FOG_MODE, FOGMODE);
		glFogfv(GL_FOG_COLOR, FOGCOLOR);
		glFogf(GL_FOG_DENSITY, FOGDENSITY);
		glFogf(GL_FOG_START, FOGSTART);
		glFogf(GL_FOG_END, FOGEND);
		glEnable(GL_FOG);
	}
	else
	{
		glDisable(GL_FOG);
	}

	// possibly draw the axes:

	// if (AxesOn != 0)
	// {
	// 	glColor3fv(&Colors[NowColor][0]);
	// 	glCallList(AxesList);
	// }

	// since we are using glScalef( ), be sure the normals get unitized:

	glEnable(GL_NORMALIZE);

	// draw the box object by calling up its display list:
	glEnable(GL_LIGHTING);

	SetPointLight(GL_LIGHT0, -5., 5., 5., 1., 1., 1.);

	// glCallList(ParticleList);
	// glPointSize(p_size);
	// glVertexPointer( 3, GL_FLOAT, sizeof(Particle), &particles[0].pos );
	// glEnableClientState( GL_VERTEX_ARRAY );
	// glDrawArrays( GL_POINTS, 0, static_cast< GLsizei >( particles.size() ) );
	// glDisableClientState( GL_VERTEX_ARRAY );

	// Iterate through your particles and draw spheres at their positions
	// glColor3f(.2, .2, .9);
	
	SetMaterial(.2, .2, .8, 10.);
	for (const auto &particle : particles)
	{
		glPushMatrix();
		// SetMaterial(particle.r, particle.g, particle.b, 10.);
		glTranslatef(particle.pos.x, particle.pos.y, particle.pos.z); // Translate to the position of the particle
		glCallList(ParticleList);									  // Call your sphere drawing function
		glPopMatrix();
	}

	if (doSimulation)
	{
		step();
	}

	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);

#ifdef DEMO_Z_FIGHTING
	if (DepthFightingOn != 0)
	{
		glPushMatrix();
		glRotatef(90.f, 0.f, 1.f, 0.f);
		glCallList(ParticleList);
		glPopMatrix();
	}
#endif

	// draw some gratuitous text that just rotates on top of the scene:
	// i commented out the actual text-drawing calls -- put them back in if you have a use for them
	// a good use for thefirst one might be to have your name on the screen
	// a good use for the second one might be to have vertex numbers on the screen alongside each vertex

	glDisable(GL_DEPTH_TEST);
	glColor3f(0.f, 1.f, 1.f);
	// DoRasterString( 0.f, 1.f, 0.f, (char *)"Text That Moves" );

	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.f, 100.f, 0.f, 100.f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glColor3f(1.f, 1.f, 1.f);
	// DoRasterString( 5.f, 5.f, 0.f, (char *)"Text That Doesn't" );

	// swap the double-buffered framebuffers:
	// take time
	ms = glutGet(GLUT_ELAPSED_TIME);
	ms %= MS_PER_CYCLE;						// makes the value of ms between 0 and MS_PER_CYCLE-1
	float f_time = (float)ms / (float)MS_PER_CYCLE; // makes the value of Time between 0. and slightly less than 1.
	fprintf(stderr, "Simulation Time: %f\n", f_time - i_time);

	glutSwapBuffers();

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush();
}

void DoAxesMenu(int id)
{
	AxesOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoColorMenu(int id)
{
	NowColor = id - RED;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoDebugMenu(int id)
{
	DebugOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoDepthBufferMenu(int id)
{
	DepthBufferOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoDepthFightingMenu(int id)
{
	DepthFightingOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoDepthMenu(int id)
{
	DepthCueOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// main menu callback:

void DoMainMenu(int id)
{
	switch (id)
	{
	case RESET:
		Reset();
		break;

	case QUIT:
		// gracefully close out the graphics:
		// gracefully close the graphics window:
		// gracefully exit the program:
		glutSetWindow(MainWindow);
		glFinish();
		glutDestroyWindow(MainWindow);
		exit(0);
		break;

	default:
		fprintf(stderr, "Don't know what to do with Main Menu ID %d\n", id);
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoProjectMenu(int id)
{
	NowProjection = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// use glut to display a string of characters using a raster font:

void DoRasterString(float x, float y, float z, char *s)
{
	glRasterPos3f((GLfloat)x, (GLfloat)y, (GLfloat)z);

	char c; // one character to print
	for (; (c = *s) != '\0'; s++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
	}
}

// use glut to display a string of characters using a stroke font:

void DoStrokeString(float x, float y, float z, float ht, char *s)
{
	glPushMatrix();
	glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
	float sf = ht / (119.05f + 33.33f);
	glScalef((GLfloat)sf, (GLfloat)sf, (GLfloat)sf);
	char c; // one character to print
	for (; (c = *s) != '\0'; s++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
	}
	glPopMatrix();
}

// return the number of seconds since the start of the program:

float ElapsedSeconds()
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet(GLUT_ELAPSED_TIME);

	// convert it to seconds:

	return (float)ms / 1000.f;
}

// initialize the glui window:

void InitMenus()
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitMenus.\n");

	glutSetWindow(MainWindow);

	int numColors = sizeof(Colors) / (3 * sizeof(float));
	int colormenu = glutCreateMenu(DoColorMenu);
	for (int i = 0; i < numColors; i++)
	{
		glutAddMenuEntry(ColorNames[i], i);
	}

	int axesmenu = glutCreateMenu(DoAxesMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int depthcuemenu = glutCreateMenu(DoDepthMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int depthbuffermenu = glutCreateMenu(DoDepthBufferMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int depthfightingmenu = glutCreateMenu(DoDepthFightingMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int debugmenu = glutCreateMenu(DoDebugMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int projmenu = glutCreateMenu(DoProjectMenu);
	glutAddMenuEntry("Orthographic", ORTHO);
	glutAddMenuEntry("Perspective", PERSP);

	int mainmenu = glutCreateMenu(DoMainMenu);
	glutAddSubMenu("Axes", axesmenu);
	glutAddSubMenu("Axis Colors", colormenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu("Depth Buffer", depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu("Depth Fighting", depthfightingmenu);
#endif

	glutAddSubMenu("Depth Cue", depthcuemenu);
	glutAddSubMenu("Projection", projmenu);
	glutAddMenuEntry("Reset", RESET);
	glutAddSubMenu("Debug", debugmenu);
	glutAddMenuEntry("Quit", QUIT);

	// attach the pop-up menu to the right mouse button:

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// initialize the glut and OpenGL libraries:
//	also setup callback functions

void InitGraphics()
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitGraphics.\n");

	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	// set the initial window configuration:

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);

	// open the window and set its title:

	MainWindow = glutCreateWindow(WINDOWTITLE);
	glutSetWindowTitle(WINDOWTITLE);

	// set the framebuffer clear values:

	glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow(MainWindow);
	glutDisplayFunc(Display);
	glutReshapeFunc(Resize);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	glutPassiveMotionFunc(MouseMotion);
	// glutPassiveMotionFunc( NULL );
	glutVisibilityFunc(Visibility);
	glutEntryFunc(NULL);
	glutSpecialFunc(NULL);
	glutSpaceballMotionFunc(NULL);
	glutSpaceballRotateFunc(NULL);
	glutSpaceballButtonFunc(NULL);
	glutButtonBoxFunc(NULL);
	glutDialsFunc(NULL);
	glutTabletMotionFunc(NULL);
	glutTabletButtonFunc(NULL);
	glutMenuStateFunc(NULL);
	glutTimerFunc(-1, NULL, 0);

	// setup glut to call Animate( ) every time it has
	// 	nothing it needs to respond to (which is most of the time)
	// we don't need to do this for this program, and really should set the argument to NULL
	// but, this sets us up nicely for doing animation

	glutIdleFunc(Animate);

	// init the glew package (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		fprintf(stderr, "glewInit Error\n");
	}
	else
		fprintf(stderr, "GLEW initialized OK\n");
	fprintf(stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	// all other setups go here, such as GLSLProgram and KeyTime setups:
}

// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void InitLists()
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitLists.\n");

	float dx = BOXSIZE / 2.f;
	float dy = BOXSIZE / 2.f;
	float dz = BOXSIZE / 2.f;
	glutSetWindow(MainWindow);

	// create the object:

	ParticleList = glGenLists(1);
	glNewList(ParticleList, GL_COMPILE);

	OsuSphere(p_size, 10, 10);

	glEndList();

	// create the axes:

	AxesList = glGenLists(1);
	glNewList(AxesList, GL_COMPILE);
	glLineWidth(AXES_WIDTH);
	Axes(1.5);
	glLineWidth(1.);
	glEndList();
}

// the keyboard callback:

void Keyboard(unsigned char c, int x, int y)
{
	if (DebugOn != 0)
		fprintf(stderr, "Keyboard: '%c' (0x%0x)\n", c, c);

	switch (c)
	{
	case 'o':
	case 'O':
		NowProjection = ORTHO;
		break;

	case 'p':
	case 'P':
		NowProjection = PERSP;
		break;

	case 'q':
	case 'Q':
	case ESCAPE:
		DoMainMenu(QUIT); // will not return here
		break;			  // happy compiler

	case 's':
	case 'S':
		doSimulation = !doSimulation;
		break;

	case ' ':
		addParticleLayers(4);
		break;

	case 'g':
	case 'G':
		useGravity = !useGravity;
		break;
	
	case '1':
		rest_density = 1.;
		break;
	
	case '2':
		rest_density = 2.;
		break;

	case '3':
		rest_density = 3.;
		break;
	
	case '4':
		rest_density = 4.;
		break;

	case '5':
		rest_density = 5.;
		break;
	
	case '6':
		rest_density = 6.;
		break;

	case '7':
		rest_density = 7.;
		break;
	
	case '8':
		rest_density = 8.;
		break;
	
	case '9':
		rest_density = 9.;
		break;
	
	case '0':
		rest_density = 10.;
		break;

	default:
		fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
	}

	// force a call to Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// called when the mouse button transitions down or up:

void MouseButton(int button, int state, int x, int y)
{
	int b = 0; // LEFT, MIDDLE, or RIGHT

	if (DebugOn != 0)
		fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y);

	// get the proper button bit mask:

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		b = LEFT;
		break;

	case GLUT_MIDDLE_BUTTON:
		b = MIDDLE;
		break;

	case GLUT_RIGHT_BUTTON:
		b = RIGHT;
		break;

	case SCROLL_WHEEL_UP:
		Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
		// keep object from turning inside-out or disappearing:
		if (Scale < MINSCALE)
			Scale = MINSCALE;
		break;

	case SCROLL_WHEEL_DOWN:
		Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
		// keep object from turning inside-out or disappearing:
		if (Scale < MINSCALE)
			Scale = MINSCALE;
		break;

	default:
		b = 0;
		fprintf(stderr, "Unknown mouse button: %d\n", button);
	}

	// button down sets the bit, up clears the bit:

	if (state == GLUT_DOWN)
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b; // set the proper bit
	}
	else
	{
		ActiveButton &= ~b; // clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// called when the mouse moves while a button is down:

void MouseMotion(int x, int y)
{
	int dx = x - Xmouse; // change in mouse coords
	int dy = y - Ymouse;

	if ((ActiveButton & LEFT) != 0)
	{
		Xrot += (ANGFACT * dy);
		Yrot += (ANGFACT * dx);
	}

	if ((ActiveButton & MIDDLE) != 0)
	{
		Scale += SCLFACT * (float)(dx - dy);

		// keep object from turning inside-out or disappearing:

		if (Scale < MINSCALE)
			Scale = MINSCALE;
	}

	Xmouse = x; // new current position
	Ymouse = y;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void Reset()
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale = 1.0;
	ShadowsOn = 0;
	NowColor = YELLOW;
	NowProjection = PERSP;
	Xrot = Yrot = 0.;

	particles.clear();
	initParticles(N);
	doSimulation = false;
	useGravity = true;
}

// called when user resizes the window:

void Resize(int width, int height)
{
	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

// handle a change to the window's visibility:

void Visibility(int state)
{
	if (DebugOn != 0)
		fprintf(stderr, "Visibility: %d\n", state);

	if (state == GLUT_VISIBLE)
	{
		glutSetWindow(MainWindow);
		glutPostRedisplay();
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}

///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////

// the stroke characters 'X' 'Y' 'Z' :

static float xx[] = {0.f, 1.f, 0.f, 1.f};

static float xy[] = {-.5f, .5f, .5f, -.5f};

static int xorder[] = {1, 2, -3, 4};

static float yx[] = {0.f, 0.f, -.5f, .5f};

static float yy[] = {0.f, .6f, 1.f, 1.f};

static int yorder[] = {1, 2, 3, -2, 4};

static float zx[] = {1.f, 0.f, 1.f, 0.f, .25f, .75f};

static float zy[] = {.5f, .5f, -.5f, -.5f, 0.f, 0.f};

static int zorder[] = {1, 2, 3, 4, -5, 6};

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void Axes(float length)
{
	glBegin(GL_LINE_STRIP);
	glVertex3f(length, 0., 0.);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., length, 0.);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., 0., length);
	glEnd();

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 4; i++)
	{
		int j = xorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(base + fact * xx[j], fact * xy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 5; i++)
	{
		int j = yorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(fact * yx[j], base + fact * yy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < 6; i++)
	{
		int j = zorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(0.0, fact * zy[j], base + fact * zx[j]);
	}
	glEnd();
}

// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void HsvRgb(float hsv[3], float rgb[3])
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while (h >= 6.)
		h -= 6.;
	while (h < 0.)
		h += 6.;

	float s = hsv[1];
	if (s < 0.)
		s = 0.;
	if (s > 1.)
		s = 1.;

	float v = hsv[2];
	if (v < 0.)
		v = 0.;
	if (v > 1.)
		v = 1.;

	// if sat==0, then is a gray:

	if (s == 0.0)
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:

	float i = (float)floor(h);
	float f = h - i;
	float p = v * (1.f - s);
	float q = v * (1.f - s * f);
	float t = v * (1.f - (s * (1.f - f)));

	float r = 0., g = 0., b = 0.; // red, green, blue
	switch ((int)i)
	{
	case 0:
		r = v;
		g = t;
		b = p;
		break;

	case 1:
		r = q;
		g = v;
		b = p;
		break;

	case 2:
		r = p;
		g = v;
		b = t;
		break;

	case 3:
		r = p;
		g = q;
		b = v;
		break;

	case 4:
		r = t;
		g = p;
		b = v;
		break;

	case 5:
		r = v;
		g = p;
		b = q;
		break;
	}

	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

float Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}

float Unit(float v[3])
{
	float dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		v[0] /= dist;
		v[1] /= dist;
		v[2] /= dist;
	}
	return dist;
}
