#include <glib-object.h>
#include <stdio.h>
#include <time.h>

typedef struct {
    int x;
    int y;
} square;

// GLib требует gpointer → gpointer (НЕ const!)
static gpointer square_copy(gpointer src) {
    square* s = src;
    square* dst = g_new(square, 1);
    *dst = *s;
    return dst;
}

static void square_free(gpointer data) {
    g_free(data);
}

static double now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(void) {
    const int N = 5 * 1000 * 1000;

    // g_type_init() больше НЕ нужен в GLib 2.80+
    // g_type_init();

    // Регистрируем boxed-тип
    GType square_type = g_boxed_type_register_static(
        "square",
        square_copy,
        square_free
    );

    square s = {10, 20};

    double t0, t1;

    printf("=== GLib / GObject ===\n");

    // --- create/free ---
    t0 = now();
    for (int i = 0; i < N; i++) {
        square* copy = g_boxed_copy(square_type, &s);
        g_boxed_free(square_type, copy);
    }
    t1 = now();
    printf("glib boxed copy/free: %.6f sec\n", t1 - t0);

    // --- lookup(name) ---
    t0 = now();
    for (int i = 0; i < N; i++) {
        g_type_from_name("square");
    }
    t1 = now();
    printf("glib lookup(name): %.6f sec\n", t1 - t0);

    // --- lookup(id) ---
    t0 = now();
    for (int i = 0; i < N; i++) {
        g_type_name(square_type);
    }
    t1 = now();
    printf("glib lookup(id): %.6f sec\n", t1 - t0);

    // --- cast через GValue ---
    GValue v1 = G_VALUE_INIT;
    GValue v2 = G_VALUE_INIT;

    g_value_init(&v1, square_type);
    g_value_init(&v2, square_type);
    g_value_set_boxed(&v1, &s);

    t0 = now();
    for (int i = 0; i < N; i++) {
        g_value_transform(&v1, &v2);
    }
    t1 = now();
    printf("glib cast (GValue transform): %.6f sec\n", t1 - t0);

    g_value_unset(&v1);
    g_value_unset(&v2);

    return 0;
}
