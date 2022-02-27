#include "posteffectlib.h"


double factorial(int n) {
    double res = 1;
    for (int k = 1; k <= n; k++) {
        res*= k;
    }
    return res;
}

double binom(int n, int k) {
    return factorial(n) / (factorial(k)*factorial(n-k));
}

double power1 (double u, int y) {
    double res = 1;
    for (int i = 1; i<= y;i++) {
        res *= u;
    }
    return res;
}

double bernstein(int n, int i, double u) {
    return binom(n,i)*power1(u, i)*power1(1-u, n-i);
}

struct WarpingGridVertex WarpingGridVertex_Create(struct vec2 p, float u, float v) {
    struct WarpingGridVertex self;
    self.m_posx = p.x;
    self.m_posy = p.y;
    self.m_posz = -1.5;
    self.m_u = u;
    self.m_v = v;
    return self;
}

bool ShowError(const char *c) {
    int Error = glGetError();
    if (Error == 0) return true;
    // std::string s = c;
    // s += ": ";
    printf("%s", c);
    switch (Error) {
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



void getLog(int shader) {
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

struct PostShader* PostShader_Create(const char* vertexshader, const char* pixelshader) {
    //  std::cout << "In Constructor" << std::endl;
    struct PostShader* self = (struct PostShader*)malloc(sizeof(struct PostShader));
    glewInit();
    self->m_prevshader = NULL;
    self->m_transformmatrix = -1;
    GLint oldframebufferbound;
    GLint oldrenderbufferbound;
    GLint oldtexture;
    // ALOGD("PostShader: In Constructor");
    OPENGLCOMMAND(glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldtexture));
    OPENGLCOMMAND(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldframebufferbound));
    OPENGLCOMMAND(glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldrenderbufferbound));
    // std::cout << "C1" << std::endl;
    // ALOGD("PostShader: C1");
    OPENGLCOMMAND(glGenFramebuffers(1, &self->m_colorframebuffer));
    OPENGLCOMMAND(glBindFramebuffer(GL_FRAMEBUFFER, self->m_colorframebuffer));
    OPENGLCOMMAND(glGenRenderbuffers(1, &self->m_depthframebuffer));
    OPENGLCOMMAND(glBindRenderbuffer(GL_RENDERBUFFER, self->m_depthframebuffer));
    OPENGLCOMMAND(glGenTextures(1, &self->m_colortexture));
    OPENGLCOMMAND(glBindTexture(GL_TEXTURE_2D, self->m_colortexture));
    // ALOGD("PostShader: C2");
    // std::cout << "C1" << std::endl;


    OPENGLCOMMAND(glBindTexture(GL_TEXTURE_2D, oldtexture));
    OPENGLCOMMAND(glBindFramebuffer(GL_FRAMEBUFFER, oldframebufferbound));

    OPENGLCOMMAND(glBindRenderbuffer(GL_RENDERBUFFER, oldrenderbufferbound));

    self->m_vertexshader = glCreateShader(GL_VERTEX_SHADER);
    int lengthvertex = strlen(vertexshader);
    OPENGLCOMMAND(glShaderSource(self->m_vertexshader, 1, &vertexshader, &lengthvertex));
    OPENGLCOMMAND(glCompileShader(self->m_vertexshader));

    int compiled;
    //  std::cout << "C1" << std::endl;
    //  ALOGD("PostShader: C3");
    OPENGLCOMMAND(glGetShaderiv(self->m_vertexshader, GL_COMPILE_STATUS, &compiled));
    if (compiled == GL_FALSE) {
        getLog(self->m_vertexshader);
        //const char * s = str.c_str();
        // ALOGD("Error: %s", s);
        exit(1);
    }

    self->m_pixelshader = glCreateShader(GL_FRAGMENT_SHADER);
    int lengthpixel = strlen(pixelshader);
    OPENGLCOMMAND(glShaderSource(self->m_pixelshader, 1, &pixelshader, &lengthpixel));
    OPENGLCOMMAND(glCompileShader(self->m_pixelshader));

    OPENGLCOMMAND(glGetShaderiv(self->m_pixelshader, GL_COMPILE_STATUS, &compiled));
    //   std::cout << "C2" << std::endl;
    //ALOGD("PostShader: C4");
    if (compiled == GL_FALSE) {
        getLog(self->m_pixelshader);
        //   const char * s = str.c_str();
        //   ALOGD("Error: %s", s);
        exit(1);
    }

    self->m_programid = glCreateProgram();
    OPENGLCOMMAND(glAttachShader(self->m_programid, self->m_vertexshader));
    OPENGLCOMMAND(glAttachShader(self->m_programid, self->m_pixelshader));

    OPENGLCOMMAND(glBindAttribLocation(self->m_programid, 0, "InVertex"));
    OPENGLCOMMAND(glBindAttribLocation(self->m_programid, 1, "InTexCoord0"));

    OPENGLCOMMAND(glLinkProgram(self->m_programid));


    // std::cout << "C3" << std::endl;
    //  ALOGD("PostShader: C5");
    OPENGLCOMMAND(glGetProgramiv(self->m_programid, GL_LINK_STATUS, &compiled));
    if (compiled == GL_FALSE) {
        getLog(self->m_programid);
        //  std::cout << str.c_str() << std::endl;
        exit(1);
    };

    GLuint location, locationdefault;
    location = glGetUniformLocation(self->m_programid, "sampler");

    self->m_transformmatrix = glGetUniformLocation(self->m_programid, "transformmatrix");
    assert(ShowError("c"));
    self->m_screensize = glGetUniformLocation(self->m_programid, "screensize");
    assert(ShowError("c"));
    //  std::cout << "sampler: " << location << std::endl;
    {
        const char* pixelshaderdefault = "\n"
            "\n"
            "           varying  vec2 outtexcoord0;\n"
            "           uniform  sampler2D sampler;\n"
            "            void main()\n"
            "                    {\n"
            "                        vec2 texcoord = outtexcoord0.xy;\n"
            "                     //  texcoord.x = texcoord.x+sin(texcoord.y*20.0)*0.1;\n"
            "                             gl_FragColor = vec4(texture2D(sampler, texcoord).xyz, 1.0)* vec4(1.0, texcoord.x, texcoord.y, 1.0);\n"
            "\n"
            "                    }\n"
            "           \n";

        const char* vertexshaderdefault = "\n"
            "           attribute vec4 InVertex;\n"
            "           attribute vec2 InTexCoord0;\n"
            "           uniform  mat4 transformmatrix;\n"
            "          varying  vec2 outtexcoord0;\n"
            "           void main()\n"
            "           {\n"
            "              gl_Position = transformmatrix * InVertex;\n"
            "               outtexcoord0 = InTexCoord0;\n"
            "           }\n"
            "\n"
            "           \n";
        //default
        self->m_vertexshaderdefault = glCreateShader(GL_VERTEX_SHADER);
        int lengthvertexdefault = strlen(vertexshaderdefault);
        const char* ch = vertexshaderdefault;
        OPENGLCOMMAND(glShaderSource(self->m_vertexshaderdefault, 1, &ch, &lengthvertexdefault));
        OPENGLCOMMAND(glCompileShader(self->m_vertexshaderdefault));

        int compiled;
        //  std::cout << "C1" << std::endl;
        //  ALOGD("PostShader: C3");
        OPENGLCOMMAND(glGetShaderiv(self->m_vertexshaderdefault, GL_COMPILE_STATUS, &compiled));
        if (compiled == GL_FALSE) {
            getLog(self->m_vertexshaderdefault);
            //const char * s = str.c_str();
            //  ALOGD("Error: %s", s);
            exit(1);
        }

        self->m_pixelshaderdefault = glCreateShader(GL_FRAGMENT_SHADER);
        int lengthpixeldefault = strlen(pixelshaderdefault);
        ch = pixelshaderdefault;
        OPENGLCOMMAND(glShaderSource(self->m_pixelshaderdefault, 1, &ch, &lengthpixeldefault));
        OPENGLCOMMAND(glCompileShader(self->m_pixelshaderdefault));

        OPENGLCOMMAND(glGetShaderiv(self->m_pixelshaderdefault, GL_COMPILE_STATUS, &compiled));
        //   std::cout << "C2" << std::endl;
        //ALOGD("PostShader: C4");
        if (compiled == GL_FALSE) {
            getLog(self->m_pixelshaderdefault);
            // const char * s = str.c_str();
            // ALOGD("Error: %s", s);
            exit(1);
        }

        self->m_programiddefault = glCreateProgram();
        OPENGLCOMMAND(glAttachShader(self->m_programiddefault, self->m_vertexshaderdefault));
        OPENGLCOMMAND(glAttachShader(self->m_programiddefault, self->m_pixelshaderdefault));

        OPENGLCOMMAND(glBindAttribLocation(self->m_programiddefault, 0, "InVertex"));
        OPENGLCOMMAND(glBindAttribLocation(self->m_programiddefault, 1, "InTexCoord0"));

        OPENGLCOMMAND(glLinkProgram(self->m_programiddefault));


        // std::cout << "C3" << std::endl;
        // ALOGD("PostShader: C5");
        OPENGLCOMMAND(glGetProgramiv(self->m_programiddefault, GL_LINK_STATUS, &compiled));
        if (compiled == GL_FALSE) {
            //std::string str =
            getLog(self->m_programiddefault);
            // std::cout << str.c_str() << std::endl;
            exit(1);
        };

        locationdefault = glGetUniformLocation(self->m_programiddefault, "sampler");
        self->m_transformmatrixdefault = glGetUniformLocation(self->m_programiddefault, "transformmatrix");
        assert(ShowError("c"));
    }


    GLint oldprogram;
    OPENGLCOMMAND(glGetIntegerv(GL_CURRENT_PROGRAM, &oldprogram));

    /*  GLint vertexLoc = glGetAttribLocation(m_programid, "InVertex");
        GLint texCoord0Loc = glGetAttribLocation(m_programid, "InTexCoord0");
        assert(vertexLoc != -1);
        assert(texCoord0Loc != -1);*/


    OPENGLCOMMAND(glUseProgram(self->m_programid));
    OPENGLCOMMAND(glUniform1i(location, 0));

    OPENGLCOMMAND(glUseProgram(self->m_programiddefault));
    OPENGLCOMMAND(glUniform1i(locationdefault, 0));

    OPENGLCOMMAND(glUseProgram(oldprogram));


    self->m_lastsizex = -1;
    self->m_lastsizey = -1;
    self->m_oldframebufferbound = -1;
    self->m_active = true;
    //   std::cout << "Out Constructor" << std::endl;
    return self;
}

void PostShader_Destroy(struct PostShader* self) {
    OPENGLCOMMAND(glDeleteFramebuffers(1, &self->m_colorframebuffer));
    OPENGLCOMMAND(glDeleteFramebuffers(1, &self->m_depthframebuffer));
    OPENGLCOMMAND(glDeleteTextures(1, &self->m_colortexture));
    OPENGLCOMMAND(glDeleteProgram(self->m_programid));
    assert(self->m_active == false);
}

void PostShader_beginSlow(struct PostShader* self) {
    // ALOGD("PostShader: beginSlow");
    /* GLint dims[4] = {0};
       glGetIntegerv(GL_VIEWPORT, dims);
       GLint fbWidth = dims[2];
       GLint fbHeight = dims[3];
       int width = fbWidth;
       int height = fbHeight;
       m_lastsizex = width;
       m_lastsizey = height;*/
    GLint dims[4] = {0};
    glGetIntegerv(GL_VIEWPORT, dims);
    GLint fbWidth = dims[2];
    GLint fbHeight = dims[3];
    int width = fbWidth;
    int height = fbHeight;
    OPENGLCOMMAND(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &self->m_oldframebufferbound));
    //  ALOGD("Old Framebuffer %i", self->m_oldframebufferbound);
    self->m_active = true;
    // ALOGD("PostShader: begin1");
    if (width != self->m_lastsizex || height != self->m_lastsizey) {

        // ALOGD("PostShader: begin1b");
        GLint oldrenderbufferbound;
        OPENGLCOMMAND(glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldrenderbufferbound));
        self->m_lastsizex = width;
        self->m_lastsizey = height;
        GLint oldtexture;
        OPENGLCOMMAND(glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldtexture));

        //ALOGD("PostShader: begin1bA");
        OPENGLCOMMAND(glBindTexture(GL_TEXTURE_2D, self->m_colortexture));
        OPENGLCOMMAND(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));

        // ALOGD("PostShader: begin1bB");
        OPENGLCOMMAND(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        OPENGLCOMMAND(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        OPENGLCOMMAND(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        OPENGLCOMMAND(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        OPENGLCOMMAND(glBindTexture(GL_TEXTURE_2D, oldtexture));

        //ALOGD("PostShader: begin1bC");
        OPENGLCOMMAND(glBindRenderbuffer(GL_RENDERBUFFER, self->m_depthframebuffer));
        OPENGLCOMMAND(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height));

        //ALOGD("PostShader: begin1bD");
        OPENGLCOMMAND(glBindRenderbuffer(GL_RENDERBUFFER, oldrenderbufferbound));
        OPENGLCOMMAND(glBindFramebuffer(GL_FRAMEBUFFER, self->m_colorframebuffer));
        OPENGLCOMMAND(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, self->m_colortexture, 0));
        //  glBindRenderbuffer(GL_RENDERBUFFER_EXT, m_depthframebuffer);
        OPENGLCOMMAND(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, self->m_depthframebuffer));
        GLenum status;
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        switch(status) {
            case GL_FRAMEBUFFER_COMPLETE:
                //std::cout<<"good" << std::endl;

                // ALOGD("PostShader: begingood");
                break;
            default:
                exit(1);
        }

        // ALOGD("PostShader: begin1c");
    }

    // ALOGD("PostShader: begin2");

    glBindFramebuffer(GL_FRAMEBUFFER, self->m_colorframebuffer);


    // ALOGD("New Framebuffer %i", self->m_colorframebuffer);
    //    std::cout << "New Framebuffer: " << m_colorframebuffer << std::endl;
    GLenum status;
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch(status) {
        case GL_FRAMEBUFFER_COMPLETE:
            // std::cout<<"good" << std::endl;
            //  ALOGD("PostShader: begingood2");
            break;
        default:
            exit(1);
    }
}

void PostShader_begin(struct PostShader* self) {
    //ALOGD("PostShader: begin");
    GLint dims[4] = {0};
    glGetIntegerv(GL_VIEWPORT, dims);
    GLint fbWidth = dims[2];
    GLint fbHeight = dims[3];
    int width = fbWidth;
    int height = fbHeight;
    OPENGLCOMMAND(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &self->m_oldframebufferbound));
    // ALOGD("Old Framebuffer %i", self->m_oldframebufferbound);
    self->m_active = true;
    // ALOGD("PostShader: begin1");
    if (width != self->m_lastsizex || height != self->m_lastsizey) {

        // ALOGD("PostShader: begin1b");
        GLint oldrenderbufferbound;
        OPENGLCOMMAND(glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldrenderbufferbound));
        self->m_lastsizex = width;
        self->m_lastsizey = height;
        GLint oldtexture;
        OPENGLCOMMAND(glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldtexture));

        //ALOGD("PostShader: begin1bA");
        OPENGLCOMMAND(glBindTexture(GL_TEXTURE_2D, self->m_colortexture));
        OPENGLCOMMAND(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));

        //ALOGD("PostShader: begin1bB");
        OPENGLCOMMAND(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        OPENGLCOMMAND(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        OPENGLCOMMAND(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        OPENGLCOMMAND(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        OPENGLCOMMAND(glBindTexture(GL_TEXTURE_2D, oldtexture));

        //ALOGD("PostShader: begin1bC");
        OPENGLCOMMAND(glBindRenderbuffer(GL_RENDERBUFFER, self->m_depthframebuffer));
        OPENGLCOMMAND(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height));

        // ALOGD("PostShader: begin1bD");
        OPENGLCOMMAND(glBindRenderbuffer(GL_RENDERBUFFER, oldrenderbufferbound));
        OPENGLCOMMAND(glBindFramebuffer(GL_FRAMEBUFFER, self->m_colorframebuffer));
        OPENGLCOMMAND(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, self->m_colortexture, 0));
        //  glBindRenderbuffer(GL_RENDERBUFFER_EXT, m_depthframebuffer);
        OPENGLCOMMAND(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, self->m_depthframebuffer));
        GLenum status;
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        switch(status) {
            case GL_FRAMEBUFFER_COMPLETE:
                //std::cout<<"good" << std::endl;

                // ALOGD("PostShader: begingood");
                break;
            default:
                exit(1);
        }

        //ALOGD("PostShader: begin1c");
    }

    // ALOGD("PostShader: begin2");

    glBindFramebuffer(GL_FRAMEBUFFER, self->m_colorframebuffer);


    //ALOGD("New Framebuffer %i", self->m_colorframebuffer);
    //    std::cout << "New Framebuffer: " << m_colorframebuffer << std::endl;
    GLenum status;
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch(status) {
        case GL_FRAMEBUFFER_COMPLETE:
            // std::cout<<"good" << std::endl;
            // ALOGD("PostShader: begingood2");
            break;
        default:
            exit(1);
    }

}

void PostShader_end(struct PostShader* self, struct WarpingGrid* grid, struct BezierSurface* surface, int selx, int sely) {

    assert(self->m_active== true);
    self->m_active = false;
    //ALOGD("PostShader: endSlow");

    OPENGLCOMMAND(glBindFramebuffer(GL_FRAMEBUFFER, self->m_oldframebufferbound));
    //ALOGD("Old Framebuffer: %i " ,self->m_oldframebufferbound );

    GLint oldprogram;
    OPENGLCOMMAND(glGetIntegerv(GL_CURRENT_PROGRAM, &oldprogram));
    GLint oldtexture;
    OPENGLCOMMAND(glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldtexture));
    GLint oldactivetexture;
    OPENGLCOMMAND(glGetIntegerv(GL_ACTIVE_TEXTURE, &oldactivetexture));
    GLint olddepthstate;
    OPENGLCOMMAND(glGetIntegerv(GL_DEPTH_TEST, &olddepthstate));
    GLint oldtexture0state;
    OPENGLCOMMAND(glGetIntegerv(GL_TEXTURE_2D, &oldtexture0state));

    GLint vertexattrib0;
    OPENGLCOMMAND(glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &vertexattrib0));
    GLint vertexattrib1;
    OPENGLCOMMAND(glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &vertexattrib1));

    GLint vertexattrib0size;
    OPENGLCOMMAND(glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_SIZE, &vertexattrib0size));
    GLint vertexattrib1size;
    OPENGLCOMMAND(glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_SIZE, &vertexattrib1size));

    GLint vertexattrib0stride;
    OPENGLCOMMAND(glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &vertexattrib0stride));
    GLint vertexattrib1stride;
    OPENGLCOMMAND(glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &vertexattrib1stride));

    GLint vertexattrib0normalized;
    OPENGLCOMMAND(glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &vertexattrib0normalized));
    GLint vertexattrib1normalized;
    OPENGLCOMMAND(glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &vertexattrib1normalized));

    GLint vertexattrib0type;
    OPENGLCOMMAND(glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_TYPE, &vertexattrib0type));
    GLint vertexattrib1type;
    OPENGLCOMMAND(glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_TYPE, &vertexattrib1type));


    void* vertexpointer0=NULL;
    OPENGLCOMMAND(glGetVertexAttribPointerv(0, GL_VERTEX_ATTRIB_ARRAY_POINTER, &vertexpointer0));
    void* vertexpointer1=NULL;
    OPENGLCOMMAND(glGetVertexAttribPointerv(1, GL_VERTEX_ATTRIB_ARRAY_POINTER, &vertexpointer1));

    GLint olddims[4] = {0};
    glGetIntegerv(GL_VIEWPORT, olddims);

    // ALOGD("PostShader: endSb");
    //TODO: Take here old buffer
    //Restore Color Attachement? and Depth Attachment
    // OPENGLCOMMAND(glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_VIEWPORT_BIT | GL_TRANSFORM_BIT | GL_DEPTH_BUFFER_BIT));
    OPENGLCOMMAND(glActiveTexture(GL_TEXTURE0));
    OPENGLCOMMAND(glEnable(GL_TEXTURE_2D));

    //ALOGD("PostShader: endSc");
    OPENGLCOMMAND(glBindTexture(GL_TEXTURE_2D, self->m_colortexture));
    OPENGLCOMMAND(glViewport(0,0,self->m_lastsizex, self->m_lastsizey));
    //ALOGD("PostShader: endSd");

    float om[16]; {
        float l=0;
        float r=1.0f;
        float b = 0.0f;
        float t = 1.0f;
        float n = 1.0f;
        float f = 2.0f;
        om[0] = 2.0f/(r-l); om[1] = 0.0f; om[2] = 0.0f; om[3] = - (r+l)/(r-l);
        om[4] = 0; om[5] = 2.0f/(t-b); om[6] = 0.0f; om[7] = - (t+b)/(t-b);
        om[8] = 0; om[9] = 0; om[10] = -2.0f/(f-n); om[11] = - (f+n)/(f-n);
        om[12] = 0; om[13] = 0; om[14] = 0; om[15] = 1;

    }
    OPENGLCOMMAND(glEnableVertexAttribArray(0));
    OPENGLCOMMAND(glEnableVertexAttribArray(1));
    float vertices[] = {
        0.0f, 0.0f, -1.5f,
        1.0f, 0.0f, -1.5f,
        0.0f,  1.0f, -1.5f,
        1.0f,  1.0f, -1.5f
    };

    float texCoords[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };

    if (grid) {
        WarpingGrid_bindToGPU(grid);
    } else {
        OPENGLCOMMAND(glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, 4*3, vertices));
        OPENGLCOMMAND(glVertexAttribPointer(1, 2, GL_FLOAT,GL_FALSE, 4*2, texCoords));
    }


    // ALOGD("PostShader: endSe");
    OPENGLCOMMAND(glUseProgram(self->m_programid));
    // ALOGD("PostShader: endSf");
    OPENGLCOMMAND(glUniformMatrix4fv(self->m_transformmatrix, 1,  GL_TRUE, om));
    if (self->m_screensize != -1) {
        OPENGLCOMMAND(glUniform2f(self->m_screensize, self->m_lastsizex, self->m_lastsizey));
    }
    OPENGLCOMMAND(glDisable(GL_CULL_FACE));
    OPENGLCOMMAND(glDisable(GL_DEPTH_TEST));
    OPENGLCOMMAND(glDisable(GL_BLEND));
    //  OPENGLCOMMAND(glDisable(GL_ALPHATEST));
    // ALOGD("PostShader: endSg");


    // OPENGLCOMMAND(glEnable(GL_DEPTH_TEST));
    if (grid) {
        OPENGLCOMMAND(glDrawArrays(GL_TRIANGLES, 0, grid->m_vertices_count));
    } else {
        OPENGLCOMMAND(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }
    //  ALOGD("PostShader: endSh");




    // OPENGLCOMMAND(glClear(GL_COLOR_BUFFER_BIT));

    OPENGLCOMMAND(glVertexAttribPointer(0, vertexattrib0size, vertexattrib0type,vertexattrib0normalized, vertexattrib0stride, vertexpointer0));
    OPENGLCOMMAND(glVertexAttribPointer(1, vertexattrib1size, vertexattrib1type,vertexattrib0normalized, vertexattrib1stride, vertexpointer1));

    if (vertexattrib0 == GL_TRUE) {
        OPENGLCOMMAND(glEnableVertexAttribArray(0));
    } else {
        OPENGLCOMMAND(glDisableVertexAttribArray(0));
    }

    if (vertexattrib1 == GL_TRUE) {
        OPENGLCOMMAND(glEnableVertexAttribArray(1));
    } else {
        OPENGLCOMMAND(glDisableVertexAttribArray(1));
    }
    if (surface) {

        OPENGLCOMMAND(glUseProgram(0));
        OPENGLCOMMAND(glDisable(GL_TEXTURE_2D));
        OPENGLCOMMAND(glMatrixMode(GL_PROJECTION));
        OPENGLCOMMAND(glPushMatrix());
        OPENGLCOMMAND(glLoadIdentity());
        glOrtho(0, 1, 0, 1, 1, 2);

        OPENGLCOMMAND(glMatrixMode(GL_MODELVIEW));
        OPENGLCOMMAND(glPushMatrix());
        OPENGLCOMMAND(glLoadIdentity());
        BezierSurface_drawPoints(surface, selx,sely);

        OPENGLCOMMAND(glMatrixMode(GL_PROJECTION));
        OPENGLCOMMAND(glPopMatrix());

        OPENGLCOMMAND(glMatrixMode(GL_MODELVIEW));

        OPENGLCOMMAND(glPopMatrix());
    }

    OPENGLCOMMAND(glUseProgram(oldprogram));
    OPENGLCOMMAND(glBindTexture(GL_TEXTURE_2D, oldtexture));
    OPENGLCOMMAND(glActiveTexture(oldactivetexture));
    // ALOGD("PostShader: endSi");


    if (oldtexture0state == GL_TRUE) {
        OPENGLCOMMAND(glEnable(GL_TEXTURE_2D));
    } else {
        OPENGLCOMMAND(glDisable(GL_TEXTURE_2D));
    }

    if (olddepthstate == GL_TRUE) {
        OPENGLCOMMAND(glEnable(GL_DEPTH_TEST));
    } else {
        OPENGLCOMMAND(glDisable(GL_DEPTH_TEST));
    }
    OPENGLCOMMAND(glViewport(olddims[0],olddims[1],olddims[2], olddims[3]));


    // ALOGD("PostShader: endSj");
}

void PostShader_endSlow(struct PostShader* self) {

    assert(self->m_active== true);
    self->m_active = false;
    // ALOGD("PostShader: endSlow");

    OPENGLCOMMAND(glBindFramebuffer(GL_FRAMEBUFFER, self->m_oldframebufferbound));
    // ALOGD("Old Framebuffer: %i " , self->m_oldframebufferbound );

    GLint oldprogram;
    OPENGLCOMMAND(glGetIntegerv(GL_CURRENT_PROGRAM, &oldprogram));
    GLint oldtexture;
    OPENGLCOMMAND(glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldtexture));
    GLint oldactivetexture;
    OPENGLCOMMAND(glGetIntegerv(GL_ACTIVE_TEXTURE, &oldactivetexture));
    GLint olddepthstate;
    OPENGLCOMMAND(glGetIntegerv(GL_DEPTH_TEST, &olddepthstate));
    GLint oldtexture0state;
    OPENGLCOMMAND(glGetIntegerv(GL_TEXTURE_2D, &oldtexture0state));

    GLint vertexattrib0;
    OPENGLCOMMAND(glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &vertexattrib0));
    GLint vertexattrib1;
    OPENGLCOMMAND(glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &vertexattrib1));

    GLint vertexattrib0size;
    OPENGLCOMMAND(glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_SIZE, &vertexattrib0size));
    GLint vertexattrib1size;
    OPENGLCOMMAND(glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_SIZE, &vertexattrib1size));

    GLint vertexattrib0stride;
    OPENGLCOMMAND(glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &vertexattrib0stride));
    GLint vertexattrib1stride;
    OPENGLCOMMAND(glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &vertexattrib1stride));

    GLint vertexattrib0normalized;
    OPENGLCOMMAND(glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &vertexattrib0normalized));
    GLint vertexattrib1normalized;
    OPENGLCOMMAND(glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &vertexattrib1normalized));

    GLint vertexattrib0type;
    OPENGLCOMMAND(glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_TYPE, &vertexattrib0type));
    GLint vertexattrib1type;
    OPENGLCOMMAND(glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_TYPE, &vertexattrib1type));


    void* vertexpointer0=NULL;
    OPENGLCOMMAND(glGetVertexAttribPointerv(0, GL_VERTEX_ATTRIB_ARRAY_POINTER, &vertexpointer0));
    void* vertexpointer1=NULL;
    OPENGLCOMMAND(glGetVertexAttribPointerv(1, GL_VERTEX_ATTRIB_ARRAY_POINTER, &vertexpointer1));

    GLint olddims[4] = {0};
    glGetIntegerv(GL_VIEWPORT, olddims);

    // ALOGD("PostShader: endSb");
    //TODO: Take here old buffer
    //Restore Color Attachement? and Depth Attachment
    // OPENGLCOMMAND(glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_VIEWPORT_BIT | GL_TRANSFORM_BIT | GL_DEPTH_BUFFER_BIT));
    OPENGLCOMMAND(glActiveTexture(GL_TEXTURE0));
    OPENGLCOMMAND(glEnable(GL_TEXTURE_2D));

    //ALOGD("PostShader: endSc");
    OPENGLCOMMAND(glBindTexture(GL_TEXTURE_2D, self->m_colortexture));
    OPENGLCOMMAND(glViewport(0,0,self->m_lastsizex, self->m_lastsizey));
    //ALOGD("PostShader: endSd");

    float om[16]; {
        float l=0;
        float r=1.0f;
        float b = 0.0f;
        float t = 1.0f;
        float n = 1.0f;
        float f = 2.0f;
        om[0] = 2.0f/(r-l); om[1] = 0.0f; om[2] = 0.0f; om[3] = - (r+l)/(r-l);
        om[4] = 0; om[5] = 2.0f/(t-b); om[6] = 0.0f; om[7] = - (t+b)/(t-b);
        om[8] = 0; om[9] = 0; om[10] = -2.0f/(f-n); om[11] = - (f+n)/(f-n);
        om[12] = 0; om[13] = 0; om[14] = 0; om[15] = 1;

    }
    OPENGLCOMMAND(glEnableVertexAttribArray(0));
    OPENGLCOMMAND(glEnableVertexAttribArray(1));
    float vertices[] = {
        0.0f, 0.0f, -1.5f,
        1.0f, 0.0f, -1.5f,
        0.0f,  1.0f, -1.5f,
        1.0f,  1.0f, -1.5f
    };

    float texCoords[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };


    OPENGLCOMMAND(glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, 4*3, vertices));
    OPENGLCOMMAND(glVertexAttribPointer(1, 2, GL_FLOAT,GL_FALSE, 4*2, texCoords));



    // ALOGD("PostShader: endSe");
    OPENGLCOMMAND(glUseProgram(self->m_programid));
    //ALOGD("PostShader: endSf");
    OPENGLCOMMAND(glUniformMatrix4fv(self->m_transformmatrix, 1,  GL_TRUE, om));
    if (self->m_screensize != -1) {
        OPENGLCOMMAND(glUniform2f(self->m_screensize, self->m_lastsizex, self->m_lastsizey));
    }
    OPENGLCOMMAND(glDisable(GL_CULL_FACE));
    OPENGLCOMMAND(glDisable(GL_DEPTH_TEST));
    OPENGLCOMMAND(glDisable(GL_BLEND));
    //  OPENGLCOMMAND(glDisable(GL_ALPHATEST));
    //ALOGD("PostShader: endSg");


    // OPENGLCOMMAND(glEnable(GL_DEPTH_TEST));


    OPENGLCOMMAND(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    // ALOGD("PostShader: endSh");


    // OPENGLCOMMAND(glClear(GL_COLOR_BUFFER_BIT));

    OPENGLCOMMAND(glVertexAttribPointer(0, vertexattrib0size, vertexattrib0type,vertexattrib0normalized, vertexattrib0stride, vertexpointer0));
    OPENGLCOMMAND(glVertexAttribPointer(1, vertexattrib1size, vertexattrib1type,vertexattrib0normalized, vertexattrib1stride, vertexpointer1));

    if (vertexattrib0 == GL_TRUE) {
        OPENGLCOMMAND(glEnableVertexAttribArray(0));
    } else {
        OPENGLCOMMAND(glDisableVertexAttribArray(0));
    }

    if (vertexattrib1 == GL_TRUE) {
        OPENGLCOMMAND(glEnableVertexAttribArray(1));
    } else {
        OPENGLCOMMAND(glDisableVertexAttribArray(1));
    }

    OPENGLCOMMAND(glUseProgram(oldprogram));
    OPENGLCOMMAND(glBindTexture(GL_TEXTURE_2D, oldtexture));
    OPENGLCOMMAND(glActiveTexture(oldactivetexture));
    //ALOGD("PostShader: endSi");
    if (olddepthstate == GL_TRUE) {
        OPENGLCOMMAND(glEnable(GL_DEPTH_TEST));
    } else {
        OPENGLCOMMAND(glDisable(GL_DEPTH_TEST));
    }
    if (oldtexture0state == GL_TRUE) {
        OPENGLCOMMAND(glEnable(GL_TEXTURE_2D));
    } else {
        OPENGLCOMMAND(glDisable(GL_TEXTURE_2D));
    }
    OPENGLCOMMAND(glViewport(olddims[0],olddims[1],olddims[2], olddims[3]));


    //ALOGD("PostShader: endSj");
}

/*
   void PostShader_end(struct PostShader* self, struct vec2 v1, struct vec2 v2, struct vec2 v3, struct vec2 v4)
   {

   assert(m_active== true);
   m_active = false;
//ALOGD("PostShader: endSlow");

OPENGLCOMMAND(glBindFramebuffer(GL_FRAMEBUFFER, self->m_oldframebufferbound));
// ALOGD("Old Framebuffer: %i " , self->m_oldframebufferbound );



GLint oldprogram;
OPENGLCOMMAND(glGetIntegerv(GL_CURRENT_PROGRAM, &oldprogram));
GLint oldtexture;
OPENGLCOMMAND(glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldtexture));
GLint oldactivetexture;
OPENGLCOMMAND(glGetIntegerv(GL_ACTIVE_TEXTURE, &oldactivetexture));
GLint olddepthstate;
OPENGLCOMMAND(glGetIntegerv(GL_DEPTH_TEST, &olddepthstate));
GLint oldtexture0state;
OPENGLCOMMAND(glGetIntegerv(GL_TEXTURE_2D, &oldtexture0state));

GLint vertexattrib0;
OPENGLCOMMAND(glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &vertexattrib0));
GLint vertexattrib1;
OPENGLCOMMAND(glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &vertexattrib1));


GLint vertexattrib0size;
OPENGLCOMMAND(glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_SIZE, &vertexattrib0size));
GLint vertexattrib1size;
OPENGLCOMMAND(glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_SIZE, &vertexattrib1size));

GLint vertexattrib0stride;
OPENGLCOMMAND(glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &vertexattrib0stride));
GLint vertexattrib1stride;
OPENGLCOMMAND(glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &vertexattrib1stride));

GLint vertexattrib0normalized;
OPENGLCOMMAND(glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &vertexattrib0normalized));
GLint vertexattrib1normalized;
OPENGLCOMMAND(glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &vertexattrib1normalized));

GLint vertexattrib0type;
OPENGLCOMMAND(glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_TYPE, &vertexattrib0type));
GLint vertexattrib1type;
OPENGLCOMMAND(glGetVertexAttribiv(1, GL_VERTEX_ATTRIB_ARRAY_TYPE, &vertexattrib1type));


void* vertexpointer0=NULL;
OPENGLCOMMAND(glGetVertexAttribPointerv(0, GL_VERTEX_ATTRIB_ARRAY_POINTER, &vertexpointer0));
void* vertexpointer1=NULL;
OPENGLCOMMAND(glGetVertexAttribPointerv(1, GL_VERTEX_ATTRIB_ARRAY_POINTER, &vertexpointer1));

GLint olddims[4] = {0};
glGetIntegerv(GL_VIEWPORT, olddims);

//ALOGD("PostShader: endSb");
//TODO: Take here old buffer
//Restore Color Attachement? and Depth Attachment
// OPENGLCOMMAND(glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_VIEWPORT_BIT | GL_TRANSFORM_BIT | GL_DEPTH_BUFFER_BIT));
OPENGLCOMMAND(glActiveTexture(GL_TEXTURE0));
OPENGLCOMMAND(glEnable(GL_TEXTURE_2D));

//ALOGD("PostShader: endSc");
OPENGLCOMMAND(glBindTexture(GL_TEXTURE_2D, self->m_colortexture));
OPENGLCOMMAND(glViewport(0,0,self->m_lastsizex, self->m_lastsizey));
// ALOGD("PostShader: endSd");

float om[16]; {
    float l=0;
    float r=m_lastsizex;
    float b = 0.0f;
    float t = m_lastsizey;
    float n = 1.0f;
    float f = 2.0f;
    om[0] = 2.0f/(r-l); om[1] = 0.0f; om[2] = 0.0f; om[3] = - (r+l)/(r-l);
    om[4] = 0; om[5] = 2.0f/(t-b); om[6] = 0.0f; om[7] = - (t+b)/(t-b);
    om[8] = 0; om[9] = 0; om[10] = -2.0f/(f-n); om[11] = - (f+n)/(f-n);
    om[12] = 0; om[13] = 0; om[14] = 0; om[15] = 1;

}

OPENGLCOMMAND(glEnableVertexAttribArray(0));
OPENGLCOMMAND(glEnableVertexAttribArray(1));
float vertices[] = {
    v1.x, v1.y, -1.5f,
    v2.x, v2.y, -1.5f,
    v3.x,  v3.y, -1.5f,
    v4.x,  v4.y, -1.5f
};

float texCoords[] = {
    v1.x / float(self->m_lastsizex), v1.y / float(self->m_lastsizey),
    v2.x / float(self->m_lastsizex), v2.y / float(self->m_lastsizey),
    v3.x / float(self->m_lastsizex), v3.y / float(self->m_lastsizey),
    v4.x / float(self->m_lastsizex), v4.y / float(self->m_lastsizey)
};

float verticesfull[] = {
    0.0f, 0.0f, -1.5f,
    float(self->m_lastsizex), 0.0f, -1.5f,
    0.0f,  float(self->m_lastsizey), -1.5f,
    float(self->m_lastsizex),  float(self->m_lastsizey), -1.5f
};

float texCoordsfull[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    0.0f, 1.0f,
    1.0f, 1.0f
};


OPENGLCOMMAND(glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, 4*3, verticesfull));
OPENGLCOMMAND(glVertexAttribPointer(1, 2, GL_FLOAT,GL_FALSE, 4*2, texCoordsfull));

if(m_prevshader) {
    OPENGLCOMMAND(glBindTexture(GL_TEXTURE_2D, self->m_prevshader->m_colortexture));
}

// ALOGD("PostShader: endSe");
OPENGLCOMMAND(glUseProgram(self->m_programiddefault));
//ALOGD("PostShader: endSf");
OPENGLCOMMAND(glUniformMatrix4fv(self->m_transformmatrixdefault, 1,  GL_TRUE, om));

OPENGLCOMMAND(glDisable(GL_CULL_FACE));
OPENGLCOMMAND(glDisable(GL_DEPTH_TEST));
OPENGLCOMMAND(glDisable(GL_BLEND));
//  OPENGLCOMMAND(glDisable(GL_ALPHATEST));
//ALOGD("PostShader: endSg");


// OPENGLCOMMAND(glEnable(GL_DEPTH_TEST));


OPENGLCOMMAND(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
// ALOGD("PostShader: endSh");


OPENGLCOMMAND(glBindTexture(GL_TEXTURE_2D, self->m_colortexture));
OPENGLCOMMAND(glUseProgram(m_programid));
// ALOGD("PostShader: endSf");
OPENGLCOMMAND(glUniformMatrix4fv(self->m_transformmatrix, 1,  GL_TRUE, om));
if (m_screensize != -1) {
    OPENGLCOMMAND(glUniform2f(self->m_screensize, self->m_lastsizex, self->m_lastsizey));
}

OPENGLCOMMAND(glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, 4*3, vertices));
OPENGLCOMMAND(glVertexAttribPointer(1, 2, GL_FLOAT,GL_FALSE, 4*2, texCoords));


OPENGLCOMMAND(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));


// OPENGLCOMMAND(glClear(GL_COLOR_BUFFER_BIT));

OPENGLCOMMAND(glVertexAttribPointer(0, vertexattrib0size, vertexattrib0type,vertexattrib0normalized, vertexattrib0stride, vertexpointer0));
OPENGLCOMMAND(glVertexAttribPointer(1, vertexattrib1size, vertexattrib1type,vertexattrib0normalized, vertexattrib1stride, vertexpointer1));

if (vertexattrib0 == GL_TRUE) {
    OPENGLCOMMAND(glEnableVertexAttribArray(0));
} else {
    OPENGLCOMMAND(glDisableVertexAttribArray(0));
}

if (vertexattrib1 == GL_TRUE) {
    OPENGLCOMMAND(glEnableVertexAttribArray(1));
} else {
    OPENGLCOMMAND(glDisableVertexAttribArray(1));
}

OPENGLCOMMAND(glUseProgram(oldprogram));
OPENGLCOMMAND(glBindTexture(GL_TEXTURE_2D, oldtexture));
OPENGLCOMMAND(glActiveTexture(oldactivetexture));
//ALOGD("PostShader: endSi");
if (olddepthstate == GL_TRUE) {
    OPENGLCOMMAND(glEnable(GL_DEPTH_TEST));
} else {
    OPENGLCOMMAND(glDisable(GL_DEPTH_TEST));
}
if (oldtexture0state == GL_TRUE) {
    OPENGLCOMMAND(glEnable(GL_TEXTURE_2D));
} else {
    OPENGLCOMMAND(glDisable(GL_TEXTURE_2D));
}
OPENGLCOMMAND(glViewport(olddims[0],olddims[1],olddims[2], olddims[3]));


//ALOGD("PostShader: endSj");
}*/
/*
   void BlurShader::createBlurArray(float sigma)
   {
   if (sigma < 0.01) {
   int k = 0;
   mBlurCenter = k;
   mBlurCount = 2*k+1;
   mBlurArray[0] = 1.0f;
   return;
   }

   int k = std::max(1, (int)(sigma*1.5f));
   mBlurCenter = k;
   mBlurCount = 2*k+1;

   float mul = 1.0f/(float)(std::sqrt(2*M_PI)*sigma);
   float expmul = -1.0f/(2.0f*sigma*sigma);
   for (int i = 0; i <= k; i++) {
   float value = mul*(float)std::exp(expmul*(float)(i*i));
   mBlurArray[k-i] = value;
   mBlurArray[k+i] = value;
   }
   float sum = 0.0f;
   for (int i = 0; i < mBlurCount; i++) {
   sum += mBlurArray[i];
   }
   sum = 1.0f/sum;
   for (int i = 0; i< mBlurCount; i++) {
   mBlurArray[i] = mBlurArray[i]*sum;
   }
   }

   BlurShader::BlurShader()
   {

   ALOGD("BlurShader DF");
   createBlurArray(6);
   std::string sp = R"(
   varying mediump vec2 outtexcoord0;

   uniform mediump sampler2D sampler;
   void main()
   {
   vec2 texcoord = outtexcoord0.xy;
   texcoord.x = texcoord.x+sin(texcoord.y*20.0)*0.1;
   gl_FragColor = texture2D(sampler, texcoord);

   }
   )";

   std::string sp2 = R"(
   varying mediump vec2 outtexcoord0;

   uniform mediump sampler2D sampler;
   void main()
   {
   mediump vec2 texcoord = outtexcoord0.xy;
//   texcoord.x = texcoord.x+sin(texcoord.y*20.0)*0.1;
gl_FragColor = vec4(texture2D(sampler, texcoord).xyz, 1.0)*0.5
+vec4(texture2D(sampler, texcoord+vec2(2.0/240.0,0.0)).xyz, 1.0)*0.25
+vec4(texture2D(sampler, texcoord+vec2(-2.0/240.0,0.0)).xyz, 1.0)*0.25;

}
)";

std::string sp3 = R"(

varying mediump vec2 outtexcoord0;
uniform mediump sampler2D sampler;
void main()
{
                mediump vec2 texcoord = outtexcoord0.xy;
              //  texcoord.x = texcoord.x+sin(texcoord.y*20.0)*0.1;
                      gl_FragColor = vec4(texture2D(sampler, texcoord).xyz, 1.0)*0.5
                            +vec4(texture2D(sampler, texcoord+vec2(0,2.0/480.0)).xyz, 1.0)*0.25
                            +vec4(texture2D(sampler, texcoord+vec2(0,-2.0/480.0)).xyz, 1.0)*0.25;

             }
    )";

std::string sv = R"(
    attribute vec4 InVertex;
    attribute vec2 InTexCoord0;
    uniform mediump mat4 transformmatrix;
    varying mediump vec2 outtexcoord0;
    void main()
    {
       gl_Position = transformmatrix * InVertex;
        outtexcoord0 = InTexCoord0;
    }

    )";
std::string pre = R"(

                             varying mediump vec2 outtexcoord0;
                             uniform mediump sampler2D sampler;
                             uniform mediump vec2 screensize;
                              void main()
                                      {
                                         mediump vec2 texcoord = outtexcoord0.xy;
                                       //  texcoord.x = texcoord.x+sin(texcoord.y*20.0)*0.1;
                                               gl_FragColor =
                             )";
std::string post = R"(
                                         }
                                )";

ALOGD("BlurShader DF2");
std::stringstream blurshader_vertical;
blurshader_vertical << pre;
std::stringstream blurshader_horizontal;
blurshader_horizontal << pre;
ALOGD("BlurShader DF2");
bool needplus = false;
for (int i = 0; i < mBlurCount; i++) {
    if(needplus) {
        blurshader_vertical <<" + ";
        blurshader_horizontal << " + ";
    }
    int pixelshift = i-mBlurCenter;
    blurshader_vertical << "vec4(texture2D(sampler, texcoord+vec2(0,"<<pixelshift<<".0/screensize.y)).xyz, 1.0)*" << mBlurArray[i] << std::endl;
    blurshader_horizontal << "vec4(texture2D(sampler, texcoord+vec2("<<pixelshift<<".0/screensize.y, 0)).xyz, 1.0)*" << mBlurArray[i] << std::endl;
    //   mBlurArray[k-i] = value;
    //   mBlurArray[k+i] = value;
    needplus = true;
}

blurshader_vertical << ";" << std::endl;
blurshader_horizontal <<  ";"<< std::endl;
blurshader_vertical <<  post<< std::endl;
blurshader_horizontal <<  post<< std::endl;

std::string vert = blurshader_vertical.str();
std::string horz = blurshader_horizontal.str();

ALOGD("%s", vert.c_str());
ALOGD("%s", horz.c_str());

m_firstshader = new PostShader(sv.c_str(), vert.c_str());
m_secondshader = new PostShader(sv.c_str(), horz.c_str());
m_firstshader->m_prevshader = m_secondshader;
}
BlurShader::~BlurShader()
{
    delete m_firstshader;
    delete m_secondshader;
}
void BlurShader::begin()
{
    m_firstshader->beginSlow();
    m_secondshader->beginSlow();
}
void BlurShader::end()
{
    m_secondshader->endSlow();
    m_firstshader->endSlow();

}
void BlurShader::end(const android::vec2& v1, const android::vec2& v2, const android::vec2& v3, const android::vec2& v4)
{
    m_secondshader->endSlow();
    m_firstshader->end(v1, v2, v3, v4);

}


void BlurShader::end(const android::vec4& v1, const android::vec4& v2, const android::vec4& v3, const android::vec4& v4)
{
    ALOGD("v1 %f, %f", v1.x, v1.y);
    ALOGD("v2 %f, %f", v2.x, v2.y);
    ALOGD("v3 %f, %f", v3.x, v3.y);
    ALOGD("v4 %f, %f", v4.x, v4.y);

    end(android::vec2(v1.x, v1.y), android::vec2(v2.x, v2.y), android::vec2(v3.x, v3.y), android::vec2(v4.x, v4.y));
}*/
/*void BlurShader::end(float x1, float y1, float x2, float y2)
  {

  }*/

struct WarpingGrid* WarpingGrid_Create(int x, int y) {
    struct WarpingGrid* self = (struct WarpingGrid*) malloc(sizeof(struct WarpingGrid));
    self->m_sizex = x;
    self->m_sizey = y;
    self->m_grid=malloc(sizeof(struct vec2)*x*y);
    self->m_vertices = NULL;
    self->m_vertices_count = 0;
    return self;
}


void WarpingGrid_Destroy(struct WarpingGrid* self) {
    if (self->m_vertices) {
        free(self->m_vertices);
    }
    free(self);
}

void WarpingGrid_set(struct WarpingGrid* self, int i, int j, struct vec2 pos) {
    assert(i >= 0 && i < self->m_sizex);
    assert(j >= 0 && j < self->m_sizey);
    self->m_grid[self->m_sizex*j+i] = pos;
}

struct vec2 WarpingGrid_get(struct WarpingGrid* self, int i, int j) {
    return self->m_grid[self->m_sizex*j+i];
}

void WarpingGrid_createMesh(struct WarpingGrid* self) {
    glFlush();
    if (self->m_vertices && self->m_vertices_count != 0 && self->m_vertices_count != (self->m_sizey-1)*(self->m_sizex-1)*6) {
        free(self->m_vertices);

        self->m_vertices = NULL;
        self->m_vertices_count = 0;
    }
    if (self->m_vertices == NULL) {
        self->m_vertices_count = (self->m_sizey-1)*(self->m_sizex-1)*2*3;
        self->m_vertices = malloc(sizeof(struct WarpingGridVertex)*self->m_vertices_count);
    }
    int writeindex = 0;
    for (int j = 0; j< self->m_sizey-1; j++) {
        for (int i = 0; i< self->m_sizex-1; i++) {
            float u = ((float)i)/((float)self->m_sizex-1);
            float v = ((float)j)/((float)self->m_sizey-1);
            float u1 = ((float)(i+1))/((float)self->m_sizex-1);
            float v1 = ((float)(j+1))/((float)self->m_sizey-1);
            self->m_vertices[writeindex] = WarpingGridVertex_Create(WarpingGrid_get(self, i,j), u,v);
            writeindex++;
            self->m_vertices[writeindex] = WarpingGridVertex_Create(WarpingGrid_get(self, i+1,j), u1,v);
            writeindex++;
            self->m_vertices[writeindex] = WarpingGridVertex_Create(WarpingGrid_get(self, i+1,j+1), u1,v1);
            writeindex++;
            self->m_vertices[writeindex] = WarpingGridVertex_Create(WarpingGrid_get(self, i,j), u,v);
            writeindex++;
            self->m_vertices[writeindex] = WarpingGridVertex_Create(WarpingGrid_get(self, i+1,j+1), u1,v1);
            writeindex++;
            self->m_vertices[writeindex] = WarpingGridVertex_Create(WarpingGrid_get(self, i,j+1), u,v1);
            writeindex++;
        }
    }
    assert(writeindex == self->m_vertices_count);
}

void WarpingGrid_bindToGPU(struct WarpingGrid* self) {
    struct WarpingGridVertex* grid = &self->m_vertices[0];
    OPENGLCOMMAND(glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, sizeof(struct WarpingGridVertex), &grid->m_posx));
    OPENGLCOMMAND(glVertexAttribPointer(1, 2, GL_FLOAT,GL_FALSE, sizeof(struct WarpingGridVertex), &grid->m_u));

    /*#define BYTESHIFTME(input, shift)\
     *((float*)((unsigned char*)(input)+(shift)))

     ALOGD("m_warping: bindToGPUX %f %f %f ", BYTESHIFTME(&grid->m_posx, 0), BYTESHIFTME(&grid->m_posx, sizeof(WarpingGridVertex)), BYTESHIFTME(&grid->m_posx, sizeof(WarpingGridVertex)*2));
     ALOGD("m_warping: bindToGPUY %f %f %f ", BYTESHIFTME(&grid->m_posx, 4), BYTESHIFTME(&grid->m_posx, sizeof(WarpingGridVertex)+4), BYTESHIFTME(&grid->m_posx, sizeof(WarpingGridVertex)*2+4));
     ALOGD("m_warping: bindToGPUZ %f %f %f ", BYTESHIFTME(&grid->m_posx, 8), BYTESHIFTME(&grid->m_posx, sizeof(WarpingGridVertex)+8), BYTESHIFTME(&grid->m_posx, sizeof(WarpingGridVertex)*2+8));*/
}


struct vec2 vec2_set(float x, float y) {
    struct vec2 v;
    v.x = x;
    v.y = y;
    return v;
}


void BezierSurface_Destroy(struct BezierSurface* self) {
    free(self);
}

struct BezierSurface* BezierSurface_Create() {
    struct BezierSurface* self = (struct BezierSurface*)malloc(sizeof(struct BezierSurface));
    for (int j = 0; j < POINTS_OF_BEZIER; j++) {
        for (int i = 0; i < POINTS_OF_BEZIER; i++) {
            self->m_controlpoints[i][j] =vec2_set(0,0);
        }
    }
    return self;
}

void BezierSurface_initBezierSurface(struct BezierSurface* self, struct vec2 minvalue, struct vec2 maxvalue) {
    for (int j = 0; j < POINTS_OF_BEZIER; j++) {
        for (int i = 0; i < POINTS_OF_BEZIER; i++) {
            self->m_controlpoints[i][j] = vec2_set(minvalue.x + ((float)i)/((float)(POINTS_OF_BEZIER-1))*(maxvalue.x-minvalue.x),
                    minvalue.y + ((float)j)/((float)(POINTS_OF_BEZIER-1))*(maxvalue.y-minvalue.y));
            //printf("BezierSurface::m_controlpoints: (%f, %f) -> (%f, %f)", (float)i, (float)j, self->m_controlpoints[i][j] .x, self->m_controlpoints[i][j] .y);
        }
    }
}

void BezierSurface_writeTo(struct BezierSurface* self, struct WarpingGrid* grid) {
    for (int j = 0; j < grid->m_sizey; j++) {
        for (int i = 0; i < grid->m_sizex; i++) {
            WarpingGrid_set(grid, i, j, BezierSurface_eval(self, ((float) i)/((float) (grid->m_sizex-1)),((float) j)/((float) (grid->m_sizey-1))));
        }
    }
}

void BezierSurface_drawPoints(struct BezierSurface* self, int xselected, int yselected) {
    //printf("BezierSurface_drawPoints");+glPointSize
    glPointSize(10);
    glBegin(GL_POINTS);
    for (int j = 0; j < POINTS_OF_BEZIER; j++) {
        for (int i = 0; i < POINTS_OF_BEZIER; i++) {
            if (i == xselected && j == yselected) {
                glColor3f(1,0,0);
            } else {
                glColor3f(0,1,0);
            }
            glVertex3f(self->m_controlpoints[i][j].x, self->m_controlpoints[i][j].y, -1.5f);
        }
    }
    glEnd();
    glBegin(GL_LINES);
    for (int j = 0; j < POINTS_OF_BEZIER; j++) {
        for (int i = 0; i < POINTS_OF_BEZIER; i++) {
            glColor3f(1,0,0);
            if (j < POINTS_OF_BEZIER-1) {
                glVertex3f(self->m_controlpoints[i][j].x, self->m_controlpoints[i][j].y, -1.5f);
                glVertex3f(self->m_controlpoints[i][j+1].x, self->m_controlpoints[i][j+1].y, -1.5f);
            }
            if (i < POINTS_OF_BEZIER-1) {
                glVertex3f(self->m_controlpoints[i][j].x, self->m_controlpoints[i][j].y, -1.5f);
                glVertex3f(self->m_controlpoints[i+1][j].x, self->m_controlpoints[i+1][j].y, -1.5f);
            }
        }
    }
    glEnd();
    glColor3f(1,1,1);
}

void BezierSurface_writeToFile(struct BezierSurface* self, const char* filename) {
    FILE * f;
    f = fopen(filename, "w");
    int version;
    version = 1;
    fprintf(f, "Version: %i\n", version);
    fprintf(f, "Controlpoints: %i\n", POINTS_OF_BEZIER);

    for (int j = 0; j < POINTS_OF_BEZIER; j++) {
        for (int i = 0; i < POINTS_OF_BEZIER; i++) {

            fprintf(f, "%f %f\n", self->m_controlpoints[i][j].x, self->m_controlpoints[i][j].y);
        }
    }
    fclose(f);
}

void BezierSurface_readFromFile(struct BezierSurface* self, const char* filename) {
    FILE * f;
    f = fopen(filename, "r");
    int version;
    version = 1;
    fscanf(f, "Version: %i\n", &version);
    fscanf(f, "Controlpoints: %i\n", &version);
    for (int j = 0; j < POINTS_OF_BEZIER; j++) {
        for (int i = 0; i < POINTS_OF_BEZIER; i++) {
            int amount = fscanf(f, "%f %f\n", &self->m_controlpoints[i][j].x, &self->m_controlpoints[i][j].y);
            //printf("read %f %f (amount %i)\n", self->m_controlpoints[i][j].x, self->m_controlpoints[i][j].y, amount);
        }
    }
    fclose(f);
}

void BezierSurface_writeToCPPFile(struct BezierSurface* self, const char* filename) {
    FILE * f;
    f = fopen(filename, "w");
    int version;
    version = 1;
    fprintf(f, "//Warping Bezier Surface: Version: %i\n", version);

    fprintf(f, "void createWarpingBezier(BezierSurface* input)\n");
    fprintf(f, "{\n");
    for (int j = 0; j < POINTS_OF_BEZIER; j++) {
        for (int i = 0; i < POINTS_OF_BEZIER; i++) {
            fprintf(f, "input->m_controlpoints[%i][%i] = android::vec2(%f, %f);\n", i,j,self->m_controlpoints[i][j].x, self->m_controlpoints[i][j].y);
        }
    }

    fprintf(f, "}\n");
    fclose(f);
}

struct vec2 BezierSurface_eval(struct BezierSurface*self, double u, double v) {
    struct vec2 res = vec2_set(0,0);
    for (int j = 0; j < POINTS_OF_BEZIER; j++) {
        for (int i = 0; i < POINTS_OF_BEZIER; i++) {
            res.x += bernstein(POINTS_OF_BEZIER-1, i, u)*bernstein(POINTS_OF_BEZIER-1,j,v)* self->m_controlpoints[i][j].x;
            res.y += bernstein(POINTS_OF_BEZIER-1, i, u)*bernstein(POINTS_OF_BEZIER-1,j,v)* self->m_controlpoints[i][j].y;
        }
    }

    //printf("BezierSurface::eval: (%f, %f) -> (%f, %f)\n", (float)u, (float)v, res.x, res.y);
    return res;
}

