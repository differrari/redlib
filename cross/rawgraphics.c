#ifdef CROSS

#include "string/string.h"
#include "ui/draw/draw.h"
#include "alloc/allocate.h"
#include <GLFW/glfw3.h>
#include "keyboard_input.h"
#include "mouse_input.h"
#include "keycode_convert.h"
#include "syscalls/syscalls.h"

extern void free(void*ptr);

#define CONVERT_COLOR(color) ((color & 0xFF00FF00) | ((color & 0xFF) << 16) | ((color >> 16) & 0xFF))

GLFWwindow* _window;

void begin_drawing(draw_ctx *ctx){
    
}

void destroy_draw_ctx(draw_ctx *ctx){
    glfwTerminate();
}

void commit_draw_ctx(draw_ctx *ctx){
    glRasterPos2i(0,ctx->height-1);
    glPixelZoom(1,-1);
    glDrawPixels(ctx->width,
     	 ctx->height,
         GL_BGRA,
     	 GL_UNSIGNED_INT_8_8_8_8_REV,
     	 ctx->fb);
    glfwSwapBuffers(_window);
    glfwPollEvents();
}

void resize_draw_ctx(draw_ctx *ctx, uint32_t width, uint32_t height){
    release(ctx->fb);
    ctx->width = width;
    ctx->height = height;
    ctx->fb = zalloc(width*height*sizeof(color));
    ctx->stride = 4 * width;
    glfwSetWindowSize(_window, width, height);
    glViewport( 0, 0, width, height );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0, width, 0, height, -1, 1 );
}

static void error_callback(int error, const char* description)
{
    print("Error: %s", description);
}

#define INPUT_BUFFER_CAPACITY 64

static kbd_event event_queue[INPUT_BUFFER_CAPACITY];
static int kbd_event_read;
static int kbd_event_write;
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    uint32_t next_index = (kbd_event_write + 1) % INPUT_BUFFER_CAPACITY;

    event_queue[kbd_event_write] = (kbd_event){
        .type = action == GLFW_PRESS ? KEY_PRESS : KEY_RELEASE,
        .key = glfw_to_redacted[key],
        .modifier = mods
    };
    kbd_event_write = next_index;

    if (kbd_event_write == kbd_event_read)
        kbd_event_read = (kbd_event_read + 1) % INPUT_BUFFER_CAPACITY;
}

double x_pos, y_pos;
static int old_x = 0;
static int old_y = 0;
static double scroll;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    x_pos = xpos;
    y_pos = ypos;
}

static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    scroll = yoffset;
}

void request_draw_ctx(draw_ctx *ctx){
    uint32_t w = ctx->width ? ctx->width : 600;
    uint32_t h = ctx->height ? ctx->height : 300;
    ctx->fb = zalloc(w*h*sizeof(color));
    ctx->width = w;
    ctx->height = h;
    ctx->stride = 4 * w;
    glfwInit();
    glfwSetErrorCallback(error_callback);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
    
    _window = glfwCreateWindow(w, h, "redlib", NULL, NULL);
    glfwMakeContextCurrent(_window);
    glViewport( 0, 0, w, h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0, w, 0, h, -1, 1 );
    glfwSetKeyCallback(_window, key_callback);
    glfwSetCursorPosCallback(_window, cursor_position_callback);
    glfwSetScrollCallback(_window, scroll_callback);
}

bool read_event(kbd_event *out){
    if (kbd_event_read == kbd_event_write) return false;

    *out = event_queue[kbd_event_read];
    kbd_event_read = (kbd_event_read + 1) % INPUT_BUFFER_CAPACITY;
    
    return true;
}

void get_mouse_status(mouse_data *in){
    in->raw.scroll = (u8)(scroll * 127);
    scroll = 0;
    in->raw.buttons = 0;
    for (int i = 0; i < 3; i++)
        in->raw.buttons |= (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT) & 1) << i;
    in->raw.x = x_pos - old_x;
    in->raw.y = y_pos - old_y;
    in->position.x = x_pos;
    in->position.y = y_pos;
    old_x = x_pos;
    old_y = y_pos;
}

bool should_close_ctx(){
    return glfwWindowShouldClose(_window);
}

#endif