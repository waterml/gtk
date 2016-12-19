/* GSK - The GTK Scene Kit
 *
 * Copyright 2016  Endless
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "gskrendernodeprivate.h"

#include "gskcairoblurprivate.h"
#include "gskdebugprivate.h"
#include "gskrendererprivate.h"
#include "gskroundedrectprivate.h"
#include "gsktextureprivate.h"

/*** GSK_COLOR_NODE ***/

typedef struct _GskColorNode GskColorNode;

struct _GskColorNode
{
  GskRenderNode render_node;

  GdkRGBA color;
  graphene_rect_t bounds;
};

static void
gsk_color_node_finalize (GskRenderNode *node)
{
}

static void
gsk_color_node_make_immutable (GskRenderNode *node)
{
}

static void
gsk_color_node_draw (GskRenderNode *node,
                     cairo_t       *cr)
{
  GskColorNode *self = (GskColorNode *) node;

  gdk_cairo_set_source_rgba (cr, &self->color);

  cairo_rectangle (cr,
                   self->bounds.origin.x, self->bounds.origin.y,
                   self->bounds.size.width, self->bounds.size.height);
  cairo_fill (cr);
}

static void
gsk_color_node_get_bounds (GskRenderNode   *node,
                           graphene_rect_t *bounds)
{
  GskColorNode *self = (GskColorNode *) node;

  graphene_rect_init_from_rect (bounds, &self->bounds); 
}

static const GskRenderNodeClass GSK_COLOR_NODE_CLASS = {
  GSK_COLOR_NODE,
  sizeof (GskColorNode),
  "GskColorNode",
  gsk_color_node_finalize,
  gsk_color_node_make_immutable,
  gsk_color_node_draw,
  gsk_color_node_get_bounds
};

const GdkRGBA *
gsk_color_node_peek_color (GskRenderNode *node)
{
  GskColorNode *self = (GskColorNode *) node;

  return &self->color;
}

/**
 * gsk_color_node_new:
 * @color: the #GskColor
 * @bounds: the rectangle to render the color into
 *
 * Creates a #GskRenderNode that will render the given
 * @color into the area given by @bounds.
 *
 * Returns: A new #GskRenderNode
 *
 * Since: 3.90
 */
GskRenderNode *
gsk_color_node_new (const GdkRGBA         *rgba,
                    const graphene_rect_t *bounds)
{
  GskColorNode *self;

  g_return_val_if_fail (rgba != NULL, NULL);
  g_return_val_if_fail (bounds != NULL, NULL);

  self = (GskColorNode *) gsk_render_node_new (&GSK_COLOR_NODE_CLASS);

  self->color = *rgba;
  graphene_rect_init_from_rect (&self->bounds, bounds);

  return &self->render_node;
}

/*** GSK_LINEAR_GRADIENT_NODE ***/

typedef struct _GskLinearGradientNode GskLinearGradientNode;

struct _GskLinearGradientNode
{
  GskRenderNode render_node;

  graphene_rect_t bounds;

  graphene_point_t start;
  graphene_point_t end;

  GskColorStop *stops;
  gsize n_stops;
};

static void
gsk_linear_gradient_node_finalize (GskRenderNode *node)
{
  GskLinearGradientNode *self = (GskLinearGradientNode *) node;

  g_free (self->stops);
}

static void
gsk_linear_gradient_node_make_immutable (GskRenderNode *node)
{
}

static void
gsk_linear_gradient_node_draw (GskRenderNode *node,
                               cairo_t       *cr)
{
  GskLinearGradientNode *self = (GskLinearGradientNode *) node;
  cairo_pattern_t *pattern;
  gsize i;

  pattern = cairo_pattern_create_linear (self->start.x, self->start.y,
                                         self->end.x, self->end.y);

  if (gsk_render_node_get_node_type (node) == GSK_REPEATING_LINEAR_GRADIENT_NODE)
    cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);

  for (i = 0; i < self->n_stops; i++)
    {
      cairo_pattern_add_color_stop_rgba (pattern,
                                         self->stops[i].offset,
                                         self->stops[i].color.red,
                                         self->stops[i].color.green,
                                         self->stops[i].color.blue,
                                         self->stops[i].color.alpha);
    }

  cairo_set_source (cr, pattern);
  cairo_pattern_destroy (pattern);

  cairo_rectangle (cr,
                   self->bounds.origin.x, self->bounds.origin.y,
                   self->bounds.size.width, self->bounds.size.height);
  cairo_fill (cr);
}

static void
gsk_linear_gradient_node_get_bounds (GskRenderNode   *node,
                                     graphene_rect_t *bounds)
{
  GskLinearGradientNode *self = (GskLinearGradientNode *) node;

  graphene_rect_init_from_rect (bounds, &self->bounds); 
}

static const GskRenderNodeClass GSK_LINEAR_GRADIENT_NODE_CLASS = {
  GSK_LINEAR_GRADIENT_NODE,
  sizeof (GskLinearGradientNode),
  "GskLinearGradientNode",
  gsk_linear_gradient_node_finalize,
  gsk_linear_gradient_node_make_immutable,
  gsk_linear_gradient_node_draw,
  gsk_linear_gradient_node_get_bounds
};

static const GskRenderNodeClass GSK_REPEATING_LINEAR_GRADIENT_NODE_CLASS = {
  GSK_REPEATING_LINEAR_GRADIENT_NODE,
  sizeof (GskLinearGradientNode),
  "GskLinearGradientNode",
  gsk_linear_gradient_node_finalize,
  gsk_linear_gradient_node_make_immutable,
  gsk_linear_gradient_node_draw,
  gsk_linear_gradient_node_get_bounds
};

/**
 * gsk_linear_gradient_node_new:
 * @linear_gradient: the #GskLinearGradient
 * @bounds: the rectangle to render the linear_gradient into
 *
 * Creates a #GskRenderNode that will render the given
 * @linear_gradient into the area given by @bounds.
 *
 * Returns: A new #GskRenderNode
 *
 * Since: 3.90
 */
GskRenderNode *
gsk_linear_gradient_node_new (const graphene_rect_t  *bounds,
                              const graphene_point_t *start,
                              const graphene_point_t *end,
                              const GskColorStop     *color_stops,
                              gsize                   n_color_stops)
{
  GskLinearGradientNode *self;

  g_return_val_if_fail (bounds != NULL, NULL);
  g_return_val_if_fail (start != NULL, NULL);
  g_return_val_if_fail (end != NULL, NULL);
  g_return_val_if_fail (color_stops != NULL, NULL);

  self = (GskLinearGradientNode *) gsk_render_node_new (&GSK_LINEAR_GRADIENT_NODE_CLASS);

  graphene_rect_init_from_rect (&self->bounds, bounds);
  graphene_point_init_from_point (&self->start, start);
  graphene_point_init_from_point (&self->end, end);

  self->stops = g_memdup (color_stops, sizeof (GskColorStop) * n_color_stops);
  self->n_stops = n_color_stops;

  return &self->render_node;
}

GskRenderNode *
gsk_repeating_linear_gradient_node_new (const graphene_rect_t  *bounds,
                                        const graphene_point_t *start,
                                        const graphene_point_t *end,
                                        const GskColorStop     *color_stops,
                                        gsize                   n_color_stops)
{
  GskLinearGradientNode *self;

  g_return_val_if_fail (bounds != NULL, NULL);
  g_return_val_if_fail (start != NULL, NULL);
  g_return_val_if_fail (end != NULL, NULL);
  g_return_val_if_fail (color_stops != NULL, NULL);

  self = (GskLinearGradientNode *) gsk_render_node_new (&GSK_REPEATING_LINEAR_GRADIENT_NODE_CLASS);

  graphene_rect_init_from_rect (&self->bounds, bounds);
  graphene_point_init_from_point (&self->start, start);
  graphene_point_init_from_point (&self->end, end);

  self->stops = g_memdup (color_stops, sizeof (GskColorStop) * n_color_stops);
  self->n_stops = n_color_stops;

  return &self->render_node;
}

/*** GSK_BORDER_NODE ***/

typedef struct _GskBorderNode GskBorderNode;

struct _GskBorderNode
{
  GskRenderNode render_node;

  GskRoundedRect outline;
  float border_width[4];
  GdkRGBA border_color[4];
};

static void
gsk_border_node_finalize (GskRenderNode *node)
{
}

static void
gsk_border_node_make_immutable (GskRenderNode *node)
{
}

static void
gsk_border_node_draw (GskRenderNode *node,
                       cairo_t       *cr)
{
  GskBorderNode *self = (GskBorderNode *) node;
  GskRoundedRect inside;

  cairo_save (cr);

  gsk_rounded_rect_init_copy (&inside, &self->outline);
  gsk_rounded_rect_shrink (&inside,
                           self->border_width[0], self->border_width[1],
                           self->border_width[2], self->border_width[3]);

  cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
  gsk_rounded_rect_path (&self->outline, cr);
  gsk_rounded_rect_path (&inside, cr);

  if (gdk_rgba_equal (&self->border_color[0], &self->border_color[1]) &&
      gdk_rgba_equal (&self->border_color[0], &self->border_color[2]) &&
      gdk_rgba_equal (&self->border_color[0], &self->border_color[3]))
    {
      gdk_cairo_set_source_rgba (cr, &self->border_color[0]);
      cairo_fill (cr);
    }
  else
    {
      const graphene_rect_t *bounds = &self->outline.bounds;
      /* distance to center "line":
       * +-------------------------+
       * |                         |
       * |                         |
       * |     ---this-line---     |
       * |                         |
       * |                         |
       * +-------------------------+
       * That line is equidistant from all sides. It's either horiontal
       * or vertical, depending on if the rect is wider or taller.
       * We use the 4 sides spanned up by connecting the line to the corner
       * points to color the regions of the rectangle differently.
       * Note that the call to cairo_fill() will add the potential final
       * segment by closing the path, so we don't have to care.
       */
      float dst = MIN (bounds->size.width, bounds->size.height) / 2.0;

      cairo_clip (cr);

      /* top */
      cairo_move_to (cr, bounds->origin.x + dst, bounds->origin.y + dst);
      cairo_rel_line_to (cr, - dst, - dst);
      cairo_rel_line_to (cr, bounds->size.width, 0);
      cairo_rel_line_to (cr, - dst, dst);
      gdk_cairo_set_source_rgba (cr, &self->border_color[0]);
      cairo_fill (cr);

      /* right */
      cairo_move_to (cr, bounds->origin.x + bounds->size.width - dst, bounds->origin.y + dst);
      cairo_rel_line_to (cr, dst, - dst);
      cairo_rel_line_to (cr, 0, bounds->size.height);
      cairo_rel_line_to (cr, - dst, - dst);
      gdk_cairo_set_source_rgba (cr, &self->border_color[1]);
      cairo_fill (cr);

      /* bottom */
      cairo_move_to (cr, bounds->origin.x + bounds->size.width - dst, bounds->origin.y + bounds->size.height - dst);
      cairo_rel_line_to (cr, dst, dst);
      cairo_rel_line_to (cr, - bounds->size.width, 0);
      cairo_rel_line_to (cr, dst, - dst);
      gdk_cairo_set_source_rgba (cr, &self->border_color[2]);
      cairo_fill (cr);

      /* left */
      cairo_move_to (cr, bounds->origin.x + dst, bounds->origin.y + bounds->size.height - dst);
      cairo_rel_line_to (cr, - dst, dst);
      cairo_rel_line_to (cr, 0, - bounds->size.height);
      cairo_rel_line_to (cr, dst, dst);
      gdk_cairo_set_source_rgba (cr, &self->border_color[3]);
      cairo_fill (cr);
    }

  cairo_restore (cr);
}

static void
gsk_border_node_get_bounds (GskRenderNode   *node,
                            graphene_rect_t *bounds)
{
  GskBorderNode *self = (GskBorderNode *) node;

  graphene_rect_init_from_rect (bounds, &self->outline.bounds);
}

static const GskRenderNodeClass GSK_BORDER_NODE_CLASS = {
  GSK_BORDER_NODE,
  sizeof (GskBorderNode),
  "GskBorderNode",
  gsk_border_node_finalize,
  gsk_border_node_make_immutable,
  gsk_border_node_draw,
  gsk_border_node_get_bounds
};

const GskRoundedRect *
gsk_border_node_peek_outline (GskRenderNode *node)
{
  GskBorderNode *self = (GskBorderNode *) node;

  return &self->outline;
}

float
gsk_border_node_get_width (GskRenderNode *node,
                           guint          i)
{
  GskBorderNode *self = (GskBorderNode *) node;

  return self->border_width[i];
}

const GdkRGBA *
gsk_border_node_peek_color (GskRenderNode *node,
                            guint          i)
{
  GskBorderNode *self = (GskBorderNode *) node;

  return &self->border_color[i];
}

/**
 * gsk_border_node_new:
 * @outline: a #GskRoundedRect describing the outline of the border
 * @border_width: the stroke width of the border on the top, right, bottom and
 *     left side respectively.
 * @border_color: the color used on the top, right, bottom and left side.
 *
 * Creates a #GskRenderNode that will stroke a border rectangle inside the
 * given @outline. The 4 sides of the border can have different widths and
 * colors.
 *
 * Returns: A new #GskRenderNode
 *
 * Since: 3.90
 */
GskRenderNode *
gsk_border_node_new (const GskRoundedRect     *outline,
                     const float               border_width[4],
                     const GdkRGBA             border_color[4])
{
  GskBorderNode *self;

  g_return_val_if_fail (outline != NULL, NULL);
  g_return_val_if_fail (border_width != NULL, NULL);
  g_return_val_if_fail (border_color != NULL, NULL);

  self = (GskBorderNode *) gsk_render_node_new (&GSK_BORDER_NODE_CLASS);

  gsk_rounded_rect_init_copy (&self->outline, outline);
  memcpy (self->border_width, border_width, sizeof (self->border_width));
  memcpy (self->border_color, border_color, sizeof (self->border_color));

  return &self->render_node;
}

/*** GSK_TEXTURE_NODE ***/

typedef struct _GskTextureNode GskTextureNode;

struct _GskTextureNode
{
  GskRenderNode render_node;

  GskTexture *texture;
  graphene_rect_t bounds;
};

static void
gsk_texture_node_finalize (GskRenderNode *node)
{
  GskTextureNode *self = (GskTextureNode *) node;

  gsk_texture_unref (self->texture);
}

static void
gsk_texture_node_make_immutable (GskRenderNode *node)
{
}

static void
gsk_texture_node_draw (GskRenderNode *node,
                       cairo_t       *cr)
{
  GskTextureNode *self = (GskTextureNode *) node;
  cairo_surface_t *surface;

  surface = gsk_texture_download (self->texture);

  cairo_save (cr);

  cairo_translate (cr, self->bounds.origin.x, self->bounds.origin.y);
  cairo_scale (cr,
               self->bounds.size.width / gsk_texture_get_width (self->texture),
               self->bounds.size.height / gsk_texture_get_height (self->texture));

  cairo_set_source_surface (cr, surface, 0, 0);
  cairo_paint (cr);

  cairo_restore (cr);

  cairo_surface_destroy (surface);
}

static void
gsk_texture_node_get_bounds (GskRenderNode   *node,
                             graphene_rect_t *bounds)
{
  GskTextureNode *self = (GskTextureNode *) node;

  graphene_rect_init_from_rect (bounds, &self->bounds); 
}

static const GskRenderNodeClass GSK_TEXTURE_NODE_CLASS = {
  GSK_TEXTURE_NODE,
  sizeof (GskTextureNode),
  "GskTextureNode",
  gsk_texture_node_finalize,
  gsk_texture_node_make_immutable,
  gsk_texture_node_draw,
  gsk_texture_node_get_bounds
};

GskTexture *
gsk_texture_node_get_texture (GskRenderNode *node)
{
  GskTextureNode *self = (GskTextureNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_TEXTURE_NODE), 0);

  return self->texture;
}

/**
 * gsk_texture_node_new:
 * @texture: the #GskTexture
 * @bounds: the rectangle to render the texture into
 *
 * Creates a #GskRenderNode that will render the given
 * @texture into the area given by @bounds.
 *
 * Returns: A new #GskRenderNode
 *
 * Since: 3.90
 */
GskRenderNode *
gsk_texture_node_new (GskTexture            *texture,
                      const graphene_rect_t *bounds)
{
  GskTextureNode *self;

  g_return_val_if_fail (GSK_IS_TEXTURE (texture), NULL);
  g_return_val_if_fail (bounds != NULL, NULL);

  self = (GskTextureNode *) gsk_render_node_new (&GSK_TEXTURE_NODE_CLASS);

  self->texture = gsk_texture_ref (texture);
  graphene_rect_init_from_rect (&self->bounds, bounds);

  return &self->render_node;
}

/*** GSK_INSET_SHADOW_NODE ***/

typedef struct _GskInsetShadowNode GskInsetShadowNode;

struct _GskInsetShadowNode
{
  GskRenderNode render_node;

  GskRoundedRect outline;
  GdkRGBA color;
  float dx;
  float dy;
  float spread;
  float blur_radius;
};

static void
gsk_inset_shadow_node_finalize (GskRenderNode *node)
{
}

static void
gsk_inset_shadow_node_make_immutable (GskRenderNode *node)
{
}

static gboolean
has_empty_clip (cairo_t *cr)
{
  double x1, y1, x2, y2;

  cairo_clip_extents (cr, &x1, &y1, &x2, &y2);
  return x1 == x2 && y1 == y2;
}

static void
draw_shadow (cairo_t             *cr,
             gboolean             inset,
	     GskRoundedRect      *box,
	     GskRoundedRect      *clip_box,
             float                radius,
             const GdkRGBA       *color,
	     GskBlurFlags         blur_flags)
{
  cairo_t *shadow_cr;
  gboolean do_blur;

  if (has_empty_clip (cr))
    return;

  gdk_cairo_set_source_rgba (cr, color);
  do_blur = (blur_flags & (GSK_BLUR_X | GSK_BLUR_Y)) != 0;
  if (do_blur)
    shadow_cr = gsk_cairo_blur_start_drawing (cr, radius, blur_flags);
  else
    shadow_cr = cr;

  cairo_set_fill_rule (shadow_cr, CAIRO_FILL_RULE_EVEN_ODD);
  gsk_rounded_rect_path (box, shadow_cr);
  if (inset)
    cairo_rectangle (cr,
                     clip_box->bounds.origin.x, clip_box->bounds.origin.y,
                     clip_box->bounds.size.width, clip_box->bounds.size.height);

  cairo_fill (shadow_cr);

  if (do_blur)
    gsk_cairo_blur_finish_drawing (shadow_cr, radius, color, blur_flags);
}

typedef struct {
  float radius;
  graphene_size_t corner;
} CornerMask;

typedef enum {
  TOP,
  RIGHT,
  BOTTOM,
  LEFT
} Side;

static guint
corner_mask_hash (CornerMask *mask)
{
  return ((guint)mask->radius << 24) ^
    ((guint)(mask->corner.width*4)) << 12 ^
    ((guint)(mask->corner.height*4)) << 0;
}

static gboolean
corner_mask_equal (CornerMask *mask1,
                   CornerMask *mask2)
{
  return
    mask1->radius == mask2->radius &&
    mask1->corner.width == mask2->corner.width &&
    mask1->corner.height == mask2->corner.height;
}

static void
draw_shadow_corner (cairo_t               *cr,
                    gboolean               inset,
                    GskRoundedRect        *box,
                    GskRoundedRect        *clip_box,
                    float                  radius,
                    const GdkRGBA         *color,
                    GskCorner              corner,
                    cairo_rectangle_int_t *drawn_rect)
{
  float clip_radius;
  int x1, x2, x3, y1, y2, y3, x, y;
  GskRoundedRect corner_box;
  cairo_t *mask_cr;
  cairo_surface_t *mask;
  cairo_pattern_t *pattern;
  cairo_matrix_t matrix;
  float sx, sy;
  static GHashTable *corner_mask_cache = NULL;
  float max_other;
  CornerMask key;
  gboolean overlapped;

  clip_radius = gsk_cairo_blur_compute_pixels (radius);

  overlapped = FALSE;
  if (corner == GSK_CORNER_TOP_LEFT || corner == GSK_CORNER_BOTTOM_LEFT)
    {
      x1 = floor (box->bounds.origin.x - clip_radius);
      x2 = ceil (box->bounds.origin.x + box->corner[corner].width + clip_radius);
      x = x1;
      sx = 1;
      max_other = MAX(box->corner[GSK_CORNER_TOP_RIGHT].width, box->corner[GSK_CORNER_BOTTOM_RIGHT].width);
      x3 = floor (box->bounds.origin.x + box->bounds.size.width - max_other - clip_radius);
      if (x2 > x3)
        overlapped = TRUE;
    }
  else
    {
      x1 = floor (box->bounds.origin.x + box->bounds.size.width - box->corner[corner].width - clip_radius);
      x2 = ceil (box->bounds.origin.x + box->bounds.size.width + clip_radius);
      x = x2;
      sx = -1;
      max_other = MAX(box->corner[GSK_CORNER_TOP_LEFT].width, box->corner[GSK_CORNER_BOTTOM_LEFT].width);
      x3 = ceil (box->bounds.origin.x + max_other + clip_radius);
      if (x3 > x1)
        overlapped = TRUE;
    }

  if (corner == GSK_CORNER_TOP_LEFT || corner == GSK_CORNER_TOP_RIGHT)
    {
      y1 = floor (box->bounds.origin.y - clip_radius);
      y2 = ceil (box->bounds.origin.y + box->corner[corner].height + clip_radius);
      y = y1;
      sy = 1;
      max_other = MAX(box->corner[GSK_CORNER_BOTTOM_LEFT].height, box->corner[GSK_CORNER_BOTTOM_RIGHT].height);
      y3 = floor (box->bounds.origin.y + box->bounds.size.height - max_other - clip_radius);
      if (y2 > y3)
        overlapped = TRUE;
    }
  else
    {
      y1 = floor (box->bounds.origin.y + box->bounds.size.height - box->corner[corner].height - clip_radius);
      y2 = ceil (box->bounds.origin.y + box->bounds.size.height + clip_radius);
      y = y2;
      sy = -1;
      max_other = MAX(box->corner[GSK_CORNER_TOP_LEFT].height, box->corner[GSK_CORNER_TOP_RIGHT].height);
      y3 = ceil (box->bounds.origin.y + max_other + clip_radius);
      if (y3 > y1)
        overlapped = TRUE;
    }

  drawn_rect->x = x1;
  drawn_rect->y = y1;
  drawn_rect->width = x2 - x1;
  drawn_rect->height = y2 - y1;

  cairo_rectangle (cr, x1, y1, x2 - x1, y2 - y1);
  cairo_clip (cr);

  if (inset || overlapped)
    {
      /* Fall back to generic path if inset or if the corner radius
         runs into each other */
      draw_shadow (cr, inset, box, clip_box, radius, color, GSK_BLUR_X | GSK_BLUR_Y);
      return;
    }

  if (has_empty_clip (cr))
    return;

  /* At this point we're drawing a blurred outset corner. The only
   * things that affect the output of the blurred mask in this case
   * is:
   *
   * What corner this is, which defines the orientation (sx,sy)
   * and position (x,y)
   *
   * The blur radius (which also defines the clip_radius)
   *
   * The the horizontal and vertical corner radius
   *
   * We apply the first position and orientation when drawing the
   * mask, so we cache rendered masks based on the blur radius and the
   * corner radius.
   */
  if (corner_mask_cache == NULL)
    corner_mask_cache = g_hash_table_new_full ((GHashFunc)corner_mask_hash,
                                               (GEqualFunc)corner_mask_equal,
                                               g_free, (GDestroyNotify)cairo_surface_destroy);

  key.radius = radius;
  key.corner = box->corner[corner];

  mask = g_hash_table_lookup (corner_mask_cache, &key);
  if (mask == NULL)
    {
      mask = cairo_surface_create_similar_image (cairo_get_target (cr), CAIRO_FORMAT_A8,
                                                 drawn_rect->width + clip_radius,
                                                 drawn_rect->height + clip_radius);
      mask_cr = cairo_create (mask);
      gsk_rounded_rect_init_from_rect (&corner_box, &GRAPHENE_RECT_INIT (clip_radius, clip_radius, 2*drawn_rect->width, 2*drawn_rect->height), 0);
      corner_box.corner[0] = box->corner[corner];
      gsk_rounded_rect_path (&corner_box, mask_cr);
      cairo_fill (mask_cr);
      gsk_cairo_blur_surface (mask, radius, GSK_BLUR_X | GSK_BLUR_Y);
      cairo_destroy (mask_cr);
      g_hash_table_insert (corner_mask_cache, g_memdup (&key, sizeof (key)), mask);
    }

  gdk_cairo_set_source_rgba (cr, color);
  pattern = cairo_pattern_create_for_surface (mask);
  cairo_matrix_init_identity (&matrix);
  cairo_matrix_scale (&matrix, sx, sy);
  cairo_matrix_translate (&matrix, -x, -y);
  cairo_pattern_set_matrix (pattern, &matrix);
  cairo_mask (cr, pattern);
  cairo_pattern_destroy (pattern);
}

static void
draw_shadow_side (cairo_t               *cr,
                  gboolean               inset,
                  GskRoundedRect        *box,
                  GskRoundedRect        *clip_box,
                  float                  radius,
                  const GdkRGBA         *color,
                  Side                   side,
                  cairo_rectangle_int_t *drawn_rect)
{
  GskBlurFlags blur_flags = GSK_BLUR_REPEAT;
  gdouble clip_radius;
  int x1, x2, y1, y2;

  clip_radius = gsk_cairo_blur_compute_pixels (radius);

  if (side == TOP || side == BOTTOM)
    {
      blur_flags |= GSK_BLUR_Y;
      x1 = floor (box->bounds.origin.x - clip_radius);
      x2 = ceil (box->bounds.origin.x + box->bounds.size.width + clip_radius);
    }
  else if (side == LEFT)
    {
      x1 = floor (box->bounds.origin.x -clip_radius);
      x2 = ceil (box->bounds.origin.x + clip_radius);
    }
  else
    {
      x1 = floor (box->bounds.origin.x + box->bounds.size.width -clip_radius);
      x2 = ceil (box->bounds.origin.x + box->bounds.size.width + clip_radius);
    }

  if (side == LEFT || side == RIGHT)
    {
      blur_flags |= GSK_BLUR_X;
      y1 = floor (box->bounds.origin.y - clip_radius);
      y2 = ceil (box->bounds.origin.y + box->bounds.size.height + clip_radius);
    }
  else if (side == TOP)
    {
      y1 = floor (box->bounds.origin.y -clip_radius);
      y2 = ceil (box->bounds.origin.y + clip_radius);
    }
  else
    {
      y1 = floor (box->bounds.origin.y + box->bounds.size.height -clip_radius);
      y2 = ceil (box->bounds.origin.y + box->bounds.size.height + clip_radius);
    }

  drawn_rect->x = x1;
  drawn_rect->y = y1;
  drawn_rect->width = x2 - x1;
  drawn_rect->height = y2 - y1;

  cairo_rectangle (cr, x1, y1, x2 - x1, y2 - y1);
  cairo_clip (cr);
  draw_shadow (cr, inset, box, clip_box, radius, color, blur_flags);
}

static gboolean
needs_blur (GskInsetShadowNode *self)
{
  /* The code doesn't actually do any blurring for radius 1, as it
   * ends up with box filter size 1 */
  if (self->blur_radius <= 1.0)
    return FALSE;

  return TRUE;
}

static void
gsk_inset_shadow_node_draw (GskRenderNode *node,
                            cairo_t       *cr)
{
  GskInsetShadowNode *self = (GskInsetShadowNode *) node;
  GskRoundedRect box, clip_box;
  int clip_radius;
  double x1c, y1c, x2c, y2c;

  /* We don't need to draw invisible shadows */
  if (gdk_rgba_is_clear (&self->color))
    return;

  cairo_clip_extents (cr, &x1c, &y1c, &x2c, &y2c);
  if (!gsk_rounded_rect_intersects_rect (&self->outline, &GRAPHENE_RECT_INIT (x1c, y1c, x2c - x1c, y2c - y1c)))
    return;

  clip_radius = gsk_cairo_blur_compute_pixels (self->blur_radius);

  cairo_save (cr);

  gsk_rounded_rect_path (&self->outline, cr);
  cairo_clip (cr);

  gsk_rounded_rect_init_copy (&box, &self->outline);
  gsk_rounded_rect_offset (&box, self->dx, self->dy);
  gsk_rounded_rect_shrink (&box, self->spread, self->spread, self->spread, self->spread);

  gsk_rounded_rect_init_copy (&clip_box, &self->outline);
  gsk_rounded_rect_shrink (&clip_box, -clip_radius, -clip_radius, -clip_radius, -clip_radius);

  if (!needs_blur (self))
    draw_shadow (cr, TRUE, &box, &clip_box, self->blur_radius, &self->color, GSK_BLUR_NONE);
  else
    {
      cairo_region_t *remaining;
      cairo_rectangle_int_t r;
      int i;

      /* For the blurred case we divide the rendering into 9 parts,
       * 4 of the corners, 4 for the horizonat/vertical lines and
       * one for the interior. We make the non-interior parts
       * large enought to fit the full radius of the blur, so that
       * the interior part can be drawn solidly.
       */

      /* In the inset case we want to paint the whole clip-box.
       * We could remove the part of "box" where the blur doesn't
       * reach, but computing that is a bit tricky since the
       * rounded corners are on the "inside" of it. */
      r.x = floor (clip_box.bounds.origin.x);
      r.y = floor (clip_box.bounds.origin.y);
      r.width = ceil (clip_box.bounds.origin.x + clip_box.bounds.size.width) - r.x;
      r.height = ceil (clip_box.bounds.origin.y + clip_box.bounds.size.height) - r.y;
      remaining = cairo_region_create_rectangle (&r);

      /* First do the corners of box */
      for (i = 0; i < 4; i++)
	{
	  cairo_save (cr);
          /* Always clip with remaining to ensure we never draw any area twice */
          gdk_cairo_region (cr, remaining);
          cairo_clip (cr);
	  draw_shadow_corner (cr, TRUE, &box, &clip_box, self->blur_radius, &self->color, i, &r);
	  cairo_restore (cr);

	  /* We drew the region, remove it from remaining */
	  cairo_region_subtract_rectangle (remaining, &r);
	}

      /* Then the sides */
      for (i = 0; i < 4; i++)
	{
	  cairo_save (cr);
          /* Always clip with remaining to ensure we never draw any area twice */
          gdk_cairo_region (cr, remaining);
          cairo_clip (cr);
	  draw_shadow_side (cr, TRUE, &box, &clip_box, self->blur_radius, &self->color, i, &r);
	  cairo_restore (cr);

	  /* We drew the region, remove it from remaining */
	  cairo_region_subtract_rectangle (remaining, &r);
	}

      /* Then the rest, which needs no blurring */

      cairo_save (cr);
      gdk_cairo_region (cr, remaining);
      cairo_clip (cr);
      draw_shadow (cr, TRUE, &box, &clip_box, self->blur_radius, &self->color, GSK_BLUR_NONE);
      cairo_restore (cr);

      cairo_region_destroy (remaining);
    }

  cairo_restore (cr);
}

static void
gsk_inset_shadow_node_get_bounds (GskRenderNode   *node,
                                  graphene_rect_t *bounds)
{
  GskInsetShadowNode *self = (GskInsetShadowNode *) node;

  graphene_rect_init_from_rect (bounds, &self->outline.bounds); 
}

static const GskRenderNodeClass GSK_INSET_SHADOW_NODE_CLASS = {
  GSK_INSET_SHADOW_NODE,
  sizeof (GskInsetShadowNode),
  "GskInsetShadowNode",
  gsk_inset_shadow_node_finalize,
  gsk_inset_shadow_node_make_immutable,
  gsk_inset_shadow_node_draw,
  gsk_inset_shadow_node_get_bounds
};

/**
 * gsk_inset_shadow_node_new:
 * @outline: outline of the region containing the shadow
 * @color: color of the shadow
 * @dx: horizontal offset of shadow
 * @dy: vertical offset of shadow
 * @spread: how far the shadow spreads towards the inside
 * @blur_radius: how much blur to apply to the shadow
 *
 * Creates a #GskRenderNode that will render an inset shadow
 * into the box given by @outline.
 *
 * Returns: A new #GskRenderNode
 *
 * Since: 3.90
 */
GskRenderNode *
gsk_inset_shadow_node_new (const GskRoundedRect *outline,
                           const GdkRGBA        *color,
                           float                 dx,
                           float                 dy,
                           float                 spread,
                           float                 blur_radius)
{
  GskInsetShadowNode *self;

  g_return_val_if_fail (outline != NULL, NULL);
  g_return_val_if_fail (color != NULL, NULL);

  self = (GskInsetShadowNode *) gsk_render_node_new (&GSK_INSET_SHADOW_NODE_CLASS);

  gsk_rounded_rect_init_copy (&self->outline, outline);
  self->color = *color;
  self->dx = dx;
  self->dy = dy;
  self->spread = spread;
  self->blur_radius = blur_radius;

  return &self->render_node;
}

/*** GSK_CAIRO_NODE ***/

typedef struct _GskCairoNode GskCairoNode;

struct _GskCairoNode
{
  GskRenderNode render_node;

  cairo_surface_t *surface;
  graphene_rect_t bounds;
};

static void
gsk_cairo_node_finalize (GskRenderNode *node)
{
  GskCairoNode *self = (GskCairoNode *) node;

  if (self->surface)
    cairo_surface_destroy (self->surface);
}

static void
gsk_cairo_node_make_immutable (GskRenderNode *node)
{
}

static void
gsk_cairo_node_draw (GskRenderNode *node,
                     cairo_t       *cr)
{
  GskCairoNode *self = (GskCairoNode *) node;

  if (self->surface == NULL)
    return;

  cairo_set_source_surface (cr, self->surface, self->bounds.origin.x, self->bounds.origin.y);
  cairo_paint (cr);
}

static void
gsk_cairo_node_get_bounds (GskRenderNode   *node,
                           graphene_rect_t *bounds)
{
  GskCairoNode *self = (GskCairoNode *) node;

  graphene_rect_init_from_rect (bounds, &self->bounds); 
}

static const GskRenderNodeClass GSK_CAIRO_NODE_CLASS = {
  GSK_CAIRO_NODE,
  sizeof (GskCairoNode),
  "GskCairoNode",
  gsk_cairo_node_finalize,
  gsk_cairo_node_make_immutable,
  gsk_cairo_node_draw,
  gsk_cairo_node_get_bounds
};

/*< private >
 * gsk_cairo_node_get_surface:
 * @node: a #GskRenderNode
 *
 * Retrieves the surface set using gsk_render_node_set_surface().
 *
 * Returns: (transfer none) (nullable): a Cairo surface
 */
cairo_surface_t *
gsk_cairo_node_get_surface (GskRenderNode *node)
{
  GskCairoNode *self = (GskCairoNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_CAIRO_NODE), NULL);

  return self->surface;
}

/**
 * gsk_cairo_node_new:
 * @bounds: the rectangle to render the to
 *
 * Creates a #GskRenderNode that will render a cairo surface
 * into the area given by @bounds. You can draw to the cairo
 * surface using gsk_cairo_node_get_draw_context()
 *
 * Returns: A new #GskRenderNode
 *
 * Since: 3.90
 */
GskRenderNode *
gsk_cairo_node_new (const graphene_rect_t *bounds)
{
  GskCairoNode *self;

  g_return_val_if_fail (bounds != NULL, NULL);

  self = (GskCairoNode *) gsk_render_node_new (&GSK_CAIRO_NODE_CLASS);

  graphene_rect_init_from_rect (&self->bounds, bounds);

  return &self->render_node;
}

/**
 * gsk_cairo_node_get_draw_context:
 * @node: a cairo #GskRenderNode
 * @renderer: (nullable): Renderer to optimize for or %NULL for any
 *
 * Creates a Cairo context for drawing using the surface associated
 * to the render node.
 * If no surface exists yet, a surface will be created optimized for
 * rendering to @renderer.
 *
 * Returns: (transfer full): a Cairo context used for drawing; use
 *   cairo_destroy() when done drawing
 *
 * Since: 3.90
 */
cairo_t *
gsk_cairo_node_get_draw_context (GskRenderNode *node,
                                 GskRenderer   *renderer)
{
  GskCairoNode *self = (GskCairoNode *) node;
  int width, height;
  cairo_t *res;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_CAIRO_NODE), NULL);
  g_return_val_if_fail (node->is_mutable, NULL);
  g_return_val_if_fail (renderer == NULL || GSK_IS_RENDERER (renderer), NULL);

  width = ceilf (self->bounds.size.width);
  height = ceilf (self->bounds.size.height);

  if (width <= 0 || height <= 0)
    {
      cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 0, 0);
      res = cairo_create (surface);
      cairo_surface_destroy (surface);
    }
  else if (self->surface == NULL)
    {
      if (renderer)
        {
          self->surface = gsk_renderer_create_cairo_surface (renderer,
                                                             CAIRO_FORMAT_ARGB32,
                                                             ceilf (self->bounds.size.width),
                                                             ceilf (self->bounds.size.height));
        }
      else
        {
          self->surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                                                      ceilf (self->bounds.size.width),
                                                      ceilf (self->bounds.size.height));
        }
      res = cairo_create (self->surface);
    }
  else
    {
      res = cairo_create (self->surface);
    }

  cairo_translate (res, -self->bounds.origin.x, -self->bounds.origin.y);

  cairo_rectangle (res,
                   self->bounds.origin.x, self->bounds.origin.y,
                   self->bounds.size.width, self->bounds.size.height);
  cairo_clip (res);

  if (GSK_DEBUG_CHECK (SURFACE))
    {
      const char *prefix;
      prefix = g_getenv ("GSK_DEBUG_PREFIX");
      if (!prefix || g_str_has_prefix (node->name, prefix))
        {
          cairo_save (res);
          cairo_rectangle (res,
                           self->bounds.origin.x + 1, self->bounds.origin.y + 1,
                           self->bounds.size.width - 2, self->bounds.size.height - 2);
          cairo_set_line_width (res, 2);
          cairo_set_source_rgb (res, 1, 0, 0);
          cairo_stroke (res);
          cairo_restore (res);
        }
    }

  return res;
}

/**** GSK_CONTAINER_NODE ***/

typedef struct _GskContainerNode GskContainerNode;

struct _GskContainerNode
{
  GskRenderNode render_node;

  GskRenderNode **children;
  guint n_children;
};

static void
gsk_container_node_finalize (GskRenderNode *node)
{
  GskContainerNode *container = (GskContainerNode *) node;
  guint i;

  for (i = 0; i < container->n_children; i++)
    gsk_render_node_unref (container->children[i]);

  g_free (container->children);
}

static void
gsk_container_node_make_immutable (GskRenderNode *node)
{
  GskContainerNode *container = (GskContainerNode *) node;
  guint i;

  for (i = 1; i < container->n_children; i++)
    {
      gsk_render_node_make_immutable (container->children[i]);
    }
}

static void
gsk_container_node_draw (GskRenderNode *node,
                         cairo_t       *cr)
{
  GskContainerNode *container = (GskContainerNode *) node;
  guint i;

  for (i = 0; i < container->n_children; i++)
    {
      gsk_render_node_draw (container->children[i], cr);
    }
}

static void
gsk_container_node_get_bounds (GskRenderNode   *node,
                               graphene_rect_t *bounds)
{
  GskContainerNode *container = (GskContainerNode *) node;
  guint i;

  if (container->n_children == 0)
    {
      graphene_rect_init_from_rect (bounds, graphene_rect_zero()); 
      return;
    }

  gsk_render_node_get_bounds (container->children[0], bounds);

  for (i = 1; i < container->n_children; i++)
    {
      graphene_rect_t child_bounds;
  
      gsk_render_node_get_bounds (container->children[i], &child_bounds);
      graphene_rect_union (bounds, &child_bounds, bounds);
    }
}

static const GskRenderNodeClass GSK_CONTAINER_NODE_CLASS = {
  GSK_CONTAINER_NODE,
  sizeof (GskContainerNode),
  "GskContainerNode",
  gsk_container_node_finalize,
  gsk_container_node_make_immutable,
  gsk_container_node_draw,
  gsk_container_node_get_bounds
};

/**
 * gsk_container_node_new:
 * @children: (array length=n_children) (transfer none): The children of the node
 * @n_children: Number of children in the @children array
 *
 * Creates a new #GskRenderNode instance for holding the given @children.
 * The new node will acquire a reference to each of the children.
 *
 * Returns: (transfer full): the new #GskRenderNode
 *
 * Since: 3.90
 */
GskRenderNode *
gsk_container_node_new (GskRenderNode **children,
                        guint           n_children)
{
  GskContainerNode *container;
  guint i;

  container = (GskContainerNode *) gsk_render_node_new (&GSK_CONTAINER_NODE_CLASS);

  container->children = g_memdup (children, sizeof (GskRenderNode *) * n_children);
  container->n_children = n_children;

  for (i = 0; i < container->n_children; i++)
    gsk_render_node_ref (container->children[i]);

  return &container->render_node;
}

/**
 * gsk_container_node_get_n_children:
 * @node: a container #GskRenderNode
 *
 * Retrieves the number of direct children of @node.
 *
 * Returns: the number of children of the #GskRenderNode
 *
 * Since: 3.90
 */
guint
gsk_container_node_get_n_children (GskRenderNode *node)
{
  GskContainerNode *container = (GskContainerNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_CONTAINER_NODE), 0);

  return container->n_children;
}

GskRenderNode *
gsk_container_node_get_child (GskRenderNode *node,
                              guint          idx)
{
  GskContainerNode *container = (GskContainerNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_CONTAINER_NODE), NULL);
  g_return_val_if_fail (idx < container->n_children, 0);

  return container->children[idx];
}

/*** GSK_TRANSFORM_NODE ***/

typedef struct _GskTransformNode GskTransformNode;

struct _GskTransformNode
{
  GskRenderNode render_node;

  GskRenderNode *child;
  graphene_matrix_t transform;
};

static void
gsk_transform_node_finalize (GskRenderNode *node)
{
  GskTransformNode *self = (GskTransformNode *) node;

  gsk_render_node_unref (self->child);
}

static void
gsk_transform_node_make_immutable (GskRenderNode *node)
{
  GskTransformNode *self = (GskTransformNode *) node;

  gsk_render_node_make_immutable (self->child);
}

static void
gsk_transform_node_draw (GskRenderNode *node,
                         cairo_t       *cr)
{
  GskTransformNode *self = (GskTransformNode *) node;
  cairo_matrix_t ctm;

  if (graphene_matrix_to_2d (&self->transform, &ctm.xx, &ctm.yx, &ctm.xy, &ctm.yy, &ctm.x0, &ctm.y0))
    {
      GSK_NOTE (CAIRO, g_print ("CTM = { .xx = %g, .yx = %g, .xy = %g, .yy = %g, .x0 = %g, .y0 = %g }\n",
                                ctm.xx, ctm.yx,
                                ctm.xy, ctm.yy,
                                ctm.x0, ctm.y0));
      cairo_transform (cr, &ctm);
  
      gsk_render_node_draw (self->child, cr);
    }
  else
    {
      graphene_rect_t bounds;

      gsk_render_node_get_bounds (node, &bounds);

      cairo_set_source_rgb (cr, 255 / 255., 105 / 255., 180 / 255.);
      cairo_rectangle (cr, bounds.origin.x, bounds.origin.x, bounds.size.width, bounds.size.height);
      cairo_fill (cr);
    }
}

static void
gsk_transform_node_get_bounds (GskRenderNode   *node,
                               graphene_rect_t *bounds)
{
  GskTransformNode *self = (GskTransformNode *) node;
  graphene_rect_t child_bounds;

  gsk_render_node_get_bounds (self->child, &child_bounds);

  graphene_matrix_transform_bounds (&self->transform,
                                    &child_bounds,
                                    bounds);
}

static const GskRenderNodeClass GSK_TRANSFORM_NODE_CLASS = {
  GSK_TRANSFORM_NODE,
  sizeof (GskTransformNode),
  "GskTransformNode",
  gsk_transform_node_finalize,
  gsk_transform_node_make_immutable,
  gsk_transform_node_draw,
  gsk_transform_node_get_bounds
};

/**
 * gsk_transform_node_new:
 * @child: The node to transform
 * @transform: The transform to apply
 *
 * Creates a #GskRenderNode that will transform the given @child
 * with the given @transform.
 *
 * Returns: A new #GskRenderNode
 *
 * Since: 3.90
 */
GskRenderNode *
gsk_transform_node_new (GskRenderNode           *child,
                        const graphene_matrix_t *transform)
{
  GskTransformNode *self;

  g_return_val_if_fail (GSK_IS_RENDER_NODE (child), NULL);
  g_return_val_if_fail (transform != NULL, NULL);

  self = (GskTransformNode *) gsk_render_node_new (&GSK_TRANSFORM_NODE_CLASS);

  self->child = gsk_render_node_ref (child);
  graphene_matrix_init_from_matrix (&self->transform, transform);

  return &self->render_node;
}

/**
 * gsk_transform_node_get_child:
 * @node: a transform @GskRenderNode
 *
 * Gets the child node that is getting transformed by the given @node.
 *
 * Returns: (transfer none): The child that is getting transformed
 **/
GskRenderNode *
gsk_transform_node_get_child (GskRenderNode *node)
{
  GskTransformNode *self = (GskTransformNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_TRANSFORM_NODE), NULL);

  return self->child;
}

void
gsk_transform_node_get_transform (GskRenderNode     *node,
                                  graphene_matrix_t *transform)
{
  GskTransformNode *self = (GskTransformNode *) node;

  g_return_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_TRANSFORM_NODE));

  graphene_matrix_init_from_matrix (transform, &self->transform);
}

/*** GSK_OPACITY_NODE ***/

typedef struct _GskOpacityNode GskOpacityNode;

struct _GskOpacityNode
{
  GskRenderNode render_node;

  GskRenderNode *child;
  double opacity;
};

static void
gsk_opacity_node_finalize (GskRenderNode *node)
{
  GskOpacityNode *self = (GskOpacityNode *) node;

  gsk_render_node_unref (self->child);
}

static void
gsk_opacity_node_make_immutable (GskRenderNode *node)
{
  GskOpacityNode *self = (GskOpacityNode *) node;

  gsk_render_node_make_immutable (self->child);
}

static void
gsk_opacity_node_draw (GskRenderNode *node,
                       cairo_t       *cr)
{
  GskOpacityNode *self = (GskOpacityNode *) node;
  graphene_rect_t bounds;

  cairo_save (cr);

  /* clip so the push_group() creates a smaller surface */
  gsk_render_node_get_bounds (self->child, &bounds);
  cairo_rectangle (cr, bounds.origin.x, bounds.origin.y, bounds.size.width, bounds.size.height);
  cairo_clip (cr);

  cairo_push_group (cr);

  gsk_render_node_draw (self->child, cr);

  cairo_pop_group_to_source (cr);
  cairo_paint_with_alpha (cr, self->opacity);

  cairo_restore (cr);
}

static void
gsk_opacity_node_get_bounds (GskRenderNode   *node,
                             graphene_rect_t *bounds)
{
  GskOpacityNode *self = (GskOpacityNode *) node;

  gsk_render_node_get_bounds (self->child, bounds);
}

static const GskRenderNodeClass GSK_OPACITY_NODE_CLASS = {
  GSK_OPACITY_NODE,
  sizeof (GskOpacityNode),
  "GskOpacityNode",
  gsk_opacity_node_finalize,
  gsk_opacity_node_make_immutable,
  gsk_opacity_node_draw,
  gsk_opacity_node_get_bounds
};

/**
 * gsk_opacity_node_new:
 * @child: The node to draw
 * @opacity: The opacity to apply
 *
 * Creates a #GskRenderNode that will drawn the @child with reduced
 * @opacity.
 *
 * Returns: A new #GskRenderNode
 *
 * Since: 3.90
 */
GskRenderNode *
gsk_opacity_node_new (GskRenderNode *child,
                      double         opacity)
{
  GskOpacityNode *self;

  g_return_val_if_fail (GSK_IS_RENDER_NODE (child), NULL);

  self = (GskOpacityNode *) gsk_render_node_new (&GSK_OPACITY_NODE_CLASS);

  self->child = gsk_render_node_ref (child);
  self->opacity = CLAMP (opacity, 0.0, 1.0);

  return &self->render_node;
}

/**
 * gsk_opacity_node_get_child:
 * @node: a opacity @GskRenderNode
 *
 * Gets the child node that is getting opacityed by the given @node.
 *
 * Returns: (transfer none): The child that is getting opacityed
 **/
GskRenderNode *
gsk_opacity_node_get_child (GskRenderNode *node)
{
  GskOpacityNode *self = (GskOpacityNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_OPACITY_NODE), NULL);

  return self->child;
}

double
gsk_opacity_node_get_opacity (GskRenderNode *node)
{
  GskOpacityNode *self = (GskOpacityNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_OPACITY_NODE), 1.0);

  return self->opacity;
}

/*** GSK_CLIP_NODE ***/

typedef struct _GskClipNode GskClipNode;

struct _GskClipNode
{
  GskRenderNode render_node;

  GskRenderNode *child;
  graphene_rect_t clip;
};

static void
gsk_clip_node_finalize (GskRenderNode *node)
{
  GskClipNode *self = (GskClipNode *) node;

  gsk_render_node_unref (self->child);
}

static void
gsk_clip_node_make_immutable (GskRenderNode *node)
{
  GskClipNode *self = (GskClipNode *) node;

  gsk_render_node_make_immutable (self->child);
}

static void
gsk_clip_node_draw (GskRenderNode *node,
                    cairo_t       *cr)
{
  GskClipNode *self = (GskClipNode *) node;

  cairo_save (cr);

  cairo_rectangle (cr,
                   self->clip.origin.x, self->clip.origin.y,
                   self->clip.size.width, self->clip.size.height);
  cairo_clip (cr);

  gsk_render_node_draw (self->child, cr);

  cairo_restore (cr);
}

static void
gsk_clip_node_get_bounds (GskRenderNode   *node,
                          graphene_rect_t *bounds)
{
  GskClipNode *self = (GskClipNode *) node;
  graphene_rect_t child_bounds;

  gsk_render_node_get_bounds (self->child, &child_bounds);

  graphene_rect_intersection (&self->clip, &child_bounds, bounds);
}

static const GskRenderNodeClass GSK_CLIP_NODE_CLASS = {
  GSK_CLIP_NODE,
  sizeof (GskClipNode),
  "GskClipNode",
  gsk_clip_node_finalize,
  gsk_clip_node_make_immutable,
  gsk_clip_node_draw,
  gsk_clip_node_get_bounds
};

/**
 * gsk_clip_node_new:
 * @child: The node to draw
 * @clip: The clip to apply
 *
 * Creates a #GskRenderNode that will clip the @child to the area
 * given by @clip.
 *
 * Returns: A new #GskRenderNode
 *
 * Since: 3.90
 */
GskRenderNode *
gsk_clip_node_new (GskRenderNode         *child,
                   const graphene_rect_t *clip)
{
  GskClipNode *self;

  g_return_val_if_fail (GSK_IS_RENDER_NODE (child), NULL);
  g_return_val_if_fail (clip != NULL, NULL);

  self = (GskClipNode *) gsk_render_node_new (&GSK_CLIP_NODE_CLASS);

  self->child = gsk_render_node_ref (child);
  graphene_rect_normalize_r (clip, &self->clip);

  return &self->render_node;
}

/**
 * gsk_clip_node_get_child:
 * @node: a clip @GskRenderNode
 *
 * Gets the child node that is getting clipped by the given @node.
 *
 * Returns: (transfer none): The child that is getting clipped
 **/
GskRenderNode *
gsk_clip_node_get_child (GskRenderNode *node)
{
  GskClipNode *self = (GskClipNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_CLIP_NODE), NULL);

  return self->child;
}

const graphene_rect_t *
gsk_clip_node_peek_clip (GskRenderNode *node)
{
  GskClipNode *self = (GskClipNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_CLIP_NODE), NULL);

  return &self->clip;
}

/*** GSK_ROUNDED_CLIP_NODE ***/

typedef struct _GskRoundedClipNode GskRoundedClipNode;

struct _GskRoundedClipNode
{
  GskRenderNode render_node;

  GskRenderNode *child;
  GskRoundedRect clip;
};

static void
gsk_rounded_clip_node_finalize (GskRenderNode *node)
{
  GskRoundedClipNode *self = (GskRoundedClipNode *) node;

  gsk_render_node_unref (self->child);
}

static void
gsk_rounded_clip_node_make_immutable (GskRenderNode *node)
{
  GskRoundedClipNode *self = (GskRoundedClipNode *) node;

  gsk_render_node_make_immutable (self->child);
}

static void
gsk_rounded_clip_node_draw (GskRenderNode *node,
                            cairo_t       *cr)
{
  GskRoundedClipNode *self = (GskRoundedClipNode *) node;

  cairo_save (cr);

  gsk_rounded_rect_path (&self->clip, cr);
  cairo_clip (cr);

  gsk_render_node_draw (self->child, cr);

  cairo_restore (cr);
}

static void
gsk_rounded_clip_node_get_bounds (GskRenderNode   *node,
                                  graphene_rect_t *bounds)
{
  GskRoundedClipNode *self = (GskRoundedClipNode *) node;
  graphene_rect_t child_bounds;

  gsk_render_node_get_bounds (self->child, &child_bounds);

  graphene_rect_intersection (&self->clip.bounds, &child_bounds, bounds);
}

static const GskRenderNodeClass GSK_ROUNDED_CLIP_NODE_CLASS = {
  GSK_ROUNDED_CLIP_NODE,
  sizeof (GskRoundedClipNode),
  "GskRoundedClipNode",
  gsk_rounded_clip_node_finalize,
  gsk_rounded_clip_node_make_immutable,
  gsk_rounded_clip_node_draw,
  gsk_rounded_clip_node_get_bounds
};

/**
 * gsk_rounded_clip_node_new:
 * @child: The node to draw
 * @clip: The clip to apply
 *
 * Creates a #GskRenderNode that will clip the @child to the area
 * given by @clip.
 *
 * Returns: A new #GskRenderNode
 *
 * Since: 3.90
 */
GskRenderNode *
gsk_rounded_clip_node_new (GskRenderNode         *child,
                           const GskRoundedRect  *clip)
{
  GskRoundedClipNode *self;

  g_return_val_if_fail (GSK_IS_RENDER_NODE (child), NULL);
  g_return_val_if_fail (clip != NULL, NULL);

  self = (GskRoundedClipNode *) gsk_render_node_new (&GSK_ROUNDED_CLIP_NODE_CLASS);

  self->child = gsk_render_node_ref (child);
  gsk_rounded_rect_init_copy (&self->clip, clip);

  return &self->render_node;
}

/**
 * gsk_rounded_clip_node_get_child:
 * @node: a clip @GskRenderNode
 *
 * Gets the child node that is getting clipped by the given @node.
 *
 * Returns: (transfer none): The child that is getting clipped
 **/
GskRenderNode *
gsk_rounded_clip_node_get_child (GskRenderNode *node)
{
  GskRoundedClipNode *self = (GskRoundedClipNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_ROUNDED_CLIP_NODE), NULL);

  return self->child;
}

const GskRoundedRect *
gsk_rounded_clip_node_peek_clip (GskRenderNode *node)
{
  GskRoundedClipNode *self = (GskRoundedClipNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_ROUNDED_CLIP_NODE), NULL);

  return &self->clip;
}

/*** GSK_SHADOW_NODE ***/

typedef struct _GskShadowNode GskShadowNode;

struct _GskShadowNode
{
  GskRenderNode render_node;

  GskRenderNode *child;

  GskShadow *shadows;
  gsize n_shadows;
};

static void
gsk_shadow_node_finalize (GskRenderNode *node)
{
  GskShadowNode *self = (GskShadowNode *) node;

  gsk_render_node_unref (self->child);

  g_free (self->shadows);
}

static void
gsk_shadow_node_make_immutable (GskRenderNode *node)
{
  GskShadowNode *self = (GskShadowNode *) node;

  gsk_render_node_make_immutable (self->child);
}

static void
gsk_shadow_node_draw (GskRenderNode *node,
                      cairo_t       *cr)
{
  GskShadowNode *self = (GskShadowNode *) node;
  cairo_pattern_t *pattern;
  gsize i;

  cairo_push_group (cr);
  gsk_render_node_draw (self->child, cr);
  pattern = cairo_pop_group (cr);

  for (i = 0; i < self->n_shadows; i++)
    {
      GskShadow *shadow = &self->shadows[i];

      /* We don't need to draw invisible shadows */
      if (gdk_rgba_is_clear (&shadow->color))
        continue;

      cairo_save (cr);
      gdk_cairo_set_source_rgba (cr, &shadow->color);
      cr = gsk_cairo_blur_start_drawing (cr, shadow->radius, GSK_BLUR_X | GSK_BLUR_Y);

      cairo_translate (cr, shadow->dx, shadow->dy);
      cairo_mask (cr, pattern);

      cr = gsk_cairo_blur_finish_drawing (cr, shadow->radius, &shadow->color, GSK_BLUR_X | GSK_BLUR_Y);
      cairo_restore (cr);
    }

  cairo_set_source (cr, pattern);
  cairo_paint (cr);

  cairo_pattern_destroy (pattern);
}

static void
gsk_shadow_node_get_bounds (GskRenderNode   *node,
                            graphene_rect_t *bounds)
{
  GskShadowNode *self = (GskShadowNode *) node;
  float top = 0, right = 0, bottom = 0, left = 0;
  gsize i;

  gsk_render_node_get_bounds (self->child, bounds);

  for (i = 0; i < self->n_shadows; i++)
    {
      float clip_radius = gsk_cairo_blur_compute_pixels (self->shadows[i].radius);
      top = MAX (top, clip_radius - self->shadows[i].dy);
      right = MAX (right, clip_radius + self->shadows[i].dx);
      bottom = MAX (bottom, clip_radius + self->shadows[i].dy);
      left = MAX (left, clip_radius - self->shadows[i].dx);
    }

  bounds->origin.x -= left;
  bounds->origin.y -= top;
  bounds->size.width += left + right;
  bounds->size.height += top + bottom;
}

static const GskRenderNodeClass GSK_SHADOW_NODE_CLASS = {
  GSK_SHADOW_NODE,
  sizeof (GskShadowNode),
  "GskShadowNode",
  gsk_shadow_node_finalize,
  gsk_shadow_node_make_immutable,
  gsk_shadow_node_draw,
  gsk_shadow_node_get_bounds
};

/**
 * gsk_shadow_node_new:
 * @child: The node to draw
 * @shadows: (array length=n_shadows): The shadows to apply
 * @n_shadows: number of entries in the @shadows array
 *
 * Creates a #GskRenderNode that will draw a @child with the given
 * @shadows below it.
 *
 * Returns: A new #GskRenderNode
 *
 * Since: 3.90
 */
GskRenderNode *
gsk_shadow_node_new (GskRenderNode         *child,
                     const GskShadow       *shadows,
                     gsize                  n_shadows)
{
  GskShadowNode *self;

  g_return_val_if_fail (GSK_IS_RENDER_NODE (child), NULL);
  g_return_val_if_fail (shadows != NULL, NULL);
  g_return_val_if_fail (n_shadows > 0, NULL);

  self = (GskShadowNode *) gsk_render_node_new (&GSK_SHADOW_NODE_CLASS);

  self->child = gsk_render_node_ref (child);
  self->shadows = g_memdup (shadows, n_shadows * sizeof (GskShadow));
  self->n_shadows = n_shadows;

  return &self->render_node;
}

GskRenderNode *
gsk_shadow_node_get_child (GskRenderNode *node)
{
  GskShadowNode *self = (GskShadowNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_SHADOW_NODE), NULL);

  return self->child;
}

const GskShadow *
gsk_shadow_node_peek_shadow (GskRenderNode *node,
                             gsize          i)
{
  GskShadowNode *self = (GskShadowNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_SHADOW_NODE), NULL);
  g_return_val_if_fail (i < self->n_shadows, NULL);

  return &self->shadows[i];
}

gsize
gsk_shadow_node_get_n_shadows (GskRenderNode *node)
{
  GskShadowNode *self = (GskShadowNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_SHADOW_NODE), 0);

  return self->n_shadows;
}

/*** GSK_BLEND_NODE ***/

typedef struct _GskBlendNode GskBlendNode;

struct _GskBlendNode
{
  GskRenderNode render_node;

  GskRenderNode *bottom;
  GskRenderNode *top;
  GskBlendMode blend_mode;
};

static cairo_operator_t
gsk_blend_mode_to_cairo_operator (GskBlendMode blend_mode)
{
  switch (blend_mode)
    {
    default:
      g_assert_not_reached ();
    case GSK_BLEND_MODE_DEFAULT:
      return CAIRO_OPERATOR_OVER;
    case GSK_BLEND_MODE_MULTIPLY:
      return CAIRO_OPERATOR_MULTIPLY;
    case GSK_BLEND_MODE_SCREEN:
      return CAIRO_OPERATOR_SCREEN;
    case GSK_BLEND_MODE_OVERLAY:
      return CAIRO_OPERATOR_OVERLAY;
    case GSK_BLEND_MODE_DARKEN:
      return CAIRO_OPERATOR_DARKEN;
    case GSK_BLEND_MODE_LIGHTEN:
      return CAIRO_OPERATOR_LIGHTEN;
    case GSK_BLEND_MODE_COLOR_DODGE:
      return CAIRO_OPERATOR_COLOR_DODGE;
    case GSK_BLEND_MODE_COLOR_BURN:
      return CAIRO_OPERATOR_COLOR_BURN;
    case GSK_BLEND_MODE_HARD_LIGHT:
      return CAIRO_OPERATOR_HARD_LIGHT;
    case GSK_BLEND_MODE_SOFT_LIGHT:
      return CAIRO_OPERATOR_SOFT_LIGHT;
    case GSK_BLEND_MODE_DIFFERENCE:
      return CAIRO_OPERATOR_DIFFERENCE;
    case GSK_BLEND_MODE_EXCLUSION:
      return CAIRO_OPERATOR_EXCLUSION;
    case GSK_BLEND_MODE_COLOR:
      return CAIRO_OPERATOR_HSL_COLOR;
    case GSK_BLEND_MODE_HUE:
      return CAIRO_OPERATOR_HSL_HUE;
    case GSK_BLEND_MODE_SATURATION:
      return CAIRO_OPERATOR_HSL_SATURATION;
    case GSK_BLEND_MODE_LUMINOSITY:
      return CAIRO_OPERATOR_HSL_LUMINOSITY;
    }
}

static void
gsk_blend_node_finalize (GskRenderNode *node)
{
  GskBlendNode *self = (GskBlendNode *) node;

  gsk_render_node_unref (self->bottom);
  gsk_render_node_unref (self->top);
}

static void
gsk_blend_node_make_immutable (GskRenderNode *node)
{
  GskBlendNode *self = (GskBlendNode *) node;

  gsk_render_node_make_immutable (self->bottom);
  gsk_render_node_make_immutable (self->top);
}

static void
gsk_blend_node_draw (GskRenderNode *node,
                     cairo_t       *cr)
{
  GskBlendNode *self = (GskBlendNode *) node;

  cairo_push_group (cr);
  gsk_render_node_draw (self->bottom, cr);

  cairo_push_group (cr);
  gsk_render_node_draw (self->top, cr);

  cairo_pop_group_to_source (cr);
  cairo_set_operator (cr, gsk_blend_mode_to_cairo_operator (self->blend_mode));
  cairo_paint (cr);

  cairo_pop_group_to_source (cr); /* resets operator */
  cairo_paint (cr);
}

static void
gsk_blend_node_get_bounds (GskRenderNode   *node,
                           graphene_rect_t *bounds)
{
  GskBlendNode *self = (GskBlendNode *) node;
  graphene_rect_t bottom_bounds, top_bounds;

  gsk_render_node_get_bounds (self->bottom, &bottom_bounds);
  gsk_render_node_get_bounds (self->top, &top_bounds);

  graphene_rect_union (&bottom_bounds, &top_bounds, bounds);
}

static const GskRenderNodeClass GSK_BLEND_NODE_CLASS = {
  GSK_BLEND_NODE,
  sizeof (GskBlendNode),
  "GskBlendNode",
  gsk_blend_node_finalize,
  gsk_blend_node_make_immutable,
  gsk_blend_node_draw,
  gsk_blend_node_get_bounds
};

/**
 * gsk_blend_node_new:
 * @bottom: The bottom node to be drawn
 * @top: The node to be blended onto the @bottom node
 * @blend_mode: The blend mode to use
 *
 * Creates a #GskRenderNode that will use @blend_mode to blend the @top
 * node onto the @bottom node.
 *
 * Returns: A new #GskRenderNode
 *
 * Since: 3.90
 */
GskRenderNode *
gsk_blend_node_new (GskRenderNode *bottom,
                    GskRenderNode *top,
                    GskBlendMode   blend_mode)
{
  GskBlendNode *self;

  g_return_val_if_fail (GSK_IS_RENDER_NODE (bottom), NULL);
  g_return_val_if_fail (GSK_IS_RENDER_NODE (top), NULL);

  self = (GskBlendNode *) gsk_render_node_new (&GSK_BLEND_NODE_CLASS);

  self->bottom = gsk_render_node_ref (bottom);
  self->top = gsk_render_node_ref (top);
  self->blend_mode = blend_mode;

  return &self->render_node;
}

GskRenderNode *
gsk_blend_node_get_bottom_child (GskRenderNode *node)
{
  GskBlendNode *self = (GskBlendNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_BLEND_NODE), NULL);

  return self->bottom;
}

GskRenderNode *
gsk_blend_node_get_top_child (GskRenderNode *node)
{
  GskBlendNode *self = (GskBlendNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_BLEND_NODE), NULL);

  return self->top;
}

GskBlendMode
gsk_blend_node_get_blend_mode (GskRenderNode *node)
{
  GskBlendNode *self = (GskBlendNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_BLEND_NODE), GSK_BLEND_MODE_DEFAULT);

  return self->blend_mode;
}

/*** GSK_CROSS_FADE_NODE ***/

typedef struct _GskCrossFadeNode GskCrossFadeNode;

struct _GskCrossFadeNode
{
  GskRenderNode render_node;

  GskRenderNode *start;
  GskRenderNode *end;
  double         progress;
};

static void
gsk_cross_fade_node_finalize (GskRenderNode *node)
{
  GskCrossFadeNode *self = (GskCrossFadeNode *) node;

  gsk_render_node_unref (self->start);
  gsk_render_node_unref (self->end);
}

static void
gsk_cross_fade_node_make_immutable (GskRenderNode *node)
{
  GskCrossFadeNode *self = (GskCrossFadeNode *) node;

  gsk_render_node_make_immutable (self->start);
  gsk_render_node_make_immutable (self->end);
}

static void
gsk_cross_fade_node_draw (GskRenderNode *node,
                          cairo_t       *cr)
{
  GskCrossFadeNode *self = (GskCrossFadeNode *) node;

  cairo_push_group (cr);
  gsk_render_node_draw (self->start, cr);

  cairo_push_group (cr);
  gsk_render_node_draw (self->end, cr);

  cairo_pop_group_to_source (cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint_with_alpha (cr, self->progress);

  cairo_pop_group_to_source (cr); /* resets operator */
  cairo_paint (cr);
}

static void
gsk_cross_fade_node_get_bounds (GskRenderNode   *node,
                                graphene_rect_t *bounds)
{
  GskCrossFadeNode *self = (GskCrossFadeNode *) node;
  graphene_rect_t start_bounds, end_bounds;

  gsk_render_node_get_bounds (self->start, &start_bounds);
  gsk_render_node_get_bounds (self->end, &end_bounds);

  graphene_rect_union (&start_bounds, &end_bounds, bounds);
}

static const GskRenderNodeClass GSK_CROSS_FADE_NODE_CLASS = {
  GSK_CROSS_FADE_NODE,
  sizeof (GskCrossFadeNode),
  "GskCrossFadeNode",
  gsk_cross_fade_node_finalize,
  gsk_cross_fade_node_make_immutable,
  gsk_cross_fade_node_draw,
  gsk_cross_fade_node_get_bounds
};

/**
 * gsk_cross_fade_node_new:
 * @start: The start node to be drawn
 * @end: The node to be cross_fadeed onto the @start node
 * @progress: How far the fade has progressed from start to end. The value will
 *     be clamped to the range [0 ... 1]
 *
 * Creates a #GskRenderNode that will do a cross-fade between @start and @end.
 *
 * Returns: A new #GskRenderNode
 *
 * Since: 3.90
 */
GskRenderNode *
gsk_cross_fade_node_new (GskRenderNode *start,
                         GskRenderNode *end,
                         double         progress)
{
  GskCrossFadeNode *self;

  g_return_val_if_fail (GSK_IS_RENDER_NODE (start), NULL);
  g_return_val_if_fail (GSK_IS_RENDER_NODE (end), NULL);

  self = (GskCrossFadeNode *) gsk_render_node_new (&GSK_CROSS_FADE_NODE_CLASS);

  self->start = gsk_render_node_ref (start);
  self->end = gsk_render_node_ref (end);
  self->progress = CLAMP (progress, 0.0, 1.0);

  return &self->render_node;
}

GskRenderNode *
gsk_cross_fade_node_get_start_child (GskRenderNode *node)
{
  GskCrossFadeNode *self = (GskCrossFadeNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_CROSS_FADE_NODE), NULL);

  return self->start;
}

GskRenderNode *
gsk_cross_fade_node_get_end_child (GskRenderNode *node)
{
  GskCrossFadeNode *self = (GskCrossFadeNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_CROSS_FADE_NODE), NULL);

  return self->end;
}

double
gsk_cross_fade_node_get_progress (GskRenderNode *node)
{
  GskCrossFadeNode *self = (GskCrossFadeNode *) node;

  g_return_val_if_fail (GSK_IS_RENDER_NODE_TYPE (node, GSK_CROSS_FADE_NODE), 0.0);

  return self->progress;
}

