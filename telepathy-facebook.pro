TEMPLATE = app
CONFIG -= console
CONFIG -= app_bundle
CONFIG -= qt

CONFIG += link_pkgconfig

PKGCONFIG += telepathy-glib json-glib-1.0 libssl libcrypto dbus-glib-1

DEFINES += VERSION=\\\"QT-TEST\\\"
DEFINES += PACKAGE_VERSION=\\\"1.0\\\"
DEFINES += _GNU_SOURCE

INCLUDEPATH += $$PWD/bitlbee-compat $$PWD/bitlbee-facebook/facebook

# marshallers
BITLBEE_SRC_DIR = $$PWD/bitlbee-facebook/facebook

SOURCES += \
    $$BITLBEE_SRC_DIR/facebook-api.c \
    $$BITLBEE_SRC_DIR/facebook-http.c \
    $$BITLBEE_SRC_DIR/facebook-json.c \
    $$BITLBEE_SRC_DIR/facebook-mqtt.c \
    $$BITLBEE_SRC_DIR/facebook-thrift.c \
    $$BITLBEE_SRC_DIR/facebook-util.c \
    bitlbee-compat/base64.c \
    bitlbee-compat/events_glib.c \
    bitlbee-compat/http_client.c \
    bitlbee-compat/misc.c \
    bitlbee-compat/proxy.c \
    bitlbee-compat/set.c \
    bitlbee-compat/sha1.c \
    bitlbee-compat/ssl_openssl.c \
    bitlbee-compat/url.c \
    tp/account-verify-channel.c \
    tp/account-verify-manager.c \
    tp/avatars.c \
    tp/connection-manager.c \
    tp/connection.c \
    tp/connection-data.c \
    tp/contact-list.c \
    tp/debug.c \
    tp/main.c \
    tp/presence.c \
    tp/protocol.c

HEADERS += \
    tp/avatars.h \
    tp/connection-data.h \
    tp/connection-manager.h \
    tp/contact-list.h \
    tp/debug.h \
    tp/presence.h \
    tp/protocol.h

GLIB_GENMARSHAL=glib-genmarshal

facebook_marshal_h.target = $$BITLBEE_SRC_DIR/facebook-marshal.h
facebook_marshal_h.commands = $${GLIB_GENMARSHAL} --prefix=fb_marshal --header $$BITLBEE_SRC_DIR/marshaller.list > $@

facebook_marshal_c.target = $$BITLBEE_SRC_DIR/facebook-marshal.c
facebook_marshal_c.commands = $${GLIB_GENMARSHAL} --prefix=fb_marshal --body $$BITLBEE_SRC_DIR/marshaller.list >> $@
SOURCES += $$facebook_marshal_c.target

PRE_TARGETDEPS += $$facebook_marshal_h.target
PRE_TARGETDEPS += $$facebook_marshal_c.target

EXTDIR=extensions
GENDIR=$${EXTDIR}/_gen
INCLUDEPATH += $${GENDIR}
TOOLSDIR=tools/
gen_svc_c.commands = ${MKDIR} $${GENDIR} && \
    python $${TOOLSDIR}/glib-ginterface-gen.py \
    --filename=$${GENDIR}/svc \
    --include=\'<telepathy-glib/telepathy-glib.h>\' \
    $${EXTDIR}/account-verify-channel.xml Fb_Svc_
gen_svc_c.target = $${GENDIR}/svc.c

gen_svc_h.target = $${GENDIR}/svc.h
gen_svc_h.depends = $$gen_svc_c.target

gen_svc_gtk_doc_h.target = $${GENDIR}/svc-gtk-doc.h
gen_svc_gtk_doc_h.depends = $$gen_svc_c.target

PRE_TARGETDEPS += $$gen_svc_c.target
SOURCES += $$gen_svc_c.target

np.target = .NOTPARALLEL
np.depends = \
    facebook_marshal_h facebook_marshal_c gen_svc_c

QMAKE_EXTRA_TARGETS += \
    facebook_marshal_h facebook_marshal_c \
    gen_svc_c gen_svc_h gen_svc_gtk_doc_h \
    np

QMAKE_CLEAN += \
    $$gen_svc_c.target $$gen_svc_h.target \
    $$gen_svc_gtk_doc_h.target \
    $$facebook_marshal_h.target $$facebook_marshal_c.target

#FIXME
QMAKE_CFLAGS += -Wno-unused-parameter -Wno-sign-compare -Wno-deprecated-declarations
QMAKE_CFLAGS += -Wno-missing-field-initializers

#QMAKE_CFLAGS += -fsanitize-address
#LIBS += -lasan

DISTFILES += \
    tp/facebook-2fa-channel.xml
