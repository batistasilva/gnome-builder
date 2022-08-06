/* ide-tweaks.c
 *
 * Copyright 2022 Christian Hergert <chergert@redhat.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define G_LOG_DOMAIN "ide-tweaks"

#include "config.h"

#include <gtk/gtk.h>

#include "ide-tweaks.h"
#include "ide-tweaks-section.h"

struct _IdeTweaks
{
  IdeTweaksItem  parent_instance;
  GtkBuilder    *builder;
};

G_DEFINE_FINAL_TYPE (IdeTweaks, ide_tweaks, IDE_TYPE_TWEAKS_ITEM)

static gboolean
ide_tweaks_accepts (IdeTweaksItem *item,
                    IdeTweaksItem *child)
{
  g_assert (IDE_IS_TWEAKS_ITEM (item));
  g_assert (IDE_IS_TWEAKS_ITEM (child));

  return IDE_IS_TWEAKS_SECTION (child);
}

static void
ide_tweaks_dispose (GObject *object)
{
  IdeTweaks *self = (IdeTweaks *)object;

  g_clear_object (&self->builder);

  G_OBJECT_CLASS (ide_tweaks_parent_class)->dispose (object);
}

static void
ide_tweaks_class_init (IdeTweaksClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  IdeTweaksItemClass *item_class = IDE_TWEAKS_ITEM_CLASS (klass);

  object_class->dispose = ide_tweaks_dispose;

  item_class->accepts = ide_tweaks_accepts;
}

static void
ide_tweaks_init (IdeTweaks *self)
{
  self->builder = gtk_builder_new ();
  gtk_builder_set_current_object (self->builder, G_OBJECT (self));
}

IdeTweaks *
ide_tweaks_new (void)
{
  return g_object_new (IDE_TYPE_TWEAKS, NULL);
}

gboolean
ide_tweaks_load_from_file (IdeTweaks     *self,
                           GFile         *file,
                           GCancellable  *cancellable,
                           GError       **error)
{
  g_autofree char *contents = NULL;
  gsize length;

  g_return_val_if_fail (IDE_IS_TWEAKS (self), FALSE);
  g_return_val_if_fail (G_IS_FILE (file), FALSE);
  g_return_val_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable), FALSE);

  if (!g_file_load_contents (file, cancellable, &contents, &length, NULL, error))
    return FALSE;

  return gtk_builder_extend_with_template (self->builder,
                                           G_OBJECT (self), G_OBJECT_TYPE (self),
                                           contents, length,
                                           error);
}

void
ide_tweaks_expose_object (IdeTweaks  *self,
                          const char *name,
                          GObject    *object)
{
  g_return_if_fail (IDE_IS_TWEAKS (self));
  g_return_if_fail (name != NULL);
  g_return_if_fail (G_IS_OBJECT (object));

  gtk_builder_expose_object (self->builder, name, object);
}
