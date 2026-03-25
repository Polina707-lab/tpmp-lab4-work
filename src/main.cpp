#include "database.h"
#include <iostream>

int main() {
    Database db;

    if (db.open("data/test.db")) {
        std::cout << "DB opened\n";
    }

    db.close();
    return 0;
}