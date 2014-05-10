#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <vector>

using namespace std ;

#define nFPS 30

float sealevel;
float polysize;

//a struct used to store a vector or a point
typedef struct _coord_t{
	GLdouble X;  GLdouble Y;  GLdouble Z;
}coord_t;

coord_t eye;      //eye location
coord_t lookAt;   //lookat vector
coord_t up;		  //up vector
coord_t rightVector ;  //rightvector

GLfloat alpha;   //roll angle
GLfloat beta ;   //pitch angle
GLfloat speed;   //plane speed


int seed(float x, float y) {
    static int a = 1588635695, b = 1117695901;
	int xi = *(int *)&x;
	int yi = *(int *)&y;
    return ((xi * a) % b) - ((yi * b) % a);
}

void mountain(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2, float s)
{
	float x01,y01,z01,x12,y12,z12,x20,y20,z20;

	if (s < polysize) {
		x01 = x1 - x0;
		y01 = y1 - y0;
		z01 = z1 - z0;

		x12 = x2 - x1;
		y12 = y2 - y1;
		z12 = z2 - z1;

		x20 = x0 - x2;
		y20 = y0 - y2;
		z20 = z0 - z2;

		float nx = y01*(-z20) - (-y20)*z01;
		float ny = z01*(-x20) - (-z20)*x01;
		float nz = x01*(-y20) - (-x20)*y01;

		float den = sqrt(nx*nx + ny*ny + nz*nz);

		if (den > 0.0) {
			nx /= den;
			ny /= den;
			nz /= den;
		}

		glNormal3f(nx,ny,nz);
		glBegin(GL_TRIANGLES);
			glVertex3f(x0,y0,z0);
			glVertex3f(x1,y1,z1);
			glVertex3f(x2,y2,z2);
		glEnd();

		return;
	}

	x01 = 0.5*(x0 + x1);
	y01 = 0.5*(y0 + y1);
	z01 = 0.5*(z0 + z1);

	x12 = 0.5*(x1 + x2);
	y12 = 0.5*(y1 + y2);
	z12 = 0.5*(z1 + z2);

	x20 = 0.5*(x2 + x0);
	y20 = 0.5*(y2 + y0);
	z20 = 0.5*(z2 + z0);

	s *= 0.5;

	srand(seed(x01,y01));
	z01 += 0.3*s*(2.0*((float)rand()/(float)RAND_MAX) - 1.0);
	srand(seed(x12,y12));
	z12 += 0.3*s*(2.0*((float)rand()/(float)RAND_MAX) - 1.0);
	srand(seed(x20,y20));
	z20 += 0.3*s*(2.0*((float)rand()/(float)RAND_MAX) - 1.0);

	mountain(x0,y0,z0,x01,y01,z01,x20,y20,z20,s);
	mountain(x1,y1,z1,x12,y12,z12,x01,y01,z01,s);
	mountain(x2,y2,z2,x20,y20,z20,x12,y12,z12,s);
	mountain(x01,y01,z01,x12,y12,z12,x20,y20,z20,s);
}

//unitize v
void unitize(coord_t &v){
	GLfloat length = sqrt(v.X*v.X+v.Y*v.Y+v.Z*v.Z);
	v.X/=length;
	v.Y/=length;
	v.Z/=length;
}

//obtain a unit vector from v
coord_t unitize_(coord_t v){
	coord_t out;
	GLfloat length = sqrt(v.X*v.X+v.Y*v.Y+v.Z*v.Z);
	out.X=v.X/length;
	out.Y=v.Y/length;
	out.Z=v.Z/length;

	return out;
}

//result= u cross v
void get_cross_product(const coord_t &u, const coord_t &v, coord_t &result){
		result.X=u.Y*v.Z-u.Z*v.Y;
		result.Y=u.Z*v.X-u.X*v.Z;
		result.Z=u.X*v.Y-u.Y*v.X;
}

// Rightvector=lookat cross up_vector
void update_R(){
	get_cross_product(lookAt,up,rightVector);
	unitize(rightVector);
}


//parameters: vec: (x,y,z) axis: (u,v,w) and rotate angle
//rotate vector vec about arbituray axis axis (assuming axis is a unit vector)
//return: the vector after rotation. 
vector<float> rotateAboutArbi(coord_t vec, coord_t axis, GLfloat angle){

	float x=vec.X; float y=vec.Y; float z=vec.Z;
	float u=axis.X; float v=axis.Y; float w=axis.Z ; 

/********************this equation is cited from outside source--see README.txt **********************/
	float newX=u*(u*x+v*y+w*z)*(1-cos(angle))+x*cos(angle)+(-w*y+v*z)*sin(angle);
    float newY=v*(u*x+v*y+w*z)*(1-cos(angle))+y*cos(angle)+( w*x-u*z)*sin(angle);
    float newZ=w*(u*x+v*y+w*z)*(1-cos(angle))+z*cos(angle)+(-v*x+u*y)*sin(angle);
/*****************************************************************************************************/

    vector<float> result;
    result.push_back(newX);    result.push_back(newY);    result.push_back(newZ);
    return result ;
}


void init(void) 
{
	GLfloat white[] = {1.0,1.0,1.0,1.0};
	GLfloat lpos[] = {0.0,1.0,0.0,0.0};

	alpha=0.01; beta=0.01; 
	speed = 0.003;
	eye = (coord_t){ .X = 0.5, .Y = 0.0, .Z = 0.25 };
	lookAt = (coord_t){ .X = -0.5, .Y =  0.0, .Z = -0.25 };
	up = (coord_t){ .X = 0.0, .Y = 0.0, .Z = 1.0 };

	update_R();

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT0, GL_POSITION, lpos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, white);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);

	glClearColor (0.5, 0.5, 1.0, 0.0);
	/* glShadeModel (GL_FLAT); */
	glEnable(GL_DEPTH_TEST);

	sealevel = 0.0;
	polysize = 0.01;
}

//rotation about lookat vector, the up vector and right 
//vectors are rotated by an angle about lookat vector
void roll(float angle){
	coord_t unit_lookAt=unitize_(lookAt);

	//cout<< "lookat vector is "<< "( "<< lookAt.X << ","<< lookAt.Y<< "," << lookAt.Z<<")"<<endl;
	//cout<< "unit_lookat vector is "<< "( "<< unit_lookAt.X << ","<< unit_lookAt.Y<< "," << unit_lookAt.Z<<")"<<endl;

// first rotated up
	vector<float> newUp=rotateAboutArbi(up,unit_lookAt, angle);
	up.X=newUp[0];
	up.Y=newUp[1];
	up.Z=newUp[2];

	//cout<< "up vector is "<< "( "<< up.X << ","<< up.Y<< "," << up.Z<<")"<<endl;
	unitize(up);
// Rightvector can be simply obtained by doing lookat cross Up:
	update_R();


}

// rotation about rightvector, while tilting, the lookat and up vectors are updated
void tilt(float angle){
	vector<float> newLookat=rotateAboutArbi(lookAt,rightVector,angle);
	lookAt.X=newLookat[0];
	lookAt.Y=newLookat[1];
	lookAt.Z=newLookat[2];

	vector<float> newUp=rotateAboutArbi(up,rightVector,angle);
	up.X=newUp[0];
	up.Y=newUp[1];
	up.Z=newUp[2];

	unitize(up);
	update_R();
}

void display(void)
{
	//static GLfloat angle = 0.0;

	GLfloat tanamb[] = {0.2,0.15,0.1,1.0};
	GLfloat tandiff[] = {0.4,0.3,0.2,1.0};

	GLfloat seaamb[] = {0.0,0.0,0.2,1.0};
	GLfloat seadiff[] = {0.0,0.0,0.8,1.0};
	GLfloat seaspec[] = {0.5,0.5,1.0,1.0};


	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f (1.0, 1.0, 1.0);

	glLoadIdentity ();             /* clear the matrix */
			/* viewing transformation  */

    // updated eye, centerPoint, and up parameters go here:
	gluLookAt (		eye.X,          eye.Y,          eye.Z, 
					eye.X+lookAt.X, eye.Y+lookAt.Y, eye.Z+lookAt.Z, 
					up.X,           up.Y,           up.Z    );

	glTranslatef (-0.5, -0.5, 0.0);      /* modeling transformation */ 

	//cout<< "up vector is "<< "( "<< up.X << ","<< up.Y<< "," << up.Z<<")"<<endl;


	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, tanamb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, tandiff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tandiff);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0);

	mountain(0.0,0.0,0.0, 1.0,0.0,0.0, 0.0,1.0,0.0, 1.0);
	mountain(1.0,1.0,0.0, 0.0,1.0,0.0, 1.0,0.0,0.0, 1.0);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, seaamb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, seadiff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, seaspec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0);

	glNormal3f(0.0,0.0,1.0);
	glBegin(GL_QUADS);
		glVertex3f(0.0,0.0,sealevel);
		glVertex3f(1.0,0.0,sealevel);
		glVertex3f(1.0,1.0,sealevel);
		glVertex3f(0.0,1.0,sealevel);
	glEnd();

	glutSwapBuffers();
	glFlush ();

	//angle -= 0.01;

	glutPostRedisplay();
}

void reshape (int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective(90.0,1.0,0.01,10.0);
	glMatrixMode (GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y)
{
   switch (key) {
		case '-':
			sealevel -= 0.01;
			break;
		case '+':
		case '=':
			sealevel += 0.01;
			break;
		case 'f':
			polysize *= 0.5;
			break;
		case 'c':
			polysize *= 2.0;
			break;
		case 'w':
			speed+=0.001;
			break;
		case 's':
			speed-=0.001;
			break;
		case 'a':
			alpha+=0.005;
			break;
		case 'd':
			alpha-=0.005;
			break;
		case 27:
			exit(0);
			break;
   }
   if(speed<=0) speed=0;
   if(speed>=0.1) speed=0.1;


}

//specialkeyboard function recognizes arrow keys
void  arrowKeyFunct(int key, int x, int y){
		
    switch (key) {
    	case GLUT_KEY_LEFT:
    	     roll(-1*alpha);
    		 break;
    	case GLUT_KEY_RIGHT:
    	     roll(alpha);
    		 break;
    	case GLUT_KEY_UP:
    	     tilt(   beta);
    		 break;
        case GLUT_KEY_DOWN:
        	 tilt(-1*beta);
        	 break;
        default: break;

    }
			
}

// implemetation of translation
// the eye position is updated along the lookat vector
void timer(int v)
{ 
	eye.X+=speed*lookAt.X;
	eye.Y+=speed*lookAt.Y;
	eye.Z+=speed*lookAt.Z;

	glutSwapBuffers();
	glutPostRedisplay(); // trigger display function by sending redraw into message queue
	glutTimerFunc(1000/nFPS,timer,v); // restart timer again	
	
}

int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize (500, 500); 
   glutInitWindowPosition (100, 100);
   glutCreateWindow (argv[0]);
   init ();
   glutDisplayFunc(display); 
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutSpecialFunc(arrowKeyFunct);
   glutTimerFunc(1000/nFPS,timer,nFPS); 

   glutMainLoop();
   return 0;
}
