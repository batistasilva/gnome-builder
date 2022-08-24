/* gbp-projectui-tweaks-addin.c
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

#define G_LOG_DOMAIN "gbp-projectui-tweaks-addin"

#include "config.h"

#include <glib/gi18n.h>

#include <gtk/gtk.h>

#include <libide-foundry.h>
#include <libide-vcs.h>

#include "gbp-projectui-tweaks-addin.h"

struct _GbpProjectuiTweaksAddin
{
  IdeTweaksAddin parent_instance;
  IdeContext *context;
};

G_DEFINE_FINAL_TYPE (GbpProjectuiTweaksAddin, gbp_projectui_tweaks_addin, IDE_TYPE_TWEAKS_ADDIN)

enum {
  PROP_0,
  PROP_BUILD_SYSTEM,
  PROP_SOURCE_DIRECTORY,
  PROP_VCS,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static char *
gbp_projectui_tweaks_addin_get_build_system (GbpProjectuiTweaksAddin *self)
{
  IdeBuildSystem *build_system;

  if (self->context &&
      (build_system = ide_build_system_from_context (self->context)))
    return ide_build_system_get_display_name (build_system);

  return NULL;
}

static char *
gbp_projectui_tweaks_addin_get_source_directory (GbpProjectuiTweaksAddin *self)
{
  g_autoptr(GFile) workdir = NULL;

  if (self->context && (workdir = ide_context_ref_workdir (self->context)))
    {
      if (g_file_is_native (workdir))
        {
          g_autofree char *srcdir = g_file_get_path (workdir);
          return ide_path_collapse (srcdir);
        }

      return g_file_get_uri (workdir);
    }

  return NULL;
}

static char *
gbp_projectui_tweaks_addin_get_vcs (GbpProjectuiTweaksAddin *self)
{
  IdeVcs *vcs;

  if (self->context &&
      (vcs = ide_vcs_from_context (self->context)))
    return ide_vcs_get_display_name (vcs);

  return NULL;
}

static void
gbp_projectui_tweaks_addin_get_property (GObject    *object,
                                         guint       prop_id,
                                         GValue     *value,
                                         GParamSpec *pspec)
{
  GbpProjectuiTweaksAddin *self = GBP_PROJECTUI_TWEAKS_ADDIN (object);

  switch (prop_id)
    {
    case PROP_BUILD_SYSTEM:
      g_value_take_string (value, gbp_projectui_tweaks_addin_get_build_system (self));
      break;

    case PROP_SOURCE_DIRECTORY:
      g_value_take_string (value, gbp_projectui_tweaks_addin_get_source_directory (self));
      break;

    case PROP_VCS:
      g_value_take_string (value, gbp_projectui_tweaks_addin_get_vcs (self));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gbp_projectui_tweaks_addin_load (IdeTweaksAddin *addin,
                                 IdeTweaks      *tweaks)
{
  GbpProjectuiTweaksAddin *self = (GbpProjectuiTweaksAddin *)addin;
  g_autoptr(GtkFlattenListModel) configs = NULL;
  g_autoptr(GListStore) store = NULL;

  g_assert (IDE_IS_MAIN_THREAD ());
  g_assert (GBP_IS_PROJECTUI_TWEAKS_ADDIN (self));
  g_assert (IDE_IS_TWEAKS (tweaks));

  ide_tweaks_addin_set_resource_paths (IDE_TWEAKS_ADDIN (self),
                                       IDE_STRV_INIT ("/plugins/projectui/tweaks.ui"));

  store = g_list_store_new (G_TYPE_LIST_MODEL);
  configs = gtk_flatten_list_model_new (G_LIST_MODEL (g_object_ref (store)));
  ide_tweaks_expose_object (tweaks, "Configurations", G_OBJECT (configs));

  self->context = ide_tweaks_get_context (tweaks);

  if (self->context != NULL)
    {
      IdeConfigManager *config_manager = ide_config_manager_from_context (self->context);
      g_list_store_append (store, config_manager);
    }

  IDE_TWEAKS_ADDIN_CLASS (gbp_projectui_tweaks_addin_parent_class)->load (addin, tweaks);
}

static void
gbp_projectui_tweaks_addin_unload (IdeTweaksAddin *addin,
                                   IdeTweaks      *tweaks)
{
  GbpProjectuiTweaksAddin *self = GBP_PROJECTUI_TWEAKS_ADDIN (addin);

  self->context = NULL;
}

static void
gbp_projectui_tweaks_addin_class_init (GbpProjectuiTweaksAddinClass *klass)
{
  IdeTweaksAddinClass *tweaks_addin_class = IDE_TWEAKS_ADDIN_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = gbp_projectui_tweaks_addin_get_property;

  tweaks_addin_class->load = gbp_projectui_tweaks_addin_load;
  tweaks_addin_class->unload = gbp_projectui_tweaks_addin_unload;

  IDE_DEFINE_STRING_PROPERTY ("build-system", NULL, G_PARAM_READABLE, BUILD_SYSTEM);
  IDE_DEFINE_STRING_PROPERTY ("source-directory", NULL, G_PARAM_READABLE, SOURCE_DIRECTORY);
  IDE_DEFINE_STRING_PROPERTY ("vcs", NULL, G_PARAM_READABLE, VCS);

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
gbp_projectui_tweaks_addin_init (GbpProjectuiTweaksAddin *self)
{
}
