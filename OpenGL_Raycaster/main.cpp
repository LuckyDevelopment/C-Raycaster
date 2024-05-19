/*
════════════════════════════════════════════════════════════════════════

"Lucky's Raycaster" built by LuckyEcho

Simple raycaster.

Last edited 5/18/2024 12:50 PM
════════════════════════════════════════════════════════════════════════
*/


// INCLUDES
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <Windows.h>

#include "Textures/All_Textures.ppm"
#include "Textures/sky.ppm"
#include "Textures/win.ppm"
#include "Textures/lose.ppm"
#include "Textures/menu.ppm"
#include "Textures/sprites.ppm"


typedef struct
{
    int w,a,d,s;
}ButtonKeys; ButtonKeys Keys;

// CONFIG CONSTANTS
#define WIDTH 960
#define HEIGHT 640
#define WINDOW_NAME "Lucky's Raycaster"
#define STARTING_X 100
#define STARTING_Y 300
const float TURN_SPEED = 0.2;
const float MOVE_SPEED = 0.2;
const float INTERACT_DISTANCE = 70;
const float MAP_WALL_COLOR[] = {1,1,1};
const float UI_BG_COLOR[] = {0.2,0.2,0.2};
const float WALL_COLOR_1[] = {0.6,0,0};
const float WALL_COLOR_2[] = {0.9,0,0};

// CONSTANTS
#define PI 3.1415926535
#define P2 PI/2
#define P3 3*PI/2
#define DR 0.0174533 // One degree in radians.

float degToRad(float a) { return a*PI/180.0;}
float FixAng(float a){ if(a>359){ a-=360;} if(a<0){ a+=360;} return a;}

// PLAYER
float px,py, pdx, pdy, pa; // Player Position and deltaX, and deltaY, and player angle.
bool DRAW_MINIMAP = false;
int movementKeys[] = {0,0,0,0};

int mapX=8,mapY=8,mapS=64; // Size of the mapW x and y in blocks, and MapS is the size of each block in pixels.
int mapW[]=          //walls
{
 1,1,1,1,1,1,1,1,
 1,5,0,1,0,0,0,1,
 1,0,0,999,0,1,0,1,
 1,1,999,1,0,0,0,1,
 1,0,0,0,0,0,0,1,
 1,0,0,0,0,1,0,1,
 1,0,0,0,0,0,0,1,
 1,1,1,1,1,1,1,1,	
};

int mapF[]=          //floors
{
 1,1,1,1,1,1,1,1,
 1,1,1,1,1,1,1,1,
 1,1,1,1,5,1,1,1,
 1,1,1,1,1,1,1,1,
 1,1,1,1,1,1,1,1,
 1,1,1,1,1,1,1,1,
 1,1,1,1,1,1,1,1,
 1,1,1,1,1,1,1,1,
};

int mapC[]=          //ceiling
{
 2,2,2,2,2,2,2,2,
 2,0,0,2,0,0,0,2,
 2,0,0,2,0,0,0,2,
 2,2,2,2,0,0,0,2,
 2,2,2,2,0,0,0,2,
 2,2,2,2,0,0,0,2,
 2,2,2,2,2,2,2,2,
 2,2,2,2,2,2,2,2,
};

int oldMap[64];

typedef struct {
    int type;
    int state;
    int map;
    float x,y,z;
    float moveSpeed = 0.055;
}sprite; sprite sp[4];
int depth[120];

int gameState=0, timer=0;
float fade=0;
float frame1, frame2, fps;

void drawSprites()
{
    for (int s = 0; s < 4; s++)
    {
        if(px<sp[s].x+30 && px>sp[s].x-30 && py<sp[s].y+30 && py>sp[s].y-30 && sp[s].map == 0){
            sp[s].state = 0;
        }
         if(px<sp[s].x+20 && px>sp[s].x-20 && py<sp[s].y+20 && py>sp[s].y-20 && sp[s].map == 2){
            fade = 0; timer = 0; gameState = 4; printf("%s", "User Lost Game");
        }

        if (sp[s].map == 2)
        {
            int spx=(int)sp[s].x>>6,          spy=(int)sp[s].y>>6;          //normal grid position
            int spx_add=((int)sp[s].x+15)>>6, spy_add=((int)sp[s].y+15)>>6; //normal grid position plus     offset
            int spx_sub=((int)sp[s].x-15)>>6, spy_sub=((int)sp[s].y-15)>>6; //normal grid position subtract offset
            if(sp[s].x>px && mapW[spy*8+spx_sub]==0){ sp[s].x-=sp[s].moveSpeed*fps;}
            if(sp[s].x<px && mapW[spy*8+spx_add]==0){ sp[s].x+=sp[s].moveSpeed*fps;}
            if(sp[s].y>py && mapW[spy_sub*8+spx]==0){ sp[s].y-=sp[s].moveSpeed*fps;}
            if(sp[s].y<py && mapW[spy_add*8+spx]==0){ sp[s].y+=sp[s].moveSpeed*fps;}
        }

        float sx=sp[s].x-px; //temp float variables
        float sy=sp[s].y-py;
        float sz=sp[s].z;

        float CS=cos(degToRad(pa)), SN=sin(degToRad(pa)); //rotate around origin
        float a=sy*CS+sx*SN; 
        float b=sx*CS-sy*SN; 
        sx=a; sy=b;

        sx=(sx*108.0/sy)+(120/2); //convert to screen x,y
        sy=(sz*108.0/sy)+( 80/2);

        int x, y;
        int scale = 32*80/b;
        if (scale < 0) {scale = 0;}
        if (scale > 120){scale = 120;}


        float t_x=0, t_y=31, t_x_step = 31.5/(float)scale, t_y_step=32/(float)scale;



        for(x=sx-scale/2;x<sx+scale/2;x++)
        {
            t_y = 31;
            for(y=0;y<scale;y++)
            {
                if (sp[s].state == 1 && x > 0 && x < 120 && b < depth[x])
        {
int pixel=((int)t_y*32+(int)t_x)*3+(sp[s].map*32*32*3);
   int red   =sprites[pixel+0];
   int green =sprites[pixel+1];
   int blue  =sprites[pixel+2];
   if (red!= 255, green != 0, blue != 255)
   {
    glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8, sy*8-y*8); glEnd();
   }
    t_y-=t_y_step; if(t_y < 0) {t_y=0;}

        }
            }
    t_x+=t_x_step;
}
    }
}


void init()
{
    glClearColor(UI_BG_COLOR[0],UI_BG_COLOR[1],UI_BG_COLOR[2], 0); // Set up background color.

    for(int y = 0; y < mapS; y++)
    {
        if (mapW[y] == 999)
        {
            mapW[y] = 4;
        }
    }

    px=150; py=400; pa=90;
 pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));

 sp[0].type=1; sp[0].state=1; sp[0].map=0; sp[0].x=1.5*64; sp[0].y=6*64;   sp[0].z=20; //key
 sp[1].type=2; sp[1].state=1; sp[1].map=1; sp[1].x=1.5*64; sp[1].y=5*64;   sp[1].z=-10; //light 1
 sp[2].type=3; sp[2].state=1; sp[2].map=1; sp[2].x=3.5*64; sp[2].y=5*64;   sp[2].z=-10; //light 2
 sp[3].type=4; sp[3].state=1; sp[3].map=2; sp[3].x=2.5*64; sp[3].y=2*64;   sp[3].z=20;  //enemy
    
    
// Maximum supported line width is in lineWidthRange[1].
}



void drawLine(float x1, float y1, float x2, float y2, float red, float green, float blue, float width = 2)
{
    glColor3f(red, green, blue);
    glLineWidth(width);
    glBegin(GL_LINES);
    glVertex2i(x1,y1);
    glVertex2i(x2,y2);
    glEnd();
}
void screen(int v)
{
    int x,y;
    int *T;
    if (v == 1)
    {
        T = menu;
    }
    if (v == 2)
    {
        T = win;
    }
    if (v == 3)
    {
        T = lose;
    }
    for (y = 0; y < 200; y++)
    {
        for (x = 0; x < 300; x++)
        {
            int pixel=(y*300+x)*3;
            int red   =T[pixel+0] * fade;
            int green =T[pixel+1] * fade;
            int blue  =T[pixel+2] * fade;
            glPointSize(4); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*3.3,y*3.3); glEnd();
        }
    }
    if (fade < 1) {fade+=0.0004 * fps;}
    if (fade > 1) {fade = 1;}
}
void drawSky()
{ int x,y;
    for (y = 0; y < 40; y++)
    {
        for (x = 0; x < 120; x++)
        {
            int xo = (int)pa*2-x; if (x < 0) {xo += 120;} xo=xo % 120;
            int pixel=(y*120+xo)*3;
            int red   =sky[pixel+0];
            int green =sky[pixel+1];
            int blue  =sky[pixel+2];
            glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(x*8,y*8); glEnd();
        }
    }

}


void drawPlayer()
{
    if (DRAW_MINIMAP)
    {
    glColor3f(1,1,0);
    glPointSize(8);
    glBegin(GL_POINTS);
    glVertex2i(px, py);
    glEnd();

    drawLine(px, py, px+pdx*5, py+pdy*5, 1, 1, 0, 3);
    }
}

void drawMap2D()
{
    int x,y,xo,yo; //X, Y, and X Offset and Y Offset
    for(y = 0; y < mapY; y++)
    {
        for(x = 0; x < mapX; x++)
        {
            // Check if it is a 1 or 0.
            if (mapW[y*mapX+x] > 0)
            {
                glColor3f(MAP_WALL_COLOR[0],MAP_WALL_COLOR[1],MAP_WALL_COLOR[2]);
            }
            else
            {
                glColor3f(0,0,0);   
            }
            xo=x*mapS; yo=y*mapS;
            
            // Start drawing, and draw a cube.
            glBegin(GL_QUADS);
            glVertex2i(xo + 1, yo + 1);
            glVertex2i(xo + 1, yo + mapS -1);
            glVertex2i(xo+mapS - 1, yo+mapS - 1);
            glVertex2i(xo+mapS - 1, yo + 1);
            glEnd();
        }
    }
}

float dist(float ax, float ay, float bx, float by, float ang)
{
    return (sqrt((bx-ax) * (bx-ax) + (by-ay) * (by-ay)));
}

void drawRays2D()
{

	
 int r,mx,my,mp,dof,side; float vx,vy,rx,ry,ra,xo,yo,disV,disH; 
 
 ra=FixAng(pa+30);                                                              //ray set back 30 degrees
 
 for(r=0;r<120;r++)
 {
  int vmt=0,hmt=0;                                                              //vertical and horizontal map texture number 
  //---Vertical--- 
  dof=0; side=0; disV=100000;
  float Tan=tan(degToRad(ra));
       if(cos(degToRad(ra))> 0.001){ rx=(((int)px>>6)<<6)+64;      ry=(px-rx)*Tan+py; xo= 64; yo=-xo*Tan;}//looking left
  else if(cos(degToRad(ra))<-0.001){ rx=(((int)px>>6)<<6) -0.0001; ry=(px-rx)*Tan+py; xo=-64; yo=-xo*Tan;}//looking right
  else { rx=px; ry=py; dof=8;}                                                  //looking up or down. no hit  

  while(dof<8) 
  { 
   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                     
   if(mp>0 && mp != 999 && mp<mapX*mapY && mapW[mp]>0){ vmt=mapW[mp]-1; dof=8; disV=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit         
   else{ rx+=xo; ry+=yo; dof+=1;}                                               //check next horizontal
  } 
  vx=rx; vy=ry;

  //---Horizontal---
  dof=0; disH=100000;
  Tan=1.0/Tan; 
       if(sin(degToRad(ra))> 0.001){ ry=(((int)py>>6)<<6) -0.0001; rx=(py-ry)*Tan+px; yo=-64; xo=-yo*Tan;}//looking up 
  else if(sin(degToRad(ra))<-0.001){ ry=(((int)py>>6)<<6)+64;      rx=(py-ry)*Tan+px; yo= 64; xo=-yo*Tan;}//looking down
  else{ rx=px; ry=py; dof=8;}                                                   //looking straight left or right
 
  while(dof<8) 
  { 
   mx=(int)(rx)>>6; my=(int)(ry)>>6; mp=my*mapX+mx;                          
   if(mp>0 && mp != 999 && mp<mapX*mapY && mapW[mp]>0){ hmt=mapW[mp]-1; dof=8; disH=cos(degToRad(ra))*(rx-px)-sin(degToRad(ra))*(ry-py);}//hit         
   else{ rx+=xo; ry+=yo; dof+=1;}                                               //check next horizontal
  } 
  
  float shade=1;
  glColor3f(0,0.8,0);
  if(disV<disH){ hmt=vmt; shade=0.5; rx=vx; ry=vy; disH=disV; glColor3f(0,0.6,0);}//horizontal hit first
  if (DRAW_MINIMAP){
  glLineWidth(2); glBegin(GL_LINES); glVertex2i(px,py); glVertex2i(rx,ry); glEnd();//draw 2D ray
 }
    
  int ca=FixAng(pa-ra); disH=disH*cos(degToRad(ca));                            //fix fisheye 
  int lineH = (mapS*640)/(disH); 
  float ty_step=32.0/(float)lineH; 
  float ty_off=0; 
  if(lineH>640){ ty_off=(lineH-640)/2.0; lineH=640;}                            //line height and limit
  int lineOff = 640/2 - (lineH>>1);                                               //line offset

depth[r] = disH;
  //---draw walls---
    int y;
  float ty=ty_off*ty_step;//+hmt*32;
  float tx;
  if(shade==1){ tx=(int)(rx/2.0)%32; if(ra>180){ tx=31-tx;}}  
  else        { tx=(int)(ry/2.0)%32; if(ra>90 && ra<270){ tx=31-tx;}}
  for(y=0;y<lineH;y++)
  {
  

    
   int pixel=((int)ty*32+(int)tx)*3+(hmt*32*32*3);
   int red   =All_Textures[pixel+0]*shade;
   int green =All_Textures[pixel+1]*shade;
   int blue  =All_Textures[pixel+2]*shade;
   glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8 ,y+lineOff); glEnd();
    
   ty+=ty_step;
    
  }
 

  //---draw floors---
 for(y=lineOff+lineH;y<640;y++)
 {
  float dy=y-(640/2.0), deg=degToRad(ra), raFix=cos(degToRad(FixAng(pa-ra)));
  tx=px/2 + cos(deg)*158*2*32/dy/raFix;
  ty=py/2 - sin(deg)*158*2*32/dy/raFix;
  int mp=mapF[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;
    int pixel=(((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
   int red   =All_Textures[pixel+0] * 0.7;
   int green =All_Textures[pixel+1] * 0.7;
   int blue  =All_Textures[pixel+2] * 0.7;
   glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8 ,y); glEnd();

 //---draw ceiling---
   mp=mapC[(int)(ty/32.0)*mapX+(int)(tx/32.0)]*32*32;
   pixel=(((int)(ty)&31)*32 + ((int)(tx)&31))*3+mp*3;
    red   =All_Textures[pixel+0];
    green =All_Textures[pixel+1];
    blue  =All_Textures[pixel+2];
   if (mp > 0 ) {glPointSize(8); glColor3ub(red,green,blue); glBegin(GL_POINTS); glVertex2i(r*8 ,640-y); glEnd();}
 }


 ra=FixAng(ra-0.5);                                                               //go to next ray, 60 total
 }
}//-----------------------------------------------------------------------------







void display()
{
    // Frames per second
    frame2 = glutGet(GLUT_ELAPSED_TIME); fps=(frame2-frame1); frame1=glutGet(GLUT_ELAPSED_TIME);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (gameState == 0) {init(); fade=0; timer=0; gameState=1;} //Init
    if (gameState == 1) {screen(1); timer +=1*fps; if (timer>6000) {timer = 0; gameState = 2;}} // Start screen
    if(gameState == 2)
        {

        //buttons
    if(Keys.a==1){ pa+=TURN_SPEED*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 	
    if(Keys.d==1){ pa-=TURN_SPEED*fps; pa=FixAng(pa); pdx=cos(degToRad(pa)); pdy=-sin(degToRad(pa));} 

    int xo=0; if(pdx<0){ xo=-20;} else{ xo=20;}                                    //x offset to check map
    int yo=0; if(pdy<0){ yo=-20;} else{ yo=20;}                                    //y offset to check map
    int ipx=px/64.0, ipx_add_xo=(px+xo)/64.0, ipx_sub_xo=(px-xo)/64.0;             //x position and offset
    int ipy=py/64.0, ipy_add_yo=(py+yo)/64.0, ipy_sub_yo=(py-yo)/64.0;             //y position and offset
    if(Keys.w==1)                                                                  //move forward
    {  
    if(mapW[ipy*mapX        + ipx_add_xo]==0){ px+=pdx*MOVE_SPEED*fps;}
    if(mapW[ipy_add_yo*mapX + ipx       ]==0){ py+=pdy*MOVE_SPEED*fps;}
    }
    if(Keys.s==1)                                                                  //move backward
    { 
    if(mapW[ipy*mapX        + ipx_sub_xo]==0){ px-=pdx*MOVE_SPEED*fps;}
    if(mapW[ipy_sub_yo*mapX + ipx       ]==0){ py-=pdy*MOVE_SPEED*fps;}
    } 
    



        
        
        drawPlayer();
        drawSky();
        drawRays2D();
        drawSprites();
        if (DRAW_MINIMAP)
        {
            drawMap2D();
        }
    }
    if (gameState == 3) {screen(2); timer +=1*fps; if (timer>6000) {timer = 0; fade = 0; gameState = 0;}} // Win Screen screen
    if (gameState == 4) {screen(3); timer += 1*fps; if (timer > 6000) {timer = 0; fade = 0; gameState = 0;}}
        glutPostRedisplay();
    glutSwapBuffers();
}

void ButtonDown(unsigned char key, int x, int y)
{
    if (key=='a') {Keys.a=1;}
    if (key=='d') {Keys.d=1;}
    if (key=='w') {Keys.w=1;}
    if (key=='s') {Keys.s=1;}
    if (key=='e' && sp[0].state == 0) {
        printf("%s", "User pressed E");
        int xo=0; if(pdx<0){ xo=-25;} else{ xo=25;}
  int yo=0; if(pdy<0){ yo=-25;} else{ yo=25;} 
  int ipx=px/64.0, ipx_add_xo=(px+xo)/64.0;
  int ipy=py/64.0, ipy_add_yo=(py+yo)/64.0;
  if(mapW[ipy_add_yo*mapX+ipx_add_xo]==4 && sp[0].state == 0){ mapW[ipy_add_yo*mapX+ipx_add_xo]=0;}
        if (mapW[ipy_add_yo*mapX + ipx_add_xo]==5) {fade = 0; timer = 0; gameState = 3; printf("%s", "User Won Game");}
    }
    if (key=='m') {DRAW_MINIMAP = !DRAW_MINIMAP;}
    glutPostRedisplay();
}
void ButtonUp(unsigned char key, int x, int y)
{
    if (key=='a') {Keys.a=0;}
    if (key=='d') {Keys.d=0;}
    if (key=='w') {Keys.w=0;}
    if (key=='s') {Keys.s=0;}
    glutPostRedisplay();
}




void resize(int w,int h)
{
    glutReshapeWindow(WIDTH, HEIGHT);
}



int main(int argc, char* argv[])
{
    // Create the main window.
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(glutGet(GLUT_SCREEN_WIDTH)/2-WIDTH/2,glutGet(GLUT_SCREEN_HEIGHT)/2-HEIGHT/2);
    glutCreateWindow(WINDOW_NAME);
    gluOrtho2D(0, WIDTH, HEIGHT, 0); // Set up window.
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutKeyboardFunc(ButtonDown);
    glutKeyboardUpFunc(ButtonUp);
   
    
    

    glutMainLoop();
}