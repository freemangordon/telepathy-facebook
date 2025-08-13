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

#define FB_DEBUG_FLAG FB_DEBUG_CONNECTION

#include <telepathy-glib/telepathy-glib.h>

#include "contact-list.h"
#include "debug.h"

struct _FbContactListClass
{
  TpBaseContactListClass parent_class;
};

struct _FbContactList
{
  TpBaseContactList parent;
};

struct _FbContactListPrivate
{
  TpBaseConnection *conn;
  TpHandleRepoIface *contact_repo;
  TpHandleSet *contacts;
  /* handle -> FbContact */
  GHashTable *fb_contacts;
  FbContact me;
};

typedef struct _FbContactListPrivate FbContactListPrivate;

G_DEFINE_TYPE_WITH_CODE(
  FbContactList,
  fb_contact_list,
  TP_TYPE_BASE_CONTACT_LIST,
  G_ADD_PRIVATE(FbContactList)
)

#define PRIVATE(o) \
  ((FbContactListPrivate *) \
   fb_contact_list_get_instance_private((FbContactList *)o));

static void
fb_contact_list_constructed(GObject *object)
{
  FbContactListPrivate *priv = PRIVATE(object);

  void (*constructed) (GObject *) =
    (G_OBJECT_CLASS(fb_contact_list_parent_class))->constructed;

  if (constructed != NULL)
    constructed(object);

  g_object_get(object, "connection", &priv->conn, NULL);
  g_assert(priv->conn != NULL);

  priv->contact_repo = tp_base_connection_get_handles(priv->conn,
                                                      TP_HANDLE_TYPE_CONTACT);
  priv->contacts = tp_handle_set_new(priv->contact_repo);
}

static void
fb_contact_clear(FbContact *c)
{
  tp_clear_pointer(&c->name, g_free);
  tp_clear_pointer(&c->icon, g_free);
  tp_clear_pointer(&c->avatar_token, g_free);
}

static void
fb_contact_destroy(gpointer p)
{
  fb_contact_clear(p);
  g_slice_free(FbContact, p);
}

static void
fb_contact_list_dispose(GObject *object)
{
  FbContactListPrivate *priv = PRIVATE(object);

  tp_clear_pointer(&priv->fb_contacts, g_hash_table_destroy);
  tp_clear_pointer(&priv->contacts, tp_handle_set_destroy);
  fb_contact_clear(&priv->me);

  G_OBJECT_CLASS(fb_contact_list_parent_class)->dispose(object);
}

static TpHandleSet *
fb_contact_list_dup_contacts(TpBaseContactList *self)
{
  FbContactListPrivate *priv = PRIVATE(self);

  return tp_handle_set_copy(priv->contacts);
}

static TpSubscriptionState
map_friendship_to_publish(FbApiFriendshipStatus fs, gchar **publish_request)
{
  switch (fs)
  {
    case FB_API_FRIENDSHIP_STATUS_ARE_FRIENDS:
    {
      return TP_SUBSCRIPTION_STATE_YES;
    }
    case FB_API_FRIENDSHIP_STATUS_INCOMING_REQUEST:
    {
      if (publish_request)
        *publish_request = g_strdup("Incoming friend request");

      return TP_SUBSCRIPTION_STATE_ASK;
    }
    case FB_API_FRIENDSHIP_STATUS_OUTGOING_REQUEST:
    case FB_API_FRIENDSHIP_STATUS_CAN_REQUEST:
    case FB_API_FRIENDSHIP_STATUS_CAN_RECONFIRM:
    case FB_API_FRIENDSHIP_STATUS_CANNOT_REQUEST:
    case FB_API_FRIENDSHIP_STATUS_NOT_FRIENDS:
    case FB_API_FRIENDSHIP_STATUS_BLOCKED:
    case FB_API_FRIENDSHIP_STATUS_DEACTIVATED:
    {
      return TP_SUBSCRIPTION_STATE_NO;
    }
    case FB_API_FRIENDSHIP_STATUS_UNKNOWN:
    default:
      return TP_SUBSCRIPTION_STATE_UNKNOWN;
  }
}

static void
fb_contact_list_dup_states(TpBaseContactList *self,
                           TpHandle contact,
                           TpSubscriptionState *subscribe,
                           TpSubscriptionState *publish,
                           gchar **publish_request)
{
  FbContact *c = fb_contact_list_get_user(FB_CONTACT_LIST(self), contact);

  if (!c)
  {
    if (subscribe)
      *subscribe = TP_SUBSCRIPTION_STATE_UNKNOWN;

    if (publish)
      *publish = TP_SUBSCRIPTION_STATE_UNKNOWN;

    if (publish_request)
      *publish_request = NULL;
  }
  else
  {
    if (subscribe)
    {
      if (c->fs == FB_API_FRIENDSHIP_STATUS_ARE_FRIENDS)
        *subscribe = TP_SUBSCRIPTION_STATE_YES ;
      else
        *subscribe = TP_SUBSCRIPTION_STATE_NO;
    }

    if (publish)
      *publish = map_friendship_to_publish(c->fs, publish_request);
  }
}

static void
fb_contact_list_init(FbContactList *self)
{
  FbContactListPrivate *priv = PRIVATE(self);

  priv->fb_contacts = g_hash_table_new_full(
    g_direct_hash, g_direct_equal, NULL, fb_contact_destroy);
}

static void
fb_contact_list_class_init(FbContactListClass *klass)
{
  TpBaseContactListClass *contact_list_class =
    TP_BASE_CONTACT_LIST_CLASS(klass);
  GObjectClass *object_class = G_OBJECT_CLASS(klass);

  object_class->constructed = fb_contact_list_constructed;
  object_class->dispose = fb_contact_list_dispose;

  contact_list_class->dup_contacts = fb_contact_list_dup_contacts;
  contact_list_class->dup_states = fb_contact_list_dup_states;
}

FbContactList *
fb_contact_list_new(TpBaseConnection *connection)
{
  return g_object_new(FB_TYPE_CONTACT_LIST,
                      "connection", connection,
                      NULL);
}

static gchar *
get_avatar_token(FbApiUser *user, GRegex *regex)
{
  g_autoptr(GMatchInfo) match_info;

  if (g_regex_match(regex, user->icon, 0, &match_info))
    return g_match_info_fetch(match_info, 1);
  else
    return g_strdup(user->csum);
}

void
fb_contact_list_fb_contacts_changed(FbContactList *self,
                                    FbApi *api, GSList *users)
{
  FbContactListPrivate *priv = PRIVATE(self);
  TpHandleSet *contacts = tp_handle_set_new(priv->contact_repo);
  FbId my_uid;
  GSList *l;
  GRegex *regex = g_regex_new("/\\d+_(\\d+)_\\d+_[a-z]\\.jpg", 0, 0, NULL);

  g_object_get(G_OBJECT(api), "uid", &my_uid, NULL);

  for (l = users; l != NULL; l = l->next)
  {
    FbApiUser *user = l->data;
    gchar uid[FB_ID_STRMAX];
    FbContact *c;
    gchar *avatar_token;
    TpHandle handle;

    FB_ID_TO_STR(user->uid, uid);

    FB_DEBUG("contact %s changed name %s icon %s",
             uid, user->name, user->icon);

    if (G_UNLIKELY(user->uid == my_uid))
    {
      handle = tp_base_connection_get_self_handle(priv->conn);
      c = &priv->me;
    }
    else
    {
      handle = tp_handle_ensure(priv->contact_repo, uid, NULL, NULL);
      c = g_hash_table_lookup(priv->fb_contacts, GUINT_TO_POINTER(handle));

      if (!c)
      {
        c = g_new0(FbContact, 1);
        g_hash_table_insert(priv->fb_contacts, GUINT_TO_POINTER(handle), c);
        tp_handle_set_add(priv->contacts, handle);
      }

      tp_handle_set_add(contacts, handle);
    }

    avatar_token = get_avatar_token(user, regex);

    if (c->avatar_token && g_strcmp0(avatar_token, c->avatar_token))
    {
      tp_svc_connection_interface_avatars_emit_avatar_updated(
        priv->conn, handle, avatar_token);
    }

    fb_contact_clear(c);

    c->name = g_strdup(user->name);
    c->icon = g_strdup(user->icon);
    c->avatar_token = avatar_token;
    c->fs = user->fs;
  }

  g_regex_unref(regex);

  if (!tp_handle_set_is_empty(contacts))
  {
    tp_base_contact_list_contacts_changed(
      TP_BASE_CONTACT_LIST(self), contacts, NULL);
  }

  tp_handle_set_destroy(contacts);
}

FbContact *
fb_contact_list_get_user(FbContactList *self, TpHandle handle)
{
  FbContactListPrivate *priv = PRIVATE(self);

  if (G_UNLIKELY(handle == tp_base_connection_get_self_handle(priv->conn)))
    return &priv->me;

  return g_hash_table_lookup(priv->fb_contacts, GUINT_TO_POINTER(handle));
}

TpHandle
fb_contact_list_ensure_handle(FbContactList *self, FbId fb_uid)
{
  FbContactListPrivate *priv = PRIVATE(self);
  gchar uid[FB_ID_STRMAX];

  FB_ID_TO_STR(fb_uid, uid);

  return tp_handle_ensure(priv->contact_repo, uid, NULL, NULL);
}
