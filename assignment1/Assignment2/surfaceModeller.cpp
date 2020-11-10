/*

*/

#include <stdio.h>
#include <windows.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include <string.h>
#include "surfaceModeller.h"
#include "subdivcurve.h"

GLdouble worldLeft = -12;
GLdouble worldRight = 12;
GLdouble worldBottom = -9;
GLdouble worldTop = 9;
GLdouble worldCenterX = 0.0;
GLdouble worldCenterY = 0.0;
GLdouble wvLeft = -12;
GLdouble wvRight = 12;
GLdouble wvBottom = -9;
GLdouble wvTop = 9;

GLint glutWindowWidth  = 800;
GLint glutWindowHeight = 600;
GLint viewportWidth    = glutWindowWidth;
GLint viewportHeight   = glutWindowHeight;
int window2D, window3D;
int window3DSizeX = 800, window3DSizeY = 600;
GLdouble aspect = (GLdouble)window3DSizeX / window3DSizeY;

int lastMouseX;
int lastMouseY;

GLdouble eyeX = 0.0, eyeY = 6.0, eyeZ = 22.0;
GLdouble zNear = 0.1, zFar = 40.0;
GLdouble fov = 60.0;


// Ground Mesh material
GLfloat groundMat_ambient[]    = {0.4, 0.4, 0.4, 1.0};
GLfloat groundMat_specular[]   = {0.01, 0.01, 0.01, 1.0};
GLfloat groundMat_diffuse[]   = {0.4, 0.4, 0.7, 1.0};
GLfloat groundMat_shininess[]  = {1.0};

// Two Lights
GLfloat light_position0[] = {4.0, 8.0, 8.0, 1.0};
GLfloat light_diffuse0[] = {1.0, 1.0, 1.0, 1.0};

GLfloat light_position1[] = {-4.0, 8.0, 8.0, 1.0};
GLfloat light_diffuse1[] = {1.0, 1.0, 1.0, 1.0};

GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0};
GLfloat model_ambient[]  = {0.5, 0.5, 0.5, 1.0};


// The profile curve is a subdivision curve

int numCirclePoints = 30;
double circleRadius = 0.2;
int hoveredCircle = -1;

int currentCurvePoint = 0;
int angle = 0;
int animate = 0;
int delay = 15; // milliseconds


SubdivisionCurve subcurve;
// Use circles to draw subdivision curve control points
Circle circles[MAXCONTROLPOINTS];

const int rows = 36;
float size = 1.0;
int angleX = 0;
int angleY = 0;
bool wire = false;

int main(int argc, char* argv[])
{
	glutInit(&argc, (char **)argv); 

	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(glutWindowWidth,glutWindowHeight);
	glutInitWindowPosition(50,100);  
	
	// The 2D Window
	window2D = glutCreateWindow("Profile Curve"); 
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	// Initialize the 2D profile curve system
	init2DCurveWindow(); 
	// A few input handlers
	glutMouseFunc(mouseButtonHandler);
	glutMotionFunc(mouseMotionHandler);
	glutPassiveMotionFunc(mouseHoverHandler);
	glutMouseWheelFunc(mouseScrollWheelHandler);
	glutKeyboardFunc(keyboardHandler);
	glutSpecialFunc(specialKeyHandler);
	

	// The 3D Window
	window3D = glutCreateWindow("Surface of Revolution"); 
	glutPositionWindow(900,100);  
	glutDisplayFunc(display3D);
	glutReshapeFunc(reshape3D);
	glutMouseFunc(mouseButtonHandler3D);
	glutMouseWheelFunc(mouseScrollWheelHandler3D);
	glutMotionFunc(mouseMotionHandler3D);
	glutKeyboardFunc(keyboardHandler3D);
	glutSpecialFunc(specialKeyHandler3D);
	// Initialize the 3D system
	init3DSurfaceWindow();

	// Annnd... ACTION!!
	glutMainLoop(); 

	return 0;
}

void init2DCurveWindow() 
{ 
	glLineWidth(3.0);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);
	initSubdivisionCurve();
	initControlPoints();
} 

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);	
	glLoadIdentity();
	gluOrtho2D(wvLeft, wvRight, wvBottom, wvTop);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	draw2DScene();	
	glutSwapBuffers();
}

void draw2DScene() 
{
	drawAxes();
	drawSubdivisionCurve();
	drawControlPoints();
}

void drawAxes()
{
	glPushMatrix();
	glColor3f(1.0, 0.0, 0);
	glBegin(GL_LINE_STRIP);
	glVertex3f(0, 8.0, 0);
	glVertex3f(0, -8.0, 0);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(-8, 0.0, 0);
	glVertex3f(8, 0.0, 0);
	glEnd();
	glPopMatrix();
}

void drawSubdivisionCurve() {
	
	// Subdivide the given curve based on control points
	computeSubdivisionCurve(&subcurve);
	
	// Draw it
	glColor3f(0.0, 1.0, 0.0);
	glPushMatrix();
	glBegin(GL_LINE_STRIP);
	for (int i=0; i<subcurve.numCurvePoints; i++)
	{
		glVertex3f(subcurve.curvePoints[i].x, subcurve.curvePoints[i].y, 0.0);
	}
	glEnd();
	glPopMatrix();
}

void drawControlPoints(){
	int i, j;
	for (i=0; i<subcurve.numControlPoints; i++){
		glPushMatrix();
		glColor3f(1.0f,0.0f,0.0f); 
		glTranslatef(circles[i].circleCenter.x, circles[i].circleCenter.y, 0);
		// for the hoveredCircle, draw an outline and change its colour
		if (i == hoveredCircle){ 
			// outline
			glColor3f(0.0, 1.0, 0.0);
			glBegin(GL_LINE_LOOP); 
			for(j=0; j < numCirclePoints; j++) {
				glVertex3f(circles[i].circlePoints[j].x, circles[i].circlePoints[j].y, 0); 
			}
			glEnd();
			// colour change
			glColor3f(0.5,0.0,1.0);
		}
		glBegin(GL_LINE_LOOP); 
		for(j=0; j < numCirclePoints; j++) {
			glVertex3f(circles[i].circlePoints[j].x, circles[i].circlePoints[j].y, 0); 
		}
		glEnd();
		glPopMatrix();
	}
}

void initSubdivisionCurve() {
	// Initialize 3 control points of the subdivision curve
	subcurve.controlPoints[0].x = 1;
	subcurve.controlPoints[1].x = 2;
	subcurve.controlPoints[2].x = 1;

	subcurve.controlPoints[0].y = 6;
	subcurve.controlPoints[1].y = 1;
	subcurve.controlPoints[2].y = -4;

	subcurve.numControlPoints = 3;
	subcurve.subdivisionSteps = 3;
}

void initControlPoints(){
	int i;
	int num = subcurve.numControlPoints;
	for (i=0; i < num; i++){
		constructCircle(circleRadius, numCirclePoints, circles[i].circlePoints);
		circles[i].circleCenter = subcurve.controlPoints[i];
	}
}

void screenToWorldCoordinates(int xScreen, int yScreen, GLdouble *xw, GLdouble *yw)
{
	GLdouble xView, yView;
	screenToCameraCoordinates(xScreen, yScreen, &xView, &yView);
	cameraToWorldCoordinates(xView, yView, xw, yw);
}

void screenToCameraCoordinates(int xScreen, int yScreen, GLdouble *xCamera, GLdouble *yCamera)
{
	*xCamera = ((wvRight-wvLeft)/glutWindowWidth)  * xScreen; 
	*yCamera = ((wvTop-wvBottom)/glutWindowHeight) * (glutWindowHeight-yScreen); 
}

void cameraToWorldCoordinates(GLdouble xcam, GLdouble ycam, GLdouble *xw, GLdouble *yw)
{
	*xw = xcam + wvLeft;
	*yw = ycam + wvBottom;
}

void worldToCameraCoordinates(GLdouble xWorld, GLdouble yWorld, GLdouble *xcam, GLdouble *ycam)
{
	double wvCenterX = wvLeft   + (wvRight - wvLeft)/2.0;
	double wvCenterY = wvBottom + (wvTop   - wvBottom)/2.0;
	*xcam = worldCenterX - wvCenterX + xWorld;
	*ycam = worldCenterY - wvCenterY + yWorld;
}

int currentButton;

void mouseButtonHandler(int button, int state, int xMouse, int yMouse)
{
	int i;
	
	currentButton = button;
	if (button == GLUT_LEFT_BUTTON)
	{  
		switch (state) {      
		case GLUT_DOWN:
			if (hoveredCircle > -1) {
				screenToWorldCoordinates(xMouse, yMouse, &circles[hoveredCircle].circleCenter.x, &circles[hoveredCircle].circleCenter.y);
				screenToWorldCoordinates(xMouse, yMouse, &subcurve.controlPoints[hoveredCircle].x, &subcurve.controlPoints[hoveredCircle].y);
			}
			break;
		case GLUT_UP:
			glutSetWindow(window3D);
			glutPostRedisplay();
			break;
		}
	}    
	else if (button == GLUT_MIDDLE_BUTTON)
	{
		switch (state) {      
		case GLUT_DOWN:
			break;
		case GLUT_UP:
			if (hoveredCircle == -1 && subcurve.numControlPoints < MAXCONTROLPOINTS){ 
				GLdouble newPointX;
				GLdouble newPointY;
				screenToWorldCoordinates(xMouse, yMouse, &newPointX, &newPointY);
				subcurve.controlPoints[subcurve.numControlPoints].x = newPointX;
				subcurve.controlPoints[subcurve.numControlPoints].y = newPointY;			
				constructCircle(circleRadius, numCirclePoints, circles[subcurve.numControlPoints].circlePoints);
				circles[subcurve.numControlPoints].circleCenter = subcurve.controlPoints[subcurve.numControlPoints];
				subcurve.numControlPoints++;
			} else if (hoveredCircle > -1 && subcurve.numControlPoints > MINCONTROLPOINTS) {
				subcurve.numControlPoints--;
				for (i=hoveredCircle; i<subcurve.numControlPoints; i++){
					subcurve.controlPoints[i].x = subcurve.controlPoints[i+1].x;
					subcurve.controlPoints[i].y = subcurve.controlPoints[i+1].y;
					circles[i].circleCenter = circles[i+1].circleCenter;
				}
			}
			
			glutSetWindow(window3D);
			glutPostRedisplay();
			break;
		}
	}

	glutSetWindow(window2D);
	glutPostRedisplay();
}

void mouseMotionHandler(int xMouse, int yMouse)
{
	if (currentButton == GLUT_LEFT_BUTTON) {  
		if (hoveredCircle > -1) {
			screenToWorldCoordinates(xMouse, yMouse, &circles[hoveredCircle].circleCenter.x, &circles[hoveredCircle].circleCenter.y);
			screenToWorldCoordinates(xMouse, yMouse, &subcurve.controlPoints[hoveredCircle].x, &subcurve.controlPoints[hoveredCircle].y);
		}
	}    
	else if (currentButton == GLUT_MIDDLE_BUTTON) {
	}
	glutPostRedisplay();
}

void mouseHoverHandler(int xMouse, int yMouse)
{
	hoveredCircle = -1;
	GLdouble worldMouseX, worldMouseY;
	screenToWorldCoordinates(xMouse, yMouse, &worldMouseX, &worldMouseY);
	int i;
	// see if we're hovering over a control point
	for (i=0; i<subcurve.numControlPoints; i++){
		GLdouble distToX = worldMouseX - circles[i].circleCenter.x;
		GLdouble distToY = worldMouseY - circles[i].circleCenter.y;
		GLdouble euclideanDist = sqrt(distToX*distToX + distToY*distToY);
		//printf("Dist from point %d is %.2f\n", i, euclideanDist);
		if (euclideanDist < 2.0){
			hoveredCircle = i;
		}
	}
	
	glutPostRedisplay();
}

void mouseScrollWheelHandler(int button, int dir, int xMouse, int yMouse)
{
	GLdouble worldViewableWidth;
	GLdouble worldViewableHeight;
	GLdouble cameraOnCenterX;
	GLdouble cameraOnCenterY;
	GLdouble anchorPointX, anchorPointY;
	double clipWindowWidth;
	double clipWindowHeight;
	double wvCenterX = wvLeft   + (wvRight - wvLeft)/2.0;
	double wvCenterY = wvBottom + (wvTop   - wvBottom)/2.0;
	double wvWidth   = wvRight - wvLeft;
	double wvHeight  = wvTop   - wvBottom;
	
	worldToCameraCoordinates(worldCenterX, worldCenterY, &cameraOnCenterX, &cameraOnCenterY);
	if (wvWidth >= (worldRight-worldLeft)*1.2){
		
		anchorPointX = cameraOnCenterX;
		anchorPointY = cameraOnCenterY;
	} else {
		// else, anchor the zoom to the mouse
		screenToWorldCoordinates(xMouse, yMouse, &anchorPointX, &anchorPointY);
	}
	GLdouble anchorToCenterX = anchorPointX - wvCenterX;
	GLdouble anchorToCenterY = anchorPointY - wvCenterY;

	// set up maximum shift
	GLdouble maxPosShift = 50;
	GLdouble maxNegShift = -50;	
	
	anchorToCenterX = (anchorToCenterX > maxPosShift)? maxPosShift : anchorToCenterX;
	anchorToCenterX = (anchorToCenterX < maxNegShift)? maxNegShift : anchorToCenterX;
	anchorToCenterY = (anchorToCenterY > maxPosShift)? maxPosShift : anchorToCenterY;
	anchorToCenterY = (anchorToCenterY < maxNegShift)? maxNegShift : anchorToCenterY;

	// move the world centre closer to this point.
	wvCenterX += anchorToCenterX/4;
	wvCenterY += anchorToCenterY/4;
	

	if (dir > 0) {
		// Zoom in to mouse point
		clipWindowWidth = wvWidth*0.8;
		clipWindowHeight= wvHeight*0.8;
		wvRight =  wvCenterX + clipWindowWidth/2.0;
		wvTop   =  wvCenterY + clipWindowHeight/2.0;
		wvLeft  =  wvCenterX - clipWindowWidth/2.0;
		wvBottom=  wvCenterY - clipWindowHeight/2.0;

	}
	else {
		// Zoom out
		clipWindowWidth = wvWidth*1.25;
		clipWindowHeight= wvHeight*1.25;
		wvRight =  wvCenterX + clipWindowWidth/2.0;
		wvTop   =  wvCenterY + clipWindowHeight/2.0;
		wvLeft  =  wvCenterX - clipWindowWidth/2.0;
		wvBottom=  wvCenterY - clipWindowHeight/2.0;
	}

	glutPostRedisplay();

}

void keyboardHandler(unsigned char key, int x, int y)
{
	int i;
	
	double clipWindowWidth;
	double clipWindowHeight;
	double wvCenterX = wvLeft   + (wvRight - wvLeft)/2.0;
	double wvCenterY = wvBottom + (wvTop   - wvBottom)/2.0;
	double wvWidth   = wvRight - wvLeft;
	double wvHeight  = wvTop   - wvBottom;

	switch(key){
	case 'q':
	case 'Q':
	case 27:
		// Esc, q, or Q key = Quit 
		exit(0);
		break;
	case 107:
	case '+':
		clipWindowWidth = wvWidth*0.8;
		clipWindowHeight= wvHeight*0.8;
		wvRight =  wvCenterX + clipWindowWidth/2.0;
		wvTop   =  wvCenterY + clipWindowHeight/2.0;
		wvLeft  =  wvCenterX - clipWindowWidth/2.0;
		wvBottom=  wvCenterY - clipWindowHeight/2.0;
		break;
	case 109:
	case '-':
		clipWindowWidth = wvWidth*1.25;
		clipWindowHeight= wvHeight*1.25;
		wvRight =  wvCenterX + clipWindowWidth/2.0;
		wvTop   =  wvCenterY + clipWindowHeight/2.0;
		wvLeft  =  wvCenterX - clipWindowWidth/2.0;
		wvBottom=  wvCenterY - clipWindowHeight/2.0;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void specialKeyHandler(int key, int x, int y)
{
	double clipWindowWidth;
	double clipWindowHeight;
	double wvCenterX = wvLeft   + (wvRight - wvLeft)/2.0;
	double wvCenterY = wvBottom + (wvTop   - wvBottom)/2.0;
	double wvWidth   = wvRight - wvLeft;
	double wvHeight  = wvTop   - wvBottom;

	switch(key)	{
	case GLUT_KEY_LEFT:
		wvLeft -= 5.0;
		wvRight-= 5.0;
		break;
	case GLUT_KEY_RIGHT:
		wvLeft += 5.0;
		wvRight+= 5.0;
		break;
	case GLUT_KEY_UP:
		wvTop   += 5.0;
		wvBottom+= 5.0;
		break;
	case GLUT_KEY_DOWN:
		wvTop   -= 5.0;
		wvBottom-= 5.0;
		break;
		// Want to zoom in/out and keep  aspect ratio = 2.0
	case GLUT_KEY_F1:
		clipWindowWidth = wvWidth*0.8;
		clipWindowHeight= wvHeight*0.8;
		wvRight =  wvCenterX + clipWindowWidth/2.0;
		wvTop   =  wvCenterY + clipWindowHeight/2.0;
		wvLeft  =  wvCenterX - clipWindowWidth/2.0;
		wvBottom=  wvCenterY - clipWindowHeight/2.0;
		break;
	case GLUT_KEY_F2:
		clipWindowWidth = wvWidth*1.25;
		clipWindowHeight= wvHeight*1.25;
		wvRight =  wvCenterX + clipWindowWidth/2.0;
		wvTop   =  wvCenterY + clipWindowHeight/2.0;
		wvLeft  =  wvCenterX - clipWindowWidth/2.0;
		wvBottom=  wvCenterY - clipWindowHeight/2.0;
		break;
	case GLUT_KEY_F3:
		printf("LeftClick\tClick and drag to rotate\n");
		printf("ScrollWheel\tZoom in and out\n");
		printf("w\t\tToggle Wireframe\n");
		printf("r\t\tReset\n");
		break;
	}
	glutPostRedisplay();
}

void reshape(int w, int h)
{
	glutWindowWidth = (GLsizei) w;
	glutWindowHeight = (GLsizei) h;
	glViewport(0, 0, glutWindowWidth, glutWindowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(wvLeft, wvRight, wvBottom, wvTop);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/************************************************************************************
 *
 *
 * 3D Window and Surface of Revolution Code 
 *
 * Fill in the code in the empty functions
 * Feel free to use your own functions or rename mine. Mine are just a guide.
 * Add whatever variables you think are necessary
 ************************************************************************************/

 //
 // Surface of Revolution consists of vertices and quads
 //
 // Set up lighting/shading and material properties for surface of revolution
GLfloat quadMat_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat quadMat_specular[] = { 0.45, 0.55, 0.45, 1.0 };
GLfloat quadMat_diffuse[] = { 0.1, 0.35, 0.1, 1.0 };
GLfloat quadMat_shininess[] = { 10.0 };


// You might use something like this for your surface of revolution
// Feel free to add to it or use your own data structures
typedef struct Vertex
{
	GLdouble x, y, z;
	Vector3D normal; // vertex normal vector
} Vertex;

// Suggested quad structure
// Feel free to use your own or use C++
// Each quad shares vertices with other neighbor quads
// So build a quad array where each quad has 4 indices into the vertex array or 4 pointers
// into the vertex array - your choice
typedef struct Quad
{
	Vertex *vertex[4]; // 4 indexes into vertex array e.g. 23, 45, 86, 71 means it uses vertices 23 in vertex array etc
	// Vertex *vertex[4]; // 4 pointers to vertices in the vertex array
	Vector3D normal; // quad normal vector for this quad
} Quad;

// Use this form of pointer to the vertex array and quad array
// This means you must allocate memory for your vertex array and your quad array.
// Take a look at how I did this in the quadmesh code I gave you in assignment 1
Vertex **vertexArray;
Quad   *quadArray;

// Alternatively, use fixed size arrays like this:
// If you use fixed size arrays then this means you have to always use the same number of control points and
// never change it
//Vertex vertexArray[NUMROWS][NUMCOLS];// you need to define NUMROWS and NUMCOLS based on how many curvepoints in the 
                                       // profile curve and how many times you rotate the curvepoints
//Quad quadArray[NUMROWS][NUMCOLS]; 

void init3DSurfaceWindow()
{
	// uncomment once everything is working
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_AMBIENT, model_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, model_ambient);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);  // Color and depth for glClear

	glViewport(0, 0, (GLsizei)window3DSizeX, (GLsizei)window3DSizeY);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, aspect, zNear, zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void reshape3D(int w, int h)
{
	glutWindowWidth = (GLsizei) w;
	glutWindowHeight = (GLsizei) h;
	glViewport(0, 0, glutWindowWidth, glutWindowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov,aspect,zNear,zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

}

void display3D()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glLoadIdentity();
	gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
		
	drawGround();

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, quadMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, quadMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, quadMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, quadMat_shininess);
	
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	glPushMatrix();

	if (wire) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	buildVertexArray();
	buildQuadArray();
	computeQuadNormals();
	computeVertexNormals();

	glScalef(size, size, size);
	glRotatef(angleX, 1, 0, 0);
	glRotatef(angleY, 0, 0, 1);

	glBegin(GL_QUADS);
	for (int i = 0; i < (subcurve.numCurvePoints - 1) * rows; i++) {
		for (int j = 3; j >= 0; j--) {
			Vertex vertex = *quadArray[i].vertex[j];
			glNormal3f(vertex.normal.x, vertex.normal.y, vertex.normal.z);
			glVertex3f(vertex.x, vertex.y, vertex.z);
		}
	}
	glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glPopMatrix();
	
	glutSwapBuffers();
}

void drawGround() {
	glPushMatrix();
	glScalef(size, size, size);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   groundMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  groundMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   groundMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, groundMat_shininess);
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glVertex3f(-22.0f, -4.0f, -22.0f);
	glVertex3f(-22.0f, -4.0f, 22.0f);
	glVertex3f(22.0f, -4.0f, 22.0f);
	glVertex3f(22.0f, -4.0f, -22.0f);
	glEnd();
	glPopMatrix();
}

void mouseButtonHandler3D(int button, int state, int x, int y)
{
	currentButton = button;
	lastMouseX = x;
	lastMouseY = y;
	
	switch(button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{

		}
			
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{

		}
			
		break;
	case GLUT_MIDDLE_BUTTON:
		if (state == GLUT_DOWN)
		{
			// add code here
			
		}
		
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void mouseScrollWheelHandler3D(int button, int dir, int xMouse, int yMouse){
	
	if (dir > 0){
		size += 0.1;
		glutPostRedisplay();
	} else {
		if (size >= 0.1) size -= 0.1;
		else size = 0.1;
		glutPostRedisplay();
	}
}

void mouseMotionHandler3D(int x, int y)
{
	int dx = x - lastMouseX;
	int dy = y - lastMouseY;
	
	if (currentButton == GLUT_LEFT_BUTTON) {
		angleX += dy;
		angleY += dx;
	}
	if (currentButton == GLUT_RIGHT_BUTTON) {
		
	}
	else if (currentButton == GLUT_MIDDLE_BUTTON) {

	}
	
	lastMouseX = x;
	lastMouseY = y;
	
	glutPostRedisplay();
}

void keyboardHandler3D(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'r':
		size = 1.0;
		angleX = 0;
		angleY = 0;
		glutPostRedisplay();
		break;
	case 'w':
		if (!wire) wire = true;
		else wire = false;
		glutPostRedisplay();
	}

	glutPostRedisplay();
}

void specialKeyHandler3D(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_F3:
		printf("LeftClick\tClick and drag to rotate\n");
		printf("ScrollWheel\tZoom in and out\n");
		printf("w\t\tToggle Wireframe\n");
		printf("r\t\tReset\n");
		break;
	}
}

void buildVertexArray()
{
	// build a new vertex array based on all the profile curves created so far by rotation
	vertexArray = (Vertex**)malloc(rows * sizeof(Vertex*));
	
	for (int i = 0; i < rows; i++) {
		vertexArray[i] = (Vertex*)malloc(subcurve.numCurvePoints * sizeof(Vertex));
	}

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < subcurve.numCurvePoints; j++) {
			float x = subcurve.curvePoints[j].x;
			float y = subcurve.curvePoints[j].y;
			Vector3D temp = rotateAroundY(x, 0, i * 10);
			vertexArray[i][j].x = temp.x;
			vertexArray[i][j].y = y;
			vertexArray[i][j].z = temp.z;
			vertexArray[i][j].normal.x = 0;
			vertexArray[i][j].normal.y = 0;
			vertexArray[i][j].normal.z = 0;
		}
	}
}

Vector3D rotateAroundY(double x, double z, double theta)
{
	Vector3D v;
	v.x = v.y = v.z = 0;
	// takes the x,z values of a point and rotates in around the y axis by theta degrees
	// use the cos/sin formula from the lecture slides where we derived 2D rotation
	v.x = (x * cos((theta / 180) * M_PI)) - (z * sin((theta / 180) * M_PI));
	v.z = (x * sin((theta / 180) * M_PI)) + (z * cos((theta / 180) * M_PI));
	return v;
}

void buildQuadArray()
{
	// for all profile curves (obtained by copying the original curve from the 2D window 
	// and rotating the curve points around the y axis by, for example, 5, 10, 15, 20, 30 degrees ... etc
	// construct quads based on the current curve and the previous curve 
	// don't forget the special case at the end where you construct quads using the last curve and the very
	// first curve
	quadArray = (Quad*)malloc((rows * (subcurve.numCurvePoints - 1)) * sizeof(Quad));
	int count = 0;
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < subcurve.numCurvePoints - 1; j++) {
			if (i == rows - 1) {
				quadArray[count].vertex[0] = &vertexArray[i][j];
				quadArray[count].vertex[1] = &vertexArray[i][j + 1];
				quadArray[count].vertex[2] = &vertexArray[0][j + 1];
				quadArray[count].vertex[3] = &vertexArray[0][j];
			}
			else {
				quadArray[count].vertex[0] = &vertexArray[i][j];
				quadArray[count].vertex[1] = &vertexArray[i][j + 1];
				quadArray[count].vertex[2] = &vertexArray[i + 1][j + 1];
				quadArray[count].vertex[3] = &vertexArray[i + 1][j];
			}
			count++;
		}
	}
}

void computeQuadNormals()
{
	// compute one normal per quad polygon and store in the quad structure
	// Use Newell's Method - see http://www.dillonbhuff.com/?p=284
	double normalX, normalY, normalZ;
	int i, j, k;
	for (i = 0; i < (rows * (subcurve.numCurvePoints - 1)); i++) {
		normalX = normalY = normalZ = 0;
		for (j = 0, k = 1; j < 4; j++, k++) {
			if (k == 4) k = 0;
			Vertex pi = *quadArray[i].vertex[j];
			Vertex pj = *quadArray[i].vertex[k];

			normalX += (((pi.z) + (pj.z)) * ((pj.y) - (pi.y)));
			normalY += (((pi.x) + (pj.x)) * ((pj.z) - (pi.z)));
			normalZ += (((pi.y) + (pj.y)) * ((pj.x) - (pi.x)));
		}

		Vector3D temp;
		temp.x = normalX;
		temp.y = normalY;
		temp.z = normalZ;

		quadArray[i].normal = temp;
		//printf("%f %f %f\n", temp.x, temp.y, temp.z);
	}
}

void computeVertexNormals()
{
	// compute a normal for each vertex of the surface by averaging the 4 normals of 
	// the 4 quads that share the vertex
	// be careful at the bottom and top of the surface where only 2 quads share a vertex
	// Hint on one way to do this: after computeQuadNormals(),  go through all quads and *add* the quad normal to each 
	// of its 4 vertices. Once this is done, go through all vertices and normalize the normal vector
	
	Vector3D temp;
	for (int i = 0; i < rows * (subcurve.numCurvePoints - 1); i++) {
		temp = quadArray[i].normal;
		for (int j = 0; j < 4; j++) {
			quadArray[i].vertex[j] -> normal.x += temp.x;
			quadArray[i].vertex[j] -> normal.y += temp.y;
			quadArray[i].vertex[j] -> normal.z += temp.z;
		}
	}
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < subcurve.numCurvePoints; j++) {
			vertexArray[i][j].normal = normalize(vertexArray[i][j].normal);
		}
	}
}

Vector3D crossProduct(Vector3D a, Vector3D b){
	Vector3D cross;
	
	cross.x = a.y * b.z - b.y * a.z;
	cross.y = a.x * b.z - b.x * a.z;
	cross.z = a.x * b.y - b.x * a.y;
	
	return cross;
}

Vector3D fourVectorAverage(Vector3D a, Vector3D b, Vector3D c, Vector3D d){
	Vector3D average;
	average.x = (a.x + b.x + c.x + d.x)/4.0;
	average.y = (a.y + b.y + c.y + d.y)/4.0;
	average.z = (a.z + b.z + c.z + d.z)/4.0;
	return average;
}

Vector3D normalize(Vector3D a){
	GLdouble norm = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
	Vector3D normalized;
	normalized.x = a.x/norm;
	normalized.y = a.y/norm;
	normalized.z = a.z/norm;
	return normalized;
}







