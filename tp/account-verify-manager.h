#ifndef __FB_ACCOUNT_VERIFY_MANAGER_H__
#define __FB_ACCOUNT_VERIFY_MANAGER_H__

#include <telepathy-glib/base-connection.h>

#include "account-verify-channel.h"

G_BEGIN_DECLS

typedef struct _FbAccountVerifyManager FbAccountVerifyManager;
typedef struct _FbAccountVerifyManagerClass FbAccountVerifyManagerClass;

GType
fb_account_verify_manager_get_type (void);

/* TYPE MACROS */
#define FB_TYPE_ACCOUNT_VERIFY_MANAGER \
  (fb_account_verify_manager_get_type())
#define FB_ACCOUNT_VERIFY_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), FB_TYPE_ACCOUNT_VERIFY_MANAGER, \
                              FbAccountVerifyManager))
#define FB_ACCOUNT_VERIFY_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), FB_TYPE_ACCOUNT_VERIFY_MANAGER, \
                           FbAccountVerifyManagerClass))
#define TP_IS_ACCOUNT_VERIFY_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), FB_TYPE_ACCOUNT_VERIFY_MANAGER))
#define FB_IS_ACCOUNT_VERIFY_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), FB_TYPE_ACCOUNT_VERIFY_MANAGER))
#define FB_ACCOUNT_VERIFY_MANAGER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS((obj), FB_TYPE_ACCOUNT_VERIFY_MANAGER, \
                             FbAccountVerifyManagerClass))

FbAccountVerifyManager *
fb_account_verify_manager_new(TpBaseConnection *connection);

void
fb_account_verify_manager_verify_async(FbAccountVerifyManager *self,
                                             const gchar *verify_url, const gchar *title, const gchar *message,
                                             GAsyncReadyCallback callback,
                                             gpointer user_data);

gboolean
fb_account_verify_manager_verify_finish(
        FbAccountVerifyManager *self,
        GAsyncResult *result, GError **error);


G_END_DECLS

#endif /* __FB_ACCOUNT_VERIFY_MANAGER_H__ */
