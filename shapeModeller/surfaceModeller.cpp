#include <stdio.h>
#include <windows.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include <string.h>
#include "surfaceModeller.h"
#include "subdivcurve.h"


boolean indicesArrayAllocated = false;
boolean quadArrayAllocated = false;
boolean varrayAllocated = false;

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

GLint glutWindowWidth = 800;
GLint glutWindowHeight = 600;
GLint viewportWidth = glutWindowWidth;
GLint viewportHeight = glutWindowHeight;

// screen window identifiers
int window2D, window3D;

int window3DSizeX = 800, window3DSizeY = 600;
GLdouble aspect = (GLdouble)window3DSizeX / window3DSizeY;

float myModelView[16];

void myLookAt(float *viewMatrix,
	float eyeX, float eyeY, float eyeZ,
	float centerX, float centerY, float centerZ,
	float upX, float upY, float upZ);


int main(int argc, char* argv[])
{
	glutInit(&argc, (char **)argv); 
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(glutWindowWidth,glutWindowHeight);
	glutInitWindowPosition(50,100);  
	
	// The 2D Window
	window2D = glutCreateWindow("Profile Curve"); 
	glutDisplayFunc(display2D);
	glutReshapeFunc(reshape2D);
	// Initialize the 2D profile curve system
	init2DCurveWindow(); 
	// A few input handlers
	glutMouseFunc(mouseButtonHandler2D);
	glutMotionFunc(mouseMotionHandler2D);
	glutPassiveMotionFunc(mouseHoverHandler2D);
	glutSpecialFunc(specialKeyHandler2D);
	glutKeyboardFunc(keyboardHandler2D);
	
	// The 3D Window
	window3D = glutCreateWindow("Surface of Revolution"); 
	glutPositionWindow(900,100);  
	glutDisplayFunc(display3D);
	glutReshapeFunc(reshape3D);
	glutMouseFunc(mouseButtonHandler3D);
	glutMotionFunc(mouseMotionHandler3D);
	glutKeyboardFunc(keyboardHandler3D);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "Glew error: %s\n", glewGetErrorString(err));
	}

	// Initialize the 3D system
	init3DSurfaceWindow();

	// Annnd... ACTION!!
	glutMainLoop(); 

	return 0;
}

/************************************************************************************
 *
 *
 * 2D Window and Profile Curve Code
 *
 * Fill in the code in the empty functions
 ************************************************************************************/

// The profile curve is a subdivision curve
SubdivisionCurve subcurve;

int hoveredCtlPt = -1;
int currentCurvePoint = 0;

// Use little circles to draw subdivision curve control points
Circle circles[MAXCONTROLPOINTS];
int numCirclePoints = 30;
double circleRadius = 0.2;


void init2DCurveWindow() 
{ 
	glLineWidth(3.0);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);
	initSubdivisionCurve();
	initControlPointCircles();
} 

void display2D()
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

void drawSubdivisionCurve() 
{
	// Subdivide the given curve
	computeSubdivisionCurve(&subcurve);
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

void drawControlPoints()
{
	int i, j;
	for (i=0; i<subcurve.numControlPoints; i++){
		glPushMatrix();
		glColor3f(1.0f,0.0f,0.0f); 
		glTranslatef(circles[i].circleCenter.x, circles[i].circleCenter.y, 0);
		// for the hoveredCtlPt, draw an outline and change its color
		if (i == hoveredCtlPt)
		{ 
			// outline
			glColor3f(0.0, 1.0, 0.0);
			glBegin(GL_LINE_LOOP); 
			for(j=0; j < numCirclePoints; j++) {
				glVertex3f(circles[i].circlePoints[j].x, circles[i].circlePoints[j].y, 0); 
			}
			glEnd();
			// color change
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

void initSubdivisionCurve() 
{
	// Initialize 5 control points of the subdivision curve

	GLdouble x, y;

	x = 2 * cos(M_PI*0.5);
	y = 2 * sin(M_PI*0.5);
	subcurve.controlPoints[0].x = x;
	subcurve.controlPoints[0].y = y;

	x = 2 * cos(M_PI*0.25);
	y = 2 * sin(M_PI*0.25);
	subcurve.controlPoints[1].x = x;
	subcurve.controlPoints[1].y = y;
	
	x = 2 * cos(M_PI*0.0);
	y = 2 * sin(M_PI*0.0);
	subcurve.controlPoints[2].x = x;
	subcurve.controlPoints[2].y = y;
	
	subcurve.numControlPoints = 3;
	subcurve.subdivisionSteps = 3;
}



void initControlPointCircles()
{
	int num = subcurve.numControlPoints;
	for (int i=0; i < num; i++){
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

void worldToCameraCoordiantes(GLdouble xWorld, GLdouble yWorld, GLdouble *xcam, GLdouble *ycam)
{
	double wvCenterX = wvLeft   + (wvRight - wvLeft)/2.0;
	double wvCenterY = wvBottom + (wvTop   - wvBottom)/2.0;
	*xcam = worldCenterX - wvCenterX + xWorld;
	*ycam = worldCenterY - wvCenterY + yWorld;
}

int currentButton;

void mouseButtonHandler2D(int button, int state, int xMouse, int yMouse)
{
	int i;
	
	currentButton = button;
	if (button == GLUT_LEFT_BUTTON)
	{  
		switch (state) {      
		case GLUT_DOWN:
			if (hoveredCtlPt > -1)
			{
			  screenToWorldCoordinates(xMouse, yMouse, &circles[hoveredCtlPt].circleCenter.x, &circles[hoveredCtlPt].circleCenter.y);
			  screenToWorldCoordinates(xMouse, yMouse, &subcurve.controlPoints[hoveredCtlPt].x, &subcurve.controlPoints[hoveredCtlPt].y);
			}
			break;
		case GLUT_UP:
			glutSetWindow(window3D);
			glutPostRedisplay();
			break;
		}
	}    
	glutSetWindow(window2D);
	glutPostRedisplay();
}

void mouseMotionHandler2D(int xMouse, int yMouse)
{
	if (currentButton == GLUT_LEFT_BUTTON) {  
		if (hoveredCtlPt > -1) 
		{
			screenToWorldCoordinates(xMouse, yMouse, &circles[hoveredCtlPt].circleCenter.x, &circles[hoveredCtlPt].circleCenter.y);
			screenToWorldCoordinates(xMouse, yMouse, &subcurve.controlPoints[hoveredCtlPt].x, &subcurve.controlPoints[hoveredCtlPt].y);
		}
	}    
	glutPostRedisplay();
	glutSetWindow(window3D);
	glutPostRedisplay();
	glutSetWindow(window2D);
}

void mouseHoverHandler2D(int xMouse, int yMouse)
{
	hoveredCtlPt = -1;
	GLdouble worldMouseX, worldMouseY;
	screenToWorldCoordinates(xMouse, yMouse, &worldMouseX, &worldMouseY);
	// See if we're hovering over a circle
	for (int i=0; i<subcurve.numControlPoints; i++){
		GLdouble distToX = worldMouseX - circles[i].circleCenter.x;
		GLdouble distToY = worldMouseY - circles[i].circleCenter.y;
		GLdouble euclideanDist = sqrt(distToX*distToX + distToY*distToY);
		if (euclideanDist < 0.5)
		  hoveredCtlPt = i;
	}
	glutPostRedisplay();
}


void keyboardHandler2D(unsigned char key, int x, int y)
{
	int i;
	
	double clipWindowWidth;
	double clipWindowHeight;
	double wvCenterX = wvLeft   + (wvRight - wvLeft)/2.0;
	double wvCenterY = wvBottom + (wvTop   - wvBottom)/2.0;
	double wvWidth   = wvRight - wvLeft;
	double wvHeight  = wvTop   - wvBottom;

	switch(key)
	{
	case 'q':
	case 'Q':
	case 27:
		// Esc, q, or Q key = Quit 
		exit(0);
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void specialKeyHandler2D(int key, int x, int y)
{
	double clipWindowWidth;
	double clipWindowHeight;
	double wvCenterX = wvLeft   + (wvRight - wvLeft)/2.0;
	double wvCenterY = wvBottom + (wvTop   - wvBottom)/2.0;
	double wvWidth   = wvRight - wvLeft;
	double wvHeight  = wvTop   - wvBottom;

	switch (key)
	{
	case GLUT_KEY_UP:
		// REQUIREMENT 1
		// See function initSubdivisionCurve and drawSubdivisionCurve() to help you with the functionality
		// increase the number of control points by 1
		// if the number of control points is > 4, set to 4
		// recompute the subdivision curve (see function drawSubdivisionCurve() to see how it calls 
		// computeSubdivisionCurve() )
		// reinitialize the control point circles (see function initSubdivisionCurve())
		// set boolean variables indicesArrayAllocated, quadArrayAllocated, varrayAllocated  to false
		if (subcurve.numControlPoints < 4) {
			GLdouble x, y;
			subcurve.numControlPoints++;
			computeSubdivisionCurve(&subcurve);
			for (int i = 0; i < subcurve.numControlPoints; i++) {
				x = 2 * cos(M_PI * i * 0.25);
				y = 2 * sin(M_PI * i * 0.25);
				subcurve.controlPoints[subcurve.numControlPoints - i - 1].x = x;
				subcurve.controlPoints[subcurve.numControlPoints - i - 1].y = y;
			}
			initControlPointCircles();
			indicesArrayAllocated = false;
			quadArrayAllocated = false;
			varrayAllocated = false;
		}
		break;
	case GLUT_KEY_DOWN:
		// REQUIREMENT 1
		// decrease the number of control points by 1
		// if the number of control points is < 2, set to 2
		// recompute the subdivision curve (see function)
		// reinitialize the control point circles (see function)
		// set boolean variables indicesArrayAllocated, quadArrayAllocated, varrayAllocated  to false
		
		if (subcurve.numControlPoints > 2) {
			GLdouble x, y;
			subcurve.numControlPoints--;
			computeSubdivisionCurve(&subcurve);
			for (int i = 0; i < subcurve.numControlPoints; i++) {
				x = 2 * cos(M_PI * i * 0.25);
				y = 2 * sin(M_PI * i * 0.25);
				subcurve.controlPoints[subcurve.numControlPoints - i - 1].x = x;
				subcurve.controlPoints[subcurve.numControlPoints - i - 1].y = y;
			}
			initControlPointCircles();
			indicesArrayAllocated = false;
			quadArrayAllocated = false;
			varrayAllocated = false;
		}
			break;
	}
	glutPostRedisplay();
}


void reshape2D(int w, int h)
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
 ************************************************************************************/

void BuildTriangleIndexArray();
void drawTris();
void drawTrisAsLines();
void drawMeshVBO();
void writeOBJ();
void readOBJ();

 // Ground Mesh material
GLfloat groundMat_ambient[] = { 0.4, 0.4, 0.4, 1.0 };
GLfloat groundMat_specular[] = { 0.01, 0.01, 0.01, 1.0 };
GLfloat groundMat_diffuse[] = { 0.4, 0.4, 0.7, 1.0 };
GLfloat groundMat_shininess[] = { 1.0 };

GLfloat light_position0[] = { 4.0, 8.0, 8.0, 1.0 };
GLfloat light_diffuse0[] = { 1.0, 1.0, 1.0, 1.0 };

GLfloat light_position1[] = { -4.0, 8.0, 8.0, 1.0 };
GLfloat light_diffuse1[] = { 1.0, 1.0, 1.0, 1.0 };

GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat model_ambient[] = { 0.5, 0.5, 0.5, 1.0 };

// 
GLdouble spin = 0.0;
GLdouble spinX = 0.0;
GLdouble spinY = 0.0;
GLdouble rotation = 0.0;
GLdouble scale = 1.0;


//
// Surface of Revolution consists of vertices and quads
//
// Set up lighting/shading and material properties for surface of revolution
GLfloat quadMat_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat quadMat_specular[] = { 0.45, 0.55, 0.45, 1.0 };
GLfloat quadMat_diffuse[] = { 0.1, 0.35, 0.1, 1.0 };
GLfloat quadMat_shininess[] = { 10.0 };


// Quads and Vertices of the surface of revolution
typedef struct Vertex
{
	GLfloat x, y, z;
	Vector3D normal; // normal vector for this vertex
	int numQuads;
	int quadIndex[4]; // index into quad array - tells us which quads share this vertex
} Vertex;

Vertex   *varray;     // array of vertices
Vector3D *positions;  // vertex positions - used for VBO draw
Vector3D *normals;    // normal vectors for each vertex - used for VBO draw

typedef struct Quad
{
	int vertexIndex[4]; // 4 vertex indices (in clockwise order) into vertex array
	Vector3D normal;
} Quad;

// Quads - qarray stores array of Quad structs (see above)
Quad *qarray;

// Index Array of Triangles derived from Quad Array - break each quad into 2 triangles
// Used when drawing with VBO as glDrawElements(GL_QUADS,...) no longer supported in newer versions of OpenGL
GLuint *indices;

// Vertices - varray stores array of Vertex structs (see above)
#define NUMBEROFSIDES 16

GLdouble safetyMargin = 0.001;
GLdouble fov = 60.0;

int lastMouseX;
int lastMouseY;

boolean drawAsLines = false;
boolean drawAsPoints = false;
boolean drawNormals = false;

GLdouble eyeX = 0.0, eyeY = 3.0, eyeZ = 10.0;
GLdouble radius = eyeZ;
GLdouble zNear = 0.1, zFar = 40.0;

GLdouble globalZ = 1.0, globalZnear = 0.1, globalZfar = 1.5;

unsigned int numTris = 0;
unsigned int numVertices = 0;
unsigned int numIndices = 0;


void init3DSurfaceWindow()
{
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

	myLookAt(myModelView, 0.0, 3.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	// REPLACE call to gluLookAt with myLookAt(0.0, 3.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
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
	myLookAt(myModelView, 0.0, 3.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	// REPLACE call to gluLookAt with myLookAt(0.0, 3.0, 10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void display3D()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// Set up the Viewing Transformation (V matrix)	
	myLookAt(myModelView, eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	// REPLACE call to gluLookAt with myLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	drawGround();

    // Build and Draw Surface of Revolution (Quad Mesh)
	buildVertexArray();
	buildQuadArray();
	BuildTriangleIndexArray();
	computeQuadNormals();
	computeVertexNormals();
	
	// Draw quad mesh
	glPushMatrix();
	
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, quadMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, quadMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, quadMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, quadMat_shininess);

	if (drawAsLines)
		drawQuadsAsLines();
	else if (drawAsPoints)
		drawQuadsAsPoints();
	else
	{
		// Write drawMeshVBO() and replace call to drawQuads()
		drawMeshVBO(); // REQUIREMENT 5
		// drawTris();
		//drawQuads();
	}

	glPopMatrix();
	glutSwapBuffers();
}

void drawGround() 
{
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, groundMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, groundMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, groundMat_diffuse);
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



unsigned int VAO;
unsigned int VBOv, VBOn, VBOi;


// REQUIREMENT 5:
// Write code to use VBOs (or a VAO and VBOs) to draw the mesh with glDrawElements().
void drawMeshVBO()
{

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBOv);
	glGenBuffers(1, &VBOn);
	glGenBuffers(1, &VBOi);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBOv);
	glBufferData(GL_ARRAY_BUFFER, NUMBEROFSIDES  * sizeof(Vector3D) * subcurve.numCurvePoints, positions, GL_STATIC_DRAW);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, VBOn);
	glBufferData(GL_ARRAY_BUFFER, NUMBEROFSIDES * sizeof(Vector3D) * subcurve.numCurvePoints, normals, GL_STATIC_DRAW);
	glNormalPointer(GL_FLOAT, 3 * sizeof(float), NULL);
	glEnableClientState(GL_NORMAL_ARRAY);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOi);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * NUMBEROFSIDES * sizeof(GLuint) * (subcurve.numCurvePoints - 1), indices, GL_STATIC_DRAW);
	
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6 * NUMBEROFSIDES * (subcurve.numCurvePoints - 1), GL_UNSIGNED_INT, NULL);

	glDeleteBuffers(1, &VAO);
	glDeleteBuffers(1, &VBOv);
	glDeleteBuffers(1, &VBOn);
	glDeleteBuffers(1, &VAO);
}


// Use immediate mode rendering to draw the mesh as triangles rather than as quads
void drawTris()
{
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, quadMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, quadMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, quadMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, quadMat_shininess);

	glPushMatrix();
	for (int i = 0; i < (subcurve.numCurvePoints - 1)*NUMBEROFSIDES * 2 * 3; i += 3)
	{
		glBegin(GL_TRIANGLES);
		Vector3D *vertexp = &positions[indices[i]];
		Vector3D *vertexn = &normals[indices[i]];
		glNormal3f(vertexn->x, vertexn->y, vertexn->z);
		glVertex3f(vertexp->x, vertexp->y, vertexp->z);
		vertexp = &positions[indices[i + 1]];
		vertexn = &normals[indices[i + 1]];
		glNormal3f(vertexn->x, vertexn->y, vertexn->z);
		glVertex3f(vertexp->x, vertexp->y, vertexp->z);
		vertexp = &positions[indices[i + 2]];
		vertexn = &normals[indices[i + 2]];
		glNormal3f(vertexn->x, vertexn->y, vertexn->z);
		glVertex3f(vertexp->x, vertexp->y, vertexp->z);
		glEnd();

	}

	glPopMatrix();
}

void drawTrisAsLines()
{
	glDisable(GL_LIGHTING);
	glPushMatrix();
	printf("Number of tris = %u\n", (subcurve.numCurvePoints - 1)*NUMBEROFSIDES * 2);

	for (int i = 0; i < (subcurve.numCurvePoints - 1)*NUMBEROFSIDES * 2 * 3; i += 3)
	{
		glColor3f(0, 0.8, 0);
		glBegin(GL_LINE_LOOP);

		Vertex *vertex = &varray[indices[i]];
		glVertex3f(vertex->x, vertex->y, vertex->z);
		vertex = &varray[indices[i + 1]];
		glVertex3f(vertex->x, vertex->y, vertex->z);
		vertex = &varray[indices[i + 2]];
		glVertex3f(vertex->x, vertex->y, vertex->z);
		glEnd();
	}
	glPopMatrix();
	glEnable(GL_LIGHTING);
}

// Use immediate mode rendering to draw the mesh as quads
void drawQuads()
{
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, quadMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, quadMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, quadMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, quadMat_shininess);

	glPushMatrix();
	for (int row = 0; row < subcurve.numCurvePoints - 1; row++)
	{
		for (int col = 0; col < NUMBEROFSIDES; col++)
		{
			glBegin(GL_QUADS);
			for (int i = 0; i < 4; i++)
			{
				Vertex *vertex = &varray[qarray[row*NUMBEROFSIDES + col].vertexIndex[i]];
				glNormal3f(vertex->normal.x, vertex->normal.y, vertex->normal.z);
				glVertex3f(vertex->x, vertex->y, vertex->z);
			}
			glEnd();
		}
	}

	glPopMatrix();
}

void drawQuadsAsPoints()
{
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, quadMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, quadMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, quadMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, quadMat_shininess);

	glPushMatrix();
	glPointSize(3);
	glBegin(GL_POINTS);
	for (int row = 0; row < subcurve.numCurvePoints; row++)
	{
		for (int col = 0; col < NUMBEROFSIDES; col++)
		{
			glVertex3f(varray[row*NUMBEROFSIDES + col].x, varray[row*NUMBEROFSIDES + col].y, varray[row*NUMBEROFSIDES + col].z);
		}
	}
	glEnd();
	glPopMatrix();
}

void drawQuadsAsLines()
{
	glDisable(GL_LIGHTING);
	glPushMatrix();

	for (int row = 0; row < subcurve.numCurvePoints - 1; row++)
	{
		for (int col = 0; col < NUMBEROFSIDES; col++)
		{
			glColor3f(0, 0.8, 0);
			glBegin(GL_LINE_LOOP);
			Vertex *vertex = &varray[qarray[row*NUMBEROFSIDES + col].vertexIndex[0]];
			glVertex3f(vertex->x, vertex->y, vertex->z);

			vertex = &varray[qarray[row*NUMBEROFSIDES + col].vertexIndex[1]];
			glVertex3f(vertex->x, vertex->y, vertex->z);

			vertex = &varray[qarray[row*NUMBEROFSIDES + col].vertexIndex[2]];
			glVertex3f(vertex->x, vertex->y, vertex->z);

			vertex = &varray[qarray[row*NUMBEROFSIDES + col].vertexIndex[3]];
			glVertex3f(vertex->x, vertex->y, vertex->z);
			glEnd();

			// draw Normals)
			glColor3f(0, 0, 0.8);
			glBegin(GL_LINES);
			vertex = &varray[qarray[row*NUMBEROFSIDES + col].vertexIndex[0]];
			Vector3D qn = qarray[row*NUMBEROFSIDES + col].normal;
			glVertex3f(vertex->x, vertex->y, vertex->z);
			glVertex3f(vertex->x + vertex->normal.x, vertex->y + vertex->normal.y, vertex->z + vertex->normal.z);
			glEnd();
		}
	}
	glPopMatrix();
	glEnable(GL_LIGHTING);
}



Vector3D normalize(Vector3D a)
{
	GLdouble norm = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
	Vector3D normalized;
	normalized.x = a.x / norm;
	normalized.y = a.y / norm;
	normalized.z = a.z / norm;
	return normalized;
}

void mouseButtonHandler3D(int button, int state, int x, int y)
{
	currentButton = button;
	lastMouseX = x;
	lastMouseY = y;
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{


		}
		break;
	default:
		break;
	}
	glutPostRedisplay();
}



// Fill in the code appropriately
void mouseMotionHandler3D(int x, int y)
{
	int dx = x - lastMouseX;
	int dy = y - lastMouseY;

	// REQUIREMENT 3 code here
	if (currentButton == GLUT_LEFT_BUTTON)
	{	
		if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
		{
			double rotate = 0.01 * dy;

			// Calculate the new eye position
			double radius = sqrt(eyeX * eyeX + eyeY * eyeY + eyeZ * eyeZ);
			double elevation = (asin(eyeY / radius) + rotate);
			double azimuth = atan2(eyeZ, eyeX);

			if (elevation > 0 && elevation < 80 * M_PI/180)
			{
				eyeY = radius * sin(elevation);
				eyeX = radius * cos(azimuth) * cos(elevation);
				eyeZ = radius * cos(elevation) * sin(azimuth);
			}
		}
		else
		{
			eyeX = eyeX * cos(0.01 * dx) - eyeZ * sin(0.01 * dx);
			eyeZ = eyeX * sin(0.01 * dx) + eyeZ * cos(0.01 * dx);
		}
	}
	// REQUIREMENT 2 code here
	if (currentButton == GLUT_RIGHT_BUTTON)
	{	
		globalZ += 0.01 * dy;
		if (globalZ < globalZnear)
			globalZ = globalZnear;
		if (globalZ > globalZfar)
			globalZ = globalZfar;
		if (glutGetModifiers() == GLUT_ACTIVE_SHIFT)
		{
			
		}
		else
		{

		}
	}
	else if (currentButton == GLUT_MIDDLE_BUTTON)
	{
		eyeZ += 0.01 * dy;

		if (eyeZ < zNear)
			eyeZ = zNear;
		if (eyeZ > zFar - 3)
			eyeZ = zFar - 3;
	}
	lastMouseX = x;
	lastMouseY = y;
	glutPostRedisplay();
}

void keyboardHandler3D(unsigned char key, int x, int y)
{

	switch (key)
	{
	case 'q':
	case 'Q':
	case 27:
		// Esc, q, or Q key = Quit 
		exit(0);
		break;
	case 'l':
		if (drawAsLines)
			drawAsLines = false;
		else
			drawAsLines = true;
		break;
	case 'w':
		writeOBJ();
		break;
	default:
		break;
	}
	glutPostRedisplay();
}




/**************************************************************************************
 *
 * These functions build the vertices, normals, quads and triangles for the 3D mesh
 *
 * Creates a positions array, normals array and indices array for use with VBO rendering
 *
 **************************************************************************************/
void buildVertexArray()
{
	int row, col;
	double newX, newZ;
	GLdouble cumulativeTheta = 0;
	GLdouble theta = 360/(GLdouble)NUMBEROFSIDES; // = 18

	if (!varrayAllocated)
	{
		varray = (Vertex *)malloc(subcurve.numCurvePoints* NUMBEROFSIDES * sizeof(Vertex));
		positions = (Vector3D *)malloc(subcurve.numCurvePoints* NUMBEROFSIDES * sizeof(Vector3D));
		normals = (Vector3D *)malloc(subcurve.numCurvePoints* NUMBEROFSIDES * sizeof(Vector3D));
		varrayAllocated = true;
	}
	for (row = 0; row < subcurve.numCurvePoints; row++) 
	{
		for (col = 0; col < NUMBEROFSIDES; col++) 
		{

			Vector3D newVector = rotateAroundY(subcurve.curvePoints[row].x, 0, cumulativeTheta);
			// REQUIREMENT 2 code here
			varray[row*NUMBEROFSIDES + col].numQuads = 0;
			varray[row*NUMBEROFSIDES + col].y = subcurve.curvePoints[row].y;
			varray[row*NUMBEROFSIDES + col].x = newVector.x;
			varray[row*NUMBEROFSIDES + col].z = newVector.z * globalZ;	
			positions[row*NUMBEROFSIDES + col].y = subcurve.curvePoints[row].y;
			positions[row*NUMBEROFSIDES + col].x = newVector.x;
			positions[row*NUMBEROFSIDES + col].z = newVector.z * globalZ;

			cumulativeTheta += theta;
		}
	}
}

Vector3D rotateAroundY(double x, double z, double theta)
{	
	Vector3D newVector;
	
	newVector.x =  cos(theta/180*M_PI) * x + sin(theta/180.0*M_PI) * z;
	newVector.z = -sin(theta/180*M_PI) * x + cos(theta/180.0*M_PI) * z;
	return newVector;
}

// Builds the quad index array.
void buildQuadArray()
{
	int col, row, numQuads;
	
	if (!quadArrayAllocated)
	{
		qarray = (Quad *)malloc(sizeof(Quad)*(subcurve.numCurvePoints-1)*NUMBEROFSIDES);
	}
	for (row = 0; row < subcurve.numCurvePoints-1; row++)
	{
		for (col = 0; col < NUMBEROFSIDES; col++)
		{
			int nextCol;

			// if the we are at last column, then the next column must wrap around back to 0th column
			if (col == NUMBEROFSIDES - 1)
				nextCol = 0;
			else
				nextCol = col + 1;

			/*  -------------
			    |  |  |  |  |
				-------------
				|  |  |  |  |
				-------------
				|  |  |  |  |
				-------------
				|  |  |  |  |
				-------------
			*/
			qarray[row*NUMBEROFSIDES + col].vertexIndex[0] = row * NUMBEROFSIDES + col;
			numQuads = varray[row*NUMBEROFSIDES + col].numQuads;
			varray[row*NUMBEROFSIDES + col].quadIndex[numQuads] = row * NUMBEROFSIDES + col;
			varray[row*NUMBEROFSIDES + col].numQuads++;

			qarray[row*NUMBEROFSIDES + col].vertexIndex[1] = (row+1) * NUMBEROFSIDES + col;
			numQuads = varray[(row+1)*NUMBEROFSIDES + col].numQuads;
			varray[(row+1)*NUMBEROFSIDES + col].quadIndex[numQuads] = row * NUMBEROFSIDES + col;
			varray[(row+1)*NUMBEROFSIDES + col].numQuads++;

			qarray[row*NUMBEROFSIDES + col].vertexIndex[2] = (row + 1) * NUMBEROFSIDES + nextCol;
			numQuads = varray[(row + 1)*NUMBEROFSIDES + nextCol].numQuads;
			varray[(row + 1)*NUMBEROFSIDES + nextCol].quadIndex[numQuads] = row * NUMBEROFSIDES + col;
			varray[(row + 1)*NUMBEROFSIDES + nextCol].numQuads++;

			qarray[row*NUMBEROFSIDES + col].vertexIndex[3] = row * NUMBEROFSIDES + nextCol;
			numQuads = varray[row*NUMBEROFSIDES + nextCol].numQuads;
			varray[row*NUMBEROFSIDES + nextCol].quadIndex[numQuads] = row * NUMBEROFSIDES + col;
			varray[row*NUMBEROFSIDES + nextCol].numQuads++;
		}
	}
}

void computeQuadNormals() 
{
	int col, row;
	for (row = 0; row < subcurve.numCurvePoints - 1; row++)
	{
		for (col = 0; col < NUMBEROFSIDES; col++)
		{
			double normal_x = 0.0;
			double normal_y = 0.0;
			double normal_z = 0.0;

			int i, j;
			Vertex *vi;
			Vertex *vj;
			Vector3D normal;
			normal.x = normal.y = normal.z = 0;

			for (i = 0, j = 1; i < 4; i++, j++) 
			{
				if (j == 4) j = 0;
				vi = &varray[qarray[row*NUMBEROFSIDES + col].vertexIndex[i]];
				vj = &varray[qarray[row*NUMBEROFSIDES + col].vertexIndex[j]];
				
				normal.x += (((vi->z) + (vj->z)) * ((vj->y) - (vi->y)));
				normal.y += (((vi->x) + (vj->x)) * ((vj->z) - (vi->z)));
				normal.z += (((vi->y) + (vj->y)) * ((vj->x) - (vi->x)));
			}
			normal.x *= -1.0;
			normal.y *= -1.0;
			normal.z *= -1.0;
			Vector3D normalized = normalize(normal);
			qarray[row*NUMBEROFSIDES + col].normal = normalized;
		}
	}
}

void computeVertexNormals() 
{
	int col, row, i;
	Vector3D vn;

	for (row = 0; row < subcurve.numCurvePoints; row++)
	{
		for (col = 0; col < NUMBEROFSIDES; col++)
		{
			int numQuads = varray[row*NUMBEROFSIDES + col].numQuads;
			vn.x = vn.y = vn.z = 0;

			for (int i = 0; i < numQuads; i++)
			{
				int quadIndex = varray[row*NUMBEROFSIDES + col].quadIndex[i];
				vn.x += qarray[quadIndex].normal.x;
				vn.y += qarray[quadIndex].normal.y;
				vn.z += qarray[quadIndex].normal.z;
			}
			varray[row*NUMBEROFSIDES + col].normal = normalize(vn);
			normals[row*NUMBEROFSIDES + col].x = varray[row*NUMBEROFSIDES + col].normal.x;
			normals[row*NUMBEROFSIDES + col].y = varray[row*NUMBEROFSIDES + col].normal.y;
			normals[row*NUMBEROFSIDES + col].z = varray[row*NUMBEROFSIDES + col].normal.z;
		}
	}
}

// Builds the indices array bsed on the quad index array. The indices array is used for VBO rendering
void BuildTriangleIndexArray()
{
	if (!indicesArrayAllocated)
	{
		indices = (GLuint *)malloc(sizeof(GLuint)*(subcurve.numCurvePoints - 1)*NUMBEROFSIDES * 2 * 3);
		indicesArrayAllocated = true;
	}

	int vi = 0;
	// Subdivide each quad into 2 triangles
	for (int row = 0; row < subcurve.numCurvePoints - 1; row++)
	{
		for (int col = 0; col < NUMBEROFSIDES; col++)
		{
			indices[vi++] = qarray[row*NUMBEROFSIDES + col].vertexIndex[0];
			indices[vi++] = qarray[row*NUMBEROFSIDES + col].vertexIndex[1];
			indices[vi++] = qarray[row*NUMBEROFSIDES + col].vertexIndex[2];

			indices[vi++] = qarray[row*NUMBEROFSIDES + col].vertexIndex[2];
			indices[vi++] = qarray[row*NUMBEROFSIDES + col].vertexIndex[3];
			indices[vi++] = qarray[row*NUMBEROFSIDES + col].vertexIndex[0];
			
		}
	}
	printf("Number of quads = %u\n", (subcurve.numCurvePoints - 1)*NUMBEROFSIDES);
	printf("Number of tris = %u\n", (subcurve.numCurvePoints - 1)*NUMBEROFSIDES*2);
	printf("Number of indices = %u\n", vi);

}

void writeOBJ()
{
	FILE *fout;

	if ((fout = fopen("mesh.obj", "w")))
	{

		if (varrayAllocated)
		{
			for (int i = 0; i < subcurve.numCurvePoints* NUMBEROFSIDES; i++)
			{
				fprintf(fout, "v %f %f %f\n", positions[i].x, positions[i].y, positions[i].z);
			}
			for (int i = 0; i < subcurve.numCurvePoints* NUMBEROFSIDES; i++)
			{
				fprintf(fout, "vn %f %f %f\n", normals[i].x, normals[i].y, normals[i].z);
			}

		}
		if (indicesArrayAllocated)
		{
			for (int i = 0; i < (subcurve.numCurvePoints - 1)*NUMBEROFSIDES * 2 * 3; i += 3)
			{
				fprintf(fout, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
					indices[i], indices[i], indices[i],
					indices[i + 1], indices[i + 1], indices[i + 1],
					indices[i + 2], indices[i + 2], indices[i + 2]);
			}

		}
		fclose(fout);
	}
}

void readOBJ()
{
	char buf[1024];
	char key[1024];
	int n;
	FILE *fin;

	int fc = 0; // face count
	int vc = 0; // vertex count
	int nc = 0; // normal count

	if ((fin = fopen("mesh.obj", "r")))
	{
		/* Process each line of the OBJ file, invoking the handler for each. */

		while (fgets(buf, 1024, fin))
			if (sscanf(buf, "%s%n", key, &n) >= 1)
			{
				const char *c = buf + n;

				if (!strcmp(key, "f"))
				{
					sscanf(buf, "%d/%d/%d", &indices[fc], &indices[fc + 1], &indices[fc + 2]);
					fc += 3;
				}
				else if (!strcmp(key, "vn"))
				{
					sscanf(buf, "%f %f %f", &positions[vc].x, &positions[vc].y, &positions[vc].z);
					vc++;
				}
				else if (!strcmp(key, "v"))
				{
					sscanf(buf, "%f %f %f", &normals[vc].x, &normals[vc].y, &normals[vc].z);
					nc++;
				}
			}
		fclose(fin);

		numTris = fc / 3;
		numIndices = fc;
		numVertices = vc;
	}
}



/*******************************************************************************************
 * Utility Functions for Creating 4x4 Matrices
 
 // The following functions are some matrix and vector helpers
 // They work for this code but in general it is recommended
 // to use more advanced matrix libraries.
 // e.g. OpenGL Mathematics (GLM)
 *******************************************************************************************/
 
float vec3Dot(float *a, float *b) {
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void vec3Cross(float *a, float *b, float *res) {
	res[0] = a[1] * b[2] - b[1] * a[2];
	res[1] = a[2] * b[0] - b[2] * a[0];
	res[2] = a[0] * b[1] - b[0] * a[1];
}

void vec3Normalize(float *a) {
	float mag = sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
	a[0] /= mag; a[1] /= mag; a[2] /= mag;
}

void mat4Identity(float *a) {
	for (int i = 0; i < 16; ++i) a[i] = 0.0f;
	for (int i = 0; i < 4; ++i) a[i + i * 4] = 1.0f;
}

void mat4Multiply(float *a, float *b, float *res) {
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			res[j * 4 + i] = 0.0f;
			for (int k = 0; k < 4; ++k) {
				res[j * 4 + i] += a[k * 4 + i] * b[j * 4 + k];
			}
		}
	}
}

void mat4Print(float* a) {
	// opengl uses column major order
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			printf("%f ", a[j * 4 + i]);
		}
		printf("\n");
	}
}

// REQUIREMENT 4:
// Write code to replace gluLookAt() with this function. 
// Use the utility methods above to contruct the camera u, v, n coordinate axes
// Then build the view matrix V appropriately
// use glMultMatrix()/glMultMatrixf() to multiply the current OpengGL Matrix by your view matrix
void myLookAt(float *viewMatrix,
	float eyeX, float eyeY, float eyeZ,
	float centerX, float centerY, float centerZ,
	float upX, float upY, float upZ) 
{
	GLfloat upVector[3] = { upX, upY, upZ };
	GLfloat normalVector[3] = { eyeX - centerX, eyeY - centerY, eyeZ - centerZ };

	GLfloat uVector[3];
	vec3Normalize(upVector);
	vec3Normalize(normalVector);
	vec3Cross(upVector, normalVector, uVector);
	vec3Normalize(uVector);
	vec3Cross(normalVector, uVector, upVector);

	GLfloat matrixT[16];
	mat4Identity(matrixT);
	GLfloat eye[3] = { eyeX,eyeY,eyeZ };
	matrixT[12] = -vec3Dot(uVector, eye); matrixT[13] = -vec3Dot(upVector, eye); matrixT[14] = -vec3Dot(normalVector, eye);

	GLfloat matrixR[16] = { 
		uVector[0], upVector[0], normalVector[0], 0,
		uVector[1], upVector[1], normalVector[1], 0,
		uVector[2], upVector[2], normalVector[2], 0,
		0, 0, 0, 1 };

	GLfloat matrixV[16];
	mat4Multiply(matrixT, matrixR, matrixV);
	
	glMultMatrixf(matrixV);
}

