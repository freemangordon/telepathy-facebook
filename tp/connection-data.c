/*
 * This file is part of telepathy-facebook
 *
 * Copyright (C) 2025 Ivaylo Dimitrov <ivo.g.dimitrov.75@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library. If not, see <https://www.gnu.org/licenses/>.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "connection-data.h"

#include <json-glib/json-glib.h>

#define FB_STATE_DIR "telepathy-facebook"
#define FB_STATE_FILE "state.json"

static gchar *
get_state_file_path(void)
{
  return g_build_filename(g_get_user_data_dir(),
                          FB_STATE_DIR,
                          FB_STATE_FILE,
                          NULL
  );
}

GHashTable *
fb_connection_data_load(const gchar *fb_id)
{
  g_return_val_if_fail(fb_id != NULL, NULL);

  gchar *path = get_state_file_path();

  if (!g_file_test(path, G_FILE_TEST_EXISTS))
  {
    g_free(path);
    return g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
  }

  GError *error = NULL;
  JsonParser *parser = json_parser_new();

  if (!json_parser_load_from_file(parser, path, &error))
  {
    g_warning("Failed to load state file: %s", error->message);
    g_error_free(error);
    g_object_unref(parser);
    g_free(path);
    return NULL;
  }

  JsonNode *root = json_parser_get_root(parser);
  JsonObject *top = json_node_get_object(root);

  if (!json_object_has_member(top, fb_id))
  {
    g_object_unref(parser);
    g_free(path);
    return g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
  }

  JsonObject *entry = json_object_get_object_member(top, fb_id);
  GList *members = json_object_get_members(entry);

  GHashTable *table = g_hash_table_new_full(g_str_hash,
                                            g_str_equal,
                                            g_free,
                                            g_free);

  for (GList *l = members; l != NULL; l = l->next)
  {
    const gchar *key = l->data;
    const gchar *value = json_object_get_string_member(entry, key);
    g_hash_table_insert(table, g_strdup(key), g_strdup(value));
  }

  g_list_free(members);
  g_object_unref(parser);
  g_free(path);
  return table;
}

gboolean
fb_connection_data_save(const gchar *fb_id, GHashTable *data)
{
  g_return_val_if_fail(fb_id != NULL, FALSE);
  g_return_val_if_fail(data != NULL, FALSE);

  gchar *dir_path = g_build_filename(g_get_user_data_dir(),
                                     FB_STATE_DIR,
                                     NULL);
  g_mkdir_with_parents(dir_path, 0700);

  gchar *path = get_state_file_path();

  GError *error = NULL;
  JsonParser *parser = json_parser_new();
  JsonObject *top = json_object_new();

  if (g_file_test(path, G_FILE_TEST_EXISTS))
  {
    if (json_parser_load_from_file(parser, path, &error))
    {
      JsonNode *root = json_parser_get_root(parser);

      if (JSON_NODE_HOLDS_OBJECT(root))
      {
        JsonObject *existing = json_node_get_object(root);
        GList *keys = json_object_get_members(existing);

        for (GList *l = keys; l != NULL; l = l->next)
        {
          const gchar *key = l->data;

          if (g_strcmp0(key, fb_id) != 0)
          {
            JsonNode *copy = json_node_copy(json_object_get_member(existing,
                                                                   key));
            json_object_set_member(top, key, copy);
          }
        }

        g_list_free(keys);
      }
    }
    else
    {
      g_warning("Failed to parse state file: %s", error->message);
      g_clear_error(&error);
    }
  }

  // Build new object for this fb_id
  JsonBuilder *builder = json_builder_new();
  json_builder_begin_object(builder);

  GHashTableIter iter;
  gpointer key, value;
  g_hash_table_iter_init(&iter, data);

  while (g_hash_table_iter_next(&iter, &key, &value))
  {
    json_builder_set_member_name(builder, key);
    json_builder_add_string_value(builder, value);
  }

  json_builder_end_object(builder);
  JsonNode *entry_node = json_builder_get_root(builder);
  json_object_set_member(top, fb_id, entry_node);

  // Save full JSON object to file
  JsonGenerator *gen = json_generator_new();
  JsonNode *root_node = json_node_new(JSON_NODE_OBJECT);
  json_node_set_object(root_node, top);
  json_generator_set_root(gen, root_node);

  gboolean ok = json_generator_to_file(gen, path, &error);

  if (!ok)
  {
    g_warning("Failed to save connection data: %s", error->message);
    g_error_free(error);
  }

  // Cleanup
  json_node_free(root_node);
  g_object_unref(gen);
  g_object_unref(builder);
  g_object_unref(parser);
  g_free(path);
  g_free(dir_path);

  return ok;
}

const gchar *
fb_connection_data_get_string(GHashTable *data, const gchar *key)
{
  g_return_val_if_fail(data != NULL && key != NULL, NULL);

  return g_hash_table_lookup(data, key);
}

gint
fb_connection_data_get_int(GHashTable *data,
                                 const gchar *key,
                                 gint default_value)
{
  const gchar *str = fb_connection_data_get_string(data, key);

  if (!str)
    return default_value;

  char *endptr = NULL;
  gint val = (gint)strtol(str, &endptr, 10);

  return (endptr != str) ? val : default_value;
}

gint64
fb_connection_data_get_int64(GHashTable *data,
                                   const gchar *key,
                                   gint64 default_value)
{
  const gchar *str = fb_connection_data_get_string(data, key);

  if (!str)
    return default_value;

  char *endptr = NULL;
  gint64 val = g_ascii_strtoll(str, &endptr, 10);

  return (endptr != str) ? val : default_value;
}

guint64
fb_connection_data_get_uint64(GHashTable *data,
                                    const gchar *key,
                                    guint64 default_value)
{
  const gchar *str = fb_connection_data_get_string(data, key);

  if (!str)
    return default_value;

  char *endptr = NULL;
  guint64 val = g_ascii_strtoull(str, &endptr, 10);

  return (endptr != str) ? val : default_value;
}

gboolean
fb_connection_data_get_bool(GHashTable *data,
                                  const gchar *key,
                                  gboolean default_value)
{
  const gchar *str = fb_connection_data_get_string(data, key);

  if (!str)
    return default_value;

  return g_ascii_strcasecmp(str, "true") == 0 || g_strcmp0(str, "1") == 0;
}

void
fb_connection_data_set_string(GHashTable *data,
                                    const gchar *key,
                                    const gchar *value)
{
  g_return_if_fail(data && key && value);

  g_hash_table_insert(data, g_strdup(key), g_strdup(value));
}

void
fb_connection_data_set_int(GHashTable *data, const gchar *key, gint value)
{
  g_return_if_fail(data && key);

  gchar buf[G_ASCII_DTOSTR_BUF_SIZE];

  g_snprintf(buf, sizeof(buf), "%d", value);
  fb_connection_data_set_string(data, key, buf);
}

void
fb_connection_data_set_int64(GHashTable *data,
                                   const gchar *key,
                                   gint64 value)
{
  g_return_if_fail(data && key);

  gchar *str = g_strdup_printf("%" G_GINT64_FORMAT, value);

  fb_connection_data_set_string(data, key, str);
  g_free(str);
}

void
fb_connection_data_set_uint64(GHashTable *data,
                                    const gchar *key,
                                    guint64 value)
{
  g_return_if_fail(data && key);

  gchar *str = g_strdup_printf("%" G_GUINT64_FORMAT, value);

  fb_connection_data_set_string(data, key, str);
  g_free(str);
}

void
fb_connection_data_set_bool(GHashTable *data,
                                  const gchar *key,
                                  gboolean value)
{
  fb_connection_data_set_string(data, key, value ? "true" : "false");
}
