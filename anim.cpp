////////////////////////////////////////////////////
// anim.cpp version 4.1
// Template code for drawing an articulated figure.
// CS 174A 
////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#ifdef WIN32
#include <windows.h>
#include "GL/glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "GL/freeglut.h"
#elif __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#define glutInitContextVersion(a,b)
#define glutInitContextProfile(a)
#define glewExperimental int glewExperimentalAPPLE
#define glewInit()
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif

#include "Ball.h"
#include "FrameSaver.h"
#include "Timer.h"
#include "Shapes.h"
#include "tga.h"

#include "Angel/Angel.h"

FrameSaver FrSaver ;
Timer TM ;

BallData *Arcball = NULL ;
int Width = 800;
int Height = 800 ;
int Button = -1 ;
float Zoom = 1 ;
int PrevY = 0 ;

int Animate = 0 ;
int Recording = 0 ;

void resetArcball() ;
void save_image();
void instructions();
void set_colour(float r, float g, float b) ;

const int STRLEN = 100;
typedef char STR[STRLEN];

#define PI 3.1415926535897
#define X 0
#define Y 1
#define Z 2

//texture

GLuint texture_cube;
GLuint texture_earth;

// Structs that hold the Vertex Array Object index and number of vertices of each shape.
ShapeData cubeData;
ShapeData sphereData;
ShapeData coneData;
ShapeData cylData;

// Matrix stack that can be used to push and pop the modelview matrix.
class MatrixStack {
    int    _index;
    int    _size;
    mat4*  _matrices;

   public:
    MatrixStack( int numMatrices = 32 ):_index(0), _size(numMatrices)
        { _matrices = new mat4[numMatrices]; }

    ~MatrixStack()
	{ delete[]_matrices; }

    void push( const mat4& m ) {
        assert( _index + 1 < _size );
        _matrices[_index++] = m;
    }

    mat4& pop( void ) {
        assert( _index - 1 >= 0 );
        _index--;
        return _matrices[_index];
    }
};

MatrixStack  mvstack;
mat4         model_view;
GLint        uModelView, uProjection, uView;
GLint        uAmbient, uDiffuse, uSpecular, uLightPos, uShininess;
GLint        uTex, uEnableTex;

// The eye point and look-at point.
// Currently unused. Use to control a camera with LookAt().
Angel::vec4 eye{0, 0.0, 50.0,1.0};
Angel::vec4 ref{0.0, 0.0, 0.0,1.0};
Angel::vec4 up{0.0,1.0,0.0,0.0};

double TIME = 0.0 ;

/////////////////////////////////////////////////////
//    PROC: drawCylinder()
//    DOES: this function 
//          render a solid cylinder  oriented along the Z axis. Both bases are of radius 1. 
//          The bases of the cylinder are placed at Z = 0, and at Z = 1.
//
//          
// Don't change.
//////////////////////////////////////////////////////
void drawCylinder(void)
{
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( cylData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cylData.numVertices );
}

//////////////////////////////////////////////////////
//    PROC: drawCone()
//    DOES: this function 
//          render a solid cone oriented along the Z axis with base radius 1. 
//          The base of the cone is placed at Z = 0, and the top at Z = 1. 
//         
// Don't change.
//////////////////////////////////////////////////////
void drawCone(void)
{
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( coneData.vao );
    glDrawArrays( GL_TRIANGLES, 0, coneData.numVertices );
}


//////////////////////////////////////////////////////
//    PROC: drawCube()
//    DOES: this function draws a cube with dimensions 1,1,1
//          centered around the origin.
// 
// Don't change.
//////////////////////////////////////////////////////

void drawCube(void)
{
    glBindTexture( GL_TEXTURE_2D, texture_cube );
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( cubeData.vao );
    glDrawArrays( GL_TRIANGLES, 0, cubeData.numVertices );
}


//////////////////////////////////////////////////////
//    PROC: drawSphere()
//    DOES: this function draws a sphere with radius 1
//          centered around the origin.
// 
// Don't change.
//////////////////////////////////////////////////////

void drawSphere(void)
{
    glBindTexture( GL_TEXTURE_2D, texture_earth);
    glUniformMatrix4fv( uModelView, 1, GL_TRUE, model_view );
    glBindVertexArray( sphereData.vao );
    glDrawArrays( GL_TRIANGLES, 0, sphereData.numVertices );
}


void resetArcball()
{
    Ball_Init(Arcball);
    Ball_Place(Arcball,qOne,0.75);
}


//////////////////////////////////////////////////////
//    PROC: myKey()
//    DOES: this function gets caled for any keypresses
// 
//////////////////////////////////////////////////////

void myKey(unsigned char key, int x, int y)
{
    float time ;
    switch (key) {
        case 'q':
        case 27:
            exit(0); 
        case 's':
            FrSaver.DumpPPM(Width,Height) ;
            break;
        case 'r':
            resetArcball() ;
            glutPostRedisplay() ;
            break ;
        case 'a': // togle animation
            Animate = 1 - Animate ;
            // reset the timer to point to the current time		
            time = TM.GetElapsedTime() ;
            TM.Reset() ;
            // printf("Elapsed time %f\n", time) ;
            break ;
        case '0':
            //reset your object
            break ;
        case 'm':
            if( Recording == 1 )
            {
                printf("Frame recording disabled.\n") ;
                Recording = 0 ;
            }
            else
            {
                printf("Frame recording enabled.\n") ;
                Recording = 1  ;
            }
            FrSaver.Toggle(Width);
            break ;
        case 'h':
        case '?':
            instructions();
            break;
    }
    (void) time;
    glutPostRedisplay() ;

}

/*********************************************************
    PROC: myinit()
    DOES: performs most of the OpenGL intialization
     -- change these with care, if you must.

**********************************************************/

void myinit(void)
{
    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
    glUseProgram(program);

    // Generate vertex arrays for geometric shapes
    generateCube(program, &cubeData);
    generateSphere(program, &sphereData);
    generateCone(program, &coneData);
    generateCylinder(program, &cylData);

    uModelView  = glGetUniformLocation( program, "ModelView"  );
    uProjection = glGetUniformLocation( program, "Projection" );
    uView       = glGetUniformLocation( program, "View"       );

    glClearColor( 0.1, 0.1, 0.2, 1.0 ); // dark blue background

    uAmbient   = glGetUniformLocation( program, "AmbientProduct"  );
    uDiffuse   = glGetUniformLocation( program, "DiffuseProduct"  );
    uSpecular  = glGetUniformLocation( program, "SpecularProduct" );
    uLightPos  = glGetUniformLocation( program, "LightPosition"   );
    uShininess = glGetUniformLocation( program, "Shininess"       );
    uTex       = glGetUniformLocation( program, "Tex"             );
    uEnableTex = glGetUniformLocation( program, "EnableTex"       );

    glUniform4f(uAmbient,    0.2f,  0.2f,  0.2f, 1.0f);
    glUniform4f(uDiffuse,    0.6f,  0.6f,  0.6f, 1.0f);
    glUniform4f(uSpecular,   0.2f,  0.2f,  0.2f, 1.0f);
    glUniform4f(uLightPos,  15.0f, 15.0f, 30.0f, 0.0f);
    glUniform1f(uShininess, 100.0f);

    glEnable(GL_DEPTH_TEST);
 
    Arcball = new BallData;
    Ball_Init(Arcball);
    Ball_Place(Arcball,qOne,0.75);
}

/*********************************************************
    PROC: set_colour();
    DOES: sets all material properties to the given colour
    -- don't change
**********************************************************/

void set_colour(float r, float g, float b)
{
    float ambient  = 0.2f;
    float diffuse  = 0.6f;
    float specular = 0.2f;
    glUniform4f(uAmbient,  ambient*r,  ambient*g,  ambient*b,  1.0f);
    glUniform4f(uDiffuse,  diffuse*r,  diffuse*g,  diffuse*b,  1.0f);
    glUniform4f(uSpecular, specular*r, specular*g, specular*b, 1.0f);
}

void drawGround(mat4 model_trans, mat4 view_trans){
    mvstack.push(model_trans);

    set_colour(0.0f, 0.8f, 0.0f);
    model_trans *= RotateX(90.0);
    model_trans *= Translate(0, 0, 10);
    model_trans *= Scale(100, 100, 1);
    model_view = view_trans * model_trans;
    drawCylinder();
    
    model_trans = mvstack.pop();
}

const float stem_width  = 0.1f;
const float stem_height = 0.5f;


void drawBulb(mat4 model_trans, mat4 view_trans, int variation) {
    const float fR = 0.125f+0.1*variation;
    const float fG = 0.69f-0.1*variation;
    const float fB = 0.66f+0.1*variation;

    set_colour(fR, fG, fB);

    model_trans *= Scale(1/stem_width, 1/stem_width, 1/(stem_height+0.05*variation));
    model_trans *= Translate(0.0, 0.0, -(stem_height+0.05*variation));
    model_trans *= Scale(1.0+0.05*variation);
    model_view = view_trans * model_trans;
    drawSphere();
}

void drawStemSegments(mat4 model_trans, mat4 view_trans, size_t num_segments, int variation){
    float swayAngle = 4.0*sin(TIME);

    model_trans *= Scale(1/stem_width, 1/stem_width, 1/(stem_height+0.05*variation));
    model_trans *= Translate(0.0, 0.0, -(stem_height+0.05*variation));
    model_trans *= RotateX(swayAngle);
    model_trans *= Translate(0.0, 0.0, -(stem_height+0.05*variation));
    model_trans *= Scale(stem_width, stem_width, stem_height+0.05*variation);
    model_view = view_trans * model_trans;
    drawCylinder();

    if ( num_segments > 1 )
      drawStemSegments(model_trans, view_trans, num_segments-1, variation);
    else
      drawBulb(model_trans, view_trans, variation);
}

void drawStem(mat4 model_trans, mat4 view_trans, size_t num_segments, int variation, float x, float y){
    const float sR = 0.5f+0.01*variation;
    const float sG = 0.25f+0.01*variation;
    const float sB = 0.1f+0.01*variation;
    float swayAngle = 4.0*sin(TIME+variation); 


    set_colour(sR, sG, sB);
    // draw root
    model_trans *= Translate(x, -9.5f, y);
    model_trans *= RotateX(swayAngle);
    model_trans *= Translate(0.0f, stem_height+0.05*variation, 0.0f);
    model_trans *= RotateX(90.0);
    model_trans *= Scale(stem_width, stem_width, stem_height+0.05*variation);
    model_view = view_trans * model_trans;
    drawCylinder();
 
    // draw the rest of the segments
    drawStemSegments(model_trans, view_trans, num_segments-1, variation);
}



void drawFlower(mat4 model_trans, mat4 view_trans, int variation, float x, float y){
    const size_t num_segments = 8;

    mvstack.push(model_trans);
    drawStem(model_trans, view_trans, num_segments, variation, x, y);
    model_trans = mvstack.pop();
}

const float abdomen_radius = 0.5f;
const float abdomen_length = 1.5f;
const float head_radius = 0.5f;
const float thorax_width = 1.0f;
const float thorax_length = 2.0f;
const float leg_width = 0.25f;
const float leg_length = 0.5f;
const float leg_distance = 5.0f;
const float wing_thickness = 0.05f;
const float wing_width = 1.0f;
const float wing_length = 3.0f;
const float wing_distance = 15.0f;

void drawWing(mat4 model_trans, mat4 view_trans){
    const float wR = 0.2f;
    const float wG = 0.2f;
    const float wB = 0.2f;
    const float wing_speed = 150.0f;

    set_colour(wR, wG, wB);
    model_trans *= Translate(thorax_width/2, 0.0f, 0.0f);
    model_trans *= RotateZ(wing_distance * sin(TIME*wing_speed));
    model_trans *= Translate(wing_length/2, 0.0f, 0.0f);
    model_trans *= Scale(wing_length, wing_thickness, wing_width);
    model_view = view_trans * model_trans;
    drawCube();
}

void drawLeg(mat4 model_trans, mat4 view_trans){
    const float lR = 0.2f;
    const float lG = 0.2f;
    const float lB = 0.2f;
    
    set_colour(lR, lG, lB);
    model_trans *= Translate(thorax_width/2, 0.0f, 0.0f);
    model_trans *= RotateZ(leg_distance * sin(TIME) - leg_distance/2);
    model_trans *= Translate(0.0f, -(thorax_width/2+leg_length/2), 0.0f);
    model_trans *= Scale(leg_width, leg_length, leg_width);
    model_view = view_trans * model_trans;
    drawCube();
   
    model_trans *= Scale(1/leg_width, 1/leg_length, 1/leg_width);
 
    model_trans *=RotateZ(leg_distance * sin(TIME) - leg_distance/2);
    model_trans *= Translate(0.0f, -leg_length, 0.0f);
    model_trans *= Scale(leg_width, leg_length, leg_width);
    model_view = view_trans * model_trans;
    drawCube();

    model_trans *= Scale(1/leg_width, 1/leg_length, 1/leg_width);

}

void drawBee(mat4 model_trans, mat4 view_trans){
    const float bee_speed = 50.0f;
    const float bee_radius = 7.0f;
    float bee_height =  1.5f * sin(TIME);

    const float tR = 0.2f;
    const float tG = 0.2f;
    const float tB = 0.2f;
   
    const float hR = 0.01f;
    const float hG = 0.01f;
    const float hB = 0.71f;

    const float aR = 1.0f;
    const float aG = 0.9f;
    const float aB = 0.0f;

    mvstack.push(model_trans);

    //draw abdomen
    set_colour(tR, tG, tB);
    model_trans *= RotateY(bee_speed*TIME);
    model_trans *= Translate(bee_radius, bee_height, 0.0f);
    model_trans *= Scale(thorax_width, thorax_width, thorax_length);
    model_view = view_trans * model_trans;
    drawCube();

    //draw head
    set_colour(hR, hG, hB);
    model_trans *= Scale(1/thorax_width, 1/thorax_width, 1/thorax_length);
    model_trans *= Translate(0.0f, 0.0f, -(thorax_length/2+head_radius));
    model_trans *= Scale(head_radius);
    model_view = view_trans * model_trans;
    drawSphere();

    //draw thorax
    set_colour(aR, aG, aB);
    model_trans *= Scale(1/head_radius);
    model_trans *= Translate(0.0f, 0.0f, (thorax_length+head_radius+abdomen_length));
    model_trans *= Scale(abdomen_radius, abdomen_radius, abdomen_length);
    model_view = view_trans * model_trans;
    drawSphere();

    //draw legs
    model_trans *= Scale(1/abdomen_radius, 1/abdomen_radius, 1/abdomen_length);
    model_trans *= Translate(0.0f, 0.0f, -abdomen_length);

    size_t num_legs = 6;
    size_t total_legs = num_legs;
    while ( num_legs > 0 )
    {
        if ( num_legs == total_legs/2 )
        {
          model_trans *= RotateY(180.0);
          model_trans *= Translate(0.0f, 0.0f, 2*(abdomen_length/total_legs));
        }
        model_trans *= Translate(0.0f, 0.0f, -2*(abdomen_length/total_legs));
        drawLeg(model_trans, view_trans);
        num_legs--;
    }

    model_trans *= Translate(0.0f, 0.0f, ( abdomen_length/2 - abdomen_length/total_legs) );
    drawWing(model_trans, view_trans);
    model_trans *= RotateY(180.0);
    drawWing(model_trans, view_trans);
  
    model_trans = mvstack.pop();


}

/*********************************************************
**********************************************************
**********************************************************

    PROC: display()
    DOES: this gets called by the event handler to draw
          the scene, so this is where you need to build
          your ROBOT --  
      
        MAKE YOUR CHANGES AND ADDITIONS HERE

    Add other procedures if you like.

**********************************************************
**********************************************************
**********************************************************/
void display(void)
{
    // Clear the screen with the background colour (set in myinit)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4 model_trans(1.0f);
    mat4 view_trans(1.0f);

    view_trans *= Translate(0.0f, 0.0f, -15.0f);
    HMatrix r;
    Ball_Value(Arcball,r);

    mat4 mat_arcball_rot(
        r[0][0], r[0][1], r[0][2], r[0][3],
        r[1][0], r[1][1], r[1][2], r[1][3],
        r[2][0], r[2][1], r[2][2], r[2][3],
        r[3][0], r[3][1], r[3][2], r[3][3]);
    view_trans *= mat_arcball_rot;
    view_trans *= Scale(Zoom);
        
    glUniformMatrix4fv( uView, 1, GL_TRUE, model_view );

    drawGround(model_trans, view_trans);

    // main flower
    drawFlower(model_trans, view_trans, 0, 0, 0); 

    // lower grove
    drawFlower(model_trans, view_trans, -10, 4, 3);
    drawFlower(model_trans, view_trans, -7, -4, -3);
    drawFlower(model_trans, view_trans, -5, -7, 3);
    drawFlower(model_trans, view_trans, -4, 7, -3);
    drawFlower(model_trans, view_trans, -2, 5, -5);


    //far grove    
    drawFlower(model_trans, view_trans, 10, 13, 4);
    drawFlower(model_trans, view_trans, 5, 12, -12);
    drawFlower(model_trans, view_trans, 0, -13, 0);
    drawFlower(model_trans, view_trans, 10, -15, 4);
    drawFlower(model_trans, view_trans, 12, 0, -15);
    drawFlower(model_trans, view_trans, 11, 0, 17);
    drawFlower(model_trans, view_trans, 0, 3, -19);
    drawFlower(model_trans, view_trans, 13, 7, 20);
    drawFlower(model_trans, view_trans, 7, 15, -20);
    drawFlower(model_trans, view_trans, 10, 13, 13);
    drawFlower(model_trans, view_trans, 0, 13, -7);
    drawFlower(model_trans, view_trans, 10, -15, -15);
    drawFlower(model_trans, view_trans, 9, -12, -17);
    drawFlower(model_trans, view_trans, 8, -17, -12);
 
    drawBee(model_trans, view_trans);
    
    glutSwapBuffers();
    if(Recording == 1)
        FrSaver.DumpPPM(Width, Height);
}

/**********************************************
    PROC: myReshape()
    DOES: handles the window being resized 
    
      -- don't change
**********************************************************/
void myReshape(int w, int h)
{
    Width = w;
    Height = h;

    glViewport(0, 0, w, h);

    mat4 projection = Perspective(50.0f, (float)w/(float)h, 1.0f, 1000.0f);
    glUniformMatrix4fv( uProjection, 1, GL_TRUE, projection );
}

void instructions() 
{
    printf("Press:\n");
    printf("  s to save the image\n");
    printf("  r to restore the original view.\n") ;
    printf("  0 to set it to the zero state.\n") ;
    printf("  a to toggle the animation.\n") ;
    printf("  m to toggle frame dumping.\n") ;
    printf("  q to quit.\n");
}

// start or end interaction
void myMouseCB(int button, int state, int x, int y)
{
    Button = button ;
    if( Button == GLUT_LEFT_BUTTON && state == GLUT_DOWN )
    {
        HVect arcball_coords;
        arcball_coords.x = 2.0*(float)x/(float)Width-1.0;
        arcball_coords.y = -2.0*(float)y/(float)Height+1.0;
        Ball_Mouse(Arcball, arcball_coords) ;
        Ball_Update(Arcball);
        Ball_BeginDrag(Arcball);

    }
    if( Button == GLUT_LEFT_BUTTON && state == GLUT_UP )
    {
        Ball_EndDrag(Arcball);
        Button = -1 ;
    }
    if( Button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN )
    {
        PrevY = y ;
    }


    // Tell the system to redraw the window
    glutPostRedisplay() ;
}

// interaction (mouse motion)
void myMotionCB(int x, int y)
{
    if( Button == GLUT_LEFT_BUTTON )
    {
        HVect arcball_coords;
        arcball_coords.x = 2.0*(float)x/(float)Width - 1.0 ;
        arcball_coords.y = -2.0*(float)y/(float)Height + 1.0 ;
        Ball_Mouse(Arcball,arcball_coords);
        Ball_Update(Arcball);
        glutPostRedisplay() ;
    }
    else if( Button == GLUT_RIGHT_BUTTON )
    {
        if( y - PrevY > 0 )
            Zoom  = Zoom * 1.03 ;
        else 
            Zoom  = Zoom * 0.97 ;
        PrevY = y ;
        glutPostRedisplay() ;
    }
}


void idleCB(void)
{
    if( Animate == 1 )
    {
        // TM.Reset() ; // commenting out this will make the time run from 0
        // leaving 'Time' counts the time interval between successive calls to idleCB
        if( Recording == 0 )
            TIME = TM.GetElapsedTime() ;
        else
            TIME += 0.033 ; // save at 30 frames per second.
        
        eye.x = 20*sin(TIME);
        eye.z = 20*cos(TIME);
        
        printf("TIME %f\n", TIME) ;
        glutPostRedisplay() ; 
    }
}
/*********************************************************
     PROC: main()
     DOES: calls initialization, then hands over control
           to the event handler, which calls 
           display() whenever the screen needs to be redrawn
**********************************************************/

int main(int argc, char** argv) 
{
    glutInit(&argc, argv);
    // If your code fails to run, uncommenting these lines may help.
    //glutInitContextVersion(3, 2);
    //glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Width,Height);
    glutCreateWindow(argv[0]);
    glutSetWindowTitle("OpenGL Bee Demo");
    printf("GL version %s\n", glGetString(GL_VERSION));
    glewExperimental = GL_TRUE;
    glewInit();
    
    myinit();

    glutIdleFunc(idleCB) ;
    glutReshapeFunc (myReshape);
    glutKeyboardFunc( myKey );
    glutMouseFunc(myMouseCB) ;
    glutMotionFunc(myMotionCB) ;
    instructions();

    glutDisplayFunc(display);
    glutMainLoop();

    TM.Reset() ;
    return 0;         // never reached
}




