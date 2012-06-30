uniform sampler2D uTexUnit;

varying vec3 vColor;
varying vec3 vNormal;
varying vec3 v;

void main(void) {
  vec4 texColor0 = vec4(vColor.x, vColor.y, vColor.z, 1);
  vec4 texColor1 = texture2D(uTexUnit, vec2(gl_TexCoord[0].s, gl_TexCoord[0].t));
  vec4 lighting;
  
  lighting = vec4(vColor.x, vColor.y, vColor.z, 1);
  
  
  //lighting = Idiff + Ispec;
  vec3 color = vec3(texColor1.x, texColor1.y, texColor1.z);

  //gl_FragColor = vec4(gl_TexCoord[0].s, gl_TexCoord[0].t, 0, 1);
  gl_FragColor = vec4(texColor1[0], texColor1[1], texColor1[2], 1);
  gl_FragColor = lighting * texColor1;
  
}

