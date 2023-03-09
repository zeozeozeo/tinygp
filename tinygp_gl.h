#ifndef TINYGP_GL2_H_INCLUDED
#define TINYGP_GL2_H_INCLUDED

#include "tinygp.h"
#include <stdbool.h>
#include <stdio.h>

#ifndef TGPGL_OFFSETOF
#define TGPGL_OFFSETOF offsetof
#include <stddef.h>
#endif

/**** header *****/

// vertex arrays are not supported by ES2/WebGL1, but Emscripten may support
// them as an extension
#ifndef GL_ES_VERSION_2_0
#define TINYGPGL_USE_VERTEX_ARRAY
#elif defined(__EMSCRIPTEN__)
#define TINYGPGL_USE_VERTEX_ARRAY
#define glBindVertexArray glBindVertexArrayOES
#define glGenVertexArrays glGenVertexArraysOES
#define glDeleteVertexArrays glDeleteVertexArraysOES
#define GL_VERTEX_ARRAY_BINDING GL_VERTEX_ARRAY_BINDING_OES
#endif

// desktop GL 3.3+ has glBindSampler()
#if !defined(GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0) &&              \
    defined(GL_VERSION_3_3)
#define TINYGPGL_HAS_BIND_SAMPLER
#endif

// desktop GL 3.1+ has GL_PRIMITIVE_RESTART state
#if !defined(GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0) &&              \
    defined(GL_VERSION_3_1)
#define TINYGPGL_HAS_PRIMITIVE_RESTART
#endif

#ifndef TINYGPGL_GLSL_VERSION_STRING_LEN
#define TINYGPGL_GLSL_VERSION_STRING_LEN 32
#endif

typedef struct {
    tgp_context* tgpctx;
    GLuint gl_version;
    char glsl_version_str[TINYGPGL_GLSL_VERSION_STRING_LEN];
    GLint attrib_location_tex;    // iChannel0
    GLuint attrib_location_coord; // coord
    GLuint attrib_location_color; // color
    uint32_t vbo_handle, elements_handle;
    GLuint shader_handle;
} tgpgl_context;

typedef struct {
    GLint enabled, size, type, normalized, stride;
    GLvoid* ptr;
} _tgpgl_vtxattrib_state;

TGPDEF void tgpgl_init_context(tgpgl_context* ctx, tgp_context* tgpctx);
TGPDEF void tgpgl_destroy_context(tgpgl_context* ctx);
TGPDEF void tgpgl_render(tgpgl_context* ctx);

/**** implementation ****/
#ifdef TGPGL_IMPLEMENTATION

static void _tgpgl_vtxattrib_get_state(_tgpgl_vtxattrib_state* state,
                                       GLint index) {
    glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &state->enabled);
    glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_SIZE, &state->size);
    glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_TYPE, &state->type);
    glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED,
                        &state->normalized);
    glGetVertexAttribiv(index, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &state->stride);
    glGetVertexAttribPointerv(index, GL_VERTEX_ATTRIB_ARRAY_POINTER,
                              &state->ptr);
}

static void _tgpgl_vtxattrib_set_state(_tgpgl_vtxattrib_state* state,
                                       GLint index) {
    glVertexAttribPointer(index, state->size, state->type,
                          (GLboolean)state->normalized, state->stride,
                          state->ptr);
    if (state->enabled) {
        glEnableVertexAttribArray(index);
    } else {
        glDisableVertexAttribArray(index);
    }
}

static bool _tgpgl_check_shader(tgpgl_context* ctx, GLuint handle,
                                const char* desc, const char* source_start,
                                const char* source) {
    GLint status = 0, log_length = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
    if ((GLboolean)status == GL_FALSE) {
        fprintf(stderr, "error: failed to compile %s. GLSL %s\n", desc,
                ctx->glsl_version_str);
    }
    if (log_length > 1) {
        char buf[log_length + 1];
        glGetShaderInfoLog(handle, log_length, NULL, buf);
        fprintf(stderr,
                "-------- SHADER SOURCE "
                "--------\n%s%s-------------------------------\n\n%s\n",
                source_start, source, buf);
    }
    return (GLboolean)status == GL_TRUE;
}

static bool _tgpgl_check_program(tgpgl_context* ctx, GLuint handle,
                                 const char* desc) {
    GLint status = 0, log_length = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
    if ((GLboolean)status == GL_FALSE) {
        fprintf(stderr, "error: failed to link %s. GLSL %s", desc,
                ctx->glsl_version_str);
    }
    if (log_length > 1) {
        GLchar buf[log_length + 1];
        glGetProgramInfoLog(handle, log_length, NULL, buf);
        fprintf(stderr, "%s\n", buf);
    }
    return (GLboolean)status == GL_TRUE;
}

static void _tgpgl_create_device_objects(tgpgl_context* ctx) {
    // backup GL state
    GLint last_array_buffer;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
#ifdef TINYGPGL_USE_VERTEX_ARRAY
    GLint last_vertex_array;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
#endif

    // parse GLSL string
    int glsl_version = 100;
    sscanf(ctx->glsl_version_str, "#version %d", &glsl_version);

    // vertex shaders
    static const GLchar* vertex_shader_glsl_120 =
        "attribute vec4 coord;\n"
        "attribute vec4 color;\n"
        "varying vec2 texUV;\n"
        "varying vec4 fragColor;\n"
        "void main() {\n"
        "    gl_Position = vec4(coord.xy, 0.0, 1.0);\n"
        "    texUV = coord.zw;\n"
        "    fragColor = color;\n"
        "}\n";

    static const GLchar* vertex_shader_glsl_130 =
        "in vec4 coord;\n"
        "in vec4 color;\n"
        "out vec2 texUV;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    gl_Position = vec4(coord.xy, 0.0, 1.0);\n"
        "    texUV = coord.zw;\n"
        "    fragColor = color;\n"
        "}\n";

    static const GLchar* vertex_shader_glsl_300_es =
        "precision highp float;\n"
        "layout(location=0) in vec4 coord;\n"
        "layout(location=1) in vec4 color;\n"
        "out vec2 texUV;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    gl_Position = vec4(coord.xy, 0.0, 1.0);\n"
        "    texUV = coord.zw;\n"
        "    fragColor = color;\n"
        "}\n";

    static const GLchar* vertex_shader_glsl_410_core =
        "layout(location=0) in vec4 coord;\n"
        "layout(location=1) in vec4 color;\n"
        "out vec2 texUV;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    gl_Position = vec4(coord.xy, 0.0, 1.0);\n"
        "    texUV = coord.zw;\n"
        "    fragColor = color;\n"
        "}\n";

    // fragment shaders
    static const GLchar* fragment_shader_glsl_120 =
        "#ifdef GL_ES\n"
        "    precision mediump float;\n"
        "#endif\n"
        "uniform sampler2D iChannel0;\n"
        "uniform vec4 iColor;\n"
        "varying vec2 texUV;\n"
        "void main() {\n"
        "    gl_FragColor = texture2D(iChannel0, texUV) * iColor;\n"
        "}\n";

    static const GLchar* fragment_shader_glsl_130 =
        "uniform sampler2D iChannel0;\n"
        "in vec4 iColor;\n"
        "in vec2 texUV;\n"
        "out vec4 Out_Color;\n"
        "void main() {\n"
        "    Out_Color = texture2D(iChannel0, texUV) * iColor;\n"
        "}\n";

    static const GLchar* fragment_shader_glsl_300_es =
        "precision mediump float;\n"
        "uniform sampler2D iChannel0;\n"
        "in vec4 iColor;\n"
        "in vec2 texUV;\n"
        "layout(location=0) out vec4 Out_Color;\n"
        "void main() {\n"
        "    Out_Color = texture2D(iChannel0, texUV) * iColor;\n"
        "}\n";

    static const GLchar* fragment_shader_glsl_410_core =
        "uniform sampler2D iChannel0;\n"
        "in vec4 iColor;\n"
        "in vec2 texUV;\n"
        "layout(location=0) out vec4 Out_Color;\n"
        "void main() {\n"
        "    Out_Color = texture2D(iChannel0, texUV) * iColor;\n"
        "}\n";

    // select matching shaders
    const GLchar* vertex_shader = NULL;
    const GLchar* fragment_shader = NULL;
    if (glsl_version < 130) {
        vertex_shader = vertex_shader_glsl_120;
        fragment_shader = fragment_shader_glsl_120;
    } else if (glsl_version >= 410) {
        vertex_shader = vertex_shader_glsl_410_core;
        fragment_shader = fragment_shader_glsl_410_core;
    } else if (glsl_version == 300) {
        vertex_shader = vertex_shader_glsl_300_es;
        fragment_shader = fragment_shader_glsl_300_es;
    } else {
        vertex_shader = vertex_shader_glsl_130;
        fragment_shader = fragment_shader_glsl_130;
    }

    // create shaders
    const GLchar* vertex_shader_with_version[2] = {ctx->glsl_version_str,
                                                   vertex_shader};
    GLuint vertex_handle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_handle, 2, vertex_shader_with_version, NULL);
    glCompileShader(vertex_handle);
    _tgpgl_check_shader(ctx, vertex_handle, "vertex shader",
                        ctx->glsl_version_str, vertex_shader);

    const GLchar* fragment_shader_with_version[2] = {ctx->glsl_version_str,
                                                     fragment_shader};
    GLuint fragment_handle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_handle, 2, fragment_shader_with_version, NULL);
    glCompileShader(fragment_handle);
    _tgpgl_check_shader(ctx, fragment_handle, "fragment shader",
                        ctx->glsl_version_str, fragment_shader);

    // link shaders
    ctx->shader_handle = glCreateProgram();
    glAttachShader(ctx->shader_handle, vertex_handle);
    glAttachShader(ctx->shader_handle, fragment_handle);
    glLinkProgram(ctx->shader_handle);
    _tgpgl_check_program(ctx, ctx->shader_handle, "shader program");

    glDetachShader(ctx->shader_handle, vertex_handle);
    glDetachShader(ctx->shader_handle, fragment_handle);
    glDeleteShader(vertex_handle);
    glDeleteShader(fragment_handle);

    ctx->attrib_location_tex =
        glGetUniformLocation(ctx->shader_handle, "iChannel0");
    ctx->attrib_location_coord =
        glGetUniformLocation(ctx->shader_handle, "coord");
    ctx->attrib_location_color =
        glGetUniformLocation(ctx->shader_handle, "color");

    // create buffers
    glGenBuffers(1, &ctx->vbo_handle);
    glGenBuffers(1, &ctx->elements_handle);

    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
#ifdef TINYGPGL_USE_VERTEX_ARRAY
    glBindVertexArray(last_vertex_array);
#endif
}

TGPDEF void tgpgl_init_context(tgpgl_context* ctx, tgp_context* tgpctx) {
    TINYGP_ASSERT(ctx != NULL && tgpctx != NULL);
    memset(ctx, 0, sizeof(*ctx));
    ctx->tgpctx = tgpctx;

// query GL version
#ifdef GL_ES_VERSION_2_0
    ctx->gl_version = 200;
#else
    GLint major = 0;
    GLint minor = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    if (major == 0 && minor == 0) {
        // in desktop GL 2.x the string starts with major.minor
        const char* gl_version = (const char*)glGetString(GL_VERSION);
        sscanf(gl_version, "%d.%d", &major, &minor);
    }
    ctx->gl_version = (GLuint)(major * 100 + minor * 10);
#endif

    // determine GLSL version
#if defined(GL_ES_VERSION_2_0)
    const char* glsl_version = "#version 100";
#elif defined(GL_ES_VERSION_3_0)
    const char* glsl_version = "#version 300 es";
#elif defined(__APPLE__)
    const char* glsl_version = "#version 150";
#else
    const char* glsl_version = "#version 130";
#endif

    TINYGP_ASSERT(strlen(glsl_version) + 2 < TINYGPGL_GLSL_VERSION_STRING_LEN);
    strcpy(ctx->glsl_version_str, glsl_version);
    strcat(ctx->glsl_version_str, "\n");

    // do a test GL call (if your program crashes here, your OpenGL loader
    // probably doesn't work)
    {
        GLint current_texture;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &current_texture);
    }

    _tgpgl_create_device_objects(ctx);
}

TGPDEF void tgpgl_destroy_context(tgpgl_context* ctx) {
    if (ctx != NULL) {
        if (ctx->vbo_handle) {
            glDeleteBuffers(1, &ctx->vbo_handle);
            ctx->vbo_handle = 0;
        }
        if (ctx->elements_handle) {
            glDeleteBuffers(1, &ctx->elements_handle);
            ctx->elements_handle = 0;
        }
        if (ctx->shader_handle) {
            glDeleteProgram(ctx->shader_handle);
            ctx->shader_handle = 0;
        }
        free(ctx);
    }
}

static void _tgpgl_setup_render_state(tgpgl_context* ctx, int width, int height,
                                      GLuint vertex_array_object) {
    // alpha blending enabled, no face culling, no depth testing,
    // enable scissor, fill polygons
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE,
                        GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_SCISSOR_TEST);
#ifdef TINYGPGL_HAS_PRIMITIVE_RESTART
    if (ctx->gl_version >= 310) {
        glDisable(GL_PRIMITIVE_RESTART);
    }
#endif
#ifdef GL_POLYGON_MODE
    // desktop GL 2.0+ have glPolygonMode, while ES does not
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

    glUseProgram(ctx->shader_handle);
    glUniform1i(ctx->attrib_location_tex, 0);

#ifdef TINYGPGL_HAS_BIND_SAMPLER
    if (ctx->gl_version >= 330) {
        // make sure GL 3.3+ apps don't force us to use their sampler state
        glBindSampler(0, 0);
    }
#endif

#ifdef TINYGPGL_USE_VERTEX_ARRAY
    glBindVertexArray(vertex_array_object);
#else
    (void)vertex_array_object;
#endif

    // bind vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo_handle);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->elements_handle);

    glEnableVertexAttribArray(ctx->attrib_location_coord);
    glEnableVertexAttribArray(ctx->attrib_location_color);

    glVertexAttribPointer(ctx->attrib_location_coord, 4, GL_FLOAT, GL_FALSE,
                          sizeof(tgp_vertex),
                          (GLvoid*)TGPGL_OFFSETOF(tgp_vertex, position));

    glVertexAttribPointer(ctx->attrib_location_color, 4, GL_FLOAT, GL_FALSE,
                          sizeof(tgp_vertex),
                          (GLvoid*)TGPGL_OFFSETOF(tgp_vertex, color));
}

TGPDEF void tgpgl_render(tgpgl_context* ctx) {
    // make sure you called tgpgl_init_context()!
    TINYGP_ASSERT(ctx != NULL && ctx->shader_handle);
    tgp_context* tgpctx = ctx->tgpctx;
    if (tgpctx->screen_size.w <= 0 || tgpctx->screen_size.h <= 0) {
        return;
    }

    // backup OpenGL state
    GLenum last_active_texture;
    glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
    glActiveTexture(GL_TEXTURE0);
    GLuint last_program;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&last_program);
    GLuint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&last_texture);
#ifdef TINYGPGL_HAS_BIND_SAMPLER
    GLuint last_sampler;
    if (ctx.gl_version >= 330) {
        glGetIntegerv(GL_SAMPLER_BINDING, (GLint*)&last_sampler);
    } else {
        last_sampler = 0;
    }
#endif
    GLuint last_array_buffer;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint*)&last_array_buffer);
#ifndef TINYGPGL_USE_VERTEX_ARRAY
    // VAO on OpenGL 3.0+ and OpenGL ES 3.0+.
    GLint last_element_array_buffer;
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);

    _tgpgl_vtxattrib_state last_vtx_attrib_state_pos;
    _tgpgl_vtxattrib_get_state(&last_vtx_attrib_state_pos,
                               ctx->attrib_location_coord);

    _tgpgl_vtxattrib_state last_vtx_attrib_state_color;
    _tgpgl_vtxattrib_get_state(&last_vtx_attrib_state_color,
                               ctx->attrib_location_color);
#endif
#ifdef TINYGPGL_USE_VERTEX_ARRAY
    GLuint last_vertex_array_object;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, (GLint*)&last_vertex_array_object);
#endif
#ifdef GL_POLYGON_MODE
    GLint last_polygon_mode[2];
    glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
#endif
    GLint last_viewport[4];
    glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLint last_scissor_box[4];
    glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
    GLenum last_blend_src_rgb;
    glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
    GLenum last_blend_dst_rgb;
    glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
    GLenum last_blend_src_alpha;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
    GLenum last_blend_dst_alpha;
    glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
    GLenum last_blend_equation_rgb;
    glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
    GLenum last_blend_equation_alpha;
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
    GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
    GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
    GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
    GLboolean last_enable_stencil_test = glIsEnabled(GL_STENCIL_TEST);
    GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
#ifdef TINYGPGL_HAS_PRIMITIVE_RESTART
    GLboolean last_enable_primitive_restart =
        (ctx.gl_version >= 310) ? glIsEnabled(GL_PRIMITIVE_RESTART) : GL_FALSE;
#endif

    // setup desired GL state
    GLuint vertex_array_object = 0;
#ifdef TINYGPGL_USE_VERTEX_ARRAY
    glGenVertexArrays(1, &vertex_array_object);
#endif
    _tgpgl_setup_render_state(ctx, tgpctx->screen_size.w, tgpctx->screen_size.h,
                              vertex_array_object);

    // we don't call glViewport here, because tgp_begin adds a viewport
    // command for us

    // render draw commands
    uint32_t idx = 0;
    tgp_command cmd;
    while (tgp_get_command_p(ctx->tgpctx, &cmd, idx++)) {
        switch (cmd.type) {
        case TGP_COMMAND_VIEWPORT: {
            tgp_irect viewport = cmd.data.viewport;
            glViewport(viewport.x, viewport.y, viewport.w, viewport.h);
            break;
        }
        case TGP_COMMAND_SCISSOR: {
            tgp_irect scissor = cmd.data.scissor;
            glScissor(scissor.x, scissor.y, scissor.w, scissor.h);
            break;
        }
        case TGP_COMMAND_DRAW: {
            tgp_draw_command draw = cmd.data.draw;
            if (draw.num_vertices == 0) {
                break;
            }
            GLsizeiptr vertices_size =
                (GLsizeiptr)draw.num_vertices * sizeof(tgp_vertex);
            glBufferData(GL_ARRAY_BUFFER, vertices_size,
                         &tgpctx->vertices[draw.vertex_idx], GL_STREAM_DRAW);
            break;
        }
        case TGP_COMMAND_NONE: break;
        }
    }

    // destroy the temporary VAO
#ifdef TINYGPGL_USE_VERTEX_ARRAY
    glDeleteVertexArrays(1, &vertex_array_object);
#endif

    // restore the OpenGL state
    if (glIsProgram(last_program)) {
        glUseProgram(last_program);
    }
    glBindTexture(GL_TEXTURE_2D, last_texture);
#ifdef TINYGPGL_HAS_BIND_SAMPLER
    if (ctx->gl_version >= 330) {
        glBindSampler(0, last_sampler);
    }
#endif
    glActiveTexture(last_active_texture);
#ifdef TINYGPGL_USE_VERTEX_ARRAY
    glBindVertexArray(last_vertex_array_object);
#endif
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
#ifndef TINYGPGL_USE_VERTEX_ARRAY
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
    _tgpgl_vtxattrib_set_state(&last_vtx_attrib_state_pos,
                               ctx->attrib_location_coord);
    _tgpgl_vtxattrib_set_state(&last_vtx_attrib_state_color,
                               ctx->attrib_location_color);
#endif
    glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
    glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb,
                        last_blend_src_alpha, last_blend_dst_alpha);
    if (last_enable_blend) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
    }
    if (last_enable_cull_face) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
    if (last_enable_depth_test) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    if (last_enable_stencil_test) {
        glEnable(GL_STENCIL_TEST);
    } else {
        glDisable(GL_STENCIL_TEST);
    }
    if (last_enable_scissor_test) {
        glEnable(GL_SCISSOR_TEST);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }
#ifdef GL_POLYGON_MODE
    glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
#endif
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2],
               (GLsizei)last_viewport[3]);
    glScissor(last_scissor_box[0], last_scissor_box[1],
              (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

#endif // TGPGL_IMPLEMENTATION
#endif // TINYGP_GL2_H_INCLUDED
