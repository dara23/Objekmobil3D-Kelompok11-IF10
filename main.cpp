#include <stdlib.h>
#include <glut.h>



void cylinder(float a,float aa,float aaa);
void kerucut(float a1, float a2, double a3);
void blok(float aa1,int aa2,int aa3);

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




void display(void)
{

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
blok(10, 5.5, 2);
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
void tampil()
{
display();
}


int main(int argc,char **argv)
{


glutInit(&argc,argv);
glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);//|GLUT_DEPTH);
glutInitWindowPosition(10,10);
glutInitWindowSize(1000,700);
glutCreateWindow("Kelompok 11 - IF 10 - 2012");
glClearColor(0.5,0.5,0.0,0.0);
glutDisplayFunc(display);
gluOrtho2D(-320., 320., -240.0, 240.0);
glutIdleFunc(tampil);

Mobil();
glutMainLoop();
return(0);
}
/*
Keterangan:
glTranslatef : coba nilainya diubah2 untuk menempatkan pada posisi yang tepat dengan sumbu (x, y, z)
glRotated : coba nilainya diubah2 untuk memutar objek agar sesuai dengan keinginan.
cylinder: untuk membentuk model tabung

*/
