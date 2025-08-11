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
#ifndef FB_PROTOCOL_H
#define FB_PROTOCOL_H

#include <glib-object.h>
#include <telepathy-glib/telepathy-glib.h>

G_BEGIN_DECLS

typedef struct _FbProtocol FbProtocol;
typedef struct _FbProtocolPrivate FbProtocolPrivate;
typedef struct _FbProtocolClass FbProtocolClass;
typedef struct _FbProtocolClassPrivate FbProtocolClassPrivate;

struct _FbProtocolClass
{
  TpBaseProtocolClass parent_class;

  FbProtocolClassPrivate *priv;
};

struct _FbProtocol
{
  TpBaseProtocol parent;

  FbProtocolPrivate *priv;
};

GType
fb_protocol_get_type (void);

#define FB_TYPE_PROTOCOL (fb_protocol_get_type())
#define FB_PROTOCOL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), \
                              FB_TYPE_PROTOCOL, \
                              FbProtocol))
#define FB_PROTOCOL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), \
                           FB_TYPE_PROTOCOL, \
                           FbProtocolClass))
#define FB_IS_PROTOCOL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), \
                           FB_TYPE_PROTOCOL))
#define FB_PROTOCOL_GET_CLASS(klass) \
  (G_TYPE_INSTANCE_GET_CLASS((obj), \
                             FB_TYPE_PROTOCOL, \
                             FbProtocolClass))

TpBaseProtocol *
fb_protocol_new (void);

G_END_DECLS

#endif
