/* gbp-sysprof-tool.c
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

#define G_LOG_DOMAIN "gbp-sysprof-tool"

#include "config.h"

#include <unistd.h>

#include <libide-gui.h>

#include "gbp-sysprof-page.h"
#include "gbp-sysprof-tool.h"

struct _GbpSysprofTool
{
  IdeRunTool parent_instance;
  char *capture_file;
};

G_DEFINE_FINAL_TYPE (GbpSysprofTool, gbp_sysprof_tool, IDE_TYPE_RUN_TOOL)

static const char *
gbp_sysprof_tool_get_capture_file (GbpSysprofTool *self)
{
  g_assert (GBP_IS_SYSPROF_TOOL (self));

  if (self->capture_file == NULL)
    {
      char tmpl[] = "gnome-builder-sysprof-XXXXXX.syscap";
      int fd = g_mkstemp (tmpl);

      self->capture_file = g_strdup (tmpl);

      if (fd != -1)
        close (fd);
    }

  return self->capture_file;
}

static gboolean
gbp_sysprof_tool_handler (IdeRunContext       *run_context,
                          const char * const  *argv,
                          const char * const  *env,
                          const char          *cwd,
                          IdeUnixFDMap        *unix_fd_map,
                          gpointer             user_data,
                          GError             **error)
{
  GbpSysprofTool *self = user_data;
  g_autoptr(GSettings) settings = NULL;
  const char *capture_file;

  g_assert (IDE_IS_RUN_CONTEXT (run_context));
  g_assert (argv != NULL);
  g_assert (env != NULL);
  g_assert (IDE_IS_UNIX_FD_MAP (unix_fd_map));
  g_assert (GBP_IS_SYSPROF_TOOL (self));

  capture_file = gbp_sysprof_tool_get_capture_file (self);
  settings = g_settings_new ("org.gnome.builder.sysprof");

  ide_run_context_append_argv (run_context, PACKAGE_LIBEXECDIR"/gnome-builder-sysprof");
  ide_run_context_append_formatted (run_context, "--capture=%s", capture_file);

  /* TODO: Setup --read-fd/--write-fd/--forward-fd/--pty */

  if (g_settings_get_boolean (settings, "cpu-aid"))
    ide_run_context_append_argv (run_context, "--cpu");

  if (g_settings_get_boolean (settings, "perf-aid"))
    ide_run_context_append_argv (run_context, "--perf");

  if (g_settings_get_boolean (settings, "memory-aid"))
    ide_run_context_append_argv (run_context, "--memory");

  if (g_settings_get_boolean (settings, "memprof-aid"))
    ide_run_context_append_argv (run_context, "--memprof");

  if (g_settings_get_boolean (settings, "diskstat-aid"))
    ide_run_context_append_argv (run_context, "--disk");

  if (g_settings_get_boolean (settings, "netstat-aid"))
    ide_run_context_append_argv (run_context, "--net");

  if (g_settings_get_boolean (settings, "energy-aid"))
    ide_run_context_append_argv (run_context, "--energy");

  if (g_settings_get_boolean (settings, "battery-aid"))
    ide_run_context_append_argv (run_context, "--battery");

  if (g_settings_get_boolean (settings, "compositor-aid"))
    ide_run_context_append_argv (run_context, "--compositor");

  if (g_settings_get_boolean (settings, "gjs-aid"))
    ide_run_context_append_argv (run_context, "--gjs");

  if (!g_settings_get_boolean (settings, "allow-throttle"))
    ide_run_context_append_argv (run_context, "--no-throttle");

  if (g_settings_get_boolean (settings, "allow-tracefd"))
    ide_run_context_append_argv (run_context, "--tracefd");

  for (guint i = 0; env[i]; i++)
    ide_run_context_append_formatted (run_context, "--env=%s", env[i]);

  ide_run_context_append_argv (run_context, "--");
  ide_run_context_append_args (run_context, argv);

  return TRUE;
}

static void
gbp_sysprof_tool_prepare_to_run (IdeRunTool    *run_tool,
                                 IdePipeline   *pipeline,
                                 IdeRunCommand *run_command,
                                 IdeRunContext *run_context)
{
  IDE_ENTRY;

  g_assert (IDE_IS_MAIN_THREAD ());
  g_assert (GBP_IS_SYSPROF_TOOL (run_tool));
  g_assert (IDE_IS_PIPELINE (pipeline));
  g_assert (IDE_IS_RUN_COMMAND (run_command));
  g_assert (IDE_IS_RUN_CONTEXT (run_context));

  ide_run_context_push_at_base (run_context,
                                gbp_sysprof_tool_handler,
                                g_object_ref (run_tool),
                                g_object_unref);

  IDE_EXIT;
}

static void
gbp_sysprof_tool_send_signal (IdeRunTool *run_tool,
                              int         signum)
{
  IDE_ENTRY;

  g_assert (IDE_IS_MAIN_THREAD ());
  g_assert (GBP_IS_SYSPROF_TOOL (run_tool));

  /* TODO: Proxy to worker */

  IDE_EXIT;
}

static void
gbp_sysprof_tool_started (IdeRunTool    *run_tool,
                          IdeSubprocess *subprocess)
{
  IDE_ENTRY;

  g_assert (IDE_IS_MAIN_THREAD ());
  g_assert (GBP_IS_SYSPROF_TOOL (run_tool));
  g_assert (IDE_IS_SUBPROCESS (subprocess));

  /* TODO: Setup connection using GIOStream to subprocess */

  IDE_EXIT;
}

static void
gbp_sysprof_tool_stopped (IdeRunTool *run_tool)
{
  GbpSysprofTool *self = (GbpSysprofTool *)run_tool;

  IDE_ENTRY;

  g_assert (IDE_IS_MAIN_THREAD ());
  g_assert (GBP_IS_SYSPROF_TOOL (run_tool));

  if (self->capture_file != NULL)
    {
      g_autoptr(GFile) file = g_file_new_for_path (self->capture_file);
      GbpSysprofPage *page = gbp_sysprof_page_new_for_file (file);
      IdeContext *context = ide_object_get_context (IDE_OBJECT (self));
      IdeWorkbench *workbench = ide_workbench_from_context (context);
      IdeWorkspace *workspace = ide_workbench_get_current_workspace (workbench);
      g_autoptr(IdePanelPosition) position = ide_panel_position_new ();

      ide_workspace_add_page (workspace, IDE_PAGE (page), position);
    }

  IDE_EXIT;
}

static void
gbp_sysprof_tool_class_init (GbpSysprofToolClass *klass)
{
  IdeRunToolClass *run_tool_class = IDE_RUN_TOOL_CLASS (klass);

  run_tool_class->prepare_to_run = gbp_sysprof_tool_prepare_to_run;
  run_tool_class->send_signal = gbp_sysprof_tool_send_signal;
  run_tool_class->started = gbp_sysprof_tool_started;
  run_tool_class->stopped = gbp_sysprof_tool_stopped;
}

static void
gbp_sysprof_tool_init (GbpSysprofTool *self)
{
  ide_run_tool_set_icon_name (IDE_RUN_TOOL (self), "builder-profiler-symbolic");
}
