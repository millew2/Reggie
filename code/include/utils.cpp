#include <iostream>

using namespace std;

#include "button.h"

///global variables
Graph graph;
int cursorBlinkFrame = 0;
bool ACTIVE_CTRL = false;
string input = "";
vector<Point> points;
int HEIGHT = 800;
int WIDTH = 1200;
int mouse_x = 0;
int mouse_y = 0;
int firstNode = -1;
int inputFunc = 0;
string info = "";
vector<string> tags;
vector<Button> demButtons;



//string split helper
vector<string> &split(const std::string &s, char delim, std::vector<std::string> &elems)
{
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) 
	{
		elems.push_back(item);
	}
	return elems;
}

//writes bitmap string to screen
//This is an example of the facade design pattern
//Writing strings to the screen is a common task
void writeString(int x, int y, void *font, const char *str)
{
	const char *cs;
	glRasterPos2i(x, y);
	for(cs = str; *cs != '\0'; cs++)
	{
		glutBitmapCharacter(font, *cs);
	}
	glutPostRedisplay();
}

//euclidean distance
float dist(Point p1, double x, double y)
{
	return sqrt(pow(p1.getX() - x, 2) + pow(p1.getY() - y, 2));
}

//check if candidate node would overlap any other nodes
bool collisionFree(int x, int y)
{
	vector<Node>* nodes = graph.getNodes();
	for(int i = 0; i<nodes->size(); i++)
	{	
		//check the distance from each node to the mousclick position. If it is too close, don't allow node creation
		if(dist((*nodes)[i].getPoint(), x, y) <= RADIUS*2 && i != firstNode)
		{
			return false;
		}
	}
	return true;
}

//check if user clicked on a node
int pickNode(int x, int y)
{
	vector<Node>* nodes = graph.getNodes();
	for(int i = 0; i<nodes->size(); i++)
	{
		if(dist((*nodes)[i].getPoint(), x, y) <= RADIUS)
		{
			return i;
		}
	}
	return -1;
}

double triArea(double aX, double aY, double bX, double bY, double cX, double cY)
{
	return abs(((bX - aX)*(cY - aY) - (cX - aX)*(bY - aY))/2.0);
}

//check if user clicked on an edge
int pickEdge(int x, int y)
{
	vector<Edge*>* edges = graph.getEdges();
	vector<Node>* nodes = graph.getNodes();

	Point nodeACoords;
	Point nodeBCoords;
	for(int i = 0; i<edges->size(); i++)
	{
		nodeACoords = (*nodes)[((*edges)[i])->getNodeA()].getPoint();
		nodeBCoords = (*nodes)[((*edges)[i])->getNodeB()].getPoint();

		//sum of triangles method
		double startX = nodeACoords.getX();
		double startY = nodeACoords.getY();

		double endX = nodeBCoords.getX();
		double endY = nodeBCoords.getY();

		double diffX = endX - startX;
		double diffY = endY - startY;
		double length = sqrt(diffX*diffX + diffY*diffY);

		double startLeftX = startX - diffY/length*3*3.14159265359/2;
		double startLeftY = startY + diffX/length*3*3.14159265359/2;

		double startRightX = startX + diffY/length*3*3.14159265359/2;
		double startRightY = startY - diffX/length*3*3.14159265359/2;

		double endLeftX = endX - diffY/length*3*3.14159265359/2;
		double endLeftY = endY + diffX/length*3*3.14159265359/2;

		double endRightX = endX + diffY/length*3*3.14159265359/2;
		double endRightY = endY - diffX/length*3*3.14159265359/2;

		double tri1 = triArea(x,y,startLeftX,startLeftY,startRightX,startRightY);
		double tri2 = triArea(x,y,startRightX,startRightY,endRightX,endRightY);
		double tri3 = triArea(x,y,endRightX,endRightY,endLeftX,endLeftY);
		double tri4 = triArea(x,y,endLeftX,endLeftY,startLeftX,startLeftY);

		double bigTri1 = triArea(startLeftX,startLeftY,startRightX,startRightY,endRightX,endRightY);
		double bigTri2 = triArea(endRightX,endRightY,endLeftX,endLeftY,startLeftX,startLeftY);

		if(tri1 + tri2 + tri3 + tri4 <= bigTri1 + bigTri2 + 2)
		{
			return i;
		}



	}	
	return -1;
}

//button drawing function
//This method is a use of the facade design pattern, since it generalizes the common task of button drawing.
void drawButton(string name, int row, int column)
{
	int left, right, top, bottom;

	left = WIDTH - 385 + 190*(column - 1);
	right = left + 180;
	top = HEIGHT - 15 - 40*(row - 1);
	bottom = top - 30;	

	glColor3f(37.0/255.0, 213.0/255.0, 0.0/255.0);
	glBegin(GL_QUADS);
	glVertex2f(left, top);
	glVertex2f(left, bottom);
	glVertex2f(right, bottom);
	glVertex2f(right, top);
	glEnd();

	glColor3f(255.0/255.0, 255.0/255.0, 255.0/255.0);
	writeString(left + 5 + (180 - 10*name.length())/2, bottom + 8, GLUT_BITMAP_HELVETICA_18, name.c_str());
}

//menu drawing function
void drawMenu()
{
	glColor3f(0.0/255.0, 160.0/255.0, 138.0/255.0);
	glBegin(GL_QUADS);
	glVertex2f(WIDTH - 400, 0);
	glVertex2f(WIDTH, 0);
	glVertex2f(WIDTH, HEIGHT);
	glVertex2f(WIDTH - 400, HEIGHT);
	glEnd();

	glColor3f(16.0/255.0, 73.0/255.0, 169.0/255.0);
	glBegin(GL_QUADS);
	glVertex2f(WIDTH - 390, 10);
	glVertex2f(WIDTH - 10, 10);
	glVertex2f(WIDTH - 10, HEIGHT - 10);
	glVertex2f(WIDTH - 390, HEIGHT - 10);
	glEnd();

	for(int i = 0; i < demButtons.size(); i++)
	{
		drawButton(demButtons[i].getName(), demButtons[i].getRow(), demButtons[i].getCol());
	}

	//draw typing area
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_QUADS);
	glVertex2f(WIDTH - 385, 15);
	glVertex2f(WIDTH - 15, 15);
	glVertex2f(WIDTH - 15, 105);
	glVertex2f(WIDTH - 385, 105);
	glEnd();

	//actual text in area
	if(MODE == "SAVING FILE" || MODE == "LOADING FILE" || MODE == "SAVING TEMPLATE" || MODE == "LOADING TEMPLATE" || MODE == "INPUT")
	{	
		glColor3f(37.0/255.0, 213.0/255.0, 0.0/255.0);
		writeString(WIDTH - 380, 85, GLUT_BITMAP_9_BY_15, MODE.c_str());

		string super = MESSAGE;
		while(super.length() < 120)
		{
			super = super + " ";
		}
		string sub[3] = {"", "", ""};
		for(int i = 0; i < 3; i++)
		{
			for(int j = 0; j < 40; j++)
			{
				sub[i] = sub[i] + super[40*i+j];
			}
		}
		writeString(WIDTH - 380, 65, GLUT_BITMAP_9_BY_15, sub[0].c_str());
		writeString(WIDTH - 380, 45, GLUT_BITMAP_9_BY_15, sub[1].c_str());
		writeString(WIDTH - 380, 25, GLUT_BITMAP_9_BY_15, sub[2].c_str());
	}
	else if(MODE == "NORMAL" || MODE == "MESSAGE")
	{		
		glColor3f(37.0/255.0, 213.0/255.0, 0.0/255.0);

		string super = MESSAGE;
		while(super.length() < 160)
		{
			super = super + " ";
		}
		string sub[4] = {"", "", "", ""};
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 40; j++)
			{
				sub[i] = sub[i] + super[40*i+j];
			}
		}
		writeString(WIDTH - 380, 85, GLUT_BITMAP_9_BY_15, sub[0].c_str());
		writeString(WIDTH - 380, 65, GLUT_BITMAP_9_BY_15, sub[1].c_str());
		writeString(WIDTH - 380, 45, GLUT_BITMAP_9_BY_15, sub[2].c_str());
		writeString(WIDTH - 380, 25, GLUT_BITMAP_9_BY_15, sub[3].c_str());
	}

}

//display function
void drawScene()
{
	glClearColor(1.0, 1.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);
	graph.draw();

	if(MODE == "SAVING FILE" || MODE == "LOADING FILE" || MODE == "SAVING TEMPLATE" || MODE == "LOADING TEMPLATE")
	{
		cursorBlinkFrame++;
		if(cursorBlinkFrame % 30 < 15)
		{
			MESSAGE = "Enter filename: " + FILENAME + "|";
		}
		else
		{
			MESSAGE = "Enter filename: " + FILENAME + "";
		}
	}
	else if(MODE == "INPUT")
	{
		cursorBlinkFrame++;
		if(cursorBlinkFrame % 30 < 15)
		{
			MESSAGE = input + "|";
		}
		else
		{
			MESSAGE = input + "";
		}
	}

	drawMenu();

	glutSwapBuffers();
}

//window resize function
void resize(int w, int h)
{
	WIDTH = w;
	HEIGHT = h;
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, w, 0.0, h, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

//keyboard function
void keyInput(unsigned char key, int x, int y)
{
	if(MODE == "SAVING FILE" || MODE == "LOADING FILE" || MODE == "SAVING TEMPLATE" || MODE == "LOADING TEMPLATE")
	{
		if(key == 13 && FILENAME != "")
		{
			if(MODE == "SAVING FILE")
			{
				MESSAGE = "";
				graph.save(FILENAME,false);			
			}
			else if(MODE == "LOADING FILE")
			{
				MESSAGE = "";
				graph.load(FILENAME,false);
			}
			else if(MODE == "SAVING TEMPLATE")
			{
				MESSAGE = "";
				graph.save(FILENAME,true);
			}
			else if(MODE == "LOADING TEMPLATE")
			{
				MESSAGE = "";
				graph.load(FILENAME,true);
			}
			FILENAME = "";
			MODE = "NORMAL";
		}
		else if(key == 8)
		{
			if(FILENAME.size() != 0)
			{
				FILENAME.resize(FILENAME.size() - 1);
			}
		}
		else if(key == 27)
		{
			FILENAME = "";
			MODE = "NORMAL";
			MESSAGE = "";
		}
		else if(key != 13)
		{
			FILENAME += key;
		}
	}

	else if(MODE == "INPUT")
	{
		//Enter Key: Check what function is currently in progress and compute appropriately
		if(key == 13)//enter key
		{
			vector<Node>* nodes = graph.getNodes();
			//add node data
			if(inputFunc == 1)
			{
				info = input.substr(17, string::npos);	
				input = "Enter Node Tags Separated by Commas: ";
				inputFunc = 2;
			}
			//add node tags
			else if(inputFunc == 2)
			{
				input = input.substr(37, string::npos);
				graph.addNode(info, split(input, ',', tags), points.back().getX(), points.back().getY());
				firstNode = -1;//graph.getNodes()->back().getIndex();
				MODE = "NORMAL";
				input = "";
				MESSAGE = "";
				tags.clear();
				inputFunc = 0;
			}
			//add edge
			else if(inputFunc == 3)
			{
				input = input.substr(21, string::npos);
				graph.addEdge(firstNode, pn, input);
				firstNode = -1;
				pn = -1;
				demButtons.pop_back();

				MODE = "NORMAL";
				input = "";
				MESSAGE = "";
				inputFunc = 0;
			}
			//edit node data
			else if(inputFunc == 4)
			{
				input = input.substr(16, string::npos);
				if(input != "")
				{
					(*nodes)[pn].setInformation(input);
					//graph.printGraph();
				}

				inputFunc = 5;
				input = "Add Node Tags: " + (*nodes)[pn].printTags();
			}
			//edit node tags
			else if (inputFunc == 5)
			{
				input = input.substr(15, string::npos);
				tags = split(input, ',', tags);
				(*nodes)[pn].setTags(tags);

				MODE = "NORMAL";
				MESSAGE = "";
				input = "";
				pn = -1;
				inputFunc = 0;
				firstNode = -1;
				tags.clear();
			}
		}
		//backspace removes characters from input buffer
		else if(key == 8)
		{
			if(input.substr(input.size()-2, string::npos) != ": ")
			{
				input.resize(input.size() - 1); 
			}
		}
		//escape exits INPUT mode
		else if(key == 27)
		{
			MODE = "NORMAL";
			input = "";
			inputFunc = 0;
			tags.clear();
			firstNode = -1;
			pn = -1;
		}
		//add other keys to input buffer
		else
		{
			input += key;
		}
	}
	else
	{
		switch(key) 
		{
			case 27:
				exit(0);
				break;
			default:
				break;
		}
	}
}

//mouse function
void mouseControl(int button, int state, int x, int y)
{
	//flag if outside of canvas area boundaries, usually in button area
	bool buttFlag = false;
	vector<Node>* nodes = graph.getNodes();

	//check if in button area
	if(x >= WIDTH - 400 - RADIUS || x < 0 + RADIUS || y < 0 + RADIUS || y > HEIGHT- RADIUS)
	{
		buttFlag = true;
	}

	if(buttFlag)
	{
		//check for button clicks
		int left, right, top, bottom;
		if(state == GLUT_DOWN && button == GLUT_LEFT_BUTTON)
		{
			for(int i = 0; i < demButtons.size(); i++)
			{
				left = WIDTH - 385 + 190*(demButtons[i].getCol() - 1);
				right = left + 180;
				top = HEIGHT - 15 - 40*(demButtons[i].getRow() - 1);
				bottom = top - 30;	

				if(x < right && x > left && HEIGHT - y < top && HEIGHT - y > bottom)
				{
					if(demButtons[i].getName() == "Load File")
					{
						MODE = "LOADING FILE";
						FILENAME = "";
					}
					else if(demButtons[i].getName() == "Save File")
					{
						MODE = "SAVING FILE";
						FILENAME = "";
					}
					else if(demButtons[i].getName() == "Load Template")
					{
						MODE = "LOADING TEMPLATE";
						FILENAME = "";
					}
					else if(demButtons[i].getName() == "Save Template")
					{
						MODE = "SAVING TEMPLATE";
						FILENAME = "";
					}
					else if(demButtons[i].getName() == "Topological Sort")
					{
						graph.saveSortedGraph();
					}
					else if(demButtons[i].getName() == "Export Calendar")
					{
						graph.exportCalendar();
					}
					else if(demButtons[i].getName() == "Delete Node")
					{
						graph.removeNode(pn);
						while(demButtons.size() > 6)
						{
							demButtons.pop_back();
						}
						pn = -1;
						MESSAGE = "The node was successfully deleted.";
					}
					else if(demButtons[i].getName() == "Delete Edge")
					{
						graph.removeEdge(pe);
						while(demButtons.size() > 6)
						{
							demButtons.pop_back();
						}
						pe = -1;
						MESSAGE = "The edge was successfully deleted.";
					}
				}
			}
		}
	}
	else
	{
		// Store the currentPoint in the points vector when left button is released.
		if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		{	
			MODE = "NORMAL";
			input = "";
			Point currentPoint = Point(x, HEIGHT - y, false);
			points.push_back(currentPoint);
			vector<Edge*>* edges = graph.getEdges();

			//check if we picked a node
			pn = pickNode(currentPoint.getX(), currentPoint.getY());
			pe = pickEdge(currentPoint.getX(), currentPoint.getY());

			if(pn != -1)
			{
				MESSAGE = string("Data: ") + string((*nodes)[pn].getInformation()) + string(", Tags: ") + (*nodes)[pn].printTags();
				firstNode = pn;
				pe = -1;
			}
			else if(pe != -1)//check if we picked an edge
			{
				firstNode = -1;
				MESSAGE = string("Relation: ") + string((*edges)[pe]->getRelation());
			}
			else if(collisionFree(x, HEIGHT - y))
			{
				MODE = "INPUT";
				input = "Enter Node Data: ";
				inputFunc = 1;
				firstNode = -1;
			}

		}
		else if (state == GLUT_UP && (button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON))
		{
			Point currentPoint = Point(x, HEIGHT - y, true);
			points.push_back(currentPoint);	
			pn = pickNode(currentPoint.getX(), currentPoint.getY());

			if(button == GLUT_LEFT_BUTTON)
			{
				if(pn != -1 && firstNode != -1 && pn != firstNode && !(*nodes)[firstNode].edgeExists(pn))
				{
					MODE = "INPUT";
					input = "Enter Edge Relation: ";
					inputFunc = 3;
				}
				else if(firstNode != -1 && collisionFree(x, HEIGHT - y))
				{
					(*graph.getNodes())[firstNode].move(x, HEIGHT - y);
				}

				//edge/node delete button
				if(pn != -1)
				{
					while(demButtons.size() > 6)
					{
						demButtons.pop_back();
					}
					demButtons.push_back(Button(4, 1, "Delete Node"));
					pe = -1;
				}
				else if(pe != -1)//check if we picked an edge
				{
					while(demButtons.size() > 6)
					{
						demButtons.pop_back();
					}
					demButtons.push_back(Button(4, 1, "Delete Edge"));
				}

				if(pe == -1 && pn == -1)
				{
					while(demButtons.size() > 6)
					{
						demButtons.pop_back();
					}
				}
			}
			else if(button == GLUT_RIGHT_BUTTON)
			{
				if(pn != -1)
				{
					MODE = "INPUT";
					input = "Edit Node Data: " + (*nodes)[pn].printInfo();
					inputFunc = 4;
				}	
			}
		}
	}	
}

//Holds opengl functions that would generally be found in main
void opengl_init(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_DOUBLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(0, 0);
	gluOrtho2D(0, 500, 0, 500);
	glutCreateWindow("Babel Graph");
	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);
	glutMouseFunc(mouseControl);
	MODE = "NORMAL";
	tags.clear();
	vector<string>test;
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	MESSAGE = "Left click in empty space to create a   node. Left click and drag from one node to another to create an edge between    them.";
	MODE = "MESSAGE";

	//buttons declaration
	demButtons.push_back(Button(1,1,"Load File"));
	demButtons.push_back(Button(1,2,"Save File"));
	demButtons.push_back(Button(2,1,"Load Template"));
	demButtons.push_back(Button(2,2,"Save Template"));
	demButtons.push_back(Button(3,1,"Topological Sort"));
	demButtons.push_back(Button(3,2,"Export Calendar"));

	glutMainLoop();
}
