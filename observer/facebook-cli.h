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
#ifndef FACEBOOKCLI_H
#define FACEBOOKCLI_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*facebook_cli_account_verified_cb) (TpProxy *proxy,
    const GError *error, gpointer user_data,
    GObject *weak_object);

TpProxyPendingCall *
facebook_cli_account_verified(
        gpointer proxy,
        gint timeout_ms,
        gboolean in_verified,
        facebook_cli_account_verified_cb callback,
        gpointer user_data,
        GDestroyNotify destroy,
        GObject *weak_object);

#ifdef __cplusplus
}
#endif

#endif // FACEBOOKCLI_H
