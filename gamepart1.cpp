#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include<stdio.h>
#include <time.h>
#include <stdlib.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

    // Link the program
    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
            0,                  // attribute 0. Vertices
            3,                  // size (x,y,z)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
            1,                  // attribute 1. Color
            3,                  // size (r,g,b)
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
            );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/
float px=-7.5,py=-10,pz=6.5,x=1.4,y=1.9,z=6.5,new_time=0,last_update=-1, old_time=0, xx=0.6, yy=0.6,zz=0.6 ;
float triangle_rot_dir = 1, zcor=0;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
int jumpleft=0,jumpright=0,jumpup=0,jumpdown=0,fastflag=0,intpx,intpy,zflag, count=0,rdup,flagplayer=1,plmoveflag=0,flagvisibility=0, ztra[11][11]={0}, visi[11][11]={0};
double last_updated_time , current_time;
float  xa=2, ya=-10, za=6, xb=-5, yb=3, zb=-6, xc=0, yc=0,zc=1;

            int winflag=0,levelleria=2,input;
//int time=0;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
                //         case GLFW_KEY_P:
                //           triangle_rot_status = !triangle_rot_status;
                //         break;
            case GLFW_KEY_X:
                // do something ..
                break;
            case GLFW_KEY_UP:
                plmoveflag=0;
                break;
            case GLFW_KEY_DOWN:
                plmoveflag=0;
                break;
            case GLFW_KEY_RIGHT:
                plmoveflag=0;
                break;
            case GLFW_KEY_LEFT:
                plmoveflag=0;
                break;

            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            case GLFW_KEY_SPACE:
                if(glfwGetKey(window, GLFW_KEY_LEFT))
                    jumpleft=1;
                else if (glfwGetKey(window, GLFW_KEY_RIGHT))
                    jumpright=1;
                else if(glfwGetKey(window, GLFW_KEY_UP))
                    jumpup=1;
                else if(glfwGetKey(window, GLFW_KEY_DOWN))
                    jumpdown=1;
                break;
            case GLFW_KEY_UP:
                if(flagplayer!=0 )
                {
                    plmoveflag=1;

                }
                break;
            case GLFW_KEY_DOWN:
                if(flagplayer!=0)
                {
                    plmoveflag=-1;}
                break;
            case GLFW_KEY_RIGHT:
                if(flagplayer!=0)
                {
                    plmoveflag=2;

                }
                break;
            case GLFW_KEY_LEFT:
                if(flagplayer!=0 )
                {
                    plmoveflag=-2;}
                break;
            default:
                break;
        }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key) {
        case 'Q':
        case 'q':
            quit(window);
            break;
        case 'r':
            flagplayer=1;
            px=-6.75;
            py=-9;
            pz=6.5;
            break;
        case 'a':
            xa=2;
            ya=-10;
            za=7;
            xb=-5;
            yb=3;
            zb=-7;
            xc=0;
            yc=0;
            zc=1;
            break;
        case 't':
            xa=0;
            ya=0;
            za=10;
            xb=0;
            yb=0;
            zb=-10;
            xc=0;
            yc=1;
            zc=0;
            break;
        case 'p':
            xa=px;
            ya=py;
            za=pz;
            xb=-px;
            yb=-py;
            zb=-pz;
            xc=0;
            yc=0;
            zc=1;
            break;
        case 'b':
            xa=px-0.2;
            ya=py-0.2;
            za=pz-0.2;
            xb=2;
            yb=2;
            zb=1;
            xc=0;
            yc=0;
            zc=1;
            break;
        case 'f':
            fastflag+=1;
            break;
        case 's':
            fastflag-=1;
            break;
        default:
            break;
    }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
                triangle_rot_dir *= -1;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
            }
            break;
        default:
            break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
       is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

    GLfloat fov = 90.0f;

    // sets the viewport of openGL renderer
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

    // set the projection matrix as perspective
    /* glMatrixMode (GL_PROJECTION);
       glLoadIdentity ();
       gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
    // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-11.0f, 15.0f, -11.0f, 16.0f, -24.0f, 24.0f);
}

VAO *queen,*triangle, *rectangle, *cube, *player, *cubegrid[11][11];

// Creates the triangle object used in this sample code
void createTriangle ()
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

    /* Define vertex array as used in glBegin (GL_TRIANGLES) */
    static const GLfloat vertex_buffer_data [] = {
        0, 1,0, // vertex 0
        -1,-1,0, // vertex 1
        1,-1,0, // vertex 2
    };

    static const GLfloat color_buffer_data [] = {
        1,0,0, // color 0
        0,1,0, // color 1
        0,0,1, // color 2
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}

void createCube ()
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

    /* Define vertex array as used in glBegin (GL_TRIANGLES) */
    static const GLfloat vertex_buffer_data [] = {

        //front face
        0,0,0,
        0,0,z,
        x,0,z,

        x,0,z,
        x,0,0,
        0,0,0,

        //backface
        0,y,0,
        0,y,z,
        x,y,z,

        x,y,z,
        x,y,0,
        0,y,0,

        //rightface
        x,0,0,
        x,0,z,
        x,y,z,

        x,y,z,
        x,y,0,
        x,0,0,

        //leftface
        0,y,0,
        0,y,z,
        0,0,z,

        0,0,z,
        0,0,0,
        0,y,0,

        //downface
        0,0,0,
        0,y,0,
        x,y,0,

        x,y,0,
        x,0,0,
        0,0,0,

        //upface
        0,0,z,
        0,y,z,
        x,y,z,

        x,y,z,
        x,0,z,
        0,0,z



    };

    static const GLfloat color_buffer_data [] = {
        0.8,0,0, // color 1
        0.8,0,0, // color 2
        0.8,0,0, // color 3

        0.6,0,0, // color 3
        0.6,0,0, // color 4
        0.6,0,0,  // color 1

        1,0,0, // color 1
        1,0,0, // color 2
        1,0,0, // color 3

        0.8,0,0, // color 3
        0.8,0,0, // color 4
        0.8,0,0,  // color 1


        0.9,0,0, // color 1
        0.9,0,0, // color 2
        0.9,0,0, // color 3

        0.9,0,0, // color 3
        0.9,0,0, // color 4
        0.9,0,0,  // color 1

        1,0,0, // color 1
        1 ,0,0, // color 2
        1,0,1, // color 3

        1,0,0, // color 3
        1,0,0, // color 4
        1,0,0,  // color 1

        1,0,0, // color 1
        1,0,0, // color 2
        1,0,0, // color 3

        1,0,0, // color 3
        1,0,0, // color 4
        1,0,0,  // color 1

        1,0,0, // color 1
        1,0,0, // color 2
        1,0,0, // color 3

        1,0,0, // color 3
        1,0,0, // color 4
        1,0,0  // color 1

    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    for(int ppp=0;ppp<10;ppp++)
        for(int qqq=0;qqq<10;qqq++)
        {
            cubegrid[ppp][qqq] = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
        }
}    
void createPlayers ()
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

    /* Define vertex array as used in glBegin (GL_TRIANGLES) */
    static const GLfloat vertex_buffer_data [] = {

        //front face
        0,0,0,
        0,0,zz,
        xx,0,zz,

        xx,0,zz,
        xx,0,0,
        0,0,0,

        //backface
        0,yy,0,
        0,yy,zz,
        xx,yy,zz,

        xx,yy,zz,
        xx,yy,0,
        0,yy,0,

        //rightface
        xx,0,0,
        xx,0,zz,
        xx,yy,zz,

        xx,yy,zz,
        xx,yy,0,
        xx,0,0,

        //leftface
        0,yy,0,
        0,yy,zz,
        0,0,zz,

        0,0,zz,
        0,0,0,
        0,yy,0,

        //downface
        0,0,0,
        0,yy,0,
        xx,yy,0,

        xx,yy,0,
        xx,0,0,
        0,0,0,

        //upface
        0,0,zz,
        0,yy,zz,
        xx,yy,zz,

        xx,yy,zz,
        xx,0,zz,
        0,0,zz
    };

    static const GLfloat color_buffer_data [] = {
        0.1,0,0, // color 1
        0.1,0,0, // color 2
        0.1,0,0, // color 3

        0.1,0,0, // color 3
        0.3,0.3,0.3, // color 4
        0.1,0,0,  // color 1

        0,0,0, // color 1
        0,0,0, // color 2
        0,0,0, // color 3

        0,0,0, // color 3
        0.3,0.3,0.3, // color 4
        0,0,0,  // color 1


        0,0,0, // color 1
        0,0,0, // color 2
        0,0,0, // color 3

        0,0,0, // color 3
        0.3,0.3,0.3, // color 4
        0,0,0,  // color 1

        0,0,0, // color 1
        0,0,0, // color 2
        0,0,0, // color 3

        0,0,0, // color 3
        0.3,0.3,0.3, // color 4
        0,0,0,  // color 1

        0,0,0, // color 1
        0,0,0, // color 2
        0,0,0, // color 3

        0,0,0, // color 3
        0.3,0.3,0.3, // color 4
        0,0,0,  // color 1

        0,0,0, // color 1
        0,0,0, // color 2
        0,0,0, // color 3

        0,0,0, // color 3
        0.3,0.3,0.3, // color 4
        0,0,0  // color 1

    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    player = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createQueen ()
{
    /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

    /* Define vertex array as used in glBegin (GL_TRIANGLES) */
    static const GLfloat vertex_buffer_data [] = {

        //front face
        0,0,0,
        0,0,zz,
        xx,0,zz,

        xx,0,zz,
        xx,0,0,
        0,0,0,

        //backface
        0,yy,0,
        0,yy,zz,
        xx,yy,zz,

        xx,yy,zz,
        xx,yy,0,
        0,yy,0,

        //rightface
        xx,0,0,
        xx,0,zz,
        xx,yy,zz,

        xx,yy,zz,
        xx,yy,0,
        xx,0,0,

        //leftface
        0,yy,0,
        0,yy,zz,
        0,0,zz,

        0,0,zz,
        0,0,0,
        0,yy,0,

        //downface
        0,0,0,
        0,yy,0,
        xx,yy,0,

        xx,yy,0,
        xx,0,0,
        0,0,0,

        //upface
        0,0,zz,
        0,yy,zz,
        xx,yy,zz,

        xx,yy,zz,
        xx,0,zz,
        0,0,zz
    };

    static const GLfloat color_buffer_data [] = {
        1,1,1, // color 1
        1,1,1, // color 2
        1,1,1, // color 3

        1,1,1, // color 3
        1,1,1, // color 4
        1,1,1,  // color 1

        1,1,1, // color 1
        1,1,1, // color 2
        1,1,1, // color 3
        
        1,1,1, // color 1
        1,1,1, // color 2
        1,1,1, // color 3

        1,1,1, // color 3
        1,1,1, // color 4
        1,1,1,  // color 1

        1,1,1, // color 1
        1,1,1, // color 2
        1,1,1, // color 3

        1,1,1, // color 1
        1,1,1, // color 2
        1,1,1, // color 3

        1,1,1, // color 3
        1,1,1, // color 4
        1,1,1,  // color 1

        1,1,1, // color 1
        1,1,1, // color 2
        1,1,1, // color 3
        
        1,1,1, // color 1
        1,1,1, // color 2
        1,1,1, // color 3

        1,1,1, // color 3
        1,1,1, // color 4
        1,1,1,  // color 1

        1,1,1, // color 1
        1,1,1, // color 2
        1,1,1, // color 3

    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    queen = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
}
// Creates the rectangle object used in this sample code
void createRectangle ()
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
        -1.2,-1,0, // vertex 1
        1.2,-1,0, // vertex 2
        1.2, 1,0, // vertex 3

        1.2, 1,0, // vertex 3
        -1.2, 1,0, // vertex 4
        -1.2,-1,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
        1,0,0, // color 1
        0,0,1, // color 2
        0,1,0, // color 3

        0,1,0, // color 3
        0.3,0.3,0.3, // color 4
        1,0,0  // color 1
    };

    // create3DObject creates and returns a handle to a VAO that can be used later
    rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

float queen_rotation=0;

void die()
{
    flagplayer=0;
}



    /* Render the scene with openGL */
    /* Edit this function according to your assignment */
    void draw ()
    {
        // clear the color and depth in the frame buffer
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // use the loaded shader program
        // Don't change unless you know what you are doing
        glUseProgram (programID);

        // Eye - Location of camera. Don't change unless you are sure!!
        glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
        // Target - Where is the camera looking at.  Don't change unless you are sure!!
        glm::vec3 target (0, 0, 0);
        // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
        glm::vec3 up (0, 1, 0);

        // Compute Camera matrix (view)
        // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
        //  Don't change unless you are sure!!
        Matrices.view = glm::lookAt(glm::vec3(xa,ya,za), glm::vec3(xb,yb,zb), glm::vec3(xc,yc,zc)); // Fixed camera for 2D (ortho) in XY plane

        // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
        //  Don't change unless you are sure!!
        glm::mat4 VP = Matrices.projection * Matrices.view;

        // Send our transformation to the currently bound shader, in the "MVP" uniform
        // For each model you render, since the MVP will be different (at least the M part)
        //  Don't change unless you are sure!!
        glm::mat4 MVP;	// MVP = Projection * View * Model

        // Load identity to model matrix
        Matrices.model = glm::mat4(1.0f);

        /* Render your scene */

        glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
        glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,1,1));  // rotate about vector (1,0,0)
        glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
        Matrices.model *= triangleTransform; 
        MVP = VP * Matrices.model; // MVP = p * V * M

        //  Don't change unless you are sure!!
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // draw3DObject draws the VAO given to it using current MVP matrix
        // draw3DObject(triangle);

        // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
        // glPopMatrix ();
        Matrices.model = glm::mat4(1.0f);

        glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
        glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
        Matrices.model *= (translateRectangle * rotateRectangle);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // draw3DObject draws the VAO given to it using current MVP matrix
        // draw3DObject(rectangle);

        // Increment angles
       // float increments = 1;

        //camera_rotation_angle++; // Simulating camera rotation
       // triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
       // rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;


        intpx=px;
        intpy=py;
        current_time= glfwGetTime();
        if(current_time-last_updated_time>7)
        {
            for(int tryi=0;tryi<10;tryi++)
                for(int tryj=0;tryj<10;tryj++)
                    visi[tryi][tryj]=0;

            for(int pp=0;pp<10;pp++)
            { //  for(int qq=0;qq<10;qq++)

                srand(time(NULL));
                int r = rand()%10;
                r=r*pp*pp*pp%10;
                //    int rdup = r*r*pp*pp%10;
                if((pp==0 && r==0) || pp+r==18 || ztra[pp][r]==1 ||(((pp*1.5)-7.5)==intpx && ((r*2)-10)==intpy))
                    int mm;
                else
                    visi[pp][r]=1;
                //          if((pp==0 && (rdup*2)%10==0) || pp+(rdup*2)%10==18 )
                //            int mnm;
                //      else
                //        visi[pp][(rdup*2)%10]=1;
            }


            last_updated_time=current_time;
        }



        if(zcor>4)
            zflag=0;
        else if(zcor<=0)
        {
            //        printf("zcor0\n");
            for(int tryi=0;tryi<10;tryi++)
                for(int tryj=0;tryj<10;tryj++)
                    ztra[tryi][tryj]=0;


            for(int tryi=0;tryi<10;tryi+=levelleria)
            {
                srand(time(NULL));
                int rdup=rand()%10;
                int rdup2=rand()%10;
                rdup = rdup*tryi%10;


                if((tryi==0 && rdup==0) || tryi+rdup==18 || visi[tryi][rdup]==1)
                    int mm;
                else
                    ztra[tryi][rdup]=1;


            }
            zflag=1;
        }
        //  else 
        //    zcor=0;



        if(zflag==1)
        {
            //      printf("zglag1\n");
            zcor+=0.02;
            //    count++;
        }
        else if(zflag==0)
        {
            zcor-=0.02;
            //  count--;
        }








        for(int i=0;i<10;i++)
        {
            for(int j=0;j<10;j++)
            {

                Matrices.model = glm::mat4(1.0f);

                glm::mat4 translateCube = glm::translate (glm::vec3((i*1.5)-7.5, (j*2)-10, zcor*ztra[i][j]));        // glTranslatef
                //  glm::mat4 rotateCube = glm::rotate((float)(60*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
                Matrices.model *= (translateCube);//* rotateCube);
                MVP = VP * Matrices.model;
                glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

                // draw3DObject draws the VAO given to it using current MVP matrix


                if(visi[i][j]==0)
                    draw3DObject(cubegrid[i][j]);


                // Increment angles
                // float increments = 1;

                //camera_rotation_angle++; // Simulating camera rotation
                // triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
                // rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;

            }

        }


        if(py<9.5 && plmoveflag==1 )
        {
            if(fastflag<=0)
                py+=0.1;
            else
                py+=0.2; 

            if(jumpup==1 && py <6)
            {
                py+=3;
                jumpup=0;
            }

        }

        if(py>-10 && plmoveflag==-1)
        {
            if(fastflag<=0)
                py-=0.1;
            else
                py-=0.2;

            if(jumpdown==1 && py >-6)
            {
                py-=3;
                jumpdown=0;
            }
        }
        if(px<7 && plmoveflag==2)
        {
            if(fastflag<=0)
                px+=0.1;
            else
                px+=0.2;
            if(jumpright==1 && px <4)
            {
                px+=3;
                jumpright=0;
            }

        }
        if(px>-7.5 && plmoveflag==-2 )
        {
            if(fastflag<=0)
                px-=0.1;
            else
                px-=0.2;
            if(jumpleft==1 && px >-4)
            {
                px-=3;
                jumpleft=0;
            }
        }



        for(int ii=0;ii<10;ii++)
            for(int jj=0;jj<10;jj++)
            {
                if(visi[ii][jj]==1 )
                {
                    float xii= (ii*1.5)-7.5;
                    float yii = (jj*2)-10;
                    if(px>xii && px < xii+1.5 && py >yii && py <yii +2)
                        die();
                }
                if(ztra[ii][jj]==1)
                {
                    float xii= (ii*1.5)-7.5+0.75;
                    float yii = (jj*2)-10+0.1;
                    if(abs(px-xii) < 1.25 && abs(py-yii)<1.5 && px<xii)
                        px=xii-1.25;
                    else if(abs(px-xii)<1.25 && abs(py-yii)<1.5 && px>xii)
                        px=xii+1.25;

                    if(abs(py-yii)<1.5 && abs(px-xii)<1 && py < yii)
                        py=yii-1.5;
                    else if(abs(py-yii)<1.5 && abs(px-xii)<1 && py > yii)
                        py=yii+1.5;
                }


            }




        Matrices.model = glm::mat4(1.0f);
       
            float kk=pz;
        if(flagplayer==0)
        {
            

            if(kk>0)
            kk-=0.1; 
            pz=kk;
        }



            glm::mat4 translatePlayers = glm::translate (glm::vec3(px, py, pz));   
      /* else if(flagplayer==0) 
                {
            float kk=pz;
            if(kk>0)
            kk-=0.1; 

        glm::mat4 translatePlayers = glm::translate (glm::vec3(px, py, kk));        
        }*/
        Matrices.model *= (translatePlayers);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // draw3DObject draws the VAO given to it using current MVP matrix
        draw3DObject(player);

        // Increment angles
        //  float increments = 1;

        //camera_rotation_angle++; // Simulating camera rotation
        // triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
        // rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;

       
        Matrices.model = glm::mat4(1.0f);
       

            glm::mat4 translateQueen = glm::translate (glm::vec3(6.5, 9, 6.5));   
      /* else if(flagplayer==0) 
                {
            float kk=pz;
            if(kk>0)
            kk-=0.1; 

        glm::mat4 translatePlayers = glm::translate (glm::vec3(px, py, kk));        
        }*/
         
                  glm::mat4 rotateQueen = glm::rotate((float)(queen_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
        Matrices.model *= (translateQueen*rotateQueen);
        MVP = VP * Matrices.model;
        glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // draw3DObject draws the VAO given to it using current MVP matrix
      if(winflag==0)
        draw3DObject(queen);

        // Increment angles
          float increments = 5;

        //camera_rotation_angle++; // Simulating camera rotation
        // triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
         queen_rotation = queen_rotation + increments;

    }

    /* Initialise glfw window, I/O callbacks and the renderer to use */
    /* Nothing to Edit here */
    GLFWwindow* initGLFW (int width, int height)
    {
        GLFWwindow* window; // window desciptor/handle

        glfwSetErrorCallback(error_callback);
        if (!glfwInit()) {
            exit(EXIT_FAILURE);
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

        if (!window) {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        glfwMakeContextCurrent(window);
        gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
        glfwSwapInterval( 1 );

        /* --- register callbacks with GLFW --- */

        /* Register function to handle window resizes */
        /* With Retina display on Mac OS X GLFW's FramebufferSize
           is different from WindowSize */
        glfwSetFramebufferSizeCallback(window, reshapeWindow);
        glfwSetWindowSizeCallback(window, reshapeWindow);

        /* Register function to handle window close */
        glfwSetWindowCloseCallback(window, quit);

        /* Register function to handle keyboard input */
        glfwSetKeyCallback(window, keyboard);      // general keyboard input
        glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

        /* Register function to handle mouse click */
        glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

        return window;
    }

    /* Initialize the OpenGL rendering properties */
    /* Add all the models to be created here */
    void initGL (GLFWwindow* window, int width, int height)
    {
        /* Objects should be created before any other gl function and shaders */
        // Create the models
        createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
        createRectangle ();
        createCube();
        createPlayers();
        createQueen();

        // Create and compile our GLSL program from the shaders
        programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
        // Get a handle for our "MVP" uniform
        Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


        reshapeWindow (window, width, height);

        // Background color of the scene
        glClearColor (0.1f, 0.4f, 0.4f, 0.0f); // R, G, B, A
        glClearDepth (1.0f);

        glEnable (GL_DEPTH_TEST);
        glDepthFunc (GL_LEQUAL);

        cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
        cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
        cout << "VERSION: " << glGetString(GL_VERSION) << endl;
        cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
    }

    int main (int argc, char** argv)
    {
        int width = 1000;
        int height = 800;
   //     int inputt;

        GLFWwindow* window = initGLFW(width, height);

        initGL (window, width, height);

        printf("\nThe Black King chases the While Dancing Queen.\n");
        printf("Can you make him reach the queen through the maze.\n");

    /*    printf("\nCHOOSE LEVEL\n");
        printf("Press 1 for easy or 2 for hard\n");
        scanf("%d",&input);
        if(input==2)
            levelleria=1;
        else
            levelleria=2;

*/

        last_updated_time=glfwGetTime();
        /* Draw in loop */
        while (!glfwWindowShouldClose(window)) {

            // OpenGL Draw commands
            draw();

            // Swap Frame Buffer in double buffering
            glfwSwapBuffers(window);

            // Poll for Keyboard and mouse events
            glfwPollEvents();

            // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
            current_time = glfwGetTime(); // Time in seconds
            //   if ((current_time - last_update_time) >= 5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            //   flagvisibility=1;
            //    last_update_time = current_time;}
            //    else
            //      flagvisibility=0;
            if(px>6 && py>8)
            {
                
                winflag++;
                if(winflag==1)
                printf("YOU WIN\n");
                
            }
            




        }

        glfwTerminate();
        exit(EXIT_SUCCESS);
    }
