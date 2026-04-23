#include <QMetaType>
#include <QVariant>
#include <QElapsedTimer>
#include <iostream>

extern "C" {
#include "include/cutils-rtti.h"
}

struct square {
    int x;
    int y;
};

Q_DECLARE_METATYPE(square)

static double bench(std::function<void()> fn) {
    QElapsedTimer t;
    t.start();
    fn();
    return t.elapsed() / 1000.0;
}

int main() {
    const int N = 5 * 1000 * 1000;

    // === Qt6 ===
    std::cout << "=== Qt6 QMetaType ===\n";

    // регистрация типа
    int qt_square_type = qRegisterMetaType<square>("square");

    square s{10, 20};

    // create/destroy
    double t_create = bench([&]() {
        for (int i = 0; i < N; i++) {
            void* obj = QMetaType(qt_square_type).create(&s);
            QMetaType(qt_square_type).destroy(obj);
        }
    });
    std::cout << "Qt create/destroy: " << t_create << " sec\n";

    // lookup(name)
    double t_lookup_name = bench([&]() {
        for (int i = 0; i < N; i++) {
            QMetaType::fromName("square");
        }
    });
    std::cout << "Qt lookup(name): " << t_lookup_name << " sec\n";

    // lookup(id)
    double t_lookup_id = bench([&]() {
        for (int i = 0; i < N; i++) {
            QMetaType(qt_square_type).name();
        }
    });
    std::cout << "Qt lookup(id): " << t_lookup_id << " sec\n";

    // cast (QVariant copy)
    QVariant v1 = QVariant::fromValue(s);
    QVariant v2;

    double t_cast = bench([&]() {
        for (int i = 0; i < N; i++) {
            v2 = v1;
        }
    });
    std::cout << "Qt cast: " << t_cast << " sec\n";

    // === cutils-rtti ===
    std::cout << "\n=== cutils-rtti ===\n";

    rtti_begin();
    rtti_register("square", sizeof(square));
    int id_square = rtti_typeid("square", NULL);

    // create/free
    double c_create = bench([&]() {
        for (int i = 0; i < N; i++) {
            rtti_t* obj = rtti_new_custom("square");
            rtti_set(obj, id_square, &s);
            rtti_free(&obj);
        }
    });
    std::cout << "cutils create/free: " << c_create << " sec\n";

    // lookup(name)
    double c_lookup_name = bench([&]() {
        for (int i = 0; i < N; i++) {
            rtti_typeid("square", NULL);
        }
    });
    std::cout << "cutils lookup(name): " << c_lookup_name << " sec\n";

    // lookup(id)
    double c_lookup_id = bench([&]() {
        for (int i = 0; i < N; i++) {
            rtti_typeid(NULL, (rtti_t*)&id_square);
        }
    });
    std::cout << "cutils lookup(id): " << c_lookup_id << " sec\n";

    // cast
    rtti_t* obj = rtti_new_custom("square");
    rtti_set(obj, id_square, &s);

    double c_cast = bench([&]() {
        for (int i = 0; i < N; i++) {
            rtti_cast(obj, id_square);
        }
    });
    std::cout << "cutils cast: " << c_cast << " sec\n";

    rtti_free(&obj);
    rtti_end();

    return 0;
}
