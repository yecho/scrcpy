#ifndef __POSTEFFECTLIB__
#define __POSTEFFECTLIB__


#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glu.h>
#endif
//#include <GL/glew.h>
//#include <GL/glut.h>
//#include <GL/glu.h>

//#include <EGL/egl.h>
//#include <EGL/eglext.h>
//#include <GLES2/gl2.h>

#define POINTS_OF_BEZIER 6

#ifndef NDEBUG
#define OPENGLCOMMAND(a) a; CHECKERROR("command");
#define CHECKERROR(a) assert(ShowError(a))
#else
#define OPENGLCOMMAND(a) a
#define CHECKERROR(a)
#endif

struct WarpingGridVertex {
   float m_posx;
   float m_posy;
   float m_posz;
   float m_u;
   float m_v;
};

//struct WarpingGridVertex WarpingGridVertex_Create(struct vec2 p, float u, float v);
struct WarpingGrid {
    int m_sizex;
    int m_sizey;
    struct vec2* m_grid;
    struct WarpingGridVertex* m_vertices;
    int m_vertices_count;

};

struct WarpingGrid* WarpingGrid_Create(int x, int y);
void WarpingGrid_set(struct WarpingGrid*self, int i, int j, struct vec2 pos);
struct vec2 WarpingGrid_get(struct WarpingGrid*self, int i, int j);
void WarpingGrid_createMesh(struct WarpingGrid*self);
void WarpingGrid_bindToGPU(struct WarpingGrid*self);
void WarpingGrid_Destroy(struct WarpingGrid* self);

double factorial(int n);
double binom(int n, int k);
double bernstein(int n, int i, double u);

struct vec2 {
   float x;
   float y;
};

struct vec2 vec2_set(float x, float y);

struct BezierSurface {

    struct vec2 m_controlpoints[POINTS_OF_BEZIER][POINTS_OF_BEZIER];

};

struct vec2 BezierSurface_eval(struct BezierSurface* self, double u, double v);
struct BezierSurface* BezierSurface_Create();
void BezierSurface_Destroy(struct BezierSurface* self);
void BezierSurface_initBezierSurface(struct BezierSurface* self, struct vec2 minvalue, struct vec2 maxvalue);
void BezierSurface_writeTo(struct BezierSurface* self, struct WarpingGrid*grid);
void BezierSurface_drawPoints(struct BezierSurface* self, int xselected, int yselected);
void BezierSurface_writeToFile(struct BezierSurface* self, const char* filename);
void BezierSurface_readFromFile(struct BezierSurface* self, const char* filename);
void BezierSurface_writeToCPPFile(struct BezierSurface* self, const char* filename);

bool ShowError(const char *c);

struct PostShader {
    GLuint m_colorframebuffer;
    GLuint m_depthframebuffer;
    GLuint m_colortexture;
    int m_lastsizex;
    int m_lastsizey;
    GLuint m_vertexshader;
    GLuint m_pixelshader;
    GLuint m_programid;
    GLuint m_vertexshaderdefault;
    GLuint m_pixelshaderdefault;
    GLuint m_programiddefault;
    GLint m_oldframebufferbound;
    GLint m_transformmatrix;
    GLint m_screensize;
    GLint m_transformmatrixdefault;
    struct PostShader* m_prevshader;
    bool m_active;

    //void PostShader_end(PostShader* self, struct vec2 v1, const android::vec2& v2, const android::vec2& v3, const android::vec2& v4);

};

struct PostShader* PostShader_Create(const char* vertexshader, const char *pixelshader);
void PostShader_Destroy(struct PostShader* self);
void PostShader_begin(struct PostShader* self);
void PostShader_end(struct PostShader* self, struct WarpingGrid* grid, struct BezierSurface* surface, int selx, int sely);
void PostShader_beginSlow(struct PostShader* self);
void PostShader_endSlow(struct PostShader* self);
/*
class BlurShader {
protected:
    PostShader* m_firstshader;
    PostShader* m_secondshader;
    float mBlurArray[128];
   int mBlurCount = 0;
   int mBlurCenter =0;
   void createBlurArray(float sigma);
public:
   BlurShader();
   ~BlurShader();
   void begin();
   void end();
    void end(const android::vec2& v1, const android::vec2& v2, const android::vec2& v3, const android::vec2& v4);
  void end(const android::vec4& v1, const android::vec4& v2, const android::vec4& v3, const android::vec4& v4);

  // void end(float x1, float y1, float x2, float y2);
};
*/
#endif
