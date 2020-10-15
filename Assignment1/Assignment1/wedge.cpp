#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gl/glut.h>
#include <utility>
#include <vector>
#include "VECTOR3D.h"
#include "cube.h"
#include "QuadMesh.h"

const int vWidth = 650;    // Viewport width in pixels
const int vHeight = 500;    // Viewport height in pixels

// My size variables
float robotBodyWidth = 10.0;
float robotBodyLength = 3.0;
float robotBodyDepth = 10.0;
float wedgeWidth = robotBodyWidth * 0.5;
float wedgeLength = robotBodyLength;
float wedgeDepth = robotBodyDepth;
float leftWheel = robotBodyLength * 0.4;
float rightWheel = robotBodyLength * 0.4;

// Control Robot body rotation on base
float robotAngle = 0.0;

// Control wedge rotation
float plateAngle = -25.0;

// Lighting/shading and material properties for robot - upcoming lecture - just copy for now
// Robot RGBA material properties (NOTE: we will learn about this later in the semester)
GLfloat robotBody_mat_ambient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat robotBody_mat_specular[] = { 0.45f,0.55f,0.45f,1.0f };
GLfloat robotBody_mat_diffuse[] = { 0.1f,0.35f,0.1f,1.0f };
GLfloat robotBody_mat_shininess[] = { 32.0F };

GLfloat robotArm_mat_ambient[] = { 0.0215f, 0.1745f, 0.0215f, 0.55f };
GLfloat robotArm_mat_diffuse[] = { 0.5f,0.0f,0.0f,1.0f };
GLfloat robotArm_mat_specular[] = { 0.7f, 0.6f, 0.6f, 1.0f };
GLfloat robotArm_mat_shininess[] = { 32.0F };

GLfloat gun_mat_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat gun_mat_diffuse[] = { 0.01f,0.0f,0.01f,0.01f };
GLfloat gun_mat_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat gun_mat_shininess[] = { 100.0F };

GLfloat robotLowerBody_mat_ambient[] = { 0.25f, 0.25f, 0.25f, 1.0f };
GLfloat robotLowerBody_mat_diffuse[] = { 0.4f, 0.4f, 0.4f, 1.0f };
GLfloat robotLowerBody_mat_specular[] = { 0.774597f, 0.774597f, 0.774597f, 1.0f };
GLfloat robotLowerBody_mat_shininess[] = { 76.8F };

// Light properties
GLfloat light_position0[] = { -4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_position1[] = { 4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };

// Mouse button
int currentButton;

// A template cube mesh
CubeMesh *cubeMesh = createCubeMesh();

// A flat open mesh
QuadMesh *groundMesh = NULL;

// Default Mesh Size
int meshSize = 16;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void animationHandler(int param);
void drawRobot();
void drawBody();
void drawWedge();
void drawPlate();
void drawLeftWheel();
void drawRightWheel();

int main(int argc, char **argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(200, 30);
	glutCreateWindow("Jeremy Ng 500882192 Wedge Flipper");

	// Initialize GL
	initOpenGL(vWidth, vHeight);

	// Register callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotionHandler);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);

	// Start event loop, never returns
	glutMainLoop();

	return 0;
}

// Set up OpenGL. For viewport and projection setup see reshape(). 
void initOpenGL(int w, int h)
{
	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);   // This second light is currently off

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Other initializatuion
	// Set up ground quad mesh
	VECTOR3D origin = VECTOR3D(-16.0f, 0.0f, 16.0f);
	VECTOR3D dir1v = VECTOR3D(1.0f, 0.0f, 0.0f);
	VECTOR3D dir2v = VECTOR3D(0.0f, 0.0f, -1.0f);
	groundMesh = new QuadMesh(meshSize, 32.0);
	groundMesh->InitMesh(meshSize, origin, 32.0, 32.0, dir1v, dir2v);

	VECTOR3D ambient = VECTOR3D(0.0f, 0.05f, 0.0f);
	VECTOR3D diffuse = VECTOR3D(0.4f, 0.8f, 0.4f);
	VECTOR3D specular = VECTOR3D(0.04f, 0.04f, 0.04f);
	float shininess = 0.2;
	groundMesh->SetMaterial(ambient, diffuse, specular, shininess);
}

// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	// Create Viewing Matrix V
	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// Draw Robot

	// Apply modelling transformations M to move robot
	// Current transformation matrix is set to IV, where I is identity matrix
	// CTM = IV
	drawRobot();
	
	glutSwapBuffers();   // Double buffering, swap buffers
}

void drawRobot()
{
	glPushMatrix();
	// spin robot on base. 
	glRotatef(robotAngle, 0.0, 1.0, 0.0);

	drawBody();
	drawWedge();
	drawPlate();
	drawLeftWheel();
	drawRightWheel();
	glPopMatrix();
}

void drawBody()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotBody_mat_shininess);

	glPushMatrix();
	glTranslatef(0, 0, 0);

	glPushMatrix();
	glScalef(robotBodyWidth, robotBodyLength, robotBodyDepth * 0.7);
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glScalef(robotBodyWidth * 0.5, robotBodyLength, robotBodyDepth * 0.3);
	glTranslatef(robotBodyWidth * 0.05, 0, robotBodyDepth * 0.15);
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glScalef(robotBodyWidth * 0.5, robotBodyLength, robotBodyDepth * 0.3);
	glTranslatef(robotBodyWidth * 0.05, 0, robotBodyDepth * -0.15);
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glScalef(robotBodyWidth, robotBodyLength, robotBodyDepth * 0.05);
	glTranslatef(0, 0, robotBodyDepth * 1.2);
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glScalef(robotBodyWidth, robotBodyLength, robotBodyDepth * 0.05);
	glTranslatef(0, 0, robotBodyDepth * -1.2);
	glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
	glScalef(robotBodyWidth * 0.05, robotBodyLength, robotBodyDepth * 1.25);
	glTranslatef(robotBodyWidth * -1, 0, 0);
	glutSolidCube(1.0);
	glPopMatrix();

	glPopMatrix();
}

void drawWedge()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);

	glPushMatrix();
	// Position wedge with respect to parent body
	glTranslatef(0.8 * robotBodyWidth, 0 - robotBodyLength * 0.5, 0.0);

	// Build wedge
	glPushMatrix();
	glScalef(wedgeWidth * 1.2, wedgeLength * 0.05, wedgeDepth * 1.25);
	glutSolidCube(1.0);
	glPopMatrix();

	glPopMatrix();
}

void drawPlate()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);

	glPushMatrix();
	glTranslatef(robotBodyWidth * 0.8, robotBodyLength * 0.45, 0.0);

	glPushMatrix();
	glTranslatef(
		0 - (wedgeWidth * 1.25) * 0.5,
		0 + (wedgeLength * 0.05) * 0.5,
		0
	);
	glRotatef(plateAngle, 0, 0, 1);
	glTranslatef(
		0 + (wedgeWidth * 1.25) * 0.5,
		0 - (wedgeLength * 0.05) * 0.5,
		0
	);

	glPushMatrix();
	glScalef(wedgeWidth * 1.25, wedgeLength * 0.05, wedgeDepth * 1.25);
	glutSolidCube(1.0);
	glPopMatrix();
	glPopMatrix();
	glPopMatrix();
}

void drawLeftWheel()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);

	glPushMatrix();

	// Position left wheel with respect to parent body
	glTranslatef(robotBodyWidth * -0.25, 0, robotBodyDepth * -0.45);

	// Build Wheels
	glPushMatrix();
	glScalef(leftWheel, leftWheel, leftWheel);
	glutSolidTorus(1, 1, 10, 100);

	glPopMatrix();
	glPopMatrix();
}

void drawRightWheel()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);

	glPushMatrix();

	// Position Right wheel with respect to parent body
	glTranslatef(robotBodyWidth * -0.25, 0, robotBodyDepth * 0.45);

	// Build Wheels
	glPushMatrix();
	glScalef(leftWheel, leftWheel, leftWheel);
	glutSolidTorus(1, 1, 10, 100);

	glPopMatrix();
	glPopMatrix();
}

// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
	// Set up viewport, projection, then change to modelview matrix mode - 
	// display function will then set up camera and do modeling transforms.
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)w / h, 0.2, 40.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

bool stop = true;

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'e':
		robotAngle += 2.0;
		break;
	case 'q':
		robotAngle -= 2.0;
		break;
	case ' ':
		if (stop)
		{
			stop = false;
			glutTimerFunc(0, animationHandler, 1);
		}
		else
		{
			glutTimerFunc(1, animationHandler, 0);
			stop = true;
		}
		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}


void animationHandler(int param)
{
	if (plateAngle < -25)
	{
		plateAngle = -25;
	}
	if (plateAngle > 45)
	{
		plateAngle = 45;
	}
	if (!stop)
	{
		plateAngle += 1.0;
		glutPostRedisplay();
		glutTimerFunc(0, animationHandler, 1);
	}
	else
	{
		plateAngle -= 1.0;
		glutPostRedisplay();
		glutTimerFunc(1, animationHandler, 0);
	}
}

// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
	// Help key
	if (key == GLUT_KEY_F1)
	{

	}
	// Do transformations with arrow keys
	//else if (...)   // GLUT_KEY_DOWN, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_LEFT
	//{
	//}

	glutPostRedisplay();   // Trigger a window redisplay
}

// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
	currentButton = button;

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			;

		}
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{
			;
		}
		break;
	default:
		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}

// Mouse motion callback - use only if you want to 
void mouseMotionHandler(int xMouse, int yMouse)
{
	if (currentButton == GLUT_LEFT_BUTTON)
	{
		;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}
