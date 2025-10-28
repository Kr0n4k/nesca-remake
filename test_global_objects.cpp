#include <QApplication>
#include <QMainWindow>
#include <iostream>

// Объявляем глобальные указатели как в nesca_3.cpp
class TestClass {
public:
    TestClass() { std::cout << "TestClass constructor" << std::endl; }
    ~TestClass() { std::cout << "TestClass destructor" << std::endl; }
};

// Глобальные указатели (как в nesca_3.cpp)
TestClass *global1 = nullptr;
TestClass *global2 = nullptr;
TestClass *global3 = nullptr;

class MinimalNesca : public QMainWindow {
public:
    MinimalNesca(QWidget *parent = 0) : QMainWindow(parent) {
        std::cout << "MinimalNesca constructor called" << std::endl;
        
        // Инициализируем глобальные объекты (как в nesca_3.cpp)
        if (global1 == nullptr) global1 = new TestClass();
        if (global2 == nullptr) global2 = new TestClass();
        if (global3 == nullptr) global3 = new TestClass();
        
        setWindowTitle("Minimal Nesca Test");
        resize(400, 300);
    }
    
    ~MinimalNesca() {
        std::cout << "MinimalNesca destructor called" << std::endl;
        
        // Очищаем глобальные объекты (как в nesca_3.cpp)
        if (global1 != nullptr) { delete global1; global1 = nullptr; }
        if (global2 != nullptr) { delete global2; global2 = nullptr; }
        if (global3 != nullptr) { delete global3; global3 = nullptr; }
    }
};

int main(int argc, char *argv[]) {
    std::cout << "Starting QApplication..." << std::endl;
    QApplication app(argc, argv);
    std::cout << "QApplication created successfully!" << std::endl;
    
    std::cout << "Creating MinimalNesca..." << std::endl;
    MinimalNesca *gui = new MinimalNesca();
    std::cout << "MinimalNesca created successfully!" << std::endl;
    
    std::cout << "Deleting MinimalNesca..." << std::endl;
    delete gui;
    std::cout << "MinimalNesca deleted successfully!" << std::endl;
    
    std::cout << "Application completed successfully!" << std::endl;
    return 0;
}
