//  ========================================================================
//  COSC422: Computer Graphics (2018);  University of Canterbury.
//
//  FILE NAME: Terrain.cpp
//  This is part of Assignment1 files.
//
//    The program generates and loads the mesh data for a terrain floor (100 verts, 81 elems).
//  Required files:  Terrain.vert (vertex shader), Terrain.frag (fragment shader), HeightMap1.tga  (height map)
//  ========================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "loadTGA.h"
using namespace std;

GLuint vaoID;
GLuint program, shaderc, shadere, shaderg;
GLuint mvpMatrixLoc, mvMatrixLoc, normMatrixLoc, lightLoc, heightLoc,
       waterLevelLoc, snowLevelLoc, isPointLoc, pointTexLoc;

glm::vec4 lightPos = glm::vec4(0, 25, -50, 1);
glm::vec3 camPos = glm::vec3(0.0, 40.0, 50.0);
glm::vec3 lookDir = glm::vec3(0.0, 0.0, -1.0);
float hCamAngle = 0.0;
GLfloat waterLevel = 2.5;
GLfloat snowLevel = 8.5;
float CDR = 3.14159265/180.0;     //Conversion from degrees to rad (required in GLM 0.9.6)
float verts[103 * 3];       //10x10 grid (100 vertices) +3 points
GLushort elems[81 * 4 + 3];       //Element array for 81 quad patches +3 points
GLint heightMapIndex = 0;
glm::mat4 proj, projView, view;
bool isWireFrame = false;

//Generate vertex and element data for the terrain floor
void generateData()
{
    int indx, start;
    //verts array
    for(int i = 0; i < 10; i++)   //100 vertices on a 10x10 grid
    {
        for(int j = 0; j < 10; j++)
        {
            indx = 10*i + j;
            verts[3*indx] = 10*i - 45;        //x  varies from -45 to +45
            verts[3*indx+1] = 0;            //y  is set to 0 (ground plane)
            verts[3*indx+2] = -10*j;        //z  varies from 0 to -100
        }
    }
    //elems array
    for(int i = 0; i < 9; i++)
    {
        for(int j = 0; j < 9; j++)
        {
            indx = 9*i +j;
            start = 10*i + j;
            elems[4*indx] = start;
            elems[4*indx+1] = start+10;
            elems[4*indx+2] = start+11;
            elems[4*indx+3] = start+1;
        }
    }
    verts[300] = 30.0;
    verts[301] = 0.0;
    verts[302] = -60.0;

    verts[303] = 10.0;
    verts[304] = 0.0;
    verts[305] = -15.0;

    verts[306] = -30.0;
    verts[307] = 0.0;
    verts[308] = -65.0;

    elems[81 * 4] = 100;
    elems[81 * 4 + 1] = 101;
    elems[81 * 4 + 2] = 102;
}

void loadTexture(GLenum tex_unit, GLuint tex_name, string tex_path)
{
    glActiveTexture(tex_unit);
    glBindTexture(GL_TEXTURE_2D, tex_name);
    loadTGA(tex_path);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

//Loads terrain texture
void loadTextures()
{
    int numTex = 8;
    string texturePaths[numTex] = {
        "HeightMap1.tga",
        "HeightMap2.tga",
        "HeightMap3.tga",
        "water.tga",
        "grass.tga",
        "rock.tga",
        "snow.tga",
        "duck.tga"
    };
    GLuint texID[numTex];
    glGenTextures(numTex, texID);
    for (int i = 0; i < numTex; i++)
    {
        loadTexture(GL_TEXTURE0 + i, texID[i], texturePaths[i]);
    }
}

//Loads a shader file and returns the reference to a shader object
GLuint loadShader(GLenum shaderType, string filename)
{
    ifstream shaderFile(filename.c_str());
    if(!shaderFile.good()) cout << "Error opening shader file." << endl;
    stringstream shaderData;
    shaderData << shaderFile.rdbuf();
    shaderFile.close();
    string shaderStr = shaderData.str();
    const char* shaderTxt = shaderStr.c_str();

    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderTxt, NULL);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
        const char *strShaderType = NULL;
        cerr <<  "Compile failure in shader: " << strInfoLog << endl;
        delete[] strInfoLog;
    }
    return shader;
}

void resetUniforms(GLint isPoint)
{
    lightLoc = glGetUniformLocation(program, "lightPos");
    mvpMatrixLoc = glGetUniformLocation(program, "mvpMatrix");
    mvMatrixLoc = glGetUniformLocation(program, "mvMatrix");
    normMatrixLoc = glGetUniformLocation(program, "normMatrix");
    heightLoc = glGetUniformLocation(program, "heightMap");
    glUniform1i(heightLoc, heightMapIndex);
    isPointLoc = glGetUniformLocation(program, "isPoint");
    glUniform1i(isPointLoc, isPoint);
    pointTexLoc = glGetUniformLocation(program, "pointSampler");
    glUniform1i(pointTexLoc, 7);
    waterLevelLoc = glGetUniformLocation(program, "waterLevel");
    glUniform1f(waterLevelLoc, waterLevel);
    snowLevelLoc = glGetUniformLocation(program, "snowLevel");
    glUniform1f(snowLevelLoc, snowLevel);
    GLuint waterLoc = glGetUniformLocation(program, "waterSampler");
    glUniform1i(waterLoc, 3);
    GLuint grassLoc = glGetUniformLocation(program, "grassSampler");
    glUniform1i(grassLoc, 4);
    GLuint rockLoc = glGetUniformLocation(program, "rockSampler");
    glUniform1i(rockLoc, 5);
    GLuint snowLoc = glGetUniformLocation(program, "snowSampler");
    glUniform1i(snowLoc, 6);
    glm::vec4 lightEye = view * lightPos;
    glUniform4fv(lightLoc, 1, &lightEye[0]);
    glUniformMatrix4fv(mvMatrixLoc, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(mvpMatrixLoc, 1, GL_FALSE, &projView[0][0]);
    glUniformMatrix4fv(normMatrixLoc, 1, GL_TRUE, &glm::inverse(view)[0][0]);
}

//Initialise the shader program, create and load buffer data
void initialise()
{
//--------Load terrain height map-----------
    loadTextures();
//--------Load shaders----------------------
    GLuint shaderv = loadShader(GL_VERTEX_SHADER, "Terrain.vert");
    shaderc = loadShader(GL_TESS_CONTROL_SHADER, "Terrain.cont");
    shadere = loadShader(GL_TESS_EVALUATION_SHADER, "Terrain.eval");
    shaderg = loadShader(GL_GEOMETRY_SHADER, "Terrain.geom");
    GLuint shaderf = loadShader(GL_FRAGMENT_SHADER, "Terrain.frag");

    program = glCreateProgram();
    glAttachShader(program, shaderv);
    glAttachShader(program, shaderc);
    glAttachShader(program, shadere);
    glAttachShader(program, shaderg);
    glAttachShader(program, shaderf);
    glLinkProgram(program);

    GLint status;
    glGetProgramiv (program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
        fprintf(stderr, "Linker failure: %s\n", strInfoLog);
        delete[] strInfoLog;
    }
    glUseProgram(program);

//--------Compute matrices----------------------
    proj = glm::perspective(30.0f*CDR, 1.25f, 20.0f, 500.0f);  //perspective projection matrix
    view = glm::lookAt(camPos, camPos + lookDir, glm::vec3(0.0, 1.0, 0.0)); //view matrix
    projView = proj * view;  //Product (mvp) matrix

    resetUniforms(0);

//---------Load buffer data-----------------------
    generateData();

    glEnable(GL_POINT_SPRITE);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
    glPatchParameteri(GL_PATCH_VERTICES, 4);

    GLuint vboID[2];
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);
    glGenBuffers(2, vboID);

    glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);  // Vertex position

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elems), elems, GL_STATIC_DRAW);

    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void specialFunc(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_UP:
            camPos += lookDir;
            break;
        case GLUT_KEY_LEFT:
            hCamAngle -= 0.1;
            lookDir.x = sin(hCamAngle);
            lookDir.z = -cos(hCamAngle);
            break;
        case GLUT_KEY_DOWN:
            camPos -= lookDir;
            break;
        case GLUT_KEY_RIGHT:
            hCamAngle += 0.1;
            lookDir.x = sin(hCamAngle);
            lookDir.z = -cos(hCamAngle);
            break;
        case GLUT_KEY_PAGE_UP:
            camPos += glm::vec3(0.0, 1.0, 0.0);
            break;
        case GLUT_KEY_PAGE_DOWN:
            camPos -= glm::vec3(0.0, 1.0, 0.0);
            break;
        case GLUT_KEY_HOME:
            lookDir.y = min(1.0, lookDir.y + 0.1);
            break;
        case GLUT_KEY_END:
            lookDir.y = max(-1.0, lookDir.y - 0.1);
            break;
        default:
            return;
    }
    view = glm::lookAt(camPos, camPos + glm::normalize(lookDir), glm::vec3(0.0, 1.0, 0.0));
    projView = proj * view;
    glutPostRedisplay();
}

void keyboardFunc(unsigned char key, int x, int y) {
    switch(key) {
        case 'w': {
            GLenum mode = isWireFrame ? GL_FILL : GL_LINE;
            glPolygonMode(GL_FRONT_AND_BACK, mode);
            isWireFrame = ! isWireFrame;
            break; }
        case '1':
            heightMapIndex = 0;
            break;
        case '2':
            heightMapIndex = 1;
            break;
        case '3':
            heightMapIndex = 2;
            break;
        case 'e':
            waterLevel = min(10.0, waterLevel + 0.1);
            break;
        case 'd':
            waterLevel = max(0.0, waterLevel - 0.1);
            break;
        case 'r':
            snowLevel = min(10.0, snowLevel + 0.1);
            break;
        case 'f':
            snowLevel = max(0.0, snowLevel - 0.1);
            break;
        default:
            return;
    }
    view = glm::lookAt(camPos, camPos + glm::normalize(lookDir), glm::vec3(0.0, 1.0, 0.0));
    projView = proj * view;
    glutPostRedisplay();
}

//Display function to compute uniform values based on transformation parameters and to draw the scene
void display()
{
    resetUniforms(0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(vaoID);
    glDrawElements(GL_PATCHES, 81*4, GL_UNSIGNED_SHORT, NULL);

    glDetachShader(program, shaderc);
    glDetachShader(program, shadere);
    glDetachShader(program, shaderg);
    glLinkProgram(program);
    glUseProgram(program);

    resetUniforms(1);
    short index = 81 * 4 * sizeof(short);
    glDrawElements(GL_POINTS, 3, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(index));

    glAttachShader(program, shaderc);
    glAttachShader(program, shadere);
    glAttachShader(program, shaderg);
    glLinkProgram(program);
    glUseProgram(program);

    glutSwapBuffers();
}



int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(1000, 800);
    glutCreateWindow("Terrain");
    glutInitContextVersion (4, 2);
    glutInitContextProfile ( GLUT_CORE_PROFILE );
    glutKeyboardFunc(keyboardFunc);
    glutSpecialFunc(specialFunc);

    if(glewInit() == GLEW_OK)
    {
        cout << "GLEW initialization successful! " << endl;
        cout << " Using GLEW version " << glewGetString(GLEW_VERSION) << endl;
    }
    else
    {
        cerr << "Unable to initialize GLEW  ...exiting." << endl;
        exit(EXIT_FAILURE);
    }

    initialise();
    glutDisplayFunc(display);
    glutMainLoop();
}

