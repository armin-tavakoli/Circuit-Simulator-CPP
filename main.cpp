// main.cpp

#include <QApplication>
#include "MainWindow.h" // فایل هدر پنجره اصلی را اضافه می‌کنیم

int main(int argc, char *argv[]) {
    QApplication app(argc, argv); // ساخت اپلیکیشن

    MainWindow window; // ساخت یک نمونه از پنجره اصلی ما
    window.show();     // نمایش پنجره

    return app.exec(); // اجرای حلقه رویداد برنامه
}