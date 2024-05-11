#include "../include/header.h"

static GtkWidget* window;
static GtkWidget* list_box;

static void on_row_activated(GtkListBox* box, GtkListBoxRow* row, gpointer data) {
    (void)box;
    (void)data;
    if (row) {
        GtkWidget* label = gtk_bin_get_child(GTK_BIN(row));
        const gchar* text = gtk_label_get_text(GTK_LABEL(label));
        g_print("%s\n", text);
    }
}

static gboolean on_motion_notify(GtkWidget* widget, GdkEventMotion* event, gpointer data) {
    (void)data;
    GtkListBox* box = GTK_LIST_BOX(widget);
    GtkListBoxRow* row = gtk_list_box_get_row_at_y(box, event->y);
    if (row) {
        gtk_list_box_select_row(box, row);
    }
    return FALSE;
}

static void destroy(GtkWidget* widget, gpointer data) {
    (void)widget;
    (void)data;
    gtk_main_quit();
}

static void activate(GtkApplication* const app, gpointer data){
    (void)data;
    window = gtk_application_window_new(app);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_type_hint(
        GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DIALOG);

    GtkStyleContext* const context = gtk_widget_get_style_context(
        window);
    GtkCssProvider* const provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, BOXCSS, -1, NULL);
    gtk_style_context_add_provider(
        context, GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider); // Unref the provider after use

    list_box = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(window), list_box);

    for (ubyte i = 0; i < 10; i++) {
        char* text = g_strdup_printf("Line %d", i + 1);
        GtkWidget* label = gtk_label_new(text);
        GtkWidget* row = gtk_list_box_row_new();
        gtk_container_add(GTK_CONTAINER(row), label);
        gtk_container_add(GTK_CONTAINER(list_box), row);
        g_free(text);
    }
    g_signal_connect(
        list_box, "motion-notify-event",
        G_CALLBACK(on_motion_notify), NULL);
    g_signal_connect(
        list_box, "row-activated",
        G_CALLBACK(on_row_activated), NULL);
    g_signal_connect(
        window, "destroy", G_CALLBACK(destroy), NULL);

    gtk_widget_show_all(window);
}

GtkApplication* app_sgt(const char* const id){
    static GtkApplication* app = NULL;
    if (id) app = gtk_application_new(
        id, G_APPLICATION_DEFAULT_FLAGS);
    else g_object_unref(app);
    return app;
}

void sig_hdl(int sig){
    app_sgt(NULL);
    exit(sig);
}

int main(int ac, char** av){
    signal(SIGINT, sig_hdl);
    signal(SIGTERM, sig_hdl);

    GtkApplication* const app = app_sgt("org.gtk.launcher");
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    const int code = g_application_run(G_APPLICATION(app), ac, av);
    g_object_unref(app);
    return code;
}
