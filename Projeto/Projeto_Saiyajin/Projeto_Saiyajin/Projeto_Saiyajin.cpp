

#include "stdafx.h"
#include <GL/glut.h>
#include <iostream>
#include <math.h>
#include "vector3.h"
#include<stdarg.h>


// float de luz do glut 
GLfloat light_diffuse_0[] = { 1, 1, 1, 1.0 };
GLfloat light_positio_0[] = { 1, 1, 1, 0 };

// Tamanho da tela
int window_size_x = 800, window_size_y = 480;

// used for the trackball implementation
const double m_ROTSCALE = 90.0;
const double m_ZOOMSCALE = 0.008;
float fit_factor = 1.f;
Vector3 trackBallMapping(int x, int y);    // Utility routine to convert mouse locations to a virtual hemisphere
Vector3 lastPoint;                         // Keep track of the last mouse location
enum MovementType { ROTATE, ZOOM, NONE };  // Keep track of the current mode of interaction (which mouse button)
MovementType Movement;                     //    Left-mouse => ROTATE, Right-mouse => ZOOM
Vector3 mouse2D, mouse3D;

GLint FPS = 60;

GLdouble posX = 0, posY = 0.5, posZ = 0; //unproject clicked
GLdouble cubeX = 0, cubeY = 0.5, cubeZ = 0;//cube position
GLdouble cubeSize = 1.f;//cube size
bool cubePicked = false;
bool platform = false;
// implementation of printf with GLUT
void glPrint(float* c, float x, float y, float z, const char *fmt, ...);

void pickingCheck() {
	//printf( " ValueX:  %g ", posX);
	//printf( " \n ");
	/*printf( " ValueY:  %g ", posY);*/
	//printf( " ValueZ:  %g ", posZ);
	//printf( " CubeX:  %f ", cubeX);
	/*printf( " CubeY:  %f", cubeY);*/
	//printf( " CubeZ:  %f", cubeZ);
	platform = true;
	if (posX >= cubeX - (cubeSize / 2) && posX <= cubeX + (cubeSize / 2 + 0.1f)) {
		if (posY >= cubeY - (cubeSize / 2) && posY <= cubeY + (cubeSize / 2 + 0.1f)) {
			if (posZ >= cubeZ - (cubeSize / 2) && posZ <= cubeZ + (cubeSize / 2 + 0.1f)) {
				cubePicked = !cubePicked;
				platform = false;
				return;
			}
		}
	}
}

// Remodelagem
void reshape(int width, int height) {
	window_size_x = width;
	window_size_y = height;

	// Determine the new aspect ratio
	GLdouble gldAspect = (GLdouble)width / (GLdouble)height;

	// Reset the projection matrix with the new aspect ratio.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40.0, gldAspect, 0.01, 60.0);
	glTranslatef(0.0, 0.0, -2.0);

	// Set the viewport to take up the entire window.
	glViewport(0, 0, width, height);
}


void unProject() {

	GLint viewport[4];                  // Where The Viewport Values Will Be Stored
	glGetIntegerv(GL_VIEWPORT, viewport);           // Retrieves The Viewport Values (X, Y, Width, Height)

	GLdouble modelview[16];                 // Where The 16 Doubles Of The Modelview Matrix Are To Be Stored
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);       // Retrieve The Modelview Matrix

	GLdouble projection[16];                // Where The 16 Doubles Of The Projection Matrix Are To Be Stored
	glGetDoublev(GL_PROJECTION_MATRIX, projection);     // Retrieve The Projection Matrix

	GLfloat winX, winY, winZ;               // Holds Our X, Y and Z Coordinates

	winX = (float)mouse2D.x;                  // Holds The Mouse X Coordinate
	winY = (float)mouse2D.y;                  // Holds The Mouse Y Coordinate

											  //winY = (float)viewport[3] - winY;           // Subtract The Current Mouse Y Coordinate From The Screen Height.
	glReadPixels(winX, winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
	//ROUND 2 DECIMAL
	posX = ((int)(posX * 10 + .5) / 10.0);
	posY = ((int)(posY * 10 + .5) / 10.0);
	posZ = ((int)(posZ * 10 + .5) / 10.0);

	//printf( "ValueX:  %f %f %f", floorf(posX * 100) / 100,floorf(posY * 100) / 100,floorf(floorf(posY * 100) * 100));
	//printf( "ValueY:  %e", posY);
	//printf( "ValueZ:  %e", posZ);
	/*if( (posX>=1 && posX<=1.5)  || (posX>=0.5 && posX<=1)){
	printf( "ValueX:  %e ", posX);
	printf("Cube");
	}*/

}

/*********************************** MOUSE *********************************/

void mouseClick(int button, int state, int x, int y) {
	// Clique do mouse (quando ocorre o click e a liberação do mesmo)
	mouse2D = Vector3(x, window_size_y - y, 0);
	if (state == GLUT_UP)  
	{
		// Turn-off rotations and zoom.
		Movement = NONE;
		glutPostRedisplay();
		return;
	}

	switch (button)
	{
	case (GLUT_LEFT_BUTTON):  // Se for o botão esquerdo

		// Faz a camera girar
		Movement = ROTATE;

		// Mapeia a posição do mouse para uma localização de esfera lógica.
		// Mantê-lo na variável de classe lastPoint.
		lastPoint = trackBallMapping(x, y);
		unProject();
		pickingCheck();

		break;

	case (GLUT_MIDDLE_BUTTON):  //Se for botão do meio

		// Ativar o zoom interativo do usuário.
		// À medida que o usuário move o mouse, a cena vai aumentar ou diminuir
		// dependendo da direção x da viagem.
		Movement = ZOOM;

		// Defina o último ponto, para que os movimentos futuros do mouse possam determinar
		// distância percorrida.
		lastPoint.x = (double)x;
		lastPoint.y = (double)y;

		break;

	case (GLUT_RIGHT_BUTTON): // Se botão direito faz nada
		Movement = NONE;
		break;
	}

	glutPostRedisplay();
}

void mouseMotion(int x, int y) {
	// manipula os movimentos necessários do mouse através do trackball
	Vector3 direction;
	double pixel_diff;
	double rot_angle, zoom_factor;
	Vector3 curPoint;

	switch (Movement)
	{
	case ROTATE: // Se botão esquerdo do mouse está sendo pressionado
	{
		curPoint = trackBallMapping(x, y);  // Mapeia a posição do mouse para uma localização de esfera lógica
		direction = curPoint - lastPoint;
		double velocity = direction.Length();
		if (velocity > 0.0001)
		{
			// Roda sobre o eixo que é perpendicular ao grande círculo que liga os movimentos do mouse
			Vector3 rotAxis;
			rotAxis = lastPoint ^ curPoint;
			rotAxis.Normalize();
			rot_angle = velocity * m_ROTSCALE;

			// We need to apply the rotation as the last transformation.
			//   1. Get the current matrix and save it.
			//   2. Set the matrix to the identity matrix (clear it).
			//   3. Apply the trackball rotation.
			//   4. Pre-multiply it by the saved matrix.
			static GLdouble m[4][4];
			glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *)m);
			glLoadIdentity();
			glRotatef(rot_angle, rotAxis.x, rotAxis.y, rotAxis.z);
			glMultMatrixf((GLfloat *)m);

			//  If we want to see it, we need to force the system to redraw the scene.
			glutPostRedisplay();
		}
		break;
	}
	case ZOOM:  // Right-mouse button is being held down
				//
				// Zoom into or away from the scene based upon how far the mouse moved in the x-direction.
				//   This implementation does this by scaling the eye-space.
				//   This should be the first operation performed by the GL_PROJECTION matrix.
				//   1. Calculate the signed distance
				//       a. movement to the left is negative (zoom out).
				//       b. movement to the right is positive (zoom in).
				//   2. Calculate a scale factor for the scene s = 1 + a*dx
				//   3. Call glScalef to have the scale be the first transformation.
				// 
		pixel_diff = y - lastPoint.y;
		zoom_factor = 1.0 + pixel_diff * m_ZOOMSCALE;
		glScalef(zoom_factor, zoom_factor, zoom_factor);

		// Set the current point, so the lastPoint will be saved properly below.
		curPoint.x = (double)x;  curPoint.y = (double)y;  (double)curPoint.z = 0;

		//  If we want to see it, we need to force the system to redraw the scene.
		glutPostRedisplay();
		break;
	}

	// Save the location of the current point for the next movement. 
	lastPoint = curPoint;	// in spherical coordinates
	mouse2D = Vector3(x, window_size_y - y, 0);	// in window coordinates

}

// draw the coordinate axes
void DrawAxes(double length) {
	glDisable(GL_LIGHTING);
	glPushMatrix();
	glScalef(length, length, length);

	glLineWidth(2.f);
	glBegin(GL_LINES);

	// x red
	glColor3f(1.f, 0.f, 0.f);
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(1.f, 0.f, 0.f);

	// y green
	glColor3f(0.f, 1.f, 0.f);
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(0.f, 1.f, 0.f);

	// z blue
	glColor3f(0.f, 0.f, 1.f);
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(0.f, 0.f, 1.f);

	glEnd();
	glLineWidth(1.f);

	glPopMatrix();
}

// draw information on the screen
void DrawInfo() {
	glDisable(GL_LIGHTING);
	// -- start Ortographic Mode
	glMatrixMode(GL_PROJECTION);					//Select the projection matrix
	glPushMatrix();									//Store the projection matrix
	glLoadIdentity();								//Reset the projection matrix
	glOrtho(0, window_size_x, window_size_y, 0, -1, 1);		//Set up an ortho screen

	glMatrixMode(GL_MODELVIEW);						//Select the modelview matrix
	glPushMatrix();									//Store the projection matrix

	glLoadIdentity();								//Reset the projection matrix

	float c[3] = { 1, 1, 1 };
	float y = 25;

	glPrint(c, 10, y, 0, "PROJETO GOKUBO"); y += 20;

	glPopMatrix();									//Restore the old projection matrix
	glMatrixMode(GL_PROJECTION);					//Select the projection matrix
	glPopMatrix();									//Restore the old projection matrix
	glMatrixMode(GL_MODELVIEW);
	// -- end Ortographic mode
}




void drawPlane() {
	int color = 1;

	for (int i = -7; i <= 7; i += 1) {
		for (int j = -7; j <= 7; j += 1) {
			if (color == 1) {
				glColor3ub(255, 255, 255);
				color = 2;
			}
			else {
				glColor3ub(0, 0, 0);
				color = 1;
			}
			glBegin(GL_QUADS);
			glVertex3d(j, 0, i + 1);
			glVertex3d(j, 0, i);
			glVertex3d(j + 1, 0, i);
			glVertex3d(j + 1, 0, i + 1);
			glEnd();
		}
	}

}
// draw scene
void draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

	DrawAxes(4);

	glEnable(GL_LIGHTING);
	drawPlane();
	glPushMatrix();

	if (cubePicked) {
		glColor3f(1.f, 1.f, 0.f);
	}
	else {
		glColor3f(1.f, 1.f, 1.f);
	}

	if (platform && cubePicked) {
		cubeX = posX;
		cubeY = posY + cubeSize / 2;
		cubeZ = posZ;
	}

	glTranslatef(cubeX, cubeY, cubeZ);
	glutSolidCube(cubeSize);
	glPopMatrix();


	DrawInfo();

	glutSwapBuffers();
}


void initGL(int width, int height) {
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse_0);
	glLightfv(GL_LIGHT0, GL_POSITION, light_positio_0);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	reshape(width, height);

	glClearColor(0.126f, 0.126f, 0.128f, 1.0f); // mudar a cor
	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glShadeModel(GL_FLAT);
	glEnable(GL_NORMALIZE);

}

// Animação tosca e desnecessaria
void animation(int t) {
	glutPostRedisplay();

	glutTimerFunc((int)1000 / FPS, animation, 0);
}

/****************** MAIN ***********************/

int main(int argc, char** argv) {


	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(window_size_x, window_size_y);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("PROJETO GOKUBO");

	glutTimerFunc((int)1000 / FPS, animation, 0);

	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(reshape);
	glutDisplayFunc(draw);
	glutTimerFunc((int)1000 / FPS, animation, 0);

	glutIgnoreKeyRepeat(false); // process keys held down

	initGL(window_size_x, window_size_y);
	//	Scene::initGL();
	glutMainLoop();
	return 0;
}

//
// Utility routine to calculate the 3D position of a 
// projected unit vector onto the xy-plane. Given any
// point on the xy-plane, we can think of it as the projection
// from a sphere down onto the plane. The inverse is what we
// are after.
//
Vector3 trackBallMapping(int x, int y) {
	Vector3 v;
	double d;

	v.x = (2.0 * x - window_size_x) / window_size_x;
	v.y = (window_size_y - 2.0 * y) / window_size_y;
	v.z = 0.0;
	d = v.Length();
	d = (d < 1.0) ? d : 1.0;  // If d is > 1, then clamp it at one.
	v.z = sqrtf(1.001 - d * d);  // project the line segment up to the surface of the sphere.

	v.Normalize();  // We forced d to be less than one, not v, so need to normalize somewhere.


	return v;
}




void glPrint(float* c, float x, float y, float z, const char *fmt, ...) {
	if (fmt == NULL)	// If There's No Text
		return;			// Do Nothing

	char text[256];		// Holds Our String
	va_list ap;			// Pointer To List Of Arguments

	va_start(ap, fmt);								// Parses The String For Variables
	vsprintf(text,/* 256 * sizeof(char),*/ fmt, ap);	// And Converts Symbols To Actual Numbers
	va_end(ap);										// Results Are Stored In Text

	size_t len = strlen(text);

	if (c != NULL)
		glColor3fv(c);

	glRasterPos3f(x, y, z);
	for (size_t i = 0; i < len; i++)
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, text[i]);

}



