#include <QtCore/QCoreApplication>

#include <Windows.h>
#include <clocale>
#include <GeneralClass.h>



int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    SetConsoleCP(65001);        // UTF‑8 вход
    SetConsoleOutputCP(65001);  // UTF‑8 вывод
    setlocale(LC_ALL, "ru_RU.UTF-8");
    
    GeneralClass myGenClass;


    return app.exec();
}
