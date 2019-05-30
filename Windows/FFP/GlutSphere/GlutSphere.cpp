#include<freeglut.h>

#pragma comment(lib,"freeglut.lib")

bool bIsFullScreen = false;

int year = 0;
int day = 0;

int main(int argc, char *argv[])
{
	//function declarations
	void initialize(void);
	void uninitialize(void);
	void reshape(int, int);
	void display(void);
	void keyboard(unsigned char, int, int);
	void mouse(int, int, int, int);
	void update(void);

	//code
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("My First OGL Application - Jayshree");

	initialize();

	//callbacks
	glutIdleFunc(update);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutCloseFunc(uninitialize);

	glutMainLoop();

	return(0);
}

void initialize(void)
{
	//code
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glShadeModel(GL_FLAT);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void uninitialize(void)
{

}

void reshape(int width, int height)
{
	//code
	if (height == 0)
	{
		height = 1;
	}

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60.0f, (GLfloat)width / (GLfloat)height, 1.0f, 20.0f);
}

void display(void)
{
	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	glPushMatrix();
	glColor3f(1.0f, 0.647f, 0.0f);
	glutWireSphere(1.0f, (GLint)20.0f,(GLint)16.0f);//sun

	glRotatef((GLfloat)year, 0.0f, 1.0f, 0.0f);
	glTranslatef(2.0f, 0.0f, 0.0f);
	glRotatef((GLfloat)day, 0.0f, 1.0f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.5f);
	glutWireSphere(0.2f, (GLint)10.0f, (GLint)8.0f);

	glPopMatrix();

	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
	//code
	switch (key)
	{
	case 27:
		glutLeaveMainLoop();
		break;

	case 'F':
	case 'f':
		if (bIsFullScreen == false)
		{
			glutFullScreen();
			bIsFullScreen = true;
		}
		else
		{
			glutLeaveFullScreen();
			bIsFullScreen = false;
		}
		break;

	case 'Y':
		year = (year + 5) % 360;
		glutPostRedisplay();
		break;

	case 'y':
		year = (year - 5) % 360;
		glutPostRedisplay();
		break;

	case 'D':
		day = (day + 10) % 360;
		glutPostRedisplay();
		break;

	case 'd':
		day = (day - 10) % 360;
		glutPostRedisplay();
		break;
	}
}

void mouse(int button, int state, int x, int y)
{
	//code
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		break;

	case GLUT_RIGHT_BUTTON:
		glutLeaveMainLoop();
		break;
	}
}

void update(void)
{
	year = (year + 1) % 360;
	day = (day + 5) % 360;
	glutPostRedisplay();
}

void DrawPyramid(void)
{
	glBegin(GL_TRIANGLES);

	//front face
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	//right face
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

	//back face
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);


	//left face
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glEnd();
}

void DrawCube(void)
{
	glBegin(GL_QUADS);
	//top face
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);//rt
	glVertex3f(-1.0f, 1.0f, -1.0f);//lt
	glVertex3f(-1.0f, 1.0f, 1.0f);//lb
	glVertex3f(1.0f, 1.0f, 1.0f);//rb
	glEnd();

	glBegin(GL_QUADS);
	//bottom face
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);//rt
	glVertex3f(-1.0f, -1.0f, -1.0f);//lt
	glVertex3f(-1.0f, -1.0f, 1.0f);//lb
	glVertex3f(1.0f, -1.0f, 1.0f);//rb
	glEnd();

	glBegin(GL_QUADS);
	//front face
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glEnd();

	glBegin(GL_QUADS);
	//back face
	glColor3f(0.0f, 1.0f, 1.0f);//cyan
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	//right face
	glColor3f(1.0f, 0.0f, 1.0f);//magenta
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd();

	glBegin(GL_QUADS);
	//left face
	glColor3f(1.0f, 1.0f, 0.0f);//yellow
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glEnd();
}