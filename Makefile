# dead-simple; doesn't handle headers...

pixelcity: Building.o Camera.o Car.o Deco.o Entity.o Ini.o Light.o Math.o Mesh.o Random.o Render.o Sky.o Texture.o Visible.o Win.o World.o glBbox.o glMatrix.o glQuat.o glRgba.o glVector2.o glVector3.o
	g++ -o $@ -g $+ -lm -lX11 -lGL -lGLU

CFLAGS=-Wall -pedantic -D_FILE_OFFSET_BITS=64 -g -O0
CXXFLAGS=-Wall -pedantic -D_FILE_OFFSET_BITS=64 -g -O0

clean:
	rm -f *.o pixelcity
