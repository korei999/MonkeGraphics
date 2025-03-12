#include "glfunc.hh"

void (*glActiveTexture)(GLenum texture);
void (*glUseProgram)(GLuint program);
GLint (*glGetUniformLocation)(GLuint program, const GLchar *name);
void (*glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void (*glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void (*glUniform3fv)(GLint location, GLsizei count, const GLfloat *value);
void (*glUniform4fv)(GLint location, GLsizei count, const GLfloat *value);
void (*glUniform1iv)(GLint location, GLsizei count, const GLint *value);
void (*glUniform1i)(GLint location, GLint v0);
void (*glUniform1f)(GLint location, GLfloat v0);
void (*glBindVertexArray)(GLuint array);
void (*glBindFramebuffer)(GLenum target, GLuint framebuffer);
GLuint (*glCreateProgram)(void);
void (*glAttachShader)(GLuint program, GLuint shader);
void (*glLinkProgram)(GLuint program);
void (*glGetProgramiv)(GLuint program, GLenum pname, GLint *params);
void (*glGetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
void (*glDeleteProgram)(GLuint program);
void (*glValidateProgram)(GLuint program);
void (*glDeleteShader)(GLuint shader);
GLuint (*glCreateShader)(GLenum type);
void (*glShaderSource)(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
void (*glCompileShader)(GLuint shader);
void (*glGetShaderiv)(GLuint shader, GLenum pname, GLint *params);
void (*glGetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
void (*glGetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
void (*glGenVertexArrays)(GLsizei n, GLuint *arrays);
void (*glGenBuffers)(GLsizei n, GLuint *buffers);
void (*glBindBuffer)(GLenum target, GLuint buffer);
void (*glBufferData)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
void (*glVertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
void (*glEnableVertexAttribArray)(GLuint index);
void (*glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);

void (*glDebugMessageCallbackARB)(GLDEBUGPROCARB callback, const void *userParam);
