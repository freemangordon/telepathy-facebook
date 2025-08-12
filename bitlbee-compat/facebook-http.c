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
#include "facebook-util.h"

struct _FbHttpPrivate
{
  SoupSession *session;
};

struct _FbHttpRequestPrivate
{
  FbHttp *http;
  gchar *url;
  gboolean post;
  SoupMessage *msg;
  FbHttpFunc func;
  gpointer data;
  FbHttpValues *headers;
  FbHttpValues *params;
  GError *error;
};

G_DEFINE_TYPE_WITH_PRIVATE(FbHttp, fb_http, G_TYPE_OBJECT);
G_DEFINE_TYPE_WITH_PRIVATE(FbHttpRequest, fb_http_request, G_TYPE_OBJECT);

#define FB_HTTP_PRIVATE(o) \
  ((FbHttpPrivate *)(fb_http_get_instance_private((o))))

#define FB_HTTP_REQUEST_PRIVATE(o) \
  ((FbHttpRequestPrivate *)(fb_http_request_get_instance_private((o))))

GQuark
fb_http_error_quark(void)
{
    static GQuark q;

    if (G_UNLIKELY(q == 0))
        q = g_quark_from_static_string("fb-http-error-quark");

    return q;
}

void
fb_http_set_agent(FbHttp *http, const gchar *agent)
{
  FbHttpPrivate *priv;

  g_return_if_fail(FB_IS_HTTP(http));

  priv = FB_HTTP_PRIVATE(http);

  g_object_set(priv->session,
               SOUP_SESSION_USER_AGENT, agent,
               NULL);
}

static void
fb_http_dispose(GObject *object)
{
    FbHttp *http = FB_HTTP(object);
    FbHttpPrivate *priv = FB_HTTP_PRIVATE(http);

    if (priv->session)
    {
      g_object_unref(priv->session);
      priv->session = NULL;
    }

    G_OBJECT_CLASS(fb_http_parent_class)->dispose(object);
}

static void
fb_http_class_init(FbHttpClass *klass)
{
    G_OBJECT_CLASS(klass)->dispose = fb_http_dispose;
}

static void
fb_http_init(FbHttp *http)
{
  FbHttpPrivate *priv = FB_HTTP_PRIVATE(http);

  priv->session = soup_session_new();
}

FbHttp *
fb_http_new(const gchar *agent)
{
    FbHttp *http;

    http = g_object_new(FB_TYPE_HTTP, NULL);
    fb_http_set_agent(http, agent);

    return http;
}

static void
_print_headers(SoupMessageHeaders *headers)
{
    SoupMessageHeadersIter iter;
    const char *name, *value;

    soup_message_headers_iter_init(&iter, headers);

    while(soup_message_headers_iter_next(&iter, &name, &value))
      fb_util_debug_info("  %s=%s", name, value);
}

static void
_print_body(SoupMessageBody *body, gboolean flatten)
{
  if (body->length > 0)
  {
    if (flatten)
      soup_message_body_flatten(body);

    gchar **lines = g_strsplit(body->data, "\n", 0);

    for (int i = 0; lines[i] != NULL; i++)
        fb_util_debug_info("  %s", lines[i]);

    g_strfreev(lines);
  }
  else
    fb_util_debug_info("  ** No body data **");
}

static void
on_request_starting(SoupMessage *msg, gpointer user_data)
{
  SoupURI *uri = soup_message_get_uri(msg);

  g_print("%s %s HTTP/1.1\n", msg->method, soup_uri_to_string(uri, FALSE));

  fb_util_debug_info("%s Request (%p): %s", msg->method, user_data,
                     soup_uri_to_string(uri, FALSE));

  _print_headers(msg->request_headers);
  _print_body(msg->request_body, TRUE);
}

static void
on_request_finished(SoupMessage *msg, gpointer user_data)
{
  SoupURI *uri = soup_message_get_uri(msg);

  g_print("%s %s HTTP/1.1\n", msg->method, soup_uri_to_string(uri, FALSE));

  fb_util_debug_info("%s Response (%p): %s %s (%d)", msg->method, user_data,
                     soup_uri_to_string(uri, FALSE),
                     soup_status_get_phrase(msg->status_code),
                     msg->status_code );

  _print_headers(msg->response_headers);
  _print_body(msg->response_body, FALSE);
}

static void
fb_http_request_disconnect_signals(FbHttpRequest *req)
{
  FbHttpRequestPrivate *priv = FB_HTTP_REQUEST_PRIVATE(req);

  g_assert(priv->msg);

  g_signal_handlers_disconnect_by_func(priv->msg, on_request_starting, req);
  g_signal_handlers_disconnect_by_func(priv->msg, on_request_finished, req);
}

static void
fb_http_request_dispose(GObject *object)
{
    FbHttpRequest *req = FB_HTTP_REQUEST(object);
    FbHttpRequestPrivate *priv = FB_HTTP_REQUEST_PRIVATE(req);

    if (priv->http)
    {
      g_object_unref(priv->http);
      priv->http = NULL;
    }

    if (priv->msg)
    {
      fb_http_request_disconnect_signals(req);
      g_object_unref(priv->msg);
      priv->msg = NULL;
    }

    if (priv->headers)
    {
      fb_http_values_free(priv->headers);
      priv->headers = NULL;
    }

    if (priv->url)
    {
      g_free(priv->url);
      priv->url = NULL;
    }

    G_OBJECT_CLASS(fb_http_request_parent_class)->dispose(object);
}

static void
fb_http_request_class_init(FbHttpRequestClass *klass)
{
    G_OBJECT_CLASS(klass)->dispose = fb_http_request_dispose;
}

static void
fb_http_request_init(FbHttpRequest *req)
{
  FbHttpRequestPrivate *priv = FB_HTTP_REQUEST_PRIVATE(req);

  priv->headers = fb_http_values_new();
  priv->params = fb_http_values_new();
}

FbHttpRequest *
fb_http_request_new(FbHttp *http, const gchar *url, gboolean post,
                    FbHttpFunc func, gpointer data)
{
  FbHttpRequest *req = g_object_new(FB_TYPE_HTTP_REQUEST, NULL);
  FbHttpRequestPrivate *priv = FB_HTTP_REQUEST_PRIVATE(req);
  static gboolean debug_enabled = FALSE;
  static gboolean debug_inited = FALSE;

  if (G_UNLIKELY(!debug_inited))
  {
    debug_enabled = (g_getenv("BITLBEE_DEBUG")) ||
        (g_getenv("BITLBEE_DEBUG_FACEBOOK"));
    debug_inited = TRUE;
  }

  g_return_if_fail(http != NULL);
  g_return_if_fail(url != NULL);

  priv->http = g_object_ref(http);
  priv->func = func;
  priv->data = data;
  priv->msg = soup_message_new(post ? SOUP_METHOD_POST : SOUP_METHOD_GET, url);
  priv->url = g_strdup(url);
  priv->post = post;

  if (debug_enabled)
  {
    g_signal_connect(priv->msg, "starting",
                     G_CALLBACK(on_request_starting), req);
    g_signal_connect(priv->msg, "finished",
                     G_CALLBACK(on_request_finished), req);
  }

  return req;
}

const gchar *
fb_http_request_get_data(FbHttpRequest *req, gsize *size)
{
  FbHttpRequestPrivate *priv;
  GBytes *bytes;

  g_return_val_if_fail(FB_IS_HTTP_REQUEST(req), (*size = 0, NULL));

  priv = FB_HTTP_REQUEST_PRIVATE(req);
  g_object_get(priv->msg, SOUP_MESSAGE_RESPONSE_BODY_DATA, &bytes, NULL);

  return g_bytes_get_data(bytes, size);
}

FbHttpValues *
fb_http_request_get_headers(FbHttpRequest *req)
{
    g_return_val_if_fail(FB_IS_HTTP_REQUEST(req), NULL);

    return FB_HTTP_REQUEST_PRIVATE(req)->headers;
}

FbHttpValues *
fb_http_request_get_params(FbHttpRequest *req)
{
    g_return_val_if_fail(FB_IS_HTTP_REQUEST(req), NULL);

    return FB_HTTP_REQUEST_PRIVATE(req)->params;
}

static void
fb_http_request_cb(SoupSession *session, SoupMessage *msg, gpointer user_data)
{
  FbHttpRequest *req = FB_HTTP_REQUEST(user_data);
  FbHttpRequestPrivate *priv = FB_HTTP_REQUEST_PRIVATE(req);

  if (G_UNLIKELY(msg->status_code != 200))
  {
    guint code = priv->msg->status_code;
    const gchar *status = soup_status_get_phrase(code);

    g_set_error(&priv->error, FB_HTTP_ERROR, msg->status_code,
                "%s", status);
  }

  fb_http_request_disconnect_signals(req);

  if (G_LIKELY(priv->func != NULL))
      priv->func(req, priv->data);

  priv->msg = NULL;
  g_object_unref(req);
}

void
fb_http_request_send(FbHttpRequest *req)
{
    FbHttpRequestPrivate *priv;
    GHashTableIter iter;
    gpointer key;
    gpointer val;
    SoupMessageHeaders *hdrs;
    gchar *query = NULL;

    g_return_if_fail(FB_IS_HTTP_REQUEST(req));

    priv = FB_HTTP_REQUEST_PRIVATE(req);

    g_object_get(priv->msg, SOUP_MESSAGE_REQUEST_HEADERS, &hdrs, NULL);
    g_hash_table_iter_init(&iter, priv->headers);

    while (g_hash_table_iter_next(&iter, &key, &val))
      soup_message_headers_append(hdrs, key, val);

    if (g_hash_table_size(priv->params))
      query = soup_form_encode_hash(priv->params);

    if (priv->post)
    {
      soup_message_set_request(priv->msg, "application/x-www-form-urlencoded",
                               SOUP_MEMORY_TAKE, query, strlen(query));
    }
    else if (query)
    {
      gchar *url = g_strdup_printf("%s?%s", priv->url, query);
      SoupURI *uri = soup_uri_new(url);

      g_object_set(priv->msg, SOUP_MESSAGE_URI, uri, NULL);
      soup_uri_free(uri);
      g_free(url);
      g_free(query);
    }

    soup_session_queue_message(FB_HTTP_PRIVATE(priv->http)->session,
                               priv->msg, fb_http_request_cb, req);
}

const gchar *
fb_http_request_get_status(FbHttpRequest *req, gint *code)
{
    FbHttpRequestPrivate *priv;

    g_return_val_if_fail(FB_IS_HTTP_REQUEST(req), NULL);

    priv = FB_HTTP_REQUEST_PRIVATE(req);

    if (!priv->msg)
    {
        if (code)
            *code = 0;

        return NULL;
    }

    if (code)
        *code = priv->msg->status_code;

    return soup_status_get_phrase(priv->msg->status_code);
}

GError *
fb_http_request_take_error(FbHttpRequest *req)
{
    FbHttpRequestPrivate *priv;
    GError *err;

    g_return_val_if_fail(FB_IS_HTTP_REQUEST(req), NULL);

    priv = FB_HTTP_REQUEST_PRIVATE(req);

    err = priv->error;
    priv->error = NULL;

    return err;
}

gboolean
fb_http_urlcmp(const gchar *url1, const gchar *url2, gboolean protocol)
{
    if (url1 == NULL || url2 == NULL)
        return url1 == url2;

    // Quick substring match shortcut (as original)
    if (strstr(url1, url2) != NULL || strstr(url2, url1) != NULL)
        return TRUE;

    SoupURI *purl1 = soup_uri_new(url1);
    SoupURI *purl2 = soup_uri_new(url2);

    if (purl1 == NULL || purl2 == NULL)
    {
        // fallback to case-insensitive string compare if parsing fails
        gboolean ret = g_ascii_strcasecmp(url1, url2) == 0;

        if (purl1)
          soup_uri_free(purl1);
        if (purl2)
          soup_uri_free(purl2);

        return ret;
    }

    gboolean ret = TRUE;

    if (!purl1->host || !purl2->host ||
        g_ascii_strcasecmp(purl1->host, purl2->host) != 0)
    {
        ret = FALSE;
    }
    else if (g_strcmp0(purl1->path, purl2->path) != 0)
        ret = FALSE;
    else if (g_strcmp0(purl1->user, purl2->user) != 0)
        ret = FALSE;
    else if (g_strcmp0(purl1->password, purl2->password) != 0)
        ret = FALSE;

    if (ret && protocol)
    {
        if (!purl1->scheme || !purl2->scheme ||
            g_ascii_strcasecmp(purl1->scheme, purl2->scheme) != 0)
        {
            ret = FALSE;
        }
        else if (purl1->port != purl2->port)
            ret = FALSE;
    }

    soup_uri_free(purl1);
    soup_uri_free(purl2);

    return ret;
}
