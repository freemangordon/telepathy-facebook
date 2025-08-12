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

#include <libsoup/soup.h>

#include "facebook-http.h"
#include "misc.h"

static gboolean
fb_http_value_equal(gconstpointer a, gconstpointer b)
{
    return g_ascii_strcasecmp(a, b) == 0;
}

FbHttpValues *
fb_http_values_new(void)
{
        return g_hash_table_new_full(g_str_hash, fb_http_value_equal,
                                     g_free, g_free);
}

void
fb_http_values_consume(FbHttpValues *values, FbHttpValues *consume)
{
    GHashTableIter iter;
    gpointer key;
    gpointer val;

    g_hash_table_iter_init(&iter, consume);

    while (g_hash_table_iter_next(&iter, &key, &val))
    {
        g_hash_table_iter_steal(&iter);
        g_hash_table_replace(values, key, val);
    }

    g_hash_table_destroy(consume);
}

void
fb_http_values_parse(FbHttpValues *values, const gchar *data, gboolean isurl)
{
  SoupURI *uri = NULL;
  gchar *query;

  if (isurl)
  {
    uri = soup_uri_new(data);

    g_return_if_fail(uri != NULL);

    query = uri->query;
  }

  if (query)
  {
    GHashTable *params = soup_form_decode(query);
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, params);

    while (g_hash_table_iter_next(&iter, &key, &value))
      fb_http_values_set_str(values, key, value);

    g_hash_table_destroy(params);

    if (uri)
      soup_uri_free(uri);
  }
}

void
fb_http_values_free(FbHttpValues *values)
{
    g_hash_table_destroy(values);
}

gboolean
fb_http_values_remove(FbHttpValues *values, const gchar *name)
{
    return g_hash_table_remove(values, name);
}

GList *
fb_http_values_get_keys(FbHttpValues *values)
{
    return g_hash_table_get_keys(values);
}

static const gchar *
fb_http_values_get(FbHttpValues *values, const gchar *name, GError **error)
{
    const gchar *ret;

    ret = g_hash_table_lookup(values, name);

    if (ret == NULL) {
        g_set_error(error, FB_HTTP_ERROR, FB_HTTP_ERROR_NOMATCH,
                    "No matches for %s", name);
        return NULL;
    }

    return ret;
}

gboolean
fb_http_values_get_bool(FbHttpValues *values, const gchar *name,
                       GError **error)
{
    const gchar *val;

    val = fb_http_values_get(values, name, error);

    if (val == NULL) {
        return FALSE;
    }

    return bool2int((gchar *) name);
}

gdouble
fb_http_values_get_dbl(FbHttpValues *values, const gchar *name,
                      GError **error)
{
    const gchar *val;

    val = fb_http_values_get(values, name, error);

    if (val == NULL) {
        return 0.0;
    }

    return g_ascii_strtod(val, NULL);
}

gint64
fb_http_values_get_int(FbHttpValues *values, const gchar *name,
                       GError **error)
{
    const gchar *val;

    val = fb_http_values_get(values, name, error);

    if (val == NULL) {
        return 0;
    }

    return g_ascii_strtoll(val, NULL, 10);
}


const gchar *
fb_http_values_get_str(FbHttpValues *values, const gchar *name,
                       GError **error)
{
    return fb_http_values_get(values, name, error);
}

gchar *
fb_http_values_dup_str(FbHttpValues *values, const gchar *name,
                       GError **error)
{
    const gchar *str;

    str = fb_http_values_get_str(values, name, error);
    return g_strdup(str);
}

static void
fb_http_values_set(FbHttpValues *values, const gchar *name, gchar *value)
{
    gchar *key;

    key = g_strdup(name);
    g_hash_table_replace(values, key, value);
}

void
fb_http_values_set_bool(FbHttpValues *values, const gchar *name,
                        gboolean value)
{
    gchar *val;

    val = g_strdup(value ? "true" : "false");
    fb_http_values_set(values, name, val);
}

void
fb_http_values_set_dbl(FbHttpValues *values, const gchar *name, gdouble value)
{
    gchar *val;

    val = g_strdup_printf("%f", value);
    fb_http_values_set(values, name, val);
}

void
fb_http_values_set_int(FbHttpValues *values, const gchar *name, gint64 value)
{
    gchar *val;

    val = g_strdup_printf("%" G_GINT64_FORMAT, value);
    fb_http_values_set(values, name, val);
}

void
fb_http_values_set_str(FbHttpValues *values, const gchar *name,
                       const gchar *value)
{
    gchar *val;

    val = g_strdup(value);
    fb_http_values_set(values, name, val);
}

void
fb_http_values_set_strf(FbHttpValues *values, const gchar *name,
                        const gchar *format, ...)
{
    gchar *val;
    va_list ap;

    va_start(ap, format);
    val = g_strdup_vprintf(format, ap);
    va_end(ap);

    fb_http_values_set(values, name, val);
}
