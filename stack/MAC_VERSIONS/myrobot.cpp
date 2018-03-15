
#include "Angel.h"
#include <assert.h>
#include <iostream>
#include <stdlib.h>
#include<string.h>

typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)
const int NumNodes = 10;
const int NumAngles = 10;

bool top_view = false;
double old_x, old_y, old_z, new_x, new_y, new_z;
float diff_x, diff_y, diff_z = 0.0;


//GLfloat left= -1.0, right=1.0, top=1.0, botto= -1.0, near= 0.5, far=3.0;


point4 at = vec4(0.0, 0.0, 0.0, 0.0);
point4 eye = vec4(0.0, 0.0, 0.0, 0.0);
vec4 up = vec4(0.0, 0.0, 0.0, 0.0);

float theta_angle = 0.0;
float phi = 0.0;
float radius = 1.0;


point4 points[NumVertices];
color4 colors[NumVertices];

point4 vertices[8] = {
    point4( -0.5, -0.5, 0.5, 1.0 ),
    point4( -0.5, 0.5, 0.5, 1.0 ),
    point4( 0.5, 0.5, 0.5, 1.0 ),
    point4( 0.5, -0.5, 0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5, 0.5, -0.5, 1.0 ),
    point4( 0.5, 0.5, -0.5, 1.0 ),
    point4( 0.5, -0.5, -0.5, 1.0 )
};

// RGBA olors
color4 vertex_colors[8] = {
    color4( 0.0, 0.0, 0.0, 1.0 ),  // black
    color4( 1.0, 0.0, 0.0, 1.0 ),  // red
    color4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    color4( 0.0, 1.0, 0.0, 1.0 ),  // green
    color4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    color4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    color4( 1.0, 1.0, 1.0, 1.0 ),  // white
    color4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};

//----------------------------------------------------------------------------

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
mat4 projection;
GLuint       ModelView, Projection;


//----------------------------------------------------------------------------

#define TORSO_HEIGHT 5.0
#define TORSO_WIDTH 1.0
#define LOWER_LEG_WIDTH  1.5
#define LOWER_LEG_HEIGHT 2.0
#define UPPER_LEG_HEIGHT 3.0
#define UPPER_LEG_WIDTH  1.5
#define HEAD_HEIGHT 2.0
#define HEAD_WIDTH 2.0
#define BOTTOM_HEIGHT 1.0
#define BOTTOM_WIDTH 3.0

// Set up menu item indices, which we can alos use with the joint angles
enum {
    Torso = 0,
    Head  = 1,
    Head1 = 1,
    Head2 = 2,
    UpperLeg = 3,
    LowerLeg = 4,
    Bottom = 5,
    Quit
};

// Joint angles with initial values
GLfloat
theta[NumAngles] = {
    0.0,    // Torso
    0.0,    // Head1
    0.0,    // Head2
    0.0,  // LeftUpperLeg
    0.0,     // LeftLowerLeg
    0.0    // RightLowerLeg
};

GLint angle = Head2;

//----------------------------------------------------------------------------

struct Node {
    mat4  transform;
    void  (*render)( void );
    Node* sibling;
    Node* child;

    Node() :
	render(NULL), sibling(NULL), child(NULL) {}

    Node( mat4& m, void (*render)( void ), Node* sibling, Node* child ) :
	transform(m), render(render), sibling(sibling), child(child) {}
};

Node  nodes[NumNodes];

//----------------------------------------------------------------------------

int Index = 0;

void
quad( int a, int b, int c, int d )
{
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[b]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[d]; Index++;
}

void
colorcube( void )
{
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 7, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
}

//----------------------------------------------------------------------------

void
traverse( Node* node )
{
    if ( node == NULL ) { return; }

     mvstack.push( model_view );

    model_view *= node->transform;
    node->render();

      traverse( node->child );
    if ( node->child != NULL) { traverse( node->child ); }

    model_view = mvstack.pop();

     traverse( node->sibling );
    if ( node->sibling != NULL) { traverse( node->sibling ); }
}

//----------------------------------------------------------------------------

void
torso()
{
    mvstack.push( model_view );

    mat4 instance = (Translate( 0.0, 0.5 * HEAD_HEIGHT, 0.0 ) *
		     Scale( HEAD_WIDTH, HEAD_HEIGHT, HEAD_WIDTH ) );

/*    mat4 instance = ( Translate( 0.0, 0.5 * TORSO_HEIGHT, 0.0 ) *
		      Scale( TORSO_WIDTH, TORSO_HEIGHT, TORSO_WIDTH ) );
   */
    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );

    model_view = mvstack.pop();
}

void
head()
{
    mvstack.push( model_view );

    mat4 instance = (Translate( 0.0, 0.5 * HEAD_HEIGHT, 0.0 ) *
		     Scale( HEAD_WIDTH, HEAD_HEIGHT, HEAD_WIDTH ) );

    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );

    model_view = mvstack.pop();
}

/*void sphere(){
    mat4 instance = Scale(1, 1, 1);

    if (isStick)
        fix_view = model_view;

    glUniformMatrix4fv( ModelView, 1, GL_TRUE,  fix_view * instance );
    glDrawArrays( GL_TRIANGLE_STRIP, 0, NumPoints2 );
}

void sphere_strip(){
    mat4 instance = Scale(1, 1, 1);

    if (isStick)
        fix_view = model_view;

    glUniformMatrix4fv( ModelView, 1, GL_TRUE, fix_view * instance );
    glDrawArrays( GL_TRIANGLE_FAN, 0, NumPoints1 );
}*/




void
upper_leg()
{
    mvstack.push( model_view );

    mat4 instance = ( Translate( 0.0, 0.5 * UPPER_LEG_HEIGHT, 0.0 ) *
		      Scale( UPPER_LEG_WIDTH,
			     UPPER_LEG_HEIGHT,
			     UPPER_LEG_WIDTH ) );

    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );

    model_view = mvstack.pop();
}

void
lower_leg()
{
    mvstack.push( model_view );

    mat4 instance = (Translate( 0.0, 0.5 * LOWER_LEG_HEIGHT, 0.0 ) *
		     Scale( LOWER_LEG_WIDTH,
			    LOWER_LEG_HEIGHT,
			    LOWER_LEG_WIDTH ) );

    glUniformMatrix4fv( ModelView, 1, GL_TRUE, model_view * instance );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );

    model_view = mvstack.pop();
}

void
bottom()
{
    mvstack.push( model_view );
    mat4 instance = ( Translate( 0.0, 0.5 * BOTTOM_HEIGHT, 0.0 ) *
                 Scale( BOTTOM_WIDTH,BOTTOM_HEIGHT,BOTTOM_WIDTH ) );

     glUniformMatrix4fv( ModelView, 1, GL_TRUE,  model_view * instance );

     glDrawArrays( GL_TRIANGLES, 0, NumVertices );

    model_view = mvstack.pop();
}


//----------------------------------------------------------------------------

void
display()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    traverse( &nodes[Bottom] );
    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
mouse( int button, int state, int x, int y )
{
    if ( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN ) {
        theta[angle] += 5.0;
        if ( theta[angle] > 360.0 ) { theta[angle] -= 360.0; }
    }

    if ( button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN ) {
        theta[angle] -= 5.0;
        if ( theta[angle] < 0.0 ) { theta[angle] += 360.0; }
    }

    //mvstack.push( model_view );

    switch( angle ) {
	case Torso:
	    nodes[Torso].transform =
		RotateY( theta[Torso] );
	    break;

	case Head1: case Head2:
	    nodes[Head].transform =
		Translate(0.0, TORSO_HEIGHT+0.5*HEAD_HEIGHT, 0.0) *
		RotateX(theta[Head1]) *
		RotateY(theta[Head2]) *
		Translate(0.0, -0.5*HEAD_HEIGHT, 0.0);
	    break;

	case UpperLeg:
	    nodes[UpperLeg].transform =
		Translate(-(TORSO_WIDTH+UPPER_LEG_WIDTH),
			  0.1*UPPER_LEG_HEIGHT, 0.0) *
		RotateX(theta[UpperLeg]);
	    break;

	case LowerLeg:
	    nodes[LowerLeg].transform =
		Translate(0.0, UPPER_LEG_HEIGHT, 0.0) *
		RotateX(theta[LowerLeg]);
	    break;

	case Bottom:
	    nodes[Bottom].transform =
		Translate(0.0,BOTTOM_HEIGHT, 0.0) *
		RotateX(theta[Bottom]);
	    break;
    }

    // model_view = mvstack.pop();
    glutPostRedisplay();
}


void addNew( float diff_x, float diff_y, float diff_z )
{
	    nodes[Head].transform =
		Translate(diff_x, diff_y , diff_z) * RotateX(theta[Head1]) *RotateY(theta[Head2]);
    // model_view = mvstack.pop();
    glutPostRedisplay();
}



//----------------------------------------------------------------------------

void
menu( int option )
{
    if ( option == Quit ) {
	exit( EXIT_SUCCESS );
    }

    angle = option;
}

//----------------------------------------------------------------------------

void
reshape( int width, int height )
{
    glViewport( 0, 0, width, height );

    GLfloat left = -10.0, right = 10.0;
    GLfloat botto = -15.0, top = 15.0;
    GLfloat zNear = -10.0, zFar = 10.0;

    GLfloat aspect = GLfloat( width ) / height;

    if ( aspect > 1.0 ) {
        left *= aspect;
        right *= aspect;
    }
    else {
        botto /= aspect;
        top /= aspect;
    }

    mat4 projection = Ortho( left, right, botto, top, zNear, zFar );

    glUniformMatrix4fv( Projection, 1, GL_TRUE, projection );

    if(top_view)
    {
         at = vec4(0.0, 0.0, 0.0, 1.0);
         eye = vec4(2.0, 2.0, 2.0, 1.0);
         up = vec4(0.0, 1.0, 0.0, 0.0);
         model_view = LookAt(eye,at, up);
    }
    else {
         at = vec4(0.0, 0.0, 0.0, 1.0);
         eye = vec4(0.0, 0.5, 0.5, 1.0);
         up = vec4(1.0, 0.0, 0.0, 0.0);
         model_view = LookAt(eye,at, up);
    }
}

//----------------------------------------------------------------------------

void
initNodes( void )
{
    mat4  m;
   /*
    m = RotateY( theta[Torso] );
    nodes[Torso] = Node( m, torso, NULL, &nodes[Head1] );
*/
    m = Translate(0.0, 0.0, 0.0) * RotateX(theta[Bottom]);
    nodes[Bottom] = Node( m, bottom, NULL, &nodes[LowerLeg]);
    m = Translate(0.0, BOTTOM_HEIGHT, 0.0) * RotateX(30);
    nodes[LowerLeg] = Node( m, lower_leg, NULL, &nodes[UpperLeg]);
    m = Translate(0.0, LOWER_LEG_HEIGHT, 0.0) *RotateX(30);
    nodes[UpperLeg] = Node( m, upper_leg, NULL,  &nodes[Head] );


    m = RotateX(theta[Head1]) * RotateY(theta[Head2]) *
        Translate(0.0, UPPER_LEG_HEIGHT, 0.0);
    nodes[Head1] = Node( m, head, NULL, NULL);



}

//----------------------------------------------------------------------------

void
init( void )
{
    colorcube();

    // Initialize tree
    initNodes();

    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
                  NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors),
                     colors );

    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader83.glsl", "fshader83.glsl" );
    glUseProgram( program );

    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(0) );

    GLuint vColor = glGetAttribLocation( program, "vColor" );
    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0,
			   BUFFER_OFFSET(sizeof(points)) );

    ModelView = glGetUniformLocation( program, "ModelView" );
    Projection = glGetUniformLocation( program, "Projection" );

    glEnable( GL_DEPTH_TEST );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

    glClearColor( 1.0, 1.0, 1.0, 1.0 );

}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
	case 033: // Escape Key
    case 'a':
        addNew(diff_x,diff_y,diff_z); //move to new x, y, z position
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;
    }
}

//----------------------------------------------------------------------------

int
main( int argc, char **argv )
{
    if(argc < 7)
    {
        std::cout<<"There are at least 7 inputs!"<<std::endl;
        return -1;
    }

    old_x = atof(argv[1]);
    old_y = atof(argv[2]);
    old_z = atof(argv[3]);

    new_x = atof(argv[4]);
    new_y = atof(argv[5]);
    new_z = atof(argv[6]);

    if(std::string(argv[7]) == "tv")
        top_view = true;


    std::cout<<"The top view is:"<<argv[7]<<std::endl;



    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
    glutCreateWindow( "robot" );
    glewExperimental = GL_TRUE;
    glewInit();



    init();

    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutKeyboardFunc( keyboard );
    glutMouseFunc( mouse );

    glutCreateMenu( menu );
    glutAddMenuEntry( "head1", Head1 );
    glutAddMenuEntry( "head2", Head2 );
    glutAddMenuEntry( "upper_leg", UpperLeg );
    glutAddMenuEntry( "lower_leg", LowerLeg );
    glutAddMenuEntry( "bottom", Bottom );
    glutAddMenuEntry( "quit", Quit );
    glutAttachMenu( GLUT_MIDDLE_BUTTON );

    glutMainLoop();
    return 0;
}
