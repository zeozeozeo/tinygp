// SEE LICENSING INFORMATION AT THE BOTTOM OF THE FILE
// tinygp.h (Tiny Graphics Painter)
//
// USAGE
//   1. Define TINYGP_IMPLEMENTATION in *one* .c/.cpp file (translation unit) to
//   generate the function definitions:
//
//     #define TINYGP_IMPLEMENTATION
//     #include "tinygp.h"
//
//   2. Create the context:
//
//     tgp_context ctx;
//     tgp_options opts = tgp_default_options();
//     tinygp_init_context(&ctx, &opts);
//
//   and also don't forget to destroy the context after you're done with it:
//     tgp_destroy_context(&ctx);

#ifndef TINYGP_H_INCLUDED
#define TINYGP_H_INCLUDED

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef TINYGP_ASSERT
#define TINYGP_ASSERT assert
#include <assert.h>
#endif

#ifndef TINYGP_DEFAULT_MAX_VERTICES
#define TINYGP_DEFAULT_MAX_VERTICES 65536
#endif
#ifndef TINYGP_DEFAULT_MAX_COMMANDS
#define TINYGP_DEFAULT_MAX_COMMANDS 16384
#endif
#ifndef TINYGP_TRANSFORM_STACK_DEPTH
#define TINYGP_TRANSFORM_STACK_DEPTH 16
#endif

#define TGP_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define TGP_MAX(a, b) (((a) > (b)) ? (a) : (b))

#ifndef TGPDEF
#define TGPDEF extern
#endif

#ifndef TGP_BATCH_OPTIMIZER_DEPTH
#define TGP_BATCH_OPTIMIZER_DEPTH 8
#endif

// TINYGP_USERDATA_TYPE
// TINYGP_COMPARE_USERDATA

/***** header *****/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int x, y, w, h;
} tgp_irect;

typedef struct {
    float x, y, w, h;
} tgp_rect;

typedef struct {
    float x1, y1, x2, y2;
} tgp_region;

typedef struct {
    int w, h;
} tgp_size;

typedef struct {
    float x, y;
} tgp_vec2;

typedef struct {
    tgp_vec2 a, b;
} tgp_line;

typedef struct {
    tgp_vec2 a, b, c;
} tgp_triangle;

typedef struct {
    float v[2][3];
} tgp_mat2x3;

typedef struct {
    float r, g, b, a;
} tgp_color;

typedef struct {
    tgp_vec2 position, texcoord;
    tgp_color color;
} tgp_vertex;

typedef enum {
    TGP_COMMAND_NONE = 0,
    TGP_COMMAND_VIEWPORT,
    TGP_COMMAND_SCISSOR,
    TGP_COMMAND_DRAW,
} tgp_command_type;

typedef struct {
    uint32_t vertex_idx;
    uint32_t num_vertices;
    tgp_region region;
} tgp_draw_command;

typedef struct {
    tgp_command_type type;
    union {
        tgp_irect viewport;
        tgp_irect scissor;
        tgp_draw_command draw;
    } data;

#ifdef TINYGP_USERDATA_TYPE
    TINYGP_USERDATA_TYPE userdata;
#endif
} tgp_command;

typedef struct {
    uint32_t max_vertices;
    uint32_t max_commands;
} tgp_options;

typedef struct {
    tgp_size screen_size;
    tgp_irect viewport;
    tgp_irect scissor;
    uint32_t max_vertices, cur_vertex;
    tgp_vertex* vertices;
    uint32_t max_commands, cur_command;
    tgp_command* commands;
    tgp_mat2x3 proj;
    tgp_mat2x3 transform;
    tgp_mat2x3 mvp;
    uint8_t cur_transform;
    tgp_mat2x3 transform_stack[TINYGP_TRANSFORM_STACK_DEPTH];
    tgp_color color;

#ifdef TINYGP_USERDATA_TYPE
    TINYGP_USERDATA_TYPE current_userdata;
#endif
} tgp_context;

TGPDEF tgp_options tgp_default_options();
TGPDEF void tgp_init_context(tgp_context* ctx, tgp_options* opts);
TGPDEF void tgp_destroy_context(tgp_context* ctx);
TGPDEF void tgp_begin(tgp_context* ctx, int width, int height);
TGPDEF tgp_command* tgp_get_command(tgp_context* ctx, uint32_t index);
TGPDEF bool tgp_get_command_p(tgp_context* ctx, tgp_command* cmd,
                              uint32_t index);
TGPDEF void tgp_project(tgp_context* ctx, float left, float right, float top,
                        float bottom);
TGPDEF void tgp_reset_projection(tgp_context* ctx);
TGPDEF void tgp_push_transform(tgp_context* ctx);
TGPDEF void tgp_pop_transform(tgp_context* ctx);
TGPDEF void tgp_reset_transform(tgp_context* ctx);
TGPDEF void tgp_translate(tgp_context* ctx, float x, float y);
TGPDEF void tgp_scale(tgp_context* ctx, float sx, float sy);
TGPDEF void tgp_scale_at(tgp_context* ctx, float sx, float sy, float x,
                         float y);
TGPDEF void tgp_rotate(tgp_context* ctx, float theta);
TGPDEF void tgp_rotate_at(tgp_context* ctx, float theta, float x, float y);
TGPDEF void tgp_set_color(tgp_context* ctx, float r, float g, float b, float a);
TGPDEF void tgp_reset_color(tgp_context* ctx);
TGPDEF void tgp_viewport(tgp_context* ctx, int x, int y, int w, int h);
TGPDEF void tgp_reset_viewport(tgp_context* ctx);
TGPDEF void tgp_scissor(tgp_context* ctx, int x, int y, int w, int h);
TGPDEF void tgp_reset_scissor(tgp_context* ctx);
TGPDEF void tgp_reset_state(tgp_context* ctx);
TGPDEF void tgp_clear(tgp_context* ctx);
TGPDEF void tgp_draw_filled_triangles(tgp_context* ctx,
                                      const tgp_triangle* triangles,
                                      uint32_t count);
TGPDEF void tgp_draw_filled_triangle(tgp_context* ctx, float x1, float y1,
                                     float x2, float y2, float x3, float y3);

/***** implementation *****/
#ifdef TINYGP_IMPLEMENTATION

static const tgp_mat2x3 _tgp_default_transform = {
    {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}};

TGPDEF tgp_options tgp_default_options() {
    return (tgp_options){
        .max_vertices = TINYGP_DEFAULT_MAX_VERTICES,
        .max_commands = TINYGP_DEFAULT_MAX_COMMANDS,
    };
}

TGPDEF void tgp_init_context(tgp_context* ctx, tgp_options* opts) {
    TINYGP_ASSERT(ctx != NULL);
    memset(ctx, 0, sizeof(*ctx));
    ctx->max_vertices = opts->max_vertices;
    ctx->max_commands = opts->max_commands;

    // allocate buffers
    ctx->vertices = malloc(opts->max_vertices * sizeof(tgp_vertex));
    ctx->commands = malloc(opts->max_commands * sizeof(tgp_command));
    TINYGP_ASSERT(ctx->vertices != NULL && ctx->commands != NULL);

    ctx->transform = _tgp_default_transform;
}

TGPDEF void tgp_destroy_context(tgp_context* ctx) {
    TINYGP_ASSERT(ctx != NULL);
    if (ctx->vertices != NULL) {
        free(ctx->vertices);
    }
    if (ctx->commands != NULL) {
        free(ctx->commands);
    }
}

static inline tgp_mat2x3 _tgp_mult_proj_and_transform_matrices(tgp_mat2x3* p,
                                                               tgp_mat2x3* t) {
    float x = p->v[0][0];
    float y = p->v[1][1];
    return (tgp_mat2x3){{
        {x * t->v[0][0], x * t->v[0][1], x * t->v[0][2] + p->v[0][2]},
        {y * t->v[1][0], y * t->v[1][1], y * t->v[1][2] + p->v[1][2]},
    }};
}

static inline void _tgp_update_mvp(tgp_context* ctx) {
    ctx->mvp =
        _tgp_mult_proj_and_transform_matrices(&ctx->proj, &ctx->transform);
}

TGPDEF void tgp_project(tgp_context* ctx, float left, float right, float top,
                        float bottom) {
    float w = right - left;
    float h = top - bottom;
    ctx->proj = (tgp_mat2x3){{
        {2.0f / w, 0.0f, -(right + left) / w},
        {0.0f, 2.0f / h, -(top + bottom) / h},
    }};
    _tgp_update_mvp(ctx);
}

static inline tgp_mat2x3 _tgp_default_projection(int w, int h) {
    TINYGP_ASSERT(w > 0 && h > 0);
    return (tgp_mat2x3){
        {{2.0f / (float)w, 0.0f, -1.0f}, {0.0f, -2.0f / (float)h, 1.0f}}};
}

TGPDEF void tgp_reset_projection(tgp_context* ctx) {
    ctx->proj = _tgp_default_projection(ctx->viewport.w, ctx->viewport.h);
    _tgp_update_mvp(ctx);
}

TGPDEF void tgp_push_transform(tgp_context* ctx) {
    TINYGP_ASSERT(ctx->cur_transform < TINYGP_TRANSFORM_STACK_DEPTH);
    ctx->transform_stack[ctx->cur_transform++] = ctx->transform;
}

TGPDEF void tgp_pop_transform(tgp_context* ctx) {
    TINYGP_ASSERT(ctx->cur_transform > 0);
    ctx->transform = ctx->transform_stack[--ctx->cur_transform];
    _tgp_update_mvp(ctx);
}

TGPDEF void tgp_reset_transform(tgp_context* ctx) {
    ctx->transform = _tgp_default_transform;
    _tgp_update_mvp(ctx);
}

TGPDEF void tgp_translate(tgp_context* ctx, float x, float y) {
    // multiply x and y by the translation matrix
    // | 1.0 | 0.0 | x   |
    // | 0.0 | 1.0 | y   |
    // | 0.0 | 0.0 | 1.0 |
    ctx->transform.v[0][2] +=
        x * ctx->transform.v[0][0] + y * ctx->transform.v[0][1];
    ctx->transform.v[1][2] +=
        x * ctx->transform.v[1][0] + y * ctx->transform.v[1][1];
    _tgp_update_mvp(ctx);
}

TGPDEF void tgp_scale(tgp_context* ctx, float sx, float sy) {
    ctx->transform.v[0][0] *= sx;
    ctx->transform.v[1][0] *= sx;
    ctx->transform.v[0][1] *= sy;
    ctx->transform.v[1][1] *= sy;
    _tgp_update_mvp(ctx);
}

TGPDEF void tgp_scale_at(tgp_context* ctx, float sx, float sy, float x,
                         float y) {
    tgp_translate(ctx, x, y);
    tgp_scale(ctx, sx, sy);
    tgp_translate(ctx, -x, -y);
}

TGPDEF void tgp_rotate(tgp_context* ctx, float theta) {
    float s = sinf(theta), c = cosf(theta);
    // multiply sinf(theta) and cosf(theta) by the rotation matrix
    // c   | -s  | 0.0
    // s   | c   | 0.0
    // 0.0 | 0.0 | 1.0
    ctx->transform =
        (tgp_mat2x3){{{c * ctx->transform.v[0][0] + s * ctx->transform.v[0][1],
                       -s * ctx->transform.v[0][0] + c * ctx->transform.v[0][1],
                       ctx->transform.v[0][2]},
                      {c * ctx->transform.v[1][0] + s * ctx->transform.v[1][1],
                       -s * ctx->transform.v[1][0] + c * ctx->transform.v[1][1],
                       ctx->transform.v[1][2]}}};
    _tgp_update_mvp(ctx);
}

TGPDEF void tgp_rotate_at(tgp_context* ctx, float theta, float x, float y) {
    tgp_translate(ctx, x, y);
    tgp_rotate(ctx, theta);
    tgp_translate(ctx, -x, -y);
}

TGPDEF void tgp_set_color(tgp_context* ctx, float r, float g, float b,
                          float a) {
    ctx->color = (tgp_color){r, g, b, a};
}

TGPDEF void tgp_reset_color(tgp_context* ctx) {
    ctx->color.r = 1.0;
    ctx->color.g = 1.0;
    ctx->color.b = 1.0;
    ctx->color.a = 1.0;
}

static inline tgp_vertex* _tgp_request_vertices(tgp_context* ctx,
                                                uint32_t count) {
    if (ctx->cur_vertex + count <= ctx->max_vertices) {
        tgp_vertex* vertices = &ctx->vertices[ctx->cur_vertex];
        ctx->cur_vertex += count;
        return vertices;
    }
    // TODO: add an error here
    return NULL;
}

static inline tgp_command* _tgp_peek_prev_commands(tgp_context* ctx,
                                                   uint32_t count) {
    if (count <= ctx->cur_command) {
        return &ctx->commands[ctx->cur_command - count];
    }
    return NULL;
}

static inline tgp_command* _tgp_next_command(tgp_context* ctx) {
    if (ctx->cur_command < ctx->max_commands) {
        return &ctx->commands[ctx->cur_command++];
    }
    // TODO: add an error here
    return NULL;
}

TGPDEF void tgp_viewport(tgp_context* ctx, int x, int y, int w, int h) {
    // don't do anything if the viewport is already the same
    if (ctx->viewport.x == x && ctx->viewport.y == y && ctx->viewport.w == w &&
        ctx->viewport.h == h) {
        return;
    }

    // if the previous command was an another viewport command, we can just
    tgp_command* cmd = _tgp_peek_prev_commands(ctx, 1);

    if (cmd == NULL || cmd->type != TGP_COMMAND_VIEWPORT) {
        // not a viewport command, create a new one
        cmd = _tgp_next_command(ctx);
        if (cmd == NULL) {
            return;
        }
    }

    tgp_irect viewport = {x, y, w, h};
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = TGP_COMMAND_VIEWPORT;
    cmd->data.viewport = viewport;

    // offset the scissor position
    if (ctx->scissor.w >= 0 && ctx->scissor.h >= 0) {
        ctx->scissor.x += x - ctx->viewport.x;
        ctx->scissor.y += y - ctx->viewport.y;
    }

    ctx->viewport = viewport;
    ctx->proj = _tgp_default_projection(w, h);
    _tgp_update_mvp(ctx);
}

TGPDEF void tgp_reset_viewport(tgp_context* ctx) {
    tgp_viewport(ctx, 0, 0, ctx->screen_size.w, ctx->screen_size.h);
}

TGPDEF void tgp_scissor(tgp_context* ctx, int x, int y, int w, int h) {
    // don't do anything if the scissor is already the same
    if (ctx->scissor.x == x && ctx->scissor.y == y && ctx->scissor.w == w &&
        ctx->scissor.h == h) {
        return;
    }

    // offset the scissor x and y coordinates by the viewport coordinates
    tgp_irect offset_scissor = {ctx->viewport.x + x, ctx->viewport.y + y, w, h};
    if (w < 0 && h < 0) {
        offset_scissor.x = 0;
        offset_scissor.y = 0;
        offset_scissor.w = ctx->screen_size.w;
        offset_scissor.h = ctx->screen_size.h;
    }

    // try to reuse previous command
    tgp_command* cmd = _tgp_peek_prev_commands(ctx, 1);
    if (cmd == NULL || cmd->type != TGP_COMMAND_SCISSOR) {
        cmd = _tgp_next_command(ctx);
        if (cmd == NULL) {
            return;
        }
    }
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = TGP_COMMAND_SCISSOR;
    cmd->data.scissor = offset_scissor;

    ctx->scissor = (tgp_irect){x, y, w, h};
}

TGPDEF void tgp_reset_scissor(tgp_context* ctx) {
    tgp_scissor(ctx, 0, 0, -1, -1);
}

TGPDEF void tgp_reset_state(tgp_context* ctx) {
    tgp_reset_color(ctx);
    tgp_reset_projection(ctx);
    tgp_reset_scissor(ctx);
    tgp_reset_transform(ctx);
    tgp_reset_viewport(ctx);
}

TGPDEF void tgp_begin(tgp_context* ctx, int width, int height) {
    static const tgp_color default_color = {1.0, 1.0, 1.0, 1.0};

    ctx->screen_size.w = width;
    ctx->screen_size.h = height;
    ctx->viewport.x = 0;
    ctx->viewport.y = 0;
    ctx->viewport.w = -1;
    ctx->viewport.h = -1;
    ctx->scissor.x = 0;
    ctx->scissor.y = 0;
    ctx->scissor.w = -1;
    ctx->scissor.h = -1;
    ctx->mvp = ctx->proj = _tgp_default_projection(width, height);
    ctx->transform = _tgp_default_transform;
    ctx->color = default_color;
    ctx->cur_command = 0;
    ctx->cur_vertex = 0;
    ctx->cur_transform = 0;

    // push a viewport command
    tgp_viewport(ctx, 0, 0, width, height);
}

TGPDEF tgp_command* tgp_get_command(tgp_context* ctx, uint32_t index) {
    if (index >= ctx->cur_command) {
        return NULL;
    }
    return &ctx->commands[index];
}

TGPDEF bool tgp_get_command_p(tgp_context* ctx, tgp_command* cmd,
                              uint32_t index) {
    tgp_command* cmd_p = tgp_get_command(ctx, index);
    if (cmd_p != NULL) {
        *cmd = *cmd_p;
        return true;
    }
    return false;
}

#define TGP_REGIONS_OVERLAP(a, b)                                              \
    (!((a).x2 <= (b).x1 || (b).x2 <= (a).x1 || (a).y2 <= (b).y1 ||             \
       (b).y2 <= (a).y1))

#ifdef TINYGP_USERDATA_TYPE
static bool _tgp_merge_command(tgp_context* ctx, TINYGP_USERDATA_TYPE userdata,
                               tgp_region region, uint32_t vertex_index,
                               uint32_t num_vertices)
#else
static bool _tgp_merge_command(tgp_context* ctx, tgp_region region,
                               uint32_t vertex_index, uint32_t num_vertices)
#endif
{
#if TGP_BATCH_OPTIMIZER_DEPTH > 0
    tgp_command* prev_cmd = NULL;
    tgp_command* inter_cmds[TGP_BATCH_OPTIMIZER_DEPTH];
    uint32_t inter_cmd_count = 0;
    uint32_t lookup_depth = TGP_BATCH_OPTIMIZER_DEPTH;

    for (uint32_t depth = 0; depth < lookup_depth; depth++) {
        tgp_command* cmd = _tgp_peek_prev_commands(ctx, depth + 1);
        if (cmd == NULL) {
            // we don't have any commands left, stop searching
            break;
        }
        if (cmd->type == TGP_COMMAND_NONE) {
            // the command was removed, continue searching
            lookup_depth++;
            continue;
        }
        if (cmd->type != TGP_COMMAND_DRAW) {
            // not a draw command, stop searching
            break;
        }

#if defined(TINYGP_USERDATA_TYPE) && defined(TINYGP_COMPARE_USERDATA)
        // make sure the commands have the same userdata
        if (TINYGP_COMPARE_USERDATA(cmd->userdata, userdata)) {
            prev_cmd = cmd;
            break;
        } else {
            inter_cmds[inter_cmd_count++] = cmd;
        }
#else
        // no userdata provided, we can always merge them
        inter_cmds[inter_cmd_count++] = cmd;
#endif
    } // for (uint32_t depth = 0; depth < lookup_depth; depth++)

    if (prev_cmd == NULL) {
        return false;
    }

    // make sure that other commands do not overlap the region of the
    // current or the previous command
    bool overlaps_next = false;
    bool overlaps_prev = false;
    tgp_region prev_region = prev_cmd->data.draw.region;
    for (uint32_t i = 0; i < inter_cmd_count; i++) {
        tgp_region inter_region = inter_cmds[i]->data.draw.region;

        if (TGP_REGIONS_OVERLAP(region, inter_region)) {
            overlaps_next = true;
            if (overlaps_prev) {
                return false;
            }
        }
        if (TGP_REGIONS_OVERLAP(prev_region, inter_region)) {
            overlaps_prev = true;
            if (overlaps_next) {
                return false;
            }
        }
    }

    if (!overlaps_next) {
        // batch the previous command
        if (inter_cmd_count > 0) {
            if (ctx->cur_vertex + num_vertices > ctx->max_vertices) {
                // not enough space for the vertices
                return false;
            }

            // rearrange vertices
            uint32_t prev_end_vertex = prev_cmd->data.draw.vertex_idx +
                                       prev_cmd->data.draw.num_vertices;
            uint32_t move_count = ctx->cur_vertex - prev_end_vertex;

            memmove(&ctx->vertices[prev_end_vertex + num_vertices],
                    &ctx->vertices[prev_end_vertex],
                    move_count * sizeof(tgp_vertex));
            memcpy(&ctx->vertices[prev_end_vertex],
                   &ctx->vertices[vertex_index + num_vertices],
                   num_vertices * sizeof(tgp_vertex));

            for (uint32_t i = 0; i < inter_cmd_count; i++) {
                inter_cmds[i]->data.draw.vertex_idx += num_vertices;
            }
        }

        // update draw region
        prev_region.x1 = TGP_MIN(prev_region.x1, region.x1);
        prev_region.y1 = TGP_MIN(prev_region.y1, region.y1);
        prev_region.x2 = TGP_MAX(prev_region.x2, region.x2);
        prev_region.y2 = TGP_MAX(prev_region.y2, region.y2);
        prev_cmd->data.draw.num_vertices += num_vertices;
        prev_cmd->data.draw.region = prev_region;
    } else {
        // batch the next command
        TINYGP_ASSERT(inter_cmd_count > 0);

        // add a new command
        tgp_command* cmd = _tgp_next_command(ctx);
        if (cmd == NULL) {
            return false;
        }

        uint32_t prev_num_vertices = prev_cmd->data.draw.num_vertices;
        if (ctx->cur_vertex + prev_num_vertices > ctx->max_vertices) {
            // not enough space
            return false;
        }

        // rearrange vertices
        memmove(&ctx->vertices[vertex_index + prev_num_vertices],
                &ctx->vertices[vertex_index],
                num_vertices * sizeof(tgp_vertex));
        memcpy(&ctx->vertices[vertex_index],
               &ctx->vertices[prev_cmd->data.draw.vertex_idx],
               prev_num_vertices * sizeof(tgp_vertex));

        // update draw region
        prev_region.x1 = TGP_MIN(prev_region.x1, region.x1);
        prev_region.y1 = TGP_MIN(prev_region.y1, region.y1);
        prev_region.x2 = TGP_MAX(prev_region.x2, region.x2);
        prev_region.y2 = TGP_MAX(prev_region.y2, region.y2);
        ctx->cur_vertex += prev_num_vertices;
        num_vertices += prev_num_vertices;

        cmd->type = TGP_COMMAND_DRAW;
        cmd->data.draw.region = prev_region;
        cmd->data.draw.vertex_idx = vertex_index;
        cmd->data.draw.num_vertices = num_vertices;
#ifdef TINYGP_USERDATA_TYPE
        cmd->userdata = userdata;
#endif

        // make sure we skip the previous command
        prev_cmd->type = TGP_COMMAND_NONE;
    }
    return true;
#else
    (void)ctx;
#ifdef TINYGP_USERDATA_TYPE
    (void)userdata;
#endif
    (void)region;
    (void)vertex_index;
    (void)num_vertices;
    return false;
#endif // #if TGP_BATCH_OPTIMIZER_DEPTH > 0
}

#ifdef TINYGP_USERDATA_TYPE
static void _tgp_queue_draw(tgp_context* ctx, TINYGP_USERDATA_TYPE userdata,
                            tgp_region region, uint32_t vertex_index,
                            uint32_t num_vertices)
#else
static void _tgp_queue_draw(tgp_context* ctx, tgp_region region,
                            uint32_t vertex_index, uint32_t num_vertices)
#endif
{
    if (region.x1 > 1.0f || region.y1 > 1.0f || region.x2 < -1.0f ||
        region.y2 < -1.0f) {
        // region is outside the screen
        ctx->cur_vertex -= num_vertices;
        return;
    }

    // try to merge with previous draw command
#ifdef TINYGP_USERDATA_TYPE
    if (_tgp_merge_command(ctx, userdata, region, vertex_index, num_vertices))
#else
    if (_tgp_merge_command(ctx, region, vertex_index, num_vertices))
#endif
    {
        return;
    }

    // couldn't merge, create new draw command
    tgp_command* cmd = _tgp_next_command(ctx);
    if (cmd == NULL) {
        ctx->cur_vertex -= num_vertices;
        return;
    }

    cmd->type = TGP_COMMAND_DRAW;
    cmd->data.draw.num_vertices = num_vertices;
    cmd->data.draw.vertex_idx = vertex_index;
    cmd->data.draw.region = region;
#ifdef TINYGP_USERDATA_TYPE
    cmd->userdata = userdata;
#endif
}

static inline tgp_vec2 _tgp_mult_mat3_vec2(const tgp_mat2x3* m, tgp_vec2 v) {
    return (tgp_vec2){m->v[0][0] * v.x + m->v[0][1] * v.y + m->v[0][2],
                      m->v[1][0] * v.x + m->v[1][1] * v.y + m->v[1][2]};
}

static inline void _tgp_transform_vec2(tgp_mat2x3* m, tgp_vec2* to,
                                       const tgp_vec2* from, uint32_t num) {
    for (uint32_t i = 0; i < num; ++i) {
        to[i] = _tgp_mult_mat3_vec2(m, from[i]);
    }
}

TGPDEF void tgp_clear(tgp_context* ctx) {
    uint32_t num_vertices = 6; // 6 vertices to draw a quad
    uint32_t vertex_index = ctx->cur_vertex;
    tgp_vertex* vertices = _tgp_request_vertices(ctx, num_vertices);
    if (vertices == NULL) {
        return;
    }

    tgp_vertex* v = vertices;
    const tgp_vec2 quad[4] = {
        {-1.0f, -1.0f}, // bottom left
        {1.0f, -1.0f},  // bottom right
        {1.0f, 1.0f},   // top right
        {-1.0f, 1.0f},  // top left
    };
    const tgp_vec2 texcoord = {0.0f, 0.0f};
    tgp_color color = ctx->color; // copy to stack for more efficency

    v[0].position = quad[0];
    v[0].texcoord = texcoord;
    v[0].color = color;
    v[1].position = quad[1];
    v[1].texcoord = texcoord;
    v[1].color = color;
    v[2].position = quad[2];
    v[2].texcoord = texcoord;
    v[2].color = color;
    v[3].position = quad[3];
    v[3].texcoord = texcoord;
    v[3].color = color;
    v[4].position = quad[0];
    v[4].texcoord = texcoord;
    v[4].color = color;
    v[5].position = quad[2];
    v[5].texcoord = texcoord;
    v[5].color = color;

    tgp_region region = {-1.0f, -1.0f, 1.0f, 1.0f};
#ifdef TINYGP_USERDATA_TYPE
    _tgp_queue_draw(ctx, ctx->current_userdata, region, vertex_index,
                    num_vertices);
#else
    _tgp_queue_draw(ctx, region, vertex_index, num_vertices);
#endif
}

#ifdef TINYGP_USERDATA_TYPE
static void _tgp_draw_vertices(tgp_context* ctx, TINYGP_USERDATA_TYPE userdata,
                               const tgp_vec2* vertices, uint32_t num_vertices)
#else
static void _tgp_draw_vertices(tgp_context* ctx, const tgp_vec2* vertices,
                               uint32_t num_vertices)
#endif
{
    uint32_t vertex_index = ctx->cur_vertex;
    tgp_vertex* draw_vertices = _tgp_request_vertices(ctx, num_vertices);
    if (draw_vertices == NULL) {
        return;
    }

    tgp_mat2x3 mvp = ctx->mvp;
    tgp_region region = {FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX};
    tgp_color color = ctx->color;

    for (uint32_t i = 0; i < num_vertices; i++) {
        tgp_vec2 pos = _tgp_mult_mat3_vec2(&mvp, vertices[i]);
        region.x1 = TGP_MIN(region.x1, pos.x);
        region.y1 = TGP_MIN(region.y1, pos.y);
        region.x2 = TGP_MAX(region.x2, pos.x);
        region.y2 = TGP_MAX(region.y2, pos.y);

        draw_vertices[i].position = pos;
        draw_vertices[i].texcoord.x = 0.0f;
        draw_vertices[i].texcoord.y = 0.0f;
        draw_vertices[i].color = color;
    }

#ifdef TINYGP_USERDATA_TYPE
    _tgp_queue_draw(ctx, userdata, region, vertex_index, num_vertices);
#else
    _tgp_queue_draw(ctx, region, vertex_index, num_vertices);
#endif
}

TGPDEF void tgp_draw_filled_triangles(tgp_context* ctx,
                                      const tgp_triangle* triangles,
                                      uint32_t count) {
    if (count == 0) {
        return;
    }
#ifdef TINYGP_USERDATA_TYPE
    _tgp_draw_vertices(ctx, ctx->current_userdata, (const tgp_vec2*)triangles,
                       count * 3);
#else
    _tgp_draw_vertices(ctx, (const tgp_vec2*)triangles, count * 3);
#endif
}

TGPDEF void tgp_draw_filled_triangle(tgp_context* ctx, float x1, float y1,
                                     float x2, float y2, float x3, float y3) {
    tgp_triangle triangle = {{x1, y1}, {x2, y2}, {x3, y3}};
    tgp_draw_filled_triangles(ctx, &triangle, 1);
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // TINYGP_IMPLEMENTATION
#endif // TINYGP_H_INCLUDED

// This library is licensed under 2 licenses; choose whichever one you want
/************************************************************/
/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/
/************************************************************/
/*
Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
