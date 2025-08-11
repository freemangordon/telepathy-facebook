QT += core gui webenginewidgets

CONFIG += c++11

CONFIG += link_pkgconfig
PKGCONFIG += telepathy-glib gobject-2.0 dbus-glib-1


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS QT_NO_KEYWORDS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        facebook-cli.c \
        facebookaccountverifyobserver.cpp \
        facebookaccountverifyui.cpp \
        main.cpp

EXTDIR=../extensions
GENDIR=$${EXTDIR}/_gen
INCLUDEPATH += $${GENDIR}
TOOLSDIR=../tools/

gen_cli_h.target = $${GENDIR}/cli.h
gen_cli_h.commands = ${MKDIR} $${GENDIR} && \
    python $${TOOLSDIR}/glib-client-gen.py \
    $${EXTDIR}/account-verify-channel.xml Fb_Cli $${GENDIR}/cli

gen_cli_body_h.target = $${GENDIR}/cli-body.h
gen_cli_body_h.depends = gen_cli_h

gen_cli_gtk_doc_h.target = $${GENDIR}/cli-gtk-doc.h
gen_cli_gtk_doc_h.depends = gen_cli_h

PRE_TARGETDEPS += $$gen_cli_h.target

np.target = .NOTPARALLEL
np.depends = gen_cli_h gen_cli_body_h gen_cli_gtk_doc_h

QMAKE_EXTRA_TARGETS += gen_cli_h gen_cli_body_h gen_cli_gtk_doc_h np

QMAKE_CLEAN += \
    $$gen_cli_h.target \
    $$gen_cli_body_h.target \
    $$gen_cli_gtk_doc_h.target

HEADERS += \
    facebook-cli.h \
    facebookaccountverifyobserver.h \
    facebookaccountverifyui.h

QMAKE_CXXFLAGS += -Wno-deprecated-declarations
