#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include "imageloader.h"
#include "vec3f.h"
#endif


void cylinder(float a,float aa,float aaa);
void kerucut(float a1, float a2, double a3);
void blok(float aa1,int aa2,int aa3);

static GLfloat spin, spin2 = 0.0;
float angle = 0;
using namespace std;

float lastx, lasty;
GLint stencilBits;
static int viewx = 50;
static int viewy = 24;
static int viewz = 80;

float rot = 0;

//train 2D
//class untuk terain 2D
class Terrain {
private:
	int w; //Width
	int l; //Length
	float** hs; //Heights
	Vec3f** normals;
	bool computedNormals; //Whether normals is up-to-date
public:
	Terrain(int w2, int l2) {
		w = w2;
		l = l2;

		hs = new float*[l];
		for (int i = 0; i < l; i++) {
			hs[i] = new float[w];
		}

		normals = new Vec3f*[l];
		for (int i = 0; i < l; i++) {
			normals[i] = new Vec3f[w];
		}

		computedNormals = false;
	}

	~Terrain() {
		for (int i = 0; i < l; i++) {
			delete[] hs[i];
		}
		delete[] hs;

		for (int i = 0; i < l; i++) {
			delete[] normals[i];
		}
		delete[] normals;
	}

	int width() {
		return w;
	}

	int length() {
		return l;
	}

	//Sets the height at (x, z) to y
	void setHeight(int x, int z, float y) {
		hs[z][x] = y;
		computedNormals = false;
	}

	//Returns the height at (x, z)
	float getHeight(int x, int z) {
		return hs[z][x];
	}

	//Computes the normals, if they haven't been computed yet
	void computeNormals() {
		if (computedNormals) {
			return;
		}

		//Compute the rough version of the normals
		Vec3f** normals2 = new Vec3f*[l];
		for (int i = 0; i < l; i++) {
			normals2[i] = new Vec3f[w];
		}

		for (int z = 0; z < l; z++) {
			for (int x = 0; x < w; x++) {
				Vec3f sum(0.0f, 0.0f, 0.0f);

				Vec3f out;
				if (z > 0) {
					out = Vec3f(0.0f, hs[z - 1][x] - hs[z][x], -1.0f);
				}
				Vec3f in;
				if (z < l - 1) {
					in = Vec3f(0.0f, hs[z + 1][x] - hs[z][x], 1.0f);
				}
				Vec3f left;
				if (x > 0) {
					left = Vec3f(-1.0f, hs[z][x - 1] - hs[z][x], 0.0f);
				}
				Vec3f right;
				if (x < w - 1) {
					right = Vec3f(1.0f, hs[z][x + 1] - hs[z][x], 0.0f);
				}

				if (x > 0 && z > 0) {
					sum += out.cross(left).normalize();
				}
				if (x > 0 && z < l - 1) {
					sum += left.cross(in).normalize();
				}
				if (x < w - 1 && z < l - 1) {
					sum += in.cross(right).normalize();
				}
				if (x < w - 1 && z > 0) {
					sum += right.cross(out).normalize();
				}

				normals2[z][x] = sum;
			}
		}

		//Smooth out the normals
		const float FALLOUT_RATIO = 0.5f;
		for (int z = 0; z < l; z++) {
			for (int x = 0; x < w; x++) {
				Vec3f sum = normals2[z][x];

				if (x > 0) {
					sum += normals2[z][x - 1] * FALLOUT_RATIO;
				}
				if (x < w - 1) {
					sum += normals2[z][x + 1] * FALLOUT_RATIO;
				}
				if (z > 0) {
					sum += normals2[z - 1][x] * FALLOUT_RATIO;
				}
				if (z < l - 1) {
					sum += normals2[z + 1][x] * FALLOUT_RATIO;
				}

				if (sum.magnitude() == 0) {
					sum = Vec3f(0.0f, 1.0f, 0.0f);
				}
				normals[z][x] = sum;
			}
		}

		for (int i = 0; i < l; i++) {
			delete[] normals2[i];
		}
		delete[] normals2;

		computedNormals = true;
	}

	//Returns the normal at (x, z)
	Vec3f getNormal(int x, int z) {
		if (!computedNormals) {
			computeNormals();
		}
		return normals[z][x];
	}
};
//end class


void initRendering() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
}

//Loads a terrain from a heightmap.  The heights of the terrain range from
//-height / 2 to height / 2.
//load terain di procedure inisialisasi
Terrain* loadTerrain(const char* filename, float height) {
	Image* image = loadBMP(filename);
	Terrain* t = new Terrain(image->width, image->height);
	for (int y = 0; y < image->height; y++) {
		for (int x = 0; x < image->width; x++) {
			unsigned char color = (unsigned char) image->pixels[3 * (y
					* image->width + x)];
			float h = height * ((color / 255.0f) - 0.5f);
			t->setHeight(x, y, h);
		}
	}

	delete image;
	t->computeNormals();
	return t;
}

float _angle = 60.0f;
//buat tipe data terain
Terrain* _terrain;
Terrain* _terrainTanah;
Terrain* _terrainAir;

const GLfloat light_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
const GLfloat light_diffuse[] = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 1.0f, 1.0f, 1.0f, 1.0f };

const GLfloat light_ambient2[] = { 0.3f, 0.3f, 0.3f, 0.0f };
const GLfloat light_diffuse2[] = { 0.3f, 0.3f, 0.3f, 0.0f };

const GLfloat mat_ambient[] = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

void cleanup() {
	delete _terrain;
	delete _terrainTanah;
}

//untuk di display
void drawSceneTanah(Terrain *terrain, GLfloat r, GLfloat g, GLfloat b) {
	//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/*
	 glMatrixMode(GL_MODELVIEW);
	 glLoadIdentity();
	 glTranslatef(0.0f, 0.0f, -10.0f);
	 glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
	 glRotatef(-_angle, 0.0f, 1.0f, 0.0f);

	 GLfloat ambientColor[] = {0.4f, 0.4f, 0.4f, 1.0f};
	 glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientColor);

	 GLfloat lightColor0[] = {0.6f, 0.6f, 0.6f, 1.0f};
	 GLfloat lightPos0[] = {-0.5f, 0.8f, 0.1f, 0.0f};
	 glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor0);
	 glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);
	 */
	float scale = 500.0f / max(terrain->width() - 1, terrain->length() - 1);
	glScalef(scale, scale, scale);
	glTranslatef(-(float) (terrain->width() - 1) / 2, 0.0f,
			-(float) (terrain->length() - 1) / 2);

	glColor3f(r, g, b);
	for (int z = 0; z < terrain->length() - 1; z++) {
		//Makes OpenGL draw a triangle at every three consecutive vertices
		glBegin(GL_TRIANGLE_STRIP);
		for (int x = 0; x < terrain->width(); x++) {
			Vec3f normal = terrain->getNormal(x, z);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, terrain->getHeight(x, z), z);
			normal = terrain->getNormal(x, z + 1);
			glNormal3f(normal[0], normal[1], normal[2]);
			glVertex3f(x, terrain->getHeight(x, z + 1), z + 1);
		}
		glEnd();
	}

}

unsigned int LoadTextureFromBmpFile(char *filename);


GLfloat  source_light[]={0.30,0.30,0.30,0.50};
double Loop,LoopAll=0;
bool on=false;


void Mobil(void)
{
glClearColor(1.0,9.0,1.0,0.0);  
glShadeModel(GL_SMOOTH);
glMatrixMode(GL_PROJECTION);
glLoadIdentity();
gluPerspective(50.0,1.5,10.0,1000.0);

glEnable (GL_DEPTH_TEST);
glPolygonMode   (GL_FRONT_AND_BACK,GL_FILL);
glEnable (GL_LIGHTING);
glLightfv (GL_LIGHT7,GL_DIFFUSE,source_light);
glEnable (GL_LIGHT7);
glEnable (GL_COLOR_MATERIAL);
glColorMaterial (GL_FRONT_AND_BACK,GL_AMBIENT);

}




//void display1(void)
//{
//
//glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
//glMatrixMode(GL_MODELVIEW);
//glLoadIdentity();
//
//
//
//glTranslatef(0,0,-50);// besar kecilnya mobil
//
//Loop++;
//  glRotatef(Loop,0.9,4.0,0.6);//mobil yang berotasi
//
//glPushMatrix();//body
////glpushmatrix();
//glColor3f(0.0,0.0,9.0);
//blok(10, 3, 2);
//
//glTranslatef(0, 9, 0);
//blok(10, 3, 2);
//glTranslatef(10, -10,0);
//blok(10, 5.5, 2);
//glRotatef(-35, 0, 0, 15);
//glTranslatef(0, 7,0);
//blok(10, 2, 2);
//glTranslatef(2, 4.9,-2.5);
//glColor3f(0.9,0.9,0.9);// warna kaca
//blok(0.5, 20, 31);
//
//glRotatef(180, 45, -45, 0);
////glTranslatef(0, 10,0);
////blok(3, 2, 5);
////cylinder(2,2,30);
////glTranslatef(-10, -10,0);
////glRotatef(90, 45,-45, 0);
////cylinder(2,2,30);
////glRotatef(90, -45, 90, 90);
////blok(5, 3, 2);
//
//glPopMatrix();
//
//
//
//glPushMatrix();//roda
//glColor3f(0.0,0.0,0.0);
//glTranslatef(20, -8,-7);
//cylinder(5, 5,3);
//
//
//
//
//glColor3f(0.0,0.0,0.0);
//glTranslatef(-20, 8,7);
//glTranslatef(-5, -8,-7);
//cylinder(5, 5,3);
//
//glColor3f(0.0,0.0,0.0);
//glTranslatef(5, 8,7);
//glRotatef(180,0,180,0);
//glTranslatef(3,-8,-17);
//cylinder(5, 5,3);
//
//glColor3f(0.0,0.0,0.0);
//glTranslatef(-3,8,17);
//glTranslatef(-22,-8,-17);
//cylinder(5, 5,3);
//
//glColor3f(1.0,1.0,1.0);
//
//glRotatef(90,1,0,0);
//glTranslatef(8, 2.5,-15);
//blok(2, 4, 5);
//
//
//glRotatef(90,0,1,0);
//glTranslatef(0,-0.2,7);
//blok(2, 4, 8);
//
//
//glRotatef(0,0,0,0);
//glTranslatef(0,19.2,0);
//blok(2, 4, 8);
//
//glRotatef(90,0,1,0);
//glTranslatef(7, 0,-8);
//blok(2, 4, 5);
//
//glColor3f(9.9,9.9,0.0);//lampu
//glRotatef(90,0,1,0);
//glTranslatef(0,-3,20);
//cylinder(2, 2,3);
//
//glColor3f(9.9,9.9,0.0);//lampu
//glRotatef(0,0,0,0);
//glTranslatef(0,-12,0);
//cylinder(2, 2,3);
//
//
//glColor3f(0.0,0.0,0.0);//ban serep
//glRotatef(0,0,0,0);
//glTranslatef(0,0,-52);
//cylinder(1, 1,3);
//
//glColor3f(1.0,0.0,0.0);
//glTranslatef(0, 0,0);
//cylinder(4, 4,3);
//
//
//
//glColor3f(1.0,1.0,1.0);
//glRotatef(90,1,0,0);
//glTranslatef(-8,3.5,-12);
//blok(2, 4, 8);
//
//
//glColor3f(9.0,0.0,0.0);//lampu aa
//glRotatef(0,0,0,0);
//glTranslatef(-8,28,0);
//cylinder(1, 1,12);
//
//
//
//glPopMatrix();
//
//
//glFlush();
//glutSwapBuffers();
//
//}

void kerucut(float a1, float a2, float a3)
{
float i;
glPushMatrix();
glTranslatef(1.0,0.0,a1/24);
glutSolidCone(a1,0,32,4);
for(i=0;i<=a3;i+=a1/24)
{
glTranslatef(0.0,0.0,a1/24);
glutSolidTorus(a1/4,a1-((i*(a1-a2))/a3),16,16);
}
glTranslatef(0.0,0.0,a1/4);
glutSolidCone(a2,0,20,1);
glColor3f(0.,1.,1.);
glPopMatrix();
}


void cylinder(float a,float aa,float aaa)
{
float i;
glPushMatrix();
glTranslatef(1.0,0.0,-a/8);
glutSolidCone(a,0,32,4);
for(i=0;i<=aaa;i+=a/24)
{
glTranslatef(0.0,0.0,a/24);
glutSolidTorus(a/4,a-((i*(a-aa))/aaa),16,16);
}
glTranslatef(0.0,0.0,a/4);
glutSolidCone(aa,0,20,1);
glColor3f(1.,0.,0.);
glPopMatrix();
}

void blok(float tebal,int aa2,int aa3)
{
float i,j;
glPushMatrix();
for(i=0;i<aa3;i++)
{
glTranslatef(-(aa2+1)*tebal/2,0.0,0.0);
for(j=0;j<aa2;j++)
{
glTranslatef(tebal,0.0,0.0);
glutSolidCube(tebal);
}
glTranslatef(-(aa2-1)*tebal/2,0.0,tebal);
}
glPopMatrix();
}
//void tampil();
//{
//display();
//}

void display(void) {
	glClearStencil(0); //clear the stencil buffer
	glClearDepth(1.0f);
	glClearColor(0.0, 0.6, 0.8, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); //clear the buffers
	glLoadIdentity();
	gluLookAt(viewx, viewy, viewz, 0.0, 0.0, 5.0, 0.0, 1.0, 0.0);

	glPushMatrix();

	//glBindTexture(GL_TEXTURE_3D, texture[0]);
	drawSceneTanah(_terrain, 0.3f, 0.9f, 0.0f);
	glPopMatrix();

	glPushMatrix();

	//glBindTexture(GL_TEXTURE_3D, texture[0]);
	drawSceneTanah(_terrainTanah, 0.7f, 0.2f, 0.1f);
	glPopMatrix();

	glPushMatrix();

	//glBindTexture(GL_TEXTURE_3D, texture[0]);
	drawSceneTanah(_terrainAir, 0.0f, 0.2f, 0.5f);
	glPopMatrix();

	glutSwapBuffers();
	glFlush();
	rot++;
	angle++;
	
	
	
glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
glMatrixMode(GL_MODELVIEW);
glLoadIdentity();



glTranslatef(0,0,-50);// besar kecilnya mobil

Loop++;
  glRotatef(Loop,0.9,4.0,0.6);//mobil yang berotasi

glPushMatrix();//body
//glpushmatrix();
glColor3f(0.0,0.0,9.0);
blok(10, 3, 2);

glTranslatef(0, 9, 0);
blok(10, 3, 2);
glTranslatef(10, -10,0);
blok(10.0, 5, 2);
glRotatef(-35, 0, 0, 15);
glTranslatef(0, 7,0);
blok(10, 2, 2);
glTranslatef(2, 4.9,-2.5);
glColor3f(0.9,0.9,0.9);// warna kaca
blok(0.5, 20, 31);

glRotatef(180, 45, -45, 0);
//glTranslatef(0, 10,0);
//blok(3, 2, 5);
//cylinder(2,2,30);
//glTranslatef(-10, -10,0);
//glRotatef(90, 45,-45, 0);
//cylinder(2,2,30);
//glRotatef(90, -45, 90, 90);
//blok(5, 3, 2);

glPopMatrix();



glPushMatrix();//roda
glColor3f(0.0,0.0,0.0);
glTranslatef(20, -8,-7);
cylinder(5, 5,3);




glColor3f(0.0,0.0,0.0);
glTranslatef(-20, 8,7);
glTranslatef(-5, -8,-7);
cylinder(5, 5,3);

glColor3f(0.0,0.0,0.0);
glTranslatef(5, 8,7);
glRotatef(180,0,180,0);
glTranslatef(3,-8,-17);
cylinder(5, 5,3);

glColor3f(0.0,0.0,0.0);
glTranslatef(-3,8,17);
glTranslatef(-22,-8,-17);
cylinder(5, 5,3);

glColor3f(1.0,1.0,1.0);

glRotatef(90,1,0,0);
glTranslatef(8, 2.5,-15);
blok(2, 4, 5);


glRotatef(90,0,1,0);
glTranslatef(0,-0.2,7);
blok(2, 4, 8);


glRotatef(0,0,0,0);
glTranslatef(0,19.2,0);
blok(2, 4, 8);

glRotatef(90,0,1,0);
glTranslatef(7, 0,-8);
blok(2, 4, 5);

glColor3f(9.9,9.9,0.0);//lampu
glRotatef(90,0,1,0);
glTranslatef(0,-3,20);
cylinder(2, 2,3);

glColor3f(9.9,9.9,0.0);//lampu
glRotatef(0,0,0,0);
glTranslatef(0,-12,0);
cylinder(2, 2,3);


glColor3f(0.0,0.0,0.0);//ban serep
glRotatef(0,0,0,0);
glTranslatef(0,0,-52);
cylinder(1, 1,3);

glColor3f(1.0,0.0,0.0);
glTranslatef(0, 0,0);
cylinder(4, 4,3);



glColor3f(1.0,1.0,1.0);
glRotatef(90,1,0,0);
glTranslatef(-8,3.5,-12);
blok(2, 4, 8);


glColor3f(9.0,0.0,0.0);//lampu aa
glRotatef(0,0,0,0);
glTranslatef(-8,28,0);
cylinder(1, 1,12);



glPopMatrix();


glFlush();
glutSwapBuffers();


}

void init(void) {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glDepthFunc(GL_LESS);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_CULL_FACE);

	_terrain = loadTerrain("heightmap.bmp", 20);
	_terrainTanah = loadTerrain("heightmapTanah.bmp", 20);
	_terrainAir = loadTerrain("heightmapAir.bmp", 20);

	//binding texture

}

static void kibor(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_HOME:
		viewy++;
		break;
	case GLUT_KEY_END:
		viewy--;
		break;
	case GLUT_KEY_UP:
		viewz--;
		break;
	case GLUT_KEY_DOWN:
		viewz++;
		break;

	case GLUT_KEY_RIGHT:
		viewx++;
		break;
	case GLUT_KEY_LEFT:
		viewx--;
		break;

	case GLUT_KEY_F1: {
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	}
		;
		break;
	case GLUT_KEY_F2: {
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient2);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse2);
		glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	}
		;
		break;
	default:
		break;
	}
}

void keyboard(unsigned char key, int x, int y) {
	if (key == 'd') {

		spin = spin - 1;
		if (spin > 360.0)
			spin = spin - 360.0;
	}
	if (key == 'a') {
		spin = spin + 1;
		if (spin > 360.0)
			spin = spin - 360.0;
	}
	if (key == 'q') {
		viewz++;
	}
	if (key == 'e') {
		viewz--;
	}
	if (key == 's') {
		viewy--;
	}
	if (key == 'w') {
		viewy++;
	}
}

void reshape(int w, int h) {
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (GLfloat) w / (GLfloat) h, 0.1, 1000.0);
	glMatrixMode(GL_MODELVIEW);
}



//void cylinder(float a,float aa,float aaa);
//void kerucut(float a1, float a2, double a3);
//void blok(float aa1,int aa2,int aa3);
//
//GLfloat  source_light[]={0.30,0.30,0.30,0.50};
//double Loop,LoopAll=0;
//bool on=false;
//
//
//void Mobil(void)
//{
//glClearColor(1.0,9.0,1.0,0.0);  
//glShadeModel(GL_SMOOTH);
//glMatrixMode(GL_PROJECTION);
//glLoadIdentity();
//gluPerspective(50.0,1.5,10.0,1000.0);
//
//glEnable (GL_DEPTH_TEST);
//glPolygonMode   (GL_FRONT_AND_BACK,GL_FILL);
//glEnable (GL_LIGHTING);
//glLightfv (GL_LIGHT7,GL_DIFFUSE,source_light);
//glEnable (GL_LIGHT7);
//glEnable (GL_COLOR_MATERIAL);
//glColorMaterial (GL_FRONT_AND_BACK,GL_AMBIENT);
//
//}
//
//
//
//
////void display1(void)
////{
////
////glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
////glMatrixMode(GL_MODELVIEW);
////glLoadIdentity();
////
////
////
////glTranslatef(0,0,-50);// besar kecilnya mobil
////
////Loop++;
////  glRotatef(Loop,0.9,4.0,0.6);//mobil yang berotasi
////
////glPushMatrix();//body
//////glpushmatrix();
////glColor3f(0.0,0.0,9.0);
////blok(10, 3, 2);
////
////glTranslatef(0, 9, 0);
////blok(10, 3, 2);
////glTranslatef(10, -10,0);
////blok(10, 5.5, 2);
////glRotatef(-35, 0, 0, 15);
////glTranslatef(0, 7,0);
////blok(10, 2, 2);
////glTranslatef(2, 4.9,-2.5);
////glColor3f(0.9,0.9,0.9);// warna kaca
////blok(0.5, 20, 31);
////
////glRotatef(180, 45, -45, 0);
//////glTranslatef(0, 10,0);
//////blok(3, 2, 5);
//////cylinder(2,2,30);
//////glTranslatef(-10, -10,0);
//////glRotatef(90, 45,-45, 0);
//////cylinder(2,2,30);
//////glRotatef(90, -45, 90, 90);
//////blok(5, 3, 2);
////
////glPopMatrix();
////
////
////
////glPushMatrix();//roda
////glColor3f(0.0,0.0,0.0);
////glTranslatef(20, -8,-7);
////cylinder(5, 5,3);
////
////
////
////
////glColor3f(0.0,0.0,0.0);
////glTranslatef(-20, 8,7);
////glTranslatef(-5, -8,-7);
////cylinder(5, 5,3);
////
////glColor3f(0.0,0.0,0.0);
////glTranslatef(5, 8,7);
////glRotatef(180,0,180,0);
////glTranslatef(3,-8,-17);
////cylinder(5, 5,3);
////
////glColor3f(0.0,0.0,0.0);
////glTranslatef(-3,8,17);
////glTranslatef(-22,-8,-17);
////cylinder(5, 5,3);
////
////glColor3f(1.0,1.0,1.0);
////
////glRotatef(90,1,0,0);
////glTranslatef(8, 2.5,-15);
////blok(2, 4, 5);
////
////
////glRotatef(90,0,1,0);
////glTranslatef(0,-0.2,7);
////blok(2, 4, 8);
////
////
////glRotatef(0,0,0,0);
////glTranslatef(0,19.2,0);
////blok(2, 4, 8);
////
////glRotatef(90,0,1,0);
////glTranslatef(7, 0,-8);
////blok(2, 4, 5);
////
////glColor3f(9.9,9.9,0.0);//lampu
////glRotatef(90,0,1,0);
////glTranslatef(0,-3,20);
////cylinder(2, 2,3);
////
////glColor3f(9.9,9.9,0.0);//lampu
////glRotatef(0,0,0,0);
////glTranslatef(0,-12,0);
////cylinder(2, 2,3);
////
////
////glColor3f(0.0,0.0,0.0);//ban serep
////glRotatef(0,0,0,0);
////glTranslatef(0,0,-52);
////cylinder(1, 1,3);
////
////glColor3f(1.0,0.0,0.0);
////glTranslatef(0, 0,0);
////cylinder(4, 4,3);
////
////
////
////glColor3f(1.0,1.0,1.0);
////glRotatef(90,1,0,0);
////glTranslatef(-8,3.5,-12);
////blok(2, 4, 8);
////
////
////glColor3f(9.0,0.0,0.0);//lampu aa
////glRotatef(0,0,0,0);
////glTranslatef(-8,28,0);
////cylinder(1, 1,12);
////
////
////
////glPopMatrix();
////
////
////glFlush();
////glutSwapBuffers();
////
////}
//
//void kerucut(float a1, float a2, float a3)
//{
//float i;
//glPushMatrix();
//glTranslatef(1.0,0.0,a1/24);
//glutSolidCone(a1,0,32,4);
//for(i=0;i<=a3;i+=a1/24)
//{
//glTranslatef(0.0,0.0,a1/24);
//glutSolidTorus(a1/4,a1-((i*(a1-a2))/a3),16,16);
//}
//glTranslatef(0.0,0.0,a1/4);
//glutSolidCone(a2,0,20,1);
//glColor3f(0.,1.,1.);
//glPopMatrix();
//}
//
//
//void cylinder(float a,float aa,float aaa)
//{
//float i;
//glPushMatrix();
//glTranslatef(1.0,0.0,-a/8);
//glutSolidCone(a,0,32,4);
//for(i=0;i<=aaa;i+=a/24)
//{
//glTranslatef(0.0,0.0,a/24);
//glutSolidTorus(a/4,a-((i*(a-aa))/aaa),16,16);
//}
//glTranslatef(0.0,0.0,a/4);
//glutSolidCone(aa,0,20,1);
//glColor3f(1.,0.,0.);
//glPopMatrix();
//}
//
//void blok(float tebal,int aa2,int aa3)
//{
//float i,j;
//glPushMatrix();
//for(i=0;i<aa3;i++)
//{
//glTranslatef(-(aa2+1)*tebal/2,0.0,0.0);
//for(j=0;j<aa2;j++)
//{
//glTranslatef(tebal,0.0,0.0);
//glutSolidCube(tebal);
//}
//glTranslatef(-(aa2-1)*tebal/2,0.0,tebal);
//}
//glPopMatrix();
//}
//void tampil()
//{
//display();
//}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL | GLUT_DEPTH); //add a stencil buffer to the window
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Sample Terain");
	init();

	glutDisplayFunc(display);
	glutIdleFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(kibor);
	
	
	//glutDisplayFunc(display1);
	//glutIdleFunc(tampil);
	

	glutKeyboardFunc(keyboard);

	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
	glColorMaterial(GL_FRONT, GL_DIFFUSE);
	
    glClearColor(0.5,0.5,0.0,0.0);
    
    gluOrtho2D(-320., 320., -240.0, 240.0);
    
    
    Mobil();
	glutMainLoop();
	return 0;





}
