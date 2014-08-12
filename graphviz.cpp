#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
//#include <GL/glut.h> 	//Linux
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "vector.h"
//#include "/home/bmild/prefiltered/color.h"
#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

struct Color {
	float r,g,b;
	Color():r(0),g(0),b(0){}
	Color(float rr, float gg, float bb): r(rr),g(gg),b(bb) {}
};
std::ostream& operator<<(std::ostream& o, const Vector& v) { 
    o << "(" << v.x << ", " << v.y << ", " << v.z << ")"; 
    return o; 
}

std::ostream& operator<<(std::ostream& o, const Point& v) { 
    o << "(" << v.x << ", " << v.y << ", " << v.z << ")"; 
    return o; 
}

std::ostream& operator<<(std::ostream& o, const Color& v) { 
    o << "(" << v.r << ", " << v.g << ", " << v.b << ")"; 
    return o; 
}
struct Display{

};

static float rfp() { 
	return (float)rand() / RAND_MAX;
}
static float rf() { 
	return 2.*rand() / RAND_MAX-1.;
}

void Write(Vector v, string str) {
	Vector vv = (v-Vector(.01,.01,0));
	glRasterPos3fv(&vv.x);
	for (int i = 0; i < str.size(); ++i)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
}

void Write(float x, float y, string str) {
	Write(Vector(x,y,0),str);
}

Vector plane_u(1,0,0);
Vector plane_v(0,1,0);
const float circ_rad = .05;
void Circle(Vector v, float radius = circ_rad) {
	glBegin(GL_LINE_LOOP);
	int res = 32;
	for (int i = 0; i < res; ++i) {
		float th = (float)i/res*M_PI*2.;
		Vector vv = (v+radius*(plane_u*cos(th)+plane_v*sin(th)));
		glVertex3fv(&(vv.x));
	}
	glEnd();
}
void Circle(float x, float y, float radius = circ_rad) {
	Circle(Vector(x,y,0),radius);
}
void Circle(Vector v, string str) {
	Circle(v, circ_rad);
	Write(v, str);
}
void Circle(Vector v, int radius, string str) {
	Circle(v, radius);
	Write(v, str);
}
void Circle(float x, float y, float radius, string str) {
	Circle(x,y,radius);
	Write(x, y, str);
}

const float arrow_angle = 30 * M_PI/180.;
const float arrow_head = .03;
void CircleArrow(Vector s, string str) {
	float radius = circ_rad;
	float dr = circ_rad/2;
	float cd = radius+circ_rad-dr;
	cd = circ_rad*sqrt(2.);
	Vector disp = cd/sqrt(2.)*(plane_u+plane_v);
	Vector ctr = s+disp;
	glBegin(GL_LINE_STRIP);
	int res = 32;
	for (int i = 0; i < res*3/4 + 1; ++i) {
		float th = (float)i/res*M_PI*2. - M_PI*.5;
		Vector vv = (ctr+radius*(plane_u*cos(th)+plane_v*sin(th)));
		glVertex3fv(&(vv.x));
	}
	glEnd();
	glBegin(GL_LINES);
	float th = M_PI*.5;
	Vector t = s+plane_v*circ_rad;
	glVertex3fv(&t.x);	
	Vector h1 = (t + arrow_head*(plane_u*cos(th+arrow_angle) 
		+ plane_v*sin(th+arrow_angle)));
	glVertex3fv(&(h1.x));
	glVertex3fv(&t.x);
	Vector h2 = (t + arrow_head*(plane_u*cos(th-arrow_angle) 
		+ plane_v*sin(th-arrow_angle)));
	glVertex3fv(&(h2.x));
	glEnd();
	Write(ctr+disp*1.2, str);
}

void Arrow(Vector s, Vector t) {
	float th = atan2((s-t).y,(s-t).x);
	glBegin(GL_LINES);
	glVertex3fv(&s.x);
	glVertex3fv(&t.x);	
	glVertex3fv(&t.x);	
	Vector h1 = (t + arrow_head*(plane_u*cos(th+arrow_angle) 
		+ plane_v*sin(th+arrow_angle)));
	glVertex3fv(&(h1.x));
	glVertex3fv(&t.x);
	Vector h2 = (t + arrow_head*(plane_u*cos(th-arrow_angle) 
		+ plane_v*sin(th-arrow_angle)));
	glVertex3fv(&(h2.x));
	glEnd();
}


void Arrow(Vector s, Vector t, float rad) {
	Vector st = s-t;
	float l = st.len();
	st = st.unit();
	Arrow(s - rad*st, t + rad*st);
}

void Arrow(Vector s, Vector t, float rad, string str) {
	Arrow(s,t,rad);
	Vector orth = (s-t).cross(Vector(0,0,1)).unit();
	Write(.45*s+.55*t + orth*circ_rad*.5, str);
}

const float G = .00005;
const float k = .01;
const float len = .2;
const float viscosity = .95;
const float g_eps = .03;
struct Physical {
	Vector x, v;
	float m;
	bool frozen;
	Physical(): x(0,0,0), v(0,0,0), m(1.), frozen(false) {};
	Physical(Vector xi): x(xi), v(0,0,0), m(1.), frozen(false) {}
	void step() { if (frozen) return; x += v; v *= viscosity; }
	void freeze() {
		frozen = true;
	}
	void unfreeze() {
		frozen = false;
		v = Vector(0,0,0);
	}
};

void gravity(Physical &s, Physical &t) {
	Vector r = t.x-s.x;
	float r_len = r.len();
	if (r_len < g_eps) r_len = g_eps;
	float scalar = G*s.m*t.m/pow(r_len,2);

	Vector F = scalar*r.unit();
	t.v += F / t.m;
	s.v -= F / s.m;
}

struct Edge {
	int to;
	string label;
	float length;
	float weight;
	Edge(): weight(1.) {}
	Edge(int t): to(t), label(""), length(len), weight(1.) {}
	Edge(int t, string l): to(t), label(l), length(len), weight(1.) {}
	void step(Physical &s, Physical &t) {
		Vector r = t.x-s.x;
		Vector F = -k*(r.len()-length)*r.unit();
		t.v += F / t.m;
		s.v -= F / s.m;
	}
};

struct Vertex {
	string label;
	Physical p;
	int accept;
	vector<Edge> edges;

	Vertex(): accept(0) {}
	Vertex(string l): label(l), accept(0) {}
	Vertex(string l, Vector x): label(l), p(x), accept(0) {}

};

const float tolerance = circ_rad *2;

struct Graph {
	vector<Vertex> vertices;
	Graph(): moused(-1) {}
	void step() {
		for (int el = 0; el < vertices.size(); ++el)
			for (auto edg_iter = vertices[el].edges.begin(); 
				edg_iter != vertices[el].edges.end(); ++edg_iter) 
				if (el != edg_iter->to)
					edg_iter->step(vertices[el].p, vertices[edg_iter->to].p);

		for (int u = 0; u < vertices.size(); ++u)
			for (int v = u+1; v < vertices.size(); ++v)
				gravity(vertices[u].p, vertices[v].p);

		for (int v = 0; v < vertices.size(); ++v)
			vertices[v].p.step();

	}
	void print() {
		for (int v = 0; v < vertices.size(); ++v)
			cout << "Vertex " << vertices[v].label << " at " << vertices[v].p.x << 
		" velocity " << vertices[v].p.v << endl;
	}
	void draw(Color vcol, Color ecol) {
		for (int v = 0; v < vertices.size(); ++v) {
			if (v==moused)
				glColor3f(1,1,1);
			else
				glColor3fv(&vcol.r);			
			Circle(vertices[v].p.x, vertices[v].label);
			if (vertices[v].accept)
				Circle(vertices[v].p.x, circ_rad*.8);

		}
		glColor3fv(&ecol.r);
		for (int el = 0; el < vertices.size(); ++el)
			for (auto edg_iter = vertices[el].edges.begin(); 
				edg_iter != vertices[el].edges.end(); ++edg_iter) 
				if (el == edg_iter->to)
					CircleArrow(vertices[el].p.x, edg_iter->label);
				else
					Arrow(vertices[el].p.x, vertices[edg_iter->to].p.x,circ_rad, edg_iter->label);
	}
	void center() {
		Vector cx, cv;
		for (int v = 0; v < vertices.size(); ++v) {
			cx += vertices[v].p.x; cv += vertices[v].p.v;
		}
		cx /= vertices.size(); cv /= vertices.size();

		for (int v = 0; v < vertices.size(); ++v) {
			vertices[v].p.x -= cx; vertices[v].p.v -= cv;
		}
	}

	int moused;
	void mouse_select(float mx, float my) {
		mx = mx*2-1; my = my*2-1;
		Vector m(mx,my,0);
		float best = tolerance;
		moused = -1;
		for (int v = 0; v < vertices.size(); ++v) {
			float d = (vertices[v].p.x - m).len();
			if (d<best) {
				best=d; moused = v;
			}
		}
		if (moused != -1) 
			vertices[moused].p.freeze();
	}
	void mouse_unselect(float mx, float my) {
		if (moused == -1) return;
		vertices[moused].p.unfreeze();
		moused = -1;
	}
	void mouse_move(float dx, float dy) {
		Vector dm(dx*2,dy*2,0);
		if (moused==-1) {
			for (int v = 0; v < vertices.size(); ++v)
				vertices[v].p.x += dm;

		} else {
			vertices[moused].p.x += dm;
		}
	}
};
Graph *graph;
string filename;
void setup_graph(string filename) {
	graph = new Graph();
	ifstream file(filename);
	if (file.fail()) return;
	int nv;
	file >> nv;
	graph->vertices.resize(nv);
	// cout << "Graph has " << nv << " vertices" << endl;
	for (int from = 0; from < nv; ++from) {
		int ne;
		string vlabel;
		int accept;
		file >> vlabel >> ne >> accept;
		// cout << "Vertex " << from << " is " << vlabel << " and has " << ne << " edges"
		// << ", is accepting? " << accept << endl;
		graph->vertices[from].label = vlabel;
		graph->vertices[from].p.x = Vector(rf(),rf(),0);
		graph->vertices[from].accept = accept;

		for (int e = 0; e < ne; ++e) {
			int to;
			string elabel;
			file >> to >> elabel;
			//cout << "Edge " << elabel << " to " << to << endl;
			graph->vertices[from].edges.push_back(Edge(to,elabel));
		}
	}
	file.close();
}



Display disp;
void framerate(){
	static int count=0;
	static int prev=0;
	int now=time(0);

	if(prev==0) prev=now;
	count++;
	if(prev!=now){
		printf("%3d/sec\r", count/(now-prev));
		fflush(stdout);
		prev=now;
		count=0;
	}
}

// glut callbacks
void redraw(void){
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1,0,0);
	glLoadIdentity();
	
	graph->step();
	graph->draw(Color(0,1,0),Color(1,0,0));

	glFlush();
	glutSwapBuffers();
	glutPostRedisplay();
	framerate();
}
int width, height;
float mouse_x, mouse_y;
void resize(int w, int h){
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	width = w; height = h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, 0, 1);

	glMatrixMode(GL_MODELVIEW);
}
void kbd(unsigned char key, int /* x */, int /* y */){
	switch (key){
	case 'r':
		setup_graph(filename);
		break;
	case 'c':
		graph->center();
	break;
	case '0':
			break;
	case 'q':
	case 033:		// ESC
	case 021:		// ctrl Q
	case 027:		// ctrl W
		exit(0);
		break;
	}
}
enum {START,MOVE,END};

void mouse(int state, int x, int y){
	y = height - y;
	float nmx = (float)x/width, nmy = (float)y/width;
	float dx = nmx-mouse_x, dy = nmy-mouse_y;
	switch(state){
	case START:
		graph->mouse_select(nmx,nmy);
		break;
	case MOVE:
		graph->mouse_move(dx,dy);

		break;
	case END:
		graph->mouse_unselect(nmx,nmy);

		break;
	}
	mouse_x = nmx; mouse_y = nmy;
}
void mousetransition(int button, int state, int x, int y){
	if(button!=GLUT_LEFT_BUTTON) return;
	mouse(state==GLUT_DOWN?START:END, x, y);
}
void mousemotion(int x, int y){
	mouse(MOVE, x, y);
}
void poll(){
}
int dim = 1200;
void runGl(int argc, char *argv[]){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
	glutInitWindowSize(dim, dim);
	//glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	glClearColor(0., 0., 0., 1.);
	glPointSize(1);
	glLineWidth(2);
	// AntiAliasing.
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);	// round, not square
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);		// antialiased lines
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	// glut callbacks
	glutDisplayFunc(redraw);
	glutReshapeFunc(resize);
	glutKeyboardFunc(kbd);
	glutIdleFunc(poll);
	glutMouseFunc(mousetransition);
	glutMotionFunc(mousemotion);

	glutMainLoop();
}
void display(int argc, char *argv[]){

	runGl(argc, argv);
}

int main(int argc, char *argv[]) {
	dim = atoi(argv[2]);
	filename = argv[1];
	setup_graph(argv[1]);
	display(argc, argv);
	return 0;
}