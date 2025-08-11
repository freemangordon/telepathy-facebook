#ifndef FB_ACCOUNT_VERIFY_CHANNEL_H
#define FB_ACCOUNT_VERIFY_CHANNEL_H

#include <glib-object.h>

#include <telepathy-glib/base-channel.h>

G_BEGIN_DECLS

typedef struct _FbAccountVerifyChannel FbAccountVerifyChannel;
typedef struct _FbAccountVerifyChannelClass
  FbAccountVerifyChannelClass;

GType
fb_account_verify_channel_get_type (void);

/* TYPE MACROS */
#define FB_TYPE_ACCOUNT_VERIFY_CHANNEL \
  (fb_account_verify_channel_get_type())
#define FB_ACCOUNT_VERIFY_CHANNEL(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), FB_TYPE_ACCOUNT_VERIFY_CHANNEL, \
                              FbAccountVerifyChannel))
#define FB_ACCOUNT_VERIFY_CHANNEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), FB_TYPE_ACCOUNT_VERIFY_CHANNEL, \
                           FbAccountVerifyChannelClass))
#define TP_IS_FB_ACCOUNT_VERIFY_CHANNEL(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), FB_TYPE_ACCOUNT_VERIFY_CHANNEL))
#define TP_IS_FB_ACCOUNT_VERIFY_CHANNEL_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), FB_TYPE_ACCOUNT_VERIFY_CHANNEL))
#define FB_ACCOUNT_VERIFY_CHANNEL_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS((obj), FB_TYPE_ACCOUNT_VERIFY_CHANNEL, \
                             FbAccountVerifyChannelClass))

FbAccountVerifyChannel *
fb_account_verify_channel_new(TpBaseConnection *connection,
                                    const char *verify_url, const char *title,
                                    const char *message);

G_END_DECLS

#endif // FB_ACCOUNT_VERIFY_CHANNEL_H
