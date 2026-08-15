#include "kvstore/engine.hpp"
#include <iostream>
#include <cassert>
#include <vector>
#include <thread>
#include <chrono>

void test_basic_crud() {
    std::cout << "[Test 1] Testing Basic CRUD Operations..." << std::endl;
    std::string test_dir = "kv_test_crud";
    
    {
        KVEngine db(test_dir);
        
        // Put & Get
        db.put("key1", "val1");
        db.put("key2", "val2");
        
        assert(db.get("key1").value_or("") == "val1");
        assert(db.get("key2").value_or("") == "val2");
        assert(!db.get("key3").has_value());
        assert(db.size() == 2);
        
        // Update
        db.put("key1", "val1_updated");
        assert(db.get("key1").value_or("") == "val1_updated");
        
        // Delete
        assert(db.remove("key1") == true);
        assert(!db.get("key1").has_value());
        assert(db.size() == 1);
        
        // Delete Non-existent
        assert(db.remove("non_existent_key") == false);
    }
    std::cout << " -> PASSED" << std::endl;
}

void test_wal_durability() {
    std::cout << "[Test 2] Testing WAL Crash Recovery Durability..." << std::endl;
    std::string test_dir = "kv_test_durability";

    // Populate data and crash
    {
        KVEngine db(test_dir);
        db.put("persist_1", "alpha");
        db.put("persist_2", "beta");
        db.put("persist_3", "gamma");
        db.remove("persist_2");
    }

    // Recover from disk
    {
        KVEngine db_recovered(test_dir);
        assert(db_recovered.get("persist_1").value_or("") == "alpha");
        assert(!db_recovered.get("persist_2").has_value()); // Deleted
        assert(db_recovered.get("persist_3").value_or("") == "gamma");
        assert(db_recovered.size() == 2);
    }
    std::cout << " -> PASSED" << std::endl;
}

void test_concurrent_access() {
    std::cout << "[Test 3] Testing Concurrent Multi-Threaded Stress..." << std::endl;
    std::string test_dir = "kv_test_concurrent";
    
    {
        KVEngine db(test_dir);
        constexpr int num_threads = 8;
        constexpr int ops_per_thread = 500;
        
        std::vector<std::thread> threads;
        threads.reserve(num_threads);

        auto worker = [&](int thread_id) {
            for (int i = 0; i < ops_per_thread; ++i) {
                std::string k = "t" + std::to_string(thread_id) + "_k" + std::to_string(i);
                std::string v = "v" + std::to_string(i);
                
                db.put(k, v);
                auto res = db.get(k);
                assert(res.has_value() && res.value() == v);
            }
        };

        auto start_time = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(worker, i);
        }

        for (auto& t : threads) {
            t.join();
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        size_t expected_total = num_threads * ops_per_thread;
        assert(db.size() == expected_total);

        std::cout << "    Executed " << (expected_total * 2) << " concurrent operations across " 
                  << num_threads << " threads in " << duration_ms << " ms" << std::endl;
    }
    std::cout << " -> PASSED" << std::endl;
}

int main() {
    std::cout << "============================================" << std::endl;
    std::cout << " Running KVStore Test Suite " << std::endl;
    std::cout << "============================================" << std::endl;

    try {
        test_basic_crud();
        test_wal_durability();
        test_concurrent_access();

        std::cout << "\n============================================" << std::endl;
        std::cout << " ALL TESTS PASSED SUCCESSFULLY! " << std::endl;
        std::cout << "============================================" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n[TEST FAILED]: " << e.what() << std::endl;
        return 1;
    }
}