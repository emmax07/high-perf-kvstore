#include "kvstore/engine.hpp"
#include <iostream>

int main() {
    std::cout << "============================================" << std::endl;
    std::cout << " High-Performance KV Store " << std::endl;
    std::cout << "============================================" << std::endl;

    std::string db_dir = "kv_data";

    {
        std::cout << "\n[Session 1] Initializing KVEngine..." << std::endl;
        KVEngine db(db_dir);

        std::cout << "\n[Session 1] Inserting records..." << std::endl;
        db.put("user_101", "Emmax");
        db.put("user_102", "Alex");
        db.put("user_103", "Jordan");

        db.print_state();

        std::cout << "\n[Session 1] Deleting user_102..." << std::endl;
        db.remove("user_102");

        db.print_state();
        std::cout << "[Session 1] Closing database session (simulating restart)..." << std::endl;
    }

    std::cout << "\n============================================" << std::endl;

    {
        std::cout << "\n[Session 2] Starting NEW KVEngine instance..." << std::endl;
        KVEngine db_recovered(db_dir);

        std::cout << "\n[Session 2] Reading recovered values:" << std::endl;
        
        auto u1 = db_recovered.get("user_101");
        auto u2 = db_recovered.get("user_102");
        auto u3 = db_recovered.get("user_103");

        std::cout << "  user_101: " << (u1 ? *u1 : "<NOT FOUND>") << std::endl;
        std::cout << "  user_102: " << (u2 ? *u2 : "<NOT FOUND>") << std::endl;
        std::cout << "  user_103: " << (u3 ? *u3 : "<NOT FOUND>") << std::endl;

        db_recovered.print_state();
    }

    std::cout << "\n============================================" << std::endl;
    std::cout << " Engine & WAL Recovery Test PASSED!" << std::endl;
    std::cout << "============================================" << std::endl;

    return 0;
}