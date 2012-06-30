/*
 *  CPE 471 lab 3 - draws cube - Note this code is slightly ugly  
 *  glut/OpenGL application which renders a rectangle.  
 *  works in conjunction with a GLSL vertex and fragment shader to do color interpolation 
 *  base code for hierarchical modeling lab
 *
 *  Created by zwood on 1/6/12 
 *  Copyright 2010 Cal Poly. All rights reserved.
 *
 *****************************************************************************/
#ifdef __APPLE__
#include "GLUT/glut.h"
#include <OPENGL/gl.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include "GLSL_helper.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "RenderingHelper.h"
#include "Sphere.h"

#define TIMER 30
#define DISTANCE_SCALE 50
#define SIZE_SCALE 3
#define SUN_SCALE .044
#define MOON_SCALE 2.5


using namespace std;
using namespace glm;

typedef struct Image {
  unsigned long sizeX;
  unsigned long sizeY;
  char *data;
} Image;

int ImageLoad(char *filename, Image *image);
GLvoid LoadTexture(char* image_file, int tex_id);

// Macro used to obtain relative offset of a field within a struct
#define FIELD_OFFSET(StructType, field) &(((StructType *)0)->field)

//declare a matrix stack
RenderingHelper ModelTrans;

//position and color data handles
GLuint triBuffObj, colBuffObj;

//flag and ID to toggle on and off the shader
int shade = 1;
int ShadeProg;

Image *TextureImage;

//Handles to the shader data
GLint h_aPosition;
GLint h_uColor;
GLint h_aNormal;
GLint h_uModelMatrix;
GLint h_uViewMatrix;
GLint h_uProjMatrix;
GLint h_aTexCoord;
GLint h_uTexUnit;
GLint h_uLightPos;
GLint h_uLightColor;
GLuint CubeBuffObj, CIndxBuffObj, GrndBuffObj, GIndxBuffObj, NormalBuffObj;
GLint h_uMatAmb, h_uMatDif, h_uMatSpec, h_uMatShine;
int g_CiboLen, g_GiboLen;

static float g_width, g_height;
float g_trans = -5.5;
float g_Mtrans = 0;
float g_angle = 0;
float g_dist = 2;
float g_zoom = 50;
static float g_scale = 1;
float g_phi, g_theta;
int g_planet, g_mode; //mode: 0 for planet, 1 for overhead
float g_camPhi = 0;
float g_inner_scale = 1;

void SetMaterial(int i) {

  glUseProgram(ShadeProg);
  switch (i) {
    case 0:
        safe_glUniform3f(h_uMatAmb, 0.4, 0.2, 0.2);
        safe_glUniform3f(h_uMatDif, 0.6, 0.4, 0.4);
        safe_glUniform3f(h_uMatSpec, 0.4, 0.3, 0.3);
        safe_glUniform1f(h_uMatShine, 1.0);
        break;
    case 1:
        safe_glUniform3f(h_uMatAmb, 0, 0, 0);
        safe_glUniform3f(h_uMatDif, 1, 1, 1);
        safe_glUniform3f(h_uMatSpec, 0.3, 0.3, 0.4);
        safe_glUniform1f(h_uMatShine, 1.0);
        break;
    case 2:
        safe_glUniform3f(h_uMatAmb, 0.1, 0.7, 0.1);
        safe_glUniform3f(h_uMatDif, 0.3, 0.7, 0.3);
        safe_glUniform3f(h_uMatSpec, 0.7, 0.7, 0.7);
        safe_glUniform1f(h_uMatShine, 1.0);
        break;
  }
}



/* projection matrix */
void SetProjectionMatrix() {
  glm::mat4 Projection = glm::perspective(80.0f, (float)g_width/g_height, 0.1f, 10000.f);	
  safe_glUniformMatrix4fv(h_uProjMatrix, glm::value_ptr(Projection));
}

/* camera controls - do not change */
void SetView() {
  //glm::mat4 Trans = glm::translate( glm::mat4(1.0f), glm::vec3(-1, 0, g_trans));
  glm::mat4 T;
  float planetRot, planetTrans;
  if (g_mode == 0) {
      
      T = glm::lookAt(glm::vec3(0,2,DISTANCE_SCALE*planetTrans*g_dist), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
      //T = T * glm::rotate(g_phi * planetRot, glm::vec3(0, 1, 0));
      //T = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, DISTANCE_SCALE*g_dist));
      //T = T * glm::rotate(glm::mat4(1.0f), g_theta, glm::vec3(0, 1, 0));
      T = glm::rotate(T, g_theta, glm::vec3(0,1,0));
  }
  else {
      T = glm::lookAt(glm::vec3(0, g_zoom ,0), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1));
  }
  
  safe_glUniformMatrix4fv(h_uViewMatrix, glm::value_ptr(T));
}
void SetModelI() {
  glm::mat4 Ident(1.0f);
  safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(Ident));
}

/* model transforms */
void SetModel() {

  safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(ModelTrans.modelViewMatrix));
}



static void initCube() {

  //float CubePos[320] = {sphereVerts};
    
  
   unsigned short idx[960];

   for (int i = 0; i < 960; i++) {
        idx[i] = i;
   }


    g_CiboLen = 960;
    glGenBuffers(1, &CubeBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, CubeBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVerts), sphereVerts, GL_STATIC_DRAW);
    
    glGenBuffers(1, &NormalBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, NormalBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphereNormals), sphereNormals, GL_STATIC_DRAW);

    glGenBuffers(1, &CIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

}

void InitGeom() {
  initCube();
}

/*function to help load the shaders (both vertex and fragment */
int InstallShader(const GLchar *vShaderName, const GLchar *fShaderName) {
        GLuint VS; //handles to shader object
        GLuint FS; //handles to frag shader object
        GLint vCompiled, fCompiled, linked; //status of shader

        VS = glCreateShader(GL_VERTEX_SHADER);
        FS = glCreateShader(GL_FRAGMENT_SHADER);

        //load the source
        glShaderSource(VS, 1, &vShaderName, NULL);
        glShaderSource(FS, 1, &fShaderName, NULL);

        //compile shader and print log
        glCompileShader(VS);
        /* check shader status requires helper functions */
        printOpenGLError();
        glGetShaderiv(VS, GL_COMPILE_STATUS, &vCompiled);
        printShaderInfoLog(VS);

        //compile shader and print log
        glCompileShader(FS);
        /* check shader status requires helper functions */
        printOpenGLError();
        glGetShaderiv(FS, GL_COMPILE_STATUS, &fCompiled);
        printShaderInfoLog(FS);

        if (!vCompiled || !fCompiled) {
                printf("Error compiling either shader %s or %s", vShaderName, fShaderName);
                return 0;
        }

        //create a program object and attach the compiled shader
        ShadeProg = glCreateProgram();
        glAttachShader(ShadeProg, VS);
        glAttachShader(ShadeProg, FS);

        glLinkProgram(ShadeProg);
        /* check shader status requires helper functions */
        printOpenGLError();
        glGetProgramiv(ShadeProg, GL_LINK_STATUS, &linked);
        printProgramInfoLog(ShadeProg);

        glUseProgram(ShadeProg);

        /* get handles to attribute data */
        h_aPosition = safe_glGetAttribLocation(ShadeProg, "aPosition");
        h_uColor = safe_glGetUniformLocation(ShadeProg,  "uColor");
    	h_uProjMatrix = safe_glGetUniformLocation(ShadeProg, "uProjMatrix");
    	h_uViewMatrix = safe_glGetUniformLocation(ShadeProg, "uViewMatrix");
    	h_uModelMatrix = safe_glGetUniformLocation(ShadeProg, "uModelMatrix");
       //h_aTexCoord = safe_glGetAttribLocation(ShadeProg,  "aTexCoord");
       h_uTexUnit = safe_glGetUniformLocation(ShadeProg, "uTexUnit");
       h_aNormal = safe_glGetAttribLocation(ShadeProg, "aNormal");
       h_uLightPos = safe_glGetUniformLocation(ShadeProg, "uLightPos");
        h_uLightColor = safe_glGetUniformLocation(ShadeProg, "uLColor");
        h_uMatAmb = safe_glGetUniformLocation(ShadeProg, "uMat.aColor");
        h_uMatDif = safe_glGetUniformLocation(ShadeProg, "uMat.dColor");
        h_uMatSpec = safe_glGetUniformLocation(ShadeProg, "uMat.sColor");
        h_uMatShine = safe_glGetUniformLocation(ShadeProg, "uMat.shine");


       printf("sucessfully installed shader %d\n", ShadeProg);
       return 1;
	
}


/* Some OpenGL initialization */
void Initialize ()					// Any GL Init Code 
{
	// Start Of User Initialization
	glClearColor (.0f, .0f, .0f, .0f);								
	// Black Background
 	glClearDepth (1.0f);	// Depth Buffer Setup
 	glDepthFunc (GL_LEQUAL);	// The Type Of Depth Testing
	glEnable (GL_DEPTH_TEST);// Enable Depth Testing

   /* some matrix stack tests */
  ModelTrans.useModelViewMatrix();
  ModelTrans.loadIdentity();
  g_phi = 0;
  g_planet = 3;
  g_mode = 0;

    glEnable(GL_TEXTURE_2D);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    glEnable(GL_LIGHT0);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

}





/* Main display function */
void Draw (void)
{

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				
	//Start our shader	
 	glUseProgram(ShadeProg);
    SetProjectionMatrix();

    SetView();
    
    GLfloat l_amb[4] = {.6f, .6f, .6f, 1};
    GLfloat l_dif[4] = {.6f, .6f, .6f, 1};
    GLfloat l_spc[4] = {.33f, .3f, .3f, 1};
    GLfloat pos[4] = {0., 0., 0., 0};
    
    safe_glUniform3f(h_uLightPos, 0, 0, 0);
    safe_glUniform3f(h_uLightColor, .9, .9, .9);

    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    
    SetMaterial(1);


    safe_glEnableVertexAttribArray(h_aPosition);
    // bind vbo
    glBindBuffer(GL_ARRAY_BUFFER, CubeBuffObj);
    safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    /*safe_glEnableVertexAttribArray(h_aTexCoord);
    glBindBuffer(GL_ARRAY_BUFFER, TexBuffObj);
    safe_glVertexAttribPointer(h_aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0); */

    	safe_glEnableVertexAttribArray(h_aNormal);
    glBindBuffer(GL_ARRAY_BUFFER, NormalBuffObj);
    safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);


    // bind ibo
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CIndxBuffObj);
    
            glUniform3f(h_uColor, 1, 1, 1);    
    

    ModelTrans.loadIdentity();

      SetModel();

      ModelTrans.pushMatrix();

        ModelTrans.pushMatrix();
            //mercury
            ModelTrans.rotate(g_phi*4.1496, glm::vec3(0, 1, 0));
            ModelTrans.translate(glm::vec3(0, 0, DISTANCE_SCALE * .3226));
            ModelTrans.scale(SIZE_SCALE * .38251 * g_inner_scale);
            safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(ModelTrans.modelViewMatrix));

            glBindTexture(GL_TEXTURE_2D, 1);
            glDrawElements(GL_TRIANGLES, g_CiboLen, GL_UNSIGNED_SHORT, 0);
        ModelTrans.popMatrix();

        ModelTrans.pushMatrix();
            //venus
            ModelTrans.rotate(g_phi * 1.62438, glm::vec3(0, 1, 0));
            ModelTrans.translate(glm::vec3(0, 0, DISTANCE_SCALE * .7223));
            ModelTrans.scale(.94885 * SIZE_SCALE  * g_inner_scale);
            safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(ModelTrans.modelViewMatrix));
            //glUniform3f(h_uColor, 0.854, 0.635, 0.125);
            glBindTexture(GL_TEXTURE_2D, 2);
            glDrawElements(GL_TRIANGLES, g_CiboLen, GL_UNSIGNED_SHORT, 0);
        ModelTrans.popMatrix();


        ModelTrans.pushMatrix();
            //earth
            ModelTrans.rotate(g_phi, glm::vec3(0, 1, 0));
            ModelTrans.translate(glm::vec3(0, 0, DISTANCE_SCALE));
            ModelTrans.pushMatrix();
                //moon
                ModelTrans.rotate(g_phi*13.36, glm::vec3(0,1,0));
                ModelTrans.translate(glm::vec3(0, 0, .026 * DISTANCE_SCALE * MOON_SCALE * g_inner_scale));
                ModelTrans.scale(SIZE_SCALE * .27241 * g_inner_scale);
                safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(ModelTrans.modelViewMatrix));
                //glUniform3f(h_uColor, 0.6, 0.6, 0.6);
                glBindTexture(GL_TEXTURE_2D, 10);
                glDrawElements(GL_TRIANGLES, g_CiboLen, GL_UNSIGNED_SHORT, 0);
            ModelTrans.popMatrix();
            ModelTrans.scale(SIZE_SCALE * g_inner_scale);
            ModelTrans.rotate(g_phi*100, glm::vec3(0, 1, 0));
            safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(ModelTrans.modelViewMatrix));
            //glUniform3f(h_uColor, 0.1, 0.78, 0.1);
            glBindTexture(GL_TEXTURE_2D, 3);
            glDrawElements(GL_TRIANGLES, g_CiboLen, GL_UNSIGNED_SHORT, 0);
         ModelTrans.popMatrix();



        ModelTrans.pushMatrix();
            //mars
            ModelTrans.rotate(g_phi*.531311, glm::vec3(0, 1, 0));
            ModelTrans.translate(glm::vec3(0, 0, DISTANCE_SCALE * 1.7));

            ModelTrans.pushMatrix();
                //phobos
            ModelTrans.popMatrix();
            ModelTrans.pushMatrix();
                //deimos
            ModelTrans.popMatrix();

            ModelTrans.scale(SIZE_SCALE * .53276 * g_inner_scale);
            safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(ModelTrans.modelViewMatrix));
            glUniform3f(h_uColor, 0.9, 0.3, 0.3);
            glBindTexture(GL_TEXTURE_2D, 4);
            glDrawElements(GL_TRIANGLES, g_CiboLen, GL_UNSIGNED_SHORT, 0);
        ModelTrans.popMatrix();


        /*                     * 
         *                     *
         *                     *
         *  Here be asteroids  *
         *                     *
         *                     *
         *                     */


        

        ModelTrans.pushMatrix();
            //jupiter
            ModelTrans.rotate(g_phi * .0842, glm::vec3(0, 1, 0));
            ModelTrans.translate(glm::vec3(0, 0, DISTANCE_SCALE * 5.209));

            ModelTrans.pushMatrix();
                //callisto
            ModelTrans.popMatrix();
            ModelTrans.pushMatrix();
                //io
            ModelTrans.popMatrix();
            ModelTrans.pushMatrix();
                //europa
                ModelTrans.rotate(g_phi * 20, glm::vec3(0,1,0));
                ModelTrans.translate(glm::vec3(0,0,DISTANCE_SCALE*.45));
                ModelTrans.scale(.244 * SIZE_SCALE * 5);
                safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(ModelTrans.modelViewMatrix));
                glBindTexture(GL_TEXTURE_2D, 12);
                glDrawElements(GL_TRIANGLES, g_CiboLen, GL_UNSIGNED_SHORT, 0);
            ModelTrans.popMatrix();
            ModelTrans.pushMatrix();
                //ganymede
            ModelTrans.popMatrix();

            ModelTrans.scale(SIZE_SCALE * 11.209);

            safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(ModelTrans.modelViewMatrix));
            //glUniform3f(h_uColor, 0.6, 0.6, 0.6);
            glBindTexture(GL_TEXTURE_2D, 5);
            glDrawElements(GL_TRIANGLES, g_CiboLen, GL_UNSIGNED_SHORT, 0);
        ModelTrans.popMatrix();


        ModelTrans.pushMatrix();
            //saturn
            ModelTrans.rotate(g_phi * .033935, glm::vec3(0, 1, 0));
            ModelTrans.translate(glm::vec3(0, 0, DISTANCE_SCALE * 9.751));

            ModelTrans.pushMatrix();
                //titan
            ModelTrans.popMatrix();

            ModelTrans.scale(SIZE_SCALE * 9.449);

            safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(ModelTrans.modelViewMatrix));
            //glUniform3f(h_uColor, 0.6, 0.6, 0.6);
            glBindTexture(GL_TEXTURE_2D, 6);
            glDrawElements(GL_TRIANGLES, g_CiboLen, GL_UNSIGNED_SHORT, 0);
        ModelTrans.popMatrix();


        ModelTrans.pushMatrix();
            //uranus
            ModelTrans.rotate(g_phi * .0119, glm::vec3(0, 1, 0));
            ModelTrans.translate(glm::vec3(0, 0, DISTANCE_SCALE * 19.212));
            ModelTrans.scale(SIZE_SCALE * 4.0073);
            safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(ModelTrans.modelViewMatrix));
            glUniform3f(h_uColor, 0.3, 0.4, 0.9);
            glBindTexture(GL_TEXTURE_2D, 7);
            glDrawElements(GL_TRIANGLES, g_CiboLen, GL_UNSIGNED_SHORT, 0);
        ModelTrans.popMatrix();


        ModelTrans.pushMatrix();
            //neptune
            ModelTrans.rotate(g_phi * .006, glm::vec3(0, 1, 0));
            ModelTrans.translate(glm::vec3(0, 0, DISTANCE_SCALE * 30.05));
            ModelTrans.scale(SIZE_SCALE * 3.89);
            safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(ModelTrans.modelViewMatrix));
            glUniform3f(h_uColor, 0.3, 0.7, 0.9);
            glBindTexture(GL_TEXTURE_2D, 8);
            glDrawElements(GL_TRIANGLES, g_CiboLen, GL_UNSIGNED_SHORT, 0);
        ModelTrans.popMatrix();





        //sun
        ModelTrans.scale(109 * SUN_SCALE);
        safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(ModelTrans.modelViewMatrix));
        glUniform3f(h_uColor, 1, .841, .857);
        glBindTexture(GL_TEXTURE_2D, 0);
        SetMaterial(2);
        glDrawElements(GL_TRIANGLES, g_CiboLen, GL_UNSIGNED_SHORT, 0);
      ModelTrans.popMatrix();

        
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushMatrix();
        glRotatef(g_phi, 0, 1, 0);
        glTranslatef(0, DISTANCE_SCALE*1.61*.5, DISTANCE_SCALE*1.61);
        glm::lookAt(glm::vec3(0,5,DISTANCE_SCALE*1.61), glm::vec3(0, -.5, -1.61), glm::vec3(0, 1, 0));
     glPopMatrix();


    glDisable(GL_TEXTURE_2D);
    // Disable the attributes used by our shader
    safe_glDisableVertexAttribArray(h_aPosition);
    safe_glDisableVertexAttribArray(h_aNormal);
    //safe_glDisableVertexAttribArray(h_aTexCoord);

	//Disable the shader
	glUseProgram(0);	
	glutSwapBuffers();

}

/* Reshape */
void ReshapeGL (int width, int height)								
{
	g_width = (float)width;
	g_height = (float)height;
	glViewport (0, 0, (GLsizei)(width), (GLsizei)(height));				
}

int g_dir = 2;
static void timer (int value)
{
    /*g_phi++;
    glutPostRedisplay();
    glutTimerFunc (TIMER, timer, 0);*/
}

//the keyboard callback
void keyboard(unsigned char key, int x, int y ){
  switch( key ) {
    case 'w':
      g_dist -= 0.1;
      g_zoom -= .5;
      g_camPhi = sqrt(pow(g_dist * DISTANCE_SCALE, 3));
      break;
    case 's':
	  g_dist += 0.1;
	  g_zoom += .5;
      g_camPhi = sqrt(pow(g_dist * DISTANCE_SCALE, 3));
	break;
    case 'a':
      g_theta--;
      break;
    case 'd':
      g_theta++;
      break;
    case 'm':
      g_mode = 0;
      g_inner_scale = 1;
      break;
    case 'n':
      g_mode = 1;
      g_zoom = 50;
      g_inner_scale = 2.5;
      break;
    case 'p':
      g_phi += .1;
      break;
    case 'o':
      g_phi -= .1;
      break;
    case 'u':
      g_phi -= 1;
      break;
    case 'i':
      g_phi += 1;
      break;
    case 'q': case 'Q' :
      exit( EXIT_SUCCESS );
      break;
  }
  glutPostRedisplay();
}


int main( int argc, char *argv[] )
{
   	glutInit( &argc, argv );
   	glutInitWindowPosition( 20, 20 );
   	glutInitWindowSize( 800, 800 );
   	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
   	glutCreateWindow("SPAAAAAAAAACE!");
   	glutReshapeFunc( ReshapeGL );
   	glutDisplayFunc( Draw );
	glutKeyboardFunc( keyboard );
    glutTimerFunc (TIMER, timer, 0);

   	Initialize();

	//texture time!
	LoadTexture((char *)"./textures/sun.bmp", 0);
	LoadTexture((char *)"./textures/mercury.bmp", 1);
	LoadTexture((char *)"./textures/venus.bmp", 2);
	LoadTexture((char *)"./textures/earth.bmp", 3);
	LoadTexture((char *)"./textures/mars.bmp", 4);
	LoadTexture((char *)"./textures/jupiter.bmp", 5);
	LoadTexture((char *)"./textures/saturn.bmp", 6);
	LoadTexture((char *)"./textures/uranus.bmp", 7);
	LoadTexture((char *)"./textures/neptune.bmp", 8);
	LoadTexture((char *)"./textures/moon.bmp", 9);
	LoadTexture((char *)"./textures/phobos.bmp", 10);
	LoadTexture((char *)"./textures/deimos.bmp", 11);
	LoadTexture((char *)"./textures/europa.bmp", 12);
	LoadTexture((char *)"./textures/ganymede.bmp", 13);
	LoadTexture((char *)"./textures/titan.bmp", 14);
	LoadTexture((char *)"./textures/calisto.bmp", 15);

	
	//test the openGL version
	getGLversion();
	//install the shader
	if (!InstallShader(textFileRead((char *)"Lab4_vert.glsl"), textFileRead((char *)"Lab4_frag.glsl"))) {
		printf("Error installing shader!\n");
		return 0;
	}
		
	InitGeom();
  	glutMainLoop();
   	return 0;
}



//routines to load in a bmp files - must be 2^nx2^m and a 24bit bmp
GLvoid LoadTexture(char* image_file, int texID) { 
  
  TextureImage = (Image *) malloc(sizeof(Image));
  if (TextureImage == NULL) {
    printf("Error allocating space for image");
    exit(1);
  }
  cout << "trying to load " << image_file << endl;
  if (!ImageLoad(image_file, TextureImage)) {
    exit(1);
  }  
  /*  2d texture, level of detail 0 (normal), 3 components (red, green, blue),            */
  /*  x size from image, y size from image,                                              */    
  /*  border 0 (normal), rgb color data, unsigned byte data, data  */ 
  glBindTexture(GL_TEXTURE_2D, texID);
  glTexImage2D(GL_TEXTURE_2D, 0, 3,
    TextureImage->sizeX, TextureImage->sizeY,
    0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage->data);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); /*  cheap scaling when image bigger than texture */    
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); /*  cheap scaling when image smalled than texture*/
  
}


/* BMP file loader loads a 24-bit bmp file only */

/*
* getint and getshort are help functions to load the bitmap byte by byte
*/
static unsigned int getint(FILE *fp) {
  int c, c1, c2, c3;
  
  /*  get 4 bytes */ 
  c = getc(fp);  
  c1 = getc(fp);  
  c2 = getc(fp);  
  c3 = getc(fp);
  
  return ((unsigned int) c) +   
    (((unsigned int) c1) << 8) + 
    (((unsigned int) c2) << 16) +
    (((unsigned int) c3) << 24);
}

static unsigned int getshort(FILE *fp){
  int c, c1;
  
  /* get 2 bytes*/
  c = getc(fp);  
  c1 = getc(fp);
  
  return ((unsigned int) c) + (((unsigned int) c1) << 8);
}

/*  quick and dirty bitmap loader...for 24 bit bitmaps with 1 plane only.  */

int ImageLoad(char *filename, Image *image) {
  FILE *file;
  unsigned long size;                 /*  size of the image in bytes. */
  unsigned long i;                    /*  standard counter. */
  unsigned short int planes;          /*  number of planes in image (must be 1)  */
  unsigned short int bpp;             /*  number of bits per pixel (must be 24) */
  char temp;                          /*  used to convert bgr to rgb color. */
  
  /*  make sure the file is there. */
  if ((file = fopen(filename, "rb"))==NULL) {
    printf("File Not Found : %s\n",filename);
    return 0;
  }
  
  /*  seek through the bmp header, up to the width height: */
  fseek(file, 18, SEEK_CUR);
  
  /*  No 100% errorchecking anymore!!! */
  
  /*  read the width */    image->sizeX = getint (file);
  
  /*  read the height */ 
  image->sizeY = getint (file);
  
  /*  calculate the size (assuming 24 bits or 3 bytes per pixel). */
  size = image->sizeX * image->sizeY * 3;
  
  /*  read the planes */    
  planes = getshort(file);
  if (planes != 1) {
    printf("Planes from %s is not 1: %u\n", filename, planes);
    return 0;
  }
  
  /*  read the bpp */    
  bpp = getshort(file);
  if (bpp != 24) {
    printf("Bpp from %s is not 24: %u\n", filename, bpp);
    return 0;
  }
  
  /*  seek past the rest of the bitmap header. */
  fseek(file, 24, SEEK_CUR);
  
  /*  read the data.  */
  image->data = (char *) malloc(size);
  if (image->data == NULL) {
    printf("Error allocating memory for color-corrected image data");
    return 0; 
  }
  
  if ((i = fread(image->data, size, 1, file)) != 1) {
    printf("Error reading image data from %s.\n", filename);
    return 0;
  }
  
  for (i=0;i<size;i+=3) { /*  reverse all of the colors. (bgr -> rgb) */
    temp = image->data[i];
    image->data[i] = image->data[i+2];
    image->data[i+2] = temp;
  }
  
  fclose(file); /* Close the file and release the filedes */
  
  /*  we're done. */
  return 1;
}



