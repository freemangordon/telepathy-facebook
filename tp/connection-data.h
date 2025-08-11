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
#ifndef __FB_CONNECTION_DATA_H__
#define __FB_CONNECTION_DATA_H__

#include <glib.h>

G_BEGIN_DECLS

/**
 * @file facebook-connection-data.h
 * @brief Persistent storage for per-account connection data.
 *
 * This API allows saving and loading non-secret account-specific data
 * (e.g., last message ID, timestamps) in a persistent JSON-based store,
 * keyed by Fb ID. All values are stored as strings via `GHashTable`.
 */

/**
 * fb_connection_data_load:
 * @fb_id: Fb user ID string (must not be NULL)
 *
 * Loads the persistent data associated with the given Fb ID from disk.
 * If the file or entry doesn't exist, returns an empty hash table.
 *
 * @returns (transfer full): a `GHashTable *` mapping string keys to string
 *                           values. Caller must free it using
 *                           `g_hash_table_destroy()`.
 */
GHashTable *
fb_connection_data_load(const gchar *fb_id);

/**
 * fb_connection_data_save:
 * @fb_id: Fb user ID string (must not be NULL)
 * @data: hash table of key/value string pairs to store (must not be NULL)
 *
 * Stores the given hash table data under the Fb ID. Replaces any existing
 * values for that user. Persists to a JSON file under the user data directory.
 *
 * @returns: TRUE on success, FALSE on failure.
 */
gboolean
fb_connection_data_save(const gchar *fb_id, GHashTable *data);

/**
 * fb_connection_data_get_string:
 * @data: Hash table to look up
 * @key: Key string
 *
 * Returns a string value or NULL if key is not present.
 * Returned string belongs to the hash table; do not free it.
 */
const gchar *
fb_connection_data_get_string(GHashTable *data, const gchar *key);

/**
 * fb_connection_data_get_int:
 * @data: Hash table to look up
 * @key: Key string
 * @default_value: Fallback if key not found or invalid
 *
 * Returns the integer value, or @default_value if not present or malformed.
 */
gint
fb_connection_data_get_int(GHashTable *data,
                                 const gchar *key,
                                 gint default_value);

/**
 * fb_connection_data_get_int64:
 * Returns 64-bit int value or default.
 */
gint64
fb_connection_data_get_int64(GHashTable *data,
                                   const gchar *key,
                                   gint64 default_value);

/**
 * fb_connection_data_get_uint64:
 * Returns 64-bit unsigned int value or default.
 */
guint64
fb_connection_data_get_uint64(GHashTable *data,
                                    const gchar *key,
                                    guint64 default_value);

/**
 * fb_connection_data_get_bool:
 * Interprets "true", "1" as TRUE, otherwise FALSE. Falls back to default.
 */
gboolean
fb_connection_data_get_bool(GHashTable *data,
                                  const gchar *key,
                                  gboolean default_value);

// Setters

/**
 * fb_connection_data_set_string:
 * @data: (inout): Hash table to modify
 * @key: Key string
 * @value: String value to set
 *
 * Stores the given string @value in @data under @key.
 * Both key and value strings are copied.
 */
void
fb_connection_data_set_string(GHashTable *data,
                                    const gchar *key,
                                    const gchar *value);

/**
 * fb_connection_data_set_int:
 * @data: (inout): Hash table to modify
 * @key: Key string
 * @value: Integer value to set
 *
 * Converts the integer @value to string and stores it in @data under @key.
 */
void
fb_connection_data_set_int(GHashTable *data, const gchar *key,
                                 gint value);

/**
 * fb_connection_data_set_int64:
 * @data: (inout): Hash table to modify
 * @key: Key string
 * @value: 64-bit integer value to set
 *
 * Converts the 64-bit integer @value to string and stores it in @data under
 * @key.
 */
void
fb_connection_data_set_int64(GHashTable *data,
                                   const gchar *key,
                                   gint64 value);

/**
 * fb_connection_data_set_uint64:
 * @data: (inout): Hash table to modify
 * @key: Key string
 * @value: 64-bit unsigned integer value to set
 *
 * Converts the 64-bit unsigned integer @value to string and stores it in @data
 * under @key.
 */
void
fb_connection_data_set_uint64(GHashTable *data,
                                    const gchar *key,
                                    guint64 value);

/**
 * fb_connection_data_set_bool:
 * @data: (inout): Hash table to modify
 * @key: Key string
 * @value: Boolean value to set
 *
 * Stores the boolean @value as string "true" or "false" in @data under @key.
 */
void
fb_connection_data_set_bool(GHashTable *data,
                                  const gchar *key,
                                  gboolean value);

G_END_DECLS

#endif /* __FB_CONNECTION_DATA_H__ */
