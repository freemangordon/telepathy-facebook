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

#include "debug.h"

#include <stdarg.h>
#include <telepathy-glib/telepathy-glib.h>

static FbDebugFlags _flags = 0;

static GDebugKey _keys[] = {
  {"connection", FB_DEBUG_CONNECTION},
  {"account-verify", FB_DEBUG_ACCOUNT_VERIFY},
  {"avatar", FB_DEBUG_AVATAR},
  {NULL, 0}
};

void
fb_debug_init (void) {
  const gchar *flags_string = g_getenv("FACEBOOK_DEBUG");
  guint nkeys;

  for (nkeys = 0; _keys[nkeys].value; nkeys++)
  {
    /* do nothing, just count nkeys */
  }

  if (flags_string)
  {
    tp_debug_set_flags(flags_string);
    _flags |= g_parse_debug_string(flags_string, _keys, nkeys);
  }

  if (g_getenv("FACEBOOK_PERSIST") != NULL)
    tp_debug_set_persistent(TRUE);
}

GHashTable *flag_to_domains = NULL;

static const gchar *
debug_flag_to_domain (FbDebugFlags flag)
{
  if (G_UNLIKELY (flag_to_domains == NULL)) {
    guint i;

    flag_to_domains = g_hash_table_new_full(NULL, NULL, NULL, g_free);

    for (i = 0; _keys[i].value; i++)
    {
      GDebugKey key = _keys[i];
      gchar *val;

      val = g_strdup_printf ("%s/%s", "facebook", key.key);
      g_hash_table_insert (flag_to_domains,
                           GUINT_TO_POINTER (key.value), val);
    }
  }

  return g_hash_table_lookup (flag_to_domains, GUINT_TO_POINTER (flag));
}

void
fb_debug_free (void)
{
  if (flag_to_domains == NULL)
    return;

  g_hash_table_destroy (flag_to_domains);
  flag_to_domains = NULL;
}

static void
log_to_debug_sender (FbDebugFlags flag, const gchar *message)
{
  TpDebugSender *dbg;
  GTimeVal now;

  dbg = tp_debug_sender_dup ();

  g_get_current_time (&now);

  tp_debug_sender_add_message(dbg, &now, debug_flag_to_domain (flag),
                              G_LOG_LEVEL_DEBUG, message);

  g_object_unref (dbg);
}

void
fb_debug(FbDebugFlags flag, const gchar *format, ...)
{
  gchar *message;
  va_list args;

  va_start (args, format);
  message = g_strdup_vprintf (format, args);
  va_end (args);

  log_to_debug_sender (flag, message);

  if (_flags & flag)
    g_log(G_LOG_DOMAIN, G_LOG_LEVEL_DEBUG, "%s", message);

  g_free (message);
}
