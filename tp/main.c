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
#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <telepathy-glib/telepathy-glib.h>

#include "connection-manager.h"
#include "debug.h"

#include "bitlbee-compat/bitlbee.h"

/* FIXME */
conf_t conf = {};
global_t global = {
  .conf = &conf
};

static TpBaseConnectionManager *
_construct_cm (void)
{
  TpBaseConnectionManager *base_cm = TP_BASE_CONNECTION_MANAGER(
        g_object_new(FB_TYPE_CONNECTION_MANAGER, NULL));

  return base_cm;
}

int
main(int argc, char **argv)
{
  TpDebugSender *debug_sender;
  int result;

  tp_debug_divert_messages(g_getenv("FACEBOOK_LOGFILE"));

  fb_debug_init();

  debug_sender = tp_debug_sender_dup();

  result = tp_run_connection_manager("telepathy-facebook", VERSION,
                                     _construct_cm, argc, argv);

  g_object_unref(debug_sender);

  return result;
}
