#ifndef __POSTEFFECTLIB__
#define __POSTEFFECTLIB__



#include <GL/glew.h>
//#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>

#ifndef NDEBUG
#define OPENGLCOMMAND(a) a; CHECKERROR("command");
#define CHECKERROR(a) assert(ShowError(a))
#else
#define OPENGLCOMMAND(a) a
#define CHECKERROR(a)
#endif


bool ShowError(const char *c);

struct PostShader
{
    GLuint m_colorframebuffer;
    GLuint m_depthframebuffer;
    GLuint m_colortexture;
    int m_lastsizex;
    int m_lastsizey;
    GLuint m_vertexshader;
    GLuint m_pixelshader;
    GLuint m_programid;
    GLint m_oldframebufferbound;
    bool m_active;


};
void PostShader_Create(struct PostShader* self, const char* vertexshader, const char *pixelshader);
void PostShader_Destroy(struct PostShader* self);
void PostShader_begin(struct PostShader* self);
void PostShader_end(struct PostShader* self);
#endif
