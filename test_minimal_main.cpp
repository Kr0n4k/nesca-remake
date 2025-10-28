#include <QApplication>
#include <QMainWindow>
#include <iostream>

int main(int argc, char *argv[]) {
    std::cout << "Starting QApplication..." << std::endl;
    QApplication app(argc, argv);
    std::cout << "QApplication created successfully!" << std::endl;
    
    std::cout << "Creating QMainWindow..." << std::endl;
    QMainWindow *window = new QMainWindow();
    std::cout << "QMainWindow created successfully!" << std::endl;
    
    std::cout << "Deleting QMainWindow..." << std::endl;
    delete window;
    std::cout << "QMainWindow deleted successfully!" << std::endl;
    
    std::cout << "Application completed successfully!" << std::endl;
    return 0;
}
