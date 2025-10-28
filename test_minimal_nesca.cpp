#include <QApplication>
#include <QMainWindow>
#include <iostream>

// Минимальная версия nesca_3 без глобальных объектов
class MinimalNesca : public QMainWindow {
public:
    MinimalNesca(QWidget *parent = 0) : QMainWindow(parent) {
        std::cout << "MinimalNesca constructor called" << std::endl;
        setWindowTitle("Minimal Nesca Test");
        resize(400, 300);
    }
    
    ~MinimalNesca() {
        std::cout << "MinimalNesca destructor called" << std::endl;
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
