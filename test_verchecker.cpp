#include <QApplication>
#include <QMainWindow>
#include <iostream>
#include "vercheckerthread.h"

int main(int argc, char *argv[]) {
    std::cout << "Starting QApplication..." << std::endl;
    QApplication app(argc, argv);
    std::cout << "QApplication created successfully!" << std::endl;
    
    std::cout << "Creating VerCheckerThread..." << std::endl;
    VerCheckerThread *vct = new VerCheckerThread();
    std::cout << "VerCheckerThread created successfully!" << std::endl;
    
    std::cout << "Deleting VerCheckerThread..." << std::endl;
    delete vct;
    std::cout << "VerCheckerThread deleted successfully!" << std::endl;
    
    std::cout << "Application completed successfully!" << std::endl;
    return 0;
}
