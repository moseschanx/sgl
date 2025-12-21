/* source/widgets/sgl_ext_img.c
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

#include <sgl_core.h>
#include <sgl_draw.h>
#include <sgl_math.h>
#include <sgl_log.h>
#include <sgl_mm.h>
#include <sgl_theme.h>
#include <sgl_cfgfix.h>
#include <string.h>
#include "sgl_ext_img.h"

typedef struct rlecontext {
    const uint8_t *bitmap;
    uint32_t index;
    sgl_color_t color;
    uint8_t remainder;
    uint8_t format;
    uint8_t alpha;
} rlecontext_t;

rlecontext_t rlectx = {
    .bitmap = NULL,
    .index = 0,
    .remainder = 0,
};

static inline void rel_init(const uint8_t *bitmap, uint8_t format, uint8_t alpha)
{
    if (rlectx.bitmap == NULL) {
        rlectx.bitmap = bitmap;
        rlectx.index = 0;
        rlectx.format = format;
        rlectx.alpha = alpha;
    }
}

static inline void rle_decompress_line(sgl_area_t *coords, sgl_area_t *area, sgl_color_t *out)
{
    for (int i = coords->x1; i <= coords->x2; i++) {
        if (rlectx.remainder == 0) {
            rlectx.remainder = rlectx.bitmap[rlectx.index++];

            if (rlectx.remainder == 0) {
                rlectx.bitmap = NULL;
                return;
            }

            if (rlectx.format == SGL_PIXMAP_FMT_RLE_RGB332) {
                rlectx.color = sgl_rgb332_to_color(rlectx.bitmap[rlectx.index]);
                rlectx.index++;
            }
            else if (rlectx.format == SGL_PIXMAP_FMT_RLE_RGB565) {
                rlectx.color = sgl_rgb565_to_color(rlectx.bitmap[rlectx.index] | (rlectx.bitmap[rlectx.index + 1] << 8));
                rlectx.index += 2;
            }
            else if (rlectx.format == SGL_PIXMAP_FMT_RLE_RGB888) {
                rlectx.color = sgl_rgb888_to_color(rlectx.bitmap[rlectx.index] | (rlectx.bitmap[rlectx.index + 1] << 8) | (rlectx.bitmap[rlectx.index + 2] << 16));
                rlectx.index += 3;
            }
        }

        if (i >= area->x1 && i <= area->x2) {
            *out = (rlectx.alpha == SGL_ALPHA_MAX ? rlectx.color : sgl_color_mixer(rlectx.color, *out, rlectx.alpha));
            out ++;
        }
        rlectx.remainder --;
    };
}


static void sgl_ext_img_construct_cb(sgl_surf_t *surf, sgl_obj_t* obj, sgl_event_t *evt)
{
    sgl_area_t clip = SGL_AREA_INVALID;
    sgl_ext_img_t *ext_img = (sgl_ext_img_t*)obj;
    const uint8_t *bitmap = ext_img->pixmap->bitmap;
    sgl_color_t *pcolor = NULL;
    uint8_t pix_byte = sgl_pixmal_get_bits(ext_img->pixmap);

    sgl_area_t area = {
        .x1 = obj->coords.x1,
        .y1 = obj->coords.y1,
        .x2 = obj->coords.x1 + ext_img->pixmap->width - 1,
        .y2 = obj->coords.y1 + ext_img->pixmap->height - 1,
    };

    if(evt->type == SGL_EVENT_DRAW_MAIN) {
        sgl_color_t *buf = NULL;

        if (!sgl_surf_clip(surf, &area, &clip)) {
            return;
        }

        /* external pixmap support */
        if (ext_img->read) {
            void *pixmap_buf = sgl_malloc(pix_byte * (clip.x2 - clip.x1 + 1));
            if (pixmap_buf == NULL) {
                SGL_LOG_ERROR("sgl_ext_img_construct_cb: malloc failed");
                return;
            }

            sgl_color_t *src = (sgl_color_t*)bitmap;
            src += (clip.y1 - area.y1) * ext_img->pixmap->width + clip.x1 - area.x1;

            for (int y = clip.y1; y <= clip.y2; y++) {

                buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, y - surf->y1);
                ext_img->read((uint8_t*)src, pixmap_buf, pix_byte * (clip.x2 - clip.x1));
                pcolor = (sgl_color_t*)pixmap_buf;

                for (int x = clip.x1; x <= clip.x2; x++, buf++) {
                    *buf = sgl_color_mixer(*pcolor++, *buf, ext_img->alpha);
                }
                src += ext_img->pixmap->width;
            }
            sgl_free(pixmap_buf);
            return;
        }

        /* RLE pixmap support */
        rel_init(bitmap, ext_img->pixmap->format, ext_img->alpha);
        for (int y = clip.y1; y <= clip.y2; y++) {
            buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, y - surf->y1);
            rle_decompress_line(&area, &obj->area, buf);
        }
    }
}


/**
 * @brief create an ext_img object
 * @param parent parent of the ext_img
 * @return ext_img object
 */
sgl_obj_t* sgl_ext_img_create(sgl_obj_t* parent)
{
    sgl_ext_img_t *ext_img = sgl_malloc(sizeof(sgl_ext_img_t));
    if(ext_img == NULL) {
        SGL_LOG_ERROR("sgl_ext_img_create: malloc failed");
        return NULL;
    }

    /* set object all member to zero */
    memset(ext_img, 0, sizeof(sgl_ext_img_t));

    sgl_obj_t *obj = &ext_img->obj;
    sgl_obj_init(&ext_img->obj, parent);
    obj->construct_fn = sgl_ext_img_construct_cb;

    ext_img->alpha = SGL_ALPHA_MAX;

    return obj;
}
