#ifndef TINYGP_GL_H_INCLUDED
#define TINYGP_GL_H_INCLUDED

#include "tinygp.h"
#include <GLES2/gl2.h>
#include <stdbool.h>
#include <stdio.h>

#ifndef TGPGL_OFFSETOF
#define TGPGL_OFFSETOF offsetof
#include <stddef.h>
#endif

/**** header *****/

#if defined(GL_ES_VERSION_3_0) && GL_ES_VERSION_3_0
#define TGPGL_GLES3
#elif defined(GL_ES_VERSION_2_0) && GL_ES_VERSION_2_0
#define TGPGL_GLES2
#endif

#define TGPGL_GLSL_VERSION_STR_SIZE 32

typedef struct {
    tgp_context* tgpctx;
    GLuint       gl_version;
    char         glsl_version_str[TGPGL_GLSL_VERSION_STR_SIZE];
    GLuint       vbo;
    GLuint       shader_handle;

    GLint  attrib_location_tex;
    GLint  attrib_location_vtx_pos;
    GLint  attrib_location_vtx_uv;
    GLint  attrib_location_vtx_color;
    GLuint white_texture;
} tgpgl_context;

TGPDEF void tgpgl_init_context(tgpgl_context* ctx, tgp_context* tgpctx);
TGPDEF void tgpgl_destroy_context(tgpgl_context* ctx);
TGPDEF void tgpgl_render(tgpgl_context* ctx);

/**** implementation *****/
// #ifdef TINYGPGL_IMPLEMENTATION

static bool tgpgl_check_shader(tgpgl_context* ctx, GLuint handle,
                               const char* desc) {
    GLint status = 0, log_length = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);

    if ((GLboolean)status == GL_FALSE) {
        fprintf(stderr,
                "error: tgpgl_create_device_objects(): failed to compile %s! "
                "GLSL %s",
                desc, ctx->glsl_version_str);
    }
    if (log_length > 1) {
        char buf[log_length];
        glGetShaderInfoLog(handle, log_length, NULL, buf);
        fprintf(stderr, "%s\n", buf);
    }
    return (GLboolean)status == GL_TRUE;
}

static bool tgpgl_check_program(tgpgl_context* ctx, GLuint handle,
                                const char* desc) {
    GLint status = 0, log_length = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);

    if ((GLboolean)status == GL_FALSE) {
        fprintf(stderr,
                "error: tgpgl_create_device_objects(): failed to link %s! "
                "GLSL %s",
                desc, ctx->glsl_version_str);
    }
    if (log_length > 1) {
        char buf[log_length];
        glGetProgramInfoLog(handle, log_length, NULL, buf);
        fprintf(stderr, "%s\n", buf);
    }
    return (GLboolean)status == GL_TRUE;
}

static void tgpgl_create_device_objects(tgpgl_context* ctx) {
    TINYGP_ASSERT(ctx != NULL);

    static const GLchar* vertex_shader_glsl_120 =
        "attribute vec2 coord;\n"
        "attribute vec2 uv;\n"
        "attribute vec4 color;\n"
        "varying vec2 fragUV;\n"
        "varying vec4 fragColor;\n"
        "void main() {\n"
        "    fragUV = uv;\n"
        "    fragColor = color;\n"
        "    gl_Position = vec4(coord.xy, 0.0, 1.0);\n"
        "}\n";

    static const GLchar* fragment_shader_glsl_120 =
        "#ifdef GL_ES\n"
        "precision mediump float;\n"
        "#endif\n"
        "uniform sampler2D tex;\n"
        "varying vec2 fragUV;\n"
        "varying vec4 fragColor;\n"
        "void main() {\n"
        "    gl_FragColor = fragColor * texture2D(tex, fragUV.st);\n"
        "}\n";

    // parse GLSL version
    int glsl_version = 130;
    sscanf(ctx->glsl_version_str, "#version %d", &glsl_version);

    // select matching GLSL shader
    const GLchar* vertex_shader = NULL;
    const GLchar* fragment_shader = NULL;
    if (glsl_version < 130) {
        vertex_shader = vertex_shader_glsl_120;
        fragment_shader = fragment_shader_glsl_120;
    } else {
        TINYGP_ASSERT(false && "not implemented");
    }

    // create shaders
    const GLchar* vertex_shader_with_version[2] = {ctx->glsl_version_str,
                                                   vertex_shader};
    GLuint        vert_handle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_handle, 2, vertex_shader_with_version, NULL);
    glCompileShader(vert_handle);
    tgpgl_check_shader(ctx, vert_handle, "vertex shader");

    const GLchar* fragment_shader_with_version[2] = {ctx->glsl_version_str,
                                                     fragment_shader};
    GLuint        frag_handle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_handle, 2, fragment_shader_with_version, NULL);
    glCompileShader(frag_handle);
    tgpgl_check_shader(ctx, frag_handle, "fragment shader");

    // link shaders
    ctx->shader_handle = glCreateProgram();
    glAttachShader(ctx->shader_handle, vert_handle);
    glAttachShader(ctx->shader_handle, frag_handle);
    glLinkProgram(ctx->shader_handle);
    tgpgl_check_program(ctx, ctx->shader_handle, "shader program");

    // the shaders are now linked into our program, we can get rid of them
    glDetachShader(ctx->shader_handle, vert_handle);
    glDetachShader(ctx->shader_handle, frag_handle);
    glDeleteShader(vert_handle);
    glDeleteShader(frag_handle);

    // find attributes
    ctx->attrib_location_tex = glGetUniformLocation(ctx->shader_handle, "tex");
    ctx->attrib_location_vtx_pos =
        glGetAttribLocation(ctx->shader_handle, "coord");
    ctx->attrib_location_vtx_uv = glGetAttribLocation(ctx->shader_handle, "uv");
    ctx->attrib_location_vtx_color =
        glGetAttribLocation(ctx->shader_handle, "color");

    // create buffers
    glGenBuffers(1, &ctx->vbo);

    // create a white texture
    uint8_t data[4 * 4 * 4];
    for (int i = 0; i < 4 * 4 * 4; i++) {
        data[i] = 255;
    }

    glGenTextures(1, &ctx->white_texture);
    glBindTexture(GL_TEXTURE_2D, ctx->white_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 data);
}

static inline void tgpgl_destroy_device_objects(tgpgl_context* ctx) {
    glDeleteBuffers(1, &ctx->vbo);
    glDeleteProgram(ctx->shader_handle);
    glDeleteTextures(1, &ctx->white_texture);
}

TGPDEF void tgpgl_init_context(tgpgl_context* ctx, tgp_context* tgpctx) {
    TINYGP_ASSERT(ctx != NULL && tgpctx != NULL);
    memset(ctx, 0, sizeof(*ctx));

    ctx->tgpctx = tgpctx;

// find out GL version
#if !defined(TGPGL_GLES2)
    GLint major = 0;
    GLint minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if (major == 0 && minor == 0) {
        // in desktop GL 2.x, the GL_VERSION string starts with major.minor
        const char* gl_version = (const char*)glGetString(GL_VERSION);
        sscanf(gl_version, "%d.%d", &major, &minor);
    }
    ctx->gl_version = (GLuint)(major * 100 + minor * 10);
#else
    ctx->gl_version = 200;
#endif

    // determine the GLSL version string (will be prepended to the shader
    // sources)
#if defined(TGPGL_GLES2)
    const char* glsl_version = "#version 100";
#elif defined(TGPGL_GLES3)
    const char* glsl_version = "#version 300 es";
#elif defined(__APPLE__)
    const char* glsl_version = "#version 150";
#else
    const char* glsl_version = "#version 130";
#endif
    TINYGP_ASSERT(strlen(glsl_version) + 2 < TGPGL_GLSL_VERSION_STR_SIZE);
    strcpy(ctx->glsl_version_str, glsl_version);
    strcat(ctx->glsl_version_str, "\n");

    tgpgl_create_device_objects(ctx);
}

TGPDEF void tgpgl_destroy_context(tgpgl_context* ctx) {
    if (ctx != NULL) {
        tgpgl_destroy_device_objects(ctx);
        free(ctx);
    }
}

static void tgpgl_setup_render_state(tgpgl_context* ctx) {
    TINYGP_ASSERT(ctx != NULL);

    // enable alpha blending, disable face culling, disable depth testing,
    // enable scissor
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                        GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_SCISSOR_TEST);
    // TODO: set glPolygonMode() and GL_PRIMITIVE_RESTART

    glUseProgram(ctx->shader_handle);
    glUniform1i(ctx->attrib_location_tex, 0);

    // bind vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);

    // setup attributes for tgp_vertex
    glEnableVertexAttribArray(ctx->attrib_location_vtx_pos);
    glEnableVertexAttribArray(ctx->attrib_location_vtx_uv);
    glEnableVertexAttribArray(ctx->attrib_location_vtx_color);
    glVertexAttribPointer(ctx->attrib_location_vtx_pos, 2, GL_FLOAT, GL_FALSE,
                          sizeof(tgp_vertex),
                          (GLvoid*)TGPGL_OFFSETOF(tgp_vertex, position));
    glVertexAttribPointer(ctx->attrib_location_vtx_uv, 2, GL_FLOAT, GL_FALSE,
                          sizeof(tgp_vertex),
                          (GLvoid*)TGPGL_OFFSETOF(tgp_vertex, texcoord));
    glVertexAttribPointer(ctx->attrib_location_vtx_color, 4, GL_FLOAT, GL_FALSE,
                          sizeof(tgp_vertex),
                          (GLvoid*)TGPGL_OFFSETOF(tgp_vertex, color));

    // TODO
    glBindTexture(GL_TEXTURE_2D, ctx->white_texture);
}

TGPDEF void tgpgl_render(tgpgl_context* ctx) {
    TINYGP_ASSERT(ctx != NULL);
    tgp_context* tgpctx = ctx->tgpctx;

    // setup desired GL state
    tgpgl_setup_render_state(ctx);

    // render draw commands
    uint32_t    i = 0;
    tgp_command cmd;

    while (tgp_get_command_p(tgpctx, &cmd, i++)) {
        switch (cmd.type) {
        case TGP_COMMAND_VIEWPORT:
            glViewport(cmd.data.viewport.x, cmd.data.viewport.y,
                       cmd.data.viewport.w, cmd.data.viewport.h);
            break;
        case TGP_COMMAND_SCISSOR:
            glScissor(cmd.data.scissor.x, cmd.data.scissor.y,
                      cmd.data.scissor.w, cmd.data.scissor.h);
            break;
        case TGP_COMMAND_DRAW: {
            tgp_draw_command draw = cmd.data.draw;

            // upload vertices
            GLsizeiptr vtxbuf_size = sizeof(tgp_vertex) * draw.num_vertices;
            glBufferData(GL_ARRAY_BUFFER, vtxbuf_size,
                         &tgpctx->vertices[draw.vertex_idx], GL_STREAM_DRAW);

            // draw
            glDrawArrays(GL_TRIANGLES, 0, draw.num_vertices);
            break;
        }
        case TGP_COMMAND_NONE: break;
        }
    }
}

// #endif // TINYGPGL_IMPLEMENTATION
#endif // TINYGP_GL_H_INCLUDED
