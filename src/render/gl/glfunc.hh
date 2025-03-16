#pragma once

#ifdef __linux__
    #define GL_GLEXT_PROTOTYPES
    #include <GL/glcorearb.h>
#elif defined _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    #include "glcorearb.h"
    #include <gl/GL.h>

extern void (*glActiveTexture)(GLenum texture);
extern void (*glUseProgram)(GLuint program);
extern GLint (*glGetUniformLocation)(GLuint program, const GLchar *name);
extern void (*glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void (*glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void (*glUniform3fv)(GLint location, GLsizei count, const GLfloat *value);
extern void (*glUniform4fv)(GLint location, GLsizei count, const GLfloat *value);
extern void (*glUniform1iv)(GLint location, GLsizei count, const GLint *value);
extern void (*glUniform1i)(GLint location, GLint v0);
extern void (*glUniform1f)(GLint location, GLfloat v0);
extern void (*glBindVertexArray)(GLuint array);
extern void (*glBindFramebuffer)(GLenum target, GLuint framebuffer);
extern GLuint (*glCreateProgram)(void);
extern void (*glAttachShader)(GLuint program, GLuint shader);
extern void (*glLinkProgram)(GLuint program);
extern void (*glGetProgramiv)(GLuint program, GLenum pname, GLint *params);
extern void (*glGetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void (*glDeleteProgram)(GLuint program);
extern void (*glValidateProgram)(GLuint program);
extern void (*glDeleteShader)(GLuint shader);
extern GLuint (*glCreateShader)(GLenum type);
extern void (*glShaderSource)(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
extern void (*glCompileShader)(GLuint shader);
extern void (*glGetShaderiv)(GLuint shader, GLenum pname, GLint *params);
extern void (*glGetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void (*glGetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
extern void (*glGenVertexArrays)(GLsizei n, GLuint *arrays);
extern void (*glGenBuffers)(GLsizei n, GLuint *buffers);
extern void (*glBindBuffer)(GLenum target, GLuint buffer);
extern void (*glBufferData)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
extern void (*glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
extern void (*glVertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
extern void (*glEnableVertexAttribArray)(GLuint index);
extern void (*glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);

extern void (*glDebugMessageCallbackARB)(GLDEBUGPROCARB callback, const void *userParam);

#else
    #error "gl platform"
#endif
