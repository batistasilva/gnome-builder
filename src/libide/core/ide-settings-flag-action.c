/* ide-settings-flag-action.c
 *
 * Copyright (C) 2015 Christian Hergert <chergert@redhat.com>
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
 */

#define G_LOG_DOMAIN "ide-settings-flag-action"

#include "config.h"

#include "ide-settings-flag-action.h"
#include "ide-macros.h"

struct _IdeSettingsFlagAction
{
  GObject parent_instance;

  GSettings *settings;

  char *schema_id;
  char *schema_key;
  char *flag_nick;
  char *name;
};

static void action_iface_init (GActionInterface *iface);

G_DEFINE_TYPE_EXTENDED (IdeSettingsFlagAction, ide_settings_flag_action, G_TYPE_OBJECT, 0,
                        G_IMPLEMENT_INTERFACE (G_TYPE_ACTION, action_iface_init))

enum {
  PROP_0,
  PROP_SCHEMA_ID,
  PROP_SCHEMA_KEY,
  PROP_FLAG_NICK,
  LAST_PROP,

  PROP_ENABLED,
  PROP_NAME,
  PROP_STATE,
  PROP_STATE_TYPE,
  PROP_PARAMETER_TYPE,
};

static GParamSpec *properties [LAST_PROP];

/**
 * ide_settings_flag_action_new:
 *
 * This creates a new action that can be used to toggle an individual flag in
 * a #GSettings key which is of a flags type.
 *
 * Returns: (transfer full): A new #GAction.
 */
GAction *
ide_settings_flag_action_new (const char *schema_id,
                              const char *schema_key,
                              const char *flag_nick)
{
  return g_object_new (IDE_TYPE_SETTINGS_FLAG_ACTION,
                       "schema-id", schema_id,
                       "schema-key", schema_key,
                       "flag-nick", flag_nick,
                       NULL);
}

static void
ide_settings_flag_action_finalize (GObject *object)
{
  IdeSettingsFlagAction *self = (IdeSettingsFlagAction *)object;

  g_clear_pointer (&self->schema_id, g_free);
  g_clear_pointer (&self->schema_key, g_free);
  g_clear_pointer (&self->flag_nick, g_free);

  G_OBJECT_CLASS (ide_settings_flag_action_parent_class)->finalize (object);
}

static void
ide_settings_flag_action_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  IdeSettingsFlagAction *self = IDE_SETTINGS_FLAG_ACTION (object);

  switch (prop_id)
    {
    case PROP_ENABLED:
      g_value_set_boolean (value, self->schema_id != NULL);
      break;

    case PROP_SCHEMA_ID:
      g_value_set_string (value, self->schema_id);
      break;

    case PROP_NAME:
      g_value_set_string (value, g_action_get_name (G_ACTION (self)));
      break;

    case PROP_SCHEMA_KEY:
      g_value_set_string (value, self->schema_key);
      break;

    case PROP_FLAG_NICK:
      g_value_set_string (value, self->flag_nick);
      break;

    case PROP_STATE:
    case PROP_STATE_TYPE:
    case PROP_PARAMETER_TYPE:
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ide_settings_flag_action_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  IdeSettingsFlagAction *self = IDE_SETTINGS_FLAG_ACTION (object);

  switch (prop_id)
    {
    case PROP_SCHEMA_ID:
      g_free (self->schema_id);
      self->schema_id = g_value_dup_string (value);
      break;

    case PROP_SCHEMA_KEY:
      g_free (self->schema_key);
      self->schema_key = g_value_dup_string (value);
      break;

    case PROP_FLAG_NICK:
      g_free (self->flag_nick);
      self->flag_nick = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
ide_settings_flag_action_class_init (IdeSettingsFlagActionClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = ide_settings_flag_action_finalize;
  object_class->get_property = ide_settings_flag_action_get_property;
  object_class->set_property = ide_settings_flag_action_set_property;

  g_object_class_override_property (object_class, PROP_NAME, "name");
  g_object_class_override_property (object_class, PROP_STATE, "state");
  g_object_class_override_property (object_class, PROP_STATE_TYPE, "state-type");
  g_object_class_override_property (object_class, PROP_PARAMETER_TYPE, "parameter-type");
  g_object_class_override_property (object_class, PROP_ENABLED, "enabled");

  properties [PROP_SCHEMA_ID] =
    g_param_spec_string ("schema-id",
                         "Schema Id",
                         "Schema Id",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  properties [PROP_SCHEMA_KEY] =
    g_param_spec_string ("schema-key",
                         "Schema Key",
                         "Schema Key",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  properties [PROP_FLAG_NICK] =
    g_param_spec_string ("flag-nick",
                         "Flag Nick",
                         "Flag Nick",
                         NULL,
                         (G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, LAST_PROP, properties);
}

static void
ide_settings_flag_action_init (IdeSettingsFlagAction *self)
{
}

static GSettings *
ide_settings_flag_action_get_settings (IdeSettingsFlagAction *self)
{
  g_assert (IDE_IS_SETTINGS_FLAG_ACTION (self));

  if (self->settings == NULL)
    self->settings = g_settings_new (self->schema_id);

  return self->settings;
}

static const char *
ide_settings_flag_action_get_name (GAction *action)
{
  IdeSettingsFlagAction *self = (IdeSettingsFlagAction *)action;

  if (self->name == NULL)
    self->name = g_strdup_printf ("%s-%s", self->schema_key, self->flag_nick);

  return self->name;
}

static const GVariantType *
ide_settings_flag_action_get_parameter_type (GAction *action)
{
  return NULL;
}

static const GVariantType *
ide_settings_flag_action_get_state_type (GAction *action)
{
  return G_VARIANT_TYPE_BOOLEAN;
}

static GVariant *
ide_settings_flag_action_get_state_hint (GAction *action)
{
  return NULL;
}

static void
ide_settings_flag_action_change_state (GAction  *action,
                                       GVariant *value)
{
}

static GVariant *
ide_settings_flag_action_get_state (GAction *action)
{
  IdeSettingsFlagAction *self = (IdeSettingsFlagAction *)action;
  GSettings *settings = ide_settings_flag_action_get_settings (self);
  g_auto(GStrv) flags = g_settings_get_strv (settings, self->schema_key);
  gboolean state = g_strv_contains ((const char * const *)flags, self->flag_nick);
  return g_variant_new_boolean (state);
}

static gboolean
ide_settings_flag_action_get_enabled (GAction *action)
{
  IdeSettingsFlagAction *self = (IdeSettingsFlagAction *)action;

  return self->schema_id && self->schema_key && self->flag_nick;
}

static void
ide_settings_flag_action_activate (GAction  *action,
                                   GVariant *parameter)
{
  IdeSettingsFlagAction *self = (IdeSettingsFlagAction *)action;
  GSettings *settings;
  GPtrArray *ar;
  gboolean found = FALSE;
  char **strv;
  guint i;

  g_assert (IDE_IS_SETTINGS_FLAG_ACTION (action));
  g_assert (parameter == NULL);

  settings = ide_settings_flag_action_get_settings (self);
  strv = g_settings_get_strv (settings, self->schema_key);
  ar = g_ptr_array_new ();

  for (i = 0; strv [i]; i++)
    {
      if (g_strcmp0 (strv [i], self->flag_nick) == 0)
        found = TRUE;
      else
        g_ptr_array_add (ar, strv [i]);
    }

  if (!found)
    g_ptr_array_add (ar, self->flag_nick);

  g_ptr_array_add (ar, NULL);

  g_settings_set_strv (settings, self->schema_key, (const char * const *)ar->pdata);

  g_strfreev (strv);
}

static void
action_iface_init (GActionInterface *iface)
{
  iface->activate = ide_settings_flag_action_activate;
  iface->change_state = ide_settings_flag_action_change_state;
  iface->get_enabled = ide_settings_flag_action_get_enabled;
  iface->get_name = ide_settings_flag_action_get_name;
  iface->get_parameter_type = ide_settings_flag_action_get_parameter_type;
  iface->get_state = ide_settings_flag_action_get_state;
  iface->get_state_hint = ide_settings_flag_action_get_state_hint;
  iface->get_state_type = ide_settings_flag_action_get_state_type;
}
