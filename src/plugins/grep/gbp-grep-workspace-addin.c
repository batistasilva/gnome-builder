/* gbp-grep-workspace-addin.c
 *
 * Copyright 2018-2019 Christian Hergert <chergert@redhat.com>
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

#define G_LOG_DOMAIN "gbp-grep-workspace-addin"

#include "config.h"

#include <glib/gi18n.h>

#include <libide-foundry.h>
#include <libide-gui.h>

#include "gbp-grep-workspace-addin.h"
#include "gbp-grep-panel.h"

#define I_(s) g_intern_static_string(s)

struct _GbpGrepWorkspaceAddin
{
  GObject  parent_instance;
  IdePane *panel;
};

static void
panel_show_action (GbpGrepWorkspaceAddin *self,
                   GVariant              *variant)
{
  g_assert (GBP_IS_GREP_WORKSPACE_ADDIN (self));

  panel_widget_raise (PANEL_WIDGET (self->panel));
  gtk_widget_grab_focus (GTK_WIDGET (self->panel));
}

IDE_DEFINE_ACTION_GROUP (GbpGrepWorkspaceAddin, gbp_grep_workspace_addin, {
  { "panel.show", panel_show_action },
});

static void
gbp_grep_workspace_addin_load (IdeWorkspaceAddin *addin,
                               IdeWorkspace      *workspace)
{
  GbpGrepWorkspaceAddin *self = (GbpGrepWorkspaceAddin *)addin;
  g_autoptr(PanelPosition) position = NULL;

  g_assert (GBP_IS_GREP_WORKSPACE_ADDIN (self));
  g_assert (IDE_IS_WORKSPACE (workspace));

  self->panel = g_object_new (GBP_TYPE_GREP_PANEL, NULL);

  position = panel_position_new ();
  panel_position_set_area (position, PANEL_AREA_BOTTOM);
  ide_workspace_add_pane (workspace, self->panel, position);
}

static void
gbp_grep_workspace_addin_unload (IdeWorkspaceAddin *addin,
                                 IdeWorkspace      *workspace)
{
  GbpGrepWorkspaceAddin *self = (GbpGrepWorkspaceAddin *)addin;

  g_assert (IDE_IS_MAIN_THREAD ());
  g_assert (GBP_IS_GREP_WORKSPACE_ADDIN (self));

  g_clear_pointer (&self->panel, ide_pane_destroy);
}

static void
workspace_addin_iface_init (IdeWorkspaceAddinInterface *iface)
{
  iface->load = gbp_grep_workspace_addin_load;
  iface->unload = gbp_grep_workspace_addin_unload;
}

G_DEFINE_TYPE_WITH_CODE (GbpGrepWorkspaceAddin, gbp_grep_workspace_addin, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_ACTION_GROUP, gbp_grep_workspace_addin_init_action_group)
                         G_IMPLEMENT_INTERFACE (IDE_TYPE_WORKSPACE_ADDIN, workspace_addin_iface_init))

static void
gbp_grep_workspace_addin_class_init (GbpGrepWorkspaceAddinClass *klass)
{
}

static void
gbp_grep_workspace_addin_init (GbpGrepWorkspaceAddin *self)
{
}
