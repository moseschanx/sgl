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
    void (*read)(const uint8_t *addr, uint8_t *out, uint32_t len_bytes);
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
    .alpha = 0,
    .format = 0,
    .read = NULL
};

static inline void rle_init(const uint8_t *bitmap, uint8_t format, uint8_t alpha, void (*read)(const uint8_t*, uint8_t*, uint32_t))
{
    if (rlectx.bitmap == NULL) {
        rlectx.bitmap = bitmap;
        rlectx.index = 0;
        rlectx.format = format;
        rlectx.alpha = alpha;
        rlectx.read = read;
    }
}

static inline void rle_decompress_line(sgl_area_t *coords, sgl_area_t *area, sgl_color_t *out)
{
    uint8_t tmp_buf[8] = {0};

    for (int i = coords->x1; i <= coords->x2; i++) {
        if (rlectx.remainder == 0) {
            if (rlectx.read != NULL) {
                rlectx.read(&rlectx.bitmap[rlectx.index], tmp_buf, sizeof(tmp_buf));
            }
            else {
                tmp_buf[0] = rlectx.bitmap[rlectx.index];
                tmp_buf[1] = rlectx.bitmap[rlectx.index + 1];
                tmp_buf[2] = rlectx.bitmap[rlectx.index + 2];
                tmp_buf[3] = rlectx.bitmap[rlectx.index + 3];
                tmp_buf[4] = rlectx.bitmap[rlectx.index + 4];
            }
            rlectx.index ++;
            rlectx.remainder = tmp_buf[0];

            if (rlectx.remainder == 0) {
                rlectx.bitmap = NULL;
                return;
            }

            if (rlectx.format == SGL_PIXMAP_FMT_RLE_RGB332) {
                rlectx.color = sgl_rgb332_to_color(tmp_buf[1]);
                rlectx.index++;
            }
            else if (rlectx.format == SGL_PIXMAP_FMT_RLE_RGB565) {
                rlectx.color = sgl_rgb565_to_color(tmp_buf[1] | (tmp_buf[2] << 8));
                rlectx.index += 2;
            }
            else if (rlectx.format == SGL_PIXMAP_FMT_RLE_RGB888) {
                rlectx.color = sgl_rgb888_to_color(tmp_buf[1] | (tmp_buf[2] << 8) | (tmp_buf[3] << 16));
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

        if (ext_img->pixmap->format < SGL_PIXMAP_FMT_RLE_RGB332) {

            if (ext_img->read != NULL) {
                uint8_t *pixmap_buf = (uint8_t*)sgl_malloc(pix_byte * (clip.x2 - clip.x1 + 1));
                sgl_color_t tmp_color;
                uint32_t offset = 0, line_ofs = 0;

                for (int y = clip.y1; y <= clip.y2; y++) {
                    buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, y - surf->y1);
                    offset = ((((y - area.y1) * ext_img->pixmap->width) + (clip.x1 - area.x1)) * pix_byte);

                    ext_img->read(bitmap + offset, pixmap_buf, pix_byte * (clip.x2 - clip.x1 + 1));
                    line_ofs = (clip.x1 - area.x1);

                    for (int x = clip.x1; x <= clip.x2; x++) {
                        if (ext_img->pixmap->format == SGL_PIXMAP_FMT_RGB332) {
                            tmp_color = sgl_rgb332_to_color(pixmap_buf[line_ofs]);
                        }
                        else if (ext_img->pixmap->format == SGL_PIXMAP_FMT_RGB565) {
                            tmp_color = sgl_rgb565_to_color(pixmap_buf[line_ofs] | (pixmap_buf[line_ofs + 1] << 8));
                        }
                        else if (ext_img->pixmap->format == SGL_PIXMAP_FMT_RGB888) {
                            tmp_color = sgl_rgb888_to_color(pixmap_buf[line_ofs] | (pixmap_buf[line_ofs + 1] << 8) | (pixmap_buf[line_ofs + 2] << 16));
                        }

                        line_ofs += pix_byte;
                        *buf = ext_img->alpha == SGL_ALPHA_MAX ? tmp_color : sgl_color_mixer(tmp_color, *buf, ext_img->alpha);
                        buf ++;
                    };
                }
                sgl_free(pixmap_buf);
            }
            else {
                sgl_color_t tmp_color;
                uint32_t offset = 0;

                for (int y = clip.y1; y <= clip.y2; y++) {
                    buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, y - surf->y1);
                    offset = ((((y - area.y1) * ext_img->pixmap->width) + (clip.x1 - area.x1)) * pix_byte);

                    for (int x = clip.x1; x <= clip.x2; x++) {
                        if (ext_img->pixmap->format == SGL_PIXMAP_FMT_RGB332) {
                            tmp_color = sgl_rgb332_to_color(bitmap[offset]);
                        }
                        else if (ext_img->pixmap->format == SGL_PIXMAP_FMT_RGB565) {
                            tmp_color = sgl_rgb565_to_color(bitmap[offset] | (bitmap[offset + 1] << 8));
                        }
                        else if (ext_img->pixmap->format == SGL_PIXMAP_FMT_RGB888) {
                            tmp_color = sgl_rgb888_to_color(bitmap[offset] | (bitmap[offset + 1] << 8) | (bitmap[offset + 2] << 16));
                        }

                        offset += pix_byte;
                        *buf = ext_img->alpha == SGL_ALPHA_MAX ? tmp_color : sgl_color_mixer(tmp_color, *buf, ext_img->alpha);
                        buf ++;
                    };
                }
            }
        }
        else {
            /* RLE pixmap support */
            rle_init(bitmap, ext_img->pixmap->format, ext_img->alpha, ext_img->read);
            for (int y = clip.y1; y <= clip.y2; y++) {
                buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, y - surf->y1);
                rle_decompress_line(&area, &obj->area, buf);
            }
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
