struct Material {
  vec3 aColor;
  vec3 dColor;
  vec3 sColor;
  float shine;
};


uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform vec3 uColor;
uniform Material uMat;
uniform vec3 uLightPos;
uniform vec3 uLColor;

attribute vec3 aPosition;
attribute vec3 aNormal;

varying vec3 vColor;
varying vec2 vTexCoord;
varying vec3 vNormal;
varying vec3 v;


void main() {
  vec4 vPosition;
  vec3 light;
  float pi = 3.14159;
 
  /* First model transforms */
  vPosition = uModelMatrix* vec4(aPosition.x, aPosition.y, aPosition.z, 1);
  vPosition = uViewMatrix* vPosition;
  gl_Position = uProjMatrix*vPosition;
  gl_FrontColor = vec4(uColor.r, uColor.g, uColor.b, 1.0);
  v = aPosition;

  //vColor = vec3(uColor.r, uColor.g, uColor.b);
  //vColor = vec3(0.56, 0.3, 0.1);
  gl_TexCoord[0].s = 0.5 + atan(aPosition.z, -aPosition.x)/(2.0*pi);
  gl_TexCoord[0].t = 0.5 + asin(aPosition.y)/pi;
  vNormal = aNormal;
    light = vec3(uLightPos.x-aPosition.x, uLightPos.y-aPosition.y, uLightPos.z-aPosition.z);
//  reflect = vec3(1, 1, 2);

  vColor = uMat.aColor*uMat.dColor + uMat.sColor + (uLColor * (uMat.dColor * normalize(abs(dot(aNormal, light) ))));
}
