#include "posteffectlib.h"


bool ShowError(const char *c)
{
    int Error = glGetError();
	if (Error == 0) return true;
   // std::string s = c;
   // s += ": ";
        printf("%s", c);
    switch (Error)
    {
    case GL_INVALID_OPERATION:
        printf("Die aufgerufene Funktion ist an dieser Stelle nicht zulaessig.\n");
        break;
    case GL_INVALID_ENUM:
        printf("Ein falscher Aufzaehlungstyp wurde als Parameter definiert.");
        break;
    case GL_INVALID_VALUE :
        printf("Ein numerischer Parameter hat einen unzulaesig.");
        break;
    case GL_STACK_OVERFLOW :
        printf("Die aufgerufene Funktion wurde zu einem uerlaufen des Stapels fhren.");
        break;
    case GL_STACK_UNDERFLOW:
        printf("Es wird versucht, von einem leeren Stapel zu lesen.");
        break;
    case GL_OUT_OF_MEMORY:
        printf("Nicht gengend Speicherplatz");
        break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        printf("Invalid Framebuffer operation");
        break;
    };
   return false;
//    	if (Error != 0) MessageBox(0, s.c_str(),L"Fehler",0);
};



void getLog(int shader)
{
   GLint loglength = 250;
   char infolog[256];
   GLint writtenlength;
   OPENGLCOMMAND(glGetInfoLogARB(shader, loglength, &writtenlength, infolog));

   infolog[writtenlength] = 0;
   printf("%s", infolog);
 //  std::string res = "";
  // for (int i = 0; i < writtenlength; i++)
//	   res += infolog[i];
  
  // return res;
}

void PostShader_Create(struct PostShader* self, const char* vertexshader, const char* pixelshader)
{
  //  std::cout << "In Constructor" << std::endl;
    glewInit();
    GLint oldframebufferbound;
    GLint oldrenderbufferbound;
    GLint oldtexture;
    OPENGLCOMMAND(glGetIntegerv(GL_TEXTURE_BINDING_RECTANGLE, &oldtexture));
    OPENGLCOMMAND(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldframebufferbound));
    OPENGLCOMMAND(glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldrenderbufferbound));
   // std::cout << "C1" << std::endl;
   OPENGLCOMMAND(glGenFramebuffers(1, &self->m_colorframebuffer));
   OPENGLCOMMAND(glBindFramebuffer(GL_FRAMEBUFFER_EXT, self->m_colorframebuffer));
   OPENGLCOMMAND(glGenRenderbuffers(1, &self->m_depthframebuffer));
   OPENGLCOMMAND(glBindRenderbuffer(GL_RENDERBUFFER_EXT, self->m_depthframebuffer));
   OPENGLCOMMAND(glGenTextures(1, &self->m_colortexture));
    OPENGLCOMMAND(glBindTexture(GL_TEXTURE_RECTANGLE, self->m_colortexture));
   // std::cout << "C1" << std::endl;


    OPENGLCOMMAND(glBindTexture(GL_TEXTURE_RECTANGLE, oldtexture));
    OPENGLCOMMAND(glBindFramebuffer(GL_FRAMEBUFFER_EXT, oldframebufferbound));

    OPENGLCOMMAND(glBindRenderbuffer(GL_RENDERBUFFER_EXT, oldrenderbufferbound));

    self->m_vertexshader = glCreateShader(GL_VERTEX_SHADER);
    int lengthvertex = strlen(vertexshader);
    OPENGLCOMMAND(glShaderSource(self->m_vertexshader, 1, &vertexshader, &lengthvertex));
    OPENGLCOMMAND(glCompileShader(self->m_vertexshader));
    
     int compiled;
  //  std::cout << "C1" << std::endl;
   OPENGLCOMMAND(glGetObjectParameterivARB(self->m_vertexshader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled));
   if (compiled == GL_FALSE) {
        getLog(self->m_vertexshader);

       exit(1);
   }
    
    self->m_pixelshader = glCreateShader(GL_FRAGMENT_SHADER);
    int lengthpixel = strlen(pixelshader);
    OPENGLCOMMAND(glShaderSource(self->m_pixelshader, 1, &pixelshader, &lengthpixel));
    OPENGLCOMMAND(glCompileShader(self->m_pixelshader));
     
   OPENGLCOMMAND(glGetObjectParameterivARB(self->m_pixelshader, GL_OBJECT_COMPILE_STATUS_ARB, &compiled));
 //   std::cout << "C2" << std::endl;
   if (compiled == GL_FALSE) {
           getLog(self->m_pixelshader);

       exit(1);
   }

    self->m_programid = glCreateProgram();
    OPENGLCOMMAND(glAttachShader(self->m_programid, self->m_vertexshader));
    OPENGLCOMMAND(glAttachShader(self->m_programid,self->m_pixelshader));
    OPENGLCOMMAND(glLinkProgram(self->m_programid));
    
    
   // std::cout << "C3" << std::endl;
   OPENGLCOMMAND(glGetObjectParameterivARB(self->m_programid, GL_OBJECT_LINK_STATUS_ARB, &compiled));
   if (compiled == GL_FALSE) {
           getLog(self->m_programid);

      exit(1);
   };
   
   GLuint location;
   location = glGetUniformLocation(self->m_programid, "sampler");
   assert(ShowError("c"));
  //  std::cout << "sampler: " << location << std::endl;
    
    GLint oldprogram;
    OPENGLCOMMAND(glGetIntegerv(GL_CURRENT_PROGRAM, &oldprogram));
    
   OPENGLCOMMAND(glUseProgram(self->m_programid));
    OPENGLCOMMAND(glUniform1i(location, 0));
    
    
   OPENGLCOMMAND(glUseProgram(oldprogram));

   self->m_lastsizex = -1;
   self->m_lastsizey = -1;
   self->m_oldframebufferbound = -1;
   self->m_active = true;
 //   std::cout << "Out Constructor" << std::endl;
}
void PostShader_Destroy(struct PostShader* self)
{
   OPENGLCOMMAND(glDeleteFramebuffers(1, &self->m_colorframebuffer));
   OPENGLCOMMAND(glDeleteFramebuffers(1, &self->m_depthframebuffer));
   OPENGLCOMMAND(glDeleteTextures(1, &self->m_colortexture));
   assert(self->m_active == false);
}
void PostShader_begin(struct PostShader* self)
{
   GLint dims[4] = {0};
   glGetIntegerv(GL_VIEWPORT, dims);
   GLint fbWidth = dims[2];
   GLint fbHeight = dims[3];
   int width = fbWidth;
   int height = fbHeight;
    OPENGLCOMMAND(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &self->m_oldframebufferbound));
    self->m_active = true;
   if (width != self->m_lastsizex || height != self->m_lastsizey) {

       GLint oldrenderbufferbound;
       OPENGLCOMMAND(glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldrenderbufferbound));
       self->m_lastsizex = width;
       self->m_lastsizey = height;
       GLint oldtexture;
       OPENGLCOMMAND(glGetIntegerv(GL_TEXTURE_BINDING_RECTANGLE, &oldtexture));

       OPENGLCOMMAND(glBindTexture(GL_TEXTURE_RECTANGLE, self->m_colortexture));
       OPENGLCOMMAND(glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));

       OPENGLCOMMAND(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
       OPENGLCOMMAND(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
       OPENGLCOMMAND(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
       OPENGLCOMMAND(glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

       OPENGLCOMMAND(glBindTexture(GL_TEXTURE_RECTANGLE, oldtexture));

       OPENGLCOMMAND(glBindRenderbuffer(GL_RENDERBUFFER_EXT, self->m_depthframebuffer));
       OPENGLCOMMAND(glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height));

       OPENGLCOMMAND(glBindRenderbuffer(GL_RENDERBUFFER_EXT, oldrenderbufferbound));
       OPENGLCOMMAND(glBindFramebuffer(GL_FRAMEBUFFER_EXT, self->m_colorframebuffer));
       OPENGLCOMMAND(glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE, self->m_colortexture, 0));
     //  glBindRenderbuffer(GL_RENDERBUFFER_EXT, m_depthframebuffer);
       OPENGLCOMMAND(glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT,self->m_depthframebuffer));
       GLenum status;
          status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
          switch(status)
          {
             case GL_FRAMEBUFFER_COMPLETE_EXT:
             //std::cout<<"good" << std::endl; 
             break;
          default:
             exit(1);
          }
   }

   glBindFramebuffer(GL_FRAMEBUFFER_EXT, self->m_colorframebuffer);
   
   // std::cout << "New Framebuffer: " << m_colorframebuffer << std::endl;
   GLenum status;
     status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
          switch(status)
          {
             case GL_FRAMEBUFFER_COMPLETE_EXT:
            // std::cout<<"good" << std::endl; 
            break;
          default:
             exit(1);
          }

}
void PostShader_end(struct PostShader* self)
{

    assert(self->m_active== true);
    self->m_active = false;
    GLint oldprogram;
    OPENGLCOMMAND(glGetIntegerv(GL_CURRENT_PROGRAM, &oldprogram));
    GLint oldtexture;
    OPENGLCOMMAND(glGetIntegerv(GL_TEXTURE_BINDING_RECTANGLE, &oldtexture));

    GLint oldactivetexture;
    OPENGLCOMMAND(glGetIntegerv(GL_ACTIVE_TEXTURE, &oldactivetexture));
    //TODO: Take here old buffer
    OPENGLCOMMAND(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, self->m_oldframebufferbound));
    //std::cout << "Old Framebuffer: " << m_oldframebufferbound << std::endl;
    //Restore Color Attachement? and Depth Attachment
    OPENGLCOMMAND(glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_VIEWPORT_BIT | GL_TRANSFORM_BIT | GL_DEPTH_BUFFER_BIT));
    OPENGLCOMMAND(glActiveTexture(GL_TEXTURE0));
    OPENGLCOMMAND(glEnable(GL_TEXTURE_RECTANGLE));
    OPENGLCOMMAND(glDisable(GL_TEXTURE_2D));

    OPENGLCOMMAND(glBindTexture(GL_TEXTURE_RECTANGLE, self->m_colortexture));
    OPENGLCOMMAND(glViewport(0,0,self->m_lastsizex, self->m_lastsizey));
    OPENGLCOMMAND(glMatrixMode(GL_PROJECTION));
    OPENGLCOMMAND(glPushMatrix());
    OPENGLCOMMAND(glLoadIdentity());
    glOrtho(0, 1, 0, 1, 1, 2); 
    OPENGLCOMMAND(glMatrixMode(GL_MODELVIEW));
    OPENGLCOMMAND(glPushMatrix());
    OPENGLCOMMAND(glLoadIdentity());
    OPENGLCOMMAND(glUseProgram(self->m_programid));
  //  OPENGLCOMMAND(glDisable(GL_CULL_FACE));
    OPENGLCOMMAND(glDisable(GL_DEPTH_TEST));
    float vertices[] = {
          0.0f, 0.0f, -1.5f,
           1.0f, 0.0f, -1.5f,
          0.0f,  1.0f, -1.5f,
           1.0f,  1.0f, -1.5f
       };

       float texCoords[] = {
          0.0f, 0.0f*self->m_lastsizey,
          1.0f*self->m_lastsizex, 0.0f*self->m_lastsizey,
          0.0f, 1.0f*self->m_lastsizey,
          1.0f*self->m_lastsizex, 1.0f*self->m_lastsizey
       };

   // OPENGLCOMMAND(glEnable(GL_DEPTH_TEST));
    OPENGLCOMMAND(glEnableClientState(GL_VERTEX_ARRAY));
    OPENGLCOMMAND(glVertexPointer(3, GL_FLOAT, 0, vertices));
    OPENGLCOMMAND(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
    OPENGLCOMMAND(glTexCoordPointer(2, GL_FLOAT, 0, texCoords));
 

   OPENGLCOMMAND(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
   
  
   

   OPENGLCOMMAND(glUseProgram(oldprogram));
    OPENGLCOMMAND(glBindTexture(GL_TEXTURE_RECTANGLE, oldtexture));
    OPENGLCOMMAND(glDisableClientState(GL_TEXTURE_COORD_ARRAY));
    OPENGLCOMMAND( glDisableClientState(GL_VERTEX_ARRAY));
    //Ending
    OPENGLCOMMAND(glMatrixMode(GL_PROJECTION));
    OPENGLCOMMAND(glPopMatrix());
    OPENGLCOMMAND(glMatrixMode(GL_MODELVIEW));
    OPENGLCOMMAND(glPopMatrix());
    OPENGLCOMMAND(glPopAttrib());

}
