all:
	g++ HierM.cpp GLSL_helper.cpp RenderingHelper.cpp -DGL_GLEXT_PROTOTYPES -lGLU -lGL -lglut
