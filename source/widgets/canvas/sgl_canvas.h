/* source/widgets/sgl_canvas.h
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

#ifndef __SGL_CANVAS_H__
#define __SGL_CANVAS_H__

#include <sgl_core.h>
#include <sgl_draw.h>
#include <sgl_math.h>
#include <sgl_log.h>
#include <sgl_mm.h>
#include <sgl_cfgfix.h>
#include <string.h>


typedef void (*painter_cb)(sgl_surf_t *surf, sgl_area_t *area, sgl_obj_t* obj); 

/**
 * @brief sgl canvas struct
 * @obj: sgl general object
 * @painter: pointer to canvas painter function
 * @desc: pointer to canvas draw descriptor
 */
typedef struct sgl_canvas {
    sgl_obj_t  obj;
    painter_cb painter;
} sgl_canvas_t;

/**
 * @brief create a canvas object
 * @param parent parent of the canvas
 * @return canvas object
 */
sgl_obj_t* sgl_canvas_create(sgl_obj_t* parent);

/**
 * @brief set canvas painter
 * @param obj canvas object
 * @param painter painter function
 */
static inline void sgl_canvas_set_painter_cb(sgl_obj_t *obj, painter_cb painter)
{
    sgl_canvas_t *canvas = (sgl_canvas_t *)obj;
    canvas->painter = painter;
}

#endif // !__SGL_CANVAS_H__
