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

#include "connection-manager.h"

#include <dbus/dbus-protocol.h>

//#include "facebook-connection.h"
//#include "facebook-handles.h" /* to check for valid nick */
#include "debug.h"
#include "protocol.h"

G_DEFINE_TYPE(FbConnectionManager,
              fb_connection_manager,
              TP_TYPE_BASE_CONNECTION_MANAGER)

static void
fb_connection_manager_init(FbConnectionManager *obj G_GNUC_UNUSED)
{}

static void
fb_connection_manager_finalize(GObject *object)
{
  fb_debug_free();

  G_OBJECT_CLASS(fb_connection_manager_parent_class)->finalize(object);
}

static void
fb_connection_manager_constructed(GObject *object)
{
  TpBaseConnectionManager *base = (TpBaseConnectionManager *)object;
  TpBaseProtocol *p;

  void (*constructed)(GObject *) =
    ((GObjectClass *)fb_connection_manager_parent_class)->constructed;

  if (constructed != NULL)
    constructed(object);

  p = fb_protocol_new();
  tp_base_connection_manager_add_protocol(base, p);
  g_object_unref(p);
}

static void
fb_connection_manager_class_init(FbConnectionManagerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS(klass);
  TpBaseConnectionManagerClass *parent_class =
    TP_BASE_CONNECTION_MANAGER_CLASS(klass);

  parent_class->cm_dbus_name = "facebook";

  object_class->finalize = fb_connection_manager_finalize;
  object_class->constructed = fb_connection_manager_constructed;
}
