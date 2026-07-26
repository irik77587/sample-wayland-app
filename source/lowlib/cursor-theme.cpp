#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <dbus/dbus.h>
const char *get_cursor_theme() {

    #define SERVICE "org.freedesktop.portal.Desktop"
    #define OBJECT "/org/freedesktop/portal/desktop"
    #define INTERFACE "org.freedesktop.portal.Settings"
    #define METHOD "ReadOne"

    // dbus method arguments must be string pointer
    const char *karg = "org.gnome.desktop.interface";
    const char *varg = "cursor-theme";

    dbus_bool_t dbus_status;
    DBusError dbus_error;
    DBusConnection *dbus_conn = nullptr;
    DBusMessage *dbus_request = nullptr;
    DBusMessage *dbus_respond = nullptr;
    // d-bus message iterator (DBusMessageIter type) must not be pointer or NULL
    DBusMessageIter dbus_iter;
    DBusMessageIter dbus_sub_iter;
    const char *dbus_result;
    auto EXIT_STATUS = EXIT_SUCCESS;

    ::dbus_error_init(&dbus_error);

    dbus_conn = ::dbus_bus_get(DBUS_BUS_SESSION, &dbus_error);

    if (nullptr == dbus_conn) {
        ::perror(dbus_error.name);
        ::perror(dbus_error.message);
        EXIT_STATUS = EXIT_FAILURE;
    } else
        dbus_request =
        ::dbus_message_new_method_call(SERVICE, OBJECT, INTERFACE, METHOD);

    if (nullptr == dbus_request) {
        ::perror("ERROR ::dbus_message_new_method_call");
        EXIT_STATUS = EXIT_FAILURE;
    } else {
        dbus_status =
        ::dbus_message_append_args(dbus_request, DBUS_TYPE_STRING, &karg,
                                   DBUS_TYPE_STRING, &varg, DBUS_TYPE_INVALID);
    }

    if (!dbus_status) {
        ::perror("ERROR ::dbus_message_append_args");
        EXIT_STATUS = EXIT_FAILURE;
    } else {
        dbus_respond = ::dbus_connection_send_with_reply_and_block(
            dbus_conn, dbus_request, DBUS_TIMEOUT_USE_DEFAULT, &dbus_error);
    }

    if (nullptr == dbus_respond) {
        ::perror(dbus_error.name);
        ::perror(dbus_error.message);
        EXIT_STATUS = EXIT_FAILURE;
    } /*
    else
    {
    dbus_status = ::dbus_message_get_args(dbus_respond, &dbus_error,
    DBUS_TYPE_STRING, dbus_result, DBUS_TYPE_INVALID);
    }
    if (!dbus_status)
    {
    ::perror("ERROR ::dbus_message_get_args");
    EXIT_STATUS = EXIT_FAILURE;
    }*/
    else {
        dbus_status = ::dbus_message_iter_init(dbus_respond, &dbus_iter);
    }

    if (!dbus_status ||
        ::dbus_message_iter_get_arg_type(&dbus_iter) != DBUS_TYPE_VARIANT) {
        ::perror("ERROR ::dbus_message_iter_init");
    } else {
        /*
         * open layers to retrieve i.e. extract the string i.e. result
         *  ┌───────────────────────────────────────┐
         *  │ v (Variant Box)                       │
         *  │   ┌─────────────────────────────────┐ │
         *  │   │ s (String Label)                │ │
         *  │   │   "breeze_cursors"              │ │
         *  │   └─────────────────────────────────┘ │
         *  └───────────────────────────────────────┘
         */
        ::dbus_message_iter_recurse(&dbus_iter, &dbus_sub_iter);
    }

    if (::dbus_message_iter_get_arg_type(&dbus_sub_iter) == DBUS_TYPE_STRING) {
        ::dbus_message_iter_get_basic(&dbus_sub_iter, &dbus_result);
    }

    if (nullptr != dbus_respond)
        ::dbus_message_unref(dbus_respond);
    if (nullptr != dbus_request)
        ::dbus_message_unref(dbus_request);
    if (nullptr != dbus_conn)
        ::dbus_connection_unref(dbus_conn);
    if (EXIT_STATUS == EXIT_FAILURE)
        return NULL;

    const char*cursor_theme = strdup(dbus_result);
    return cursor_theme;
}
