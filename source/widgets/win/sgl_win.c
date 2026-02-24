/* source/widgets/sgl_win.c
 *
 * MIT License
 *
 * Copyright(c) 2023-present All contributors of SGL  
 * Document reference link: docs directory
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <sgl.h>

static void win_exit_cb(sgl_event_t *evt)
{
    sgl_win_t *win = (sgl_win_t *)evt->param;
    if (evt->type == SGL_EVENT_RELEASED) {
        sgl_obj_set_destroyed(win->body);
        sgl_obj_set_destroyed(&win->obj);
    }
}

static void sgl_win_construct_cb(sgl_surf_t *surf, sgl_obj_t* obj, sgl_event_t *evt)
{
    sgl_win_t *win = sgl_container_of(obj, sgl_win_t, obj);
    sgl_rect_t bg = obj->coords;
    sgl_obj_t *body = win->body;
    int16_t exit_cx, exit_cy, exit_r;

    win->title_h = sgl_max(obj->radius, win->title_h);
    bg.y1 -= obj->radius;

    if (evt->type == SGL_EVENT_DRAW_MAIN) {
        sgl_draw_rect(surf, &obj->area, &bg, &win->bg);
        sgl_obj_set_pos(body, obj->coords.x1, obj->coords.y1 - win->title_h + obj->border);
        sgl_obj_clear_dirty(body);
    }
    else if (evt->type == SGL_EVENT_DRAW_INIT) {
        exit_r  = win->title_h / 2;
        exit_cx = sgl_obj_get_width(obj) - exit_r - exit_r / 2;
        exit_cy = win->title_h / 2 - exit_r / 2 + 1;

        sgl_obj_set_size(body, sgl_obj_get_width(obj), win->title_h + obj->border + obj->radius);
        sgl_obj_set_pos(body, obj->coords.x1, obj->coords.y1 - win->title_h + obj->border);
        sgl_rect_set_border_width(body, obj->border);
        sgl_rect_set_border_color(body, win->bg.border_color);
        sgl_rect_set_radius(body, obj->radius);
        sgl_rect_set_alpha(body, win->bg.alpha);
        sgl_rect_set_color(body, win->title_bg_color);

        sgl_obj_t *title_text = sgl_label_create(body);
        if (title_text == NULL) {
            SGL_LOG_ERROR("sgl_win_create: sgl_label_create failed");
            return;
        }
        sgl_obj_set_size(title_text, sgl_obj_get_width(obj) - win->title_h, win->title_h);
        sgl_obj_set_pos(title_text, 0, 0);
        sgl_label_set_text(title_text, win->title_text);
        sgl_label_set_font(title_text, win->title_font);
        sgl_label_set_text_color(title_text, win->title_text_color);
        sgl_label_set_text_align(title_text, win->title_align);
        sgl_label_set_text_offset(title_text, obj->radius, 0);

        sgl_obj_t *exit = sgl_circle_create(body);
        if (exit == NULL) {
            SGL_LOG_ERROR("sgl_win_create: sgl_circle_create failed");
            return;
        }
        sgl_obj_set_size(exit, exit_r, exit_r);
        sgl_obj_set_pos(exit, exit_cx, exit_cy);
        sgl_circle_set_radius(exit, exit_r);
        sgl_circle_set_color(exit, SGL_COLOR_RED);
        sgl_circle_set_border_width(exit, 0);
        sgl_circle_set_alpha(exit, win->bg.alpha);
        sgl_obj_set_event_cb(exit, win_exit_cb, (size_t)win);
    }
    else if (evt->type == SGL_EVENT_DESTROYED) {
        sgl_obj_set_destroyed(win->body);
    }
}

/**
 * @brief create a window object
 * @param parent parent of the window
 * @return window object
 */
sgl_obj_t* sgl_win_create(sgl_obj_t* parent)
{
    sgl_win_t *win = sgl_malloc(sizeof(sgl_win_t));
    if(win == NULL) {
        SGL_LOG_ERROR("sgl_win_create: malloc failed");
        return NULL;
    }

    /* set object all member to zero */
    memset(win, 0, sizeof(sgl_win_t));

    sgl_obj_t *obj = &win->obj;
    sgl_obj_init(&win->obj, parent);
    obj->construct_fn = sgl_win_construct_cb;
    obj->needinit = 1;

    win->bg.alpha = SGL_THEME_ALPHA;
    win->bg.color = SGL_THEME_COLOR;
    win->bg.radius = SGL_THEME_RADIUS;
    win->bg.border = 0;
    win->bg.border_color = SGL_THEME_BORDER_COLOR;
    win->title_font = sgl_get_system_font();
    win->title_align = SGL_ALIGN_LEFT_MID;

    sgl_obj_t *body = sgl_rect_create(obj->parent);
    if (body == NULL) {
        SGL_LOG_ERROR("sgl_win_create: sgl_rect_create failed");
        return NULL;
    }
    win->body = body;
    sgl_obj_move_down(body);
    return obj;
}
