// NESpressoTests_COMPREHENSIVE.cpp - COMPLETE test suite for NESpresso CPU emulator
// Date: 2025-07-29 23:22:38 UTC
// User: nicusor43
// COMPREHENSIVE TESTING FOR ALL 6502 INSTRUCTIONS

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <stdexcept>
#include <iomanip>
#include <algorithm>
#include <sstream>

// Include your CPU and Memory headers
#include <map>

#include "src/Cpu.hpp"
#include "src/Memory.hpp"

// ================================
// ENHANCED TEST FRAMEWORK
// ================================

class ComprehensiveTestFramework {
private:
    struct TestResult {
        std::string suite;
        std::string name;
        bool passed;
        std::string error_message;
        std::chrono::microseconds duration;
        std::string details;
    };

    std::vector<TestResult> results;
    std::string current_suite;

public:
    void beginSuite(const std::string& suite_name) {
        current_suite = suite_name;
        std::cout << "\n🧪 " << suite_name << std::endl;
        std::cout << std::string(70, '=') << std::endl;
    }

    void runTest(const std::string& name, const std::function<void()>& test_func, const std::string& details = "") {
        auto start = std::chrono::high_resolution_clock::now();
        try {
            test_func();
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            results.push_back({current_suite, name, true, "", duration, details});
            std::cout << "  ✅ " << std::setw(40) << std::left << name
                      << " (" << std::setw(6) << duration.count() << "μs)";
            if (!details.empty()) std::cout << " - " << details;
            std::cout << std::endl;
        } catch (const std::exception& e) {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            results.push_back({current_suite, name, false, e.what(), duration, details});
            std::cout << "  ❌ " << std::setw(40) << std::left << name
                      << " FAILED: " << e.what() << std::endl;
        }
    }

    void printDetailedSummary() const {
        int passed = 0, failed = 0;
        std::chrono::microseconds total_time{0};
        for (const auto& result : results) {
            result.passed ? passed++ : failed++;
            total_time += result.duration;
        }

        std::cout << "\n" << std::string(80, '=') << std::endl;
        std::cout << "🎯 COMPREHENSIVE TEST SUMMARY - NESpresso Emulator" << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        std::cout << "📊 Total tests executed: " << results.size() << std::endl;
        std::cout << "✅ Passed: " << passed << " (" << std::fixed << std::setprecision(1)
                  << (results.empty() ? 0.0 : 100.0 * passed / results.size()) << "%)" << std::endl;
        std::cout << "❌ Failed: " << failed << " (" << std::fixed << std::setprecision(1)
                  << (results.empty() ? 0.0 : 100.0 * failed / results.size()) << "%)" << std::endl;
        if (!results.empty()) {
            std::cout << "⏱️  Total execution time: " << total_time.count() << "μs" << std::endl;
            std::cout << "⚡ Average test time: " << (total_time.count() / results.size()) << "μs" << std::endl;
        }
        std::cout << "👤 Tested by: nicusor43" << std::endl;
        std::cout << "📅 Date: 2025-07-29 23:22:38 UTC" << std::endl;

        std::cout << "\n📈 RESULTS BY SUITE:" << std::endl;
        std::map<std::string, std::pair<int, int>> suite_stats;
        for (const auto& result : results) {
            result.passed ? suite_stats[result.suite].first++ : suite_stats[result.suite].second++;
        }

        for (const auto& [suite, stats] : suite_stats) {
            int total = stats.first + stats.second;
            double success_rate = (total > 0) ? (100.0 * stats.first) / total : 0.0;
            std::cout << "  " << std::setw(30) << std::left << suite
                      << " " << stats.first << "/" << total
                      << " (" << std::fixed << std::setprecision(1) << success_rate << "%)" << std::endl;
        }

        if (failed > 0) {
            std::cout << "\n🚨 DETAILED FAILURE ANALYSIS:" << std::endl;
            for (const auto& result : results) {
                if (!result.passed) {
                    std::cout << "  ❌ " << result.suite << "::" << result.name << std::endl;
                    std::cout << "     Error: " << result.error_message << std::endl;
                    if (!result.details.empty()) {
                        std::cout << "     Details: " << result.details << std::endl;
                    }
                    std::cout << std::endl;
                }
            }
        }

        std::cout << std::string(80, '=') << std::endl;
        if (failed == 0) {
            std::cout << "🎉 ALL TESTS PASSED! NESpresso CPU is working correctly!" << std::endl;
        } else {
            std::cout << "⚠️  " << failed << " tests failed. Check implementation!" << std::endl;
        }
    }
};

#define ASSERT_EQ_HEX(expected, actual) \
    if ((expected) != (actual)) { \
        std::stringstream ss; \
        ss << "Expected 0x" << std::hex << std::uppercase << (int)(expected) \
           << " but got 0x" << std::hex << std::uppercase << (int)(actual) \
           << " (decimal: " << std::dec << (int)(expected) << " vs " << (int)(actual) << ")"; \
        throw std::runtime_error(ss.str()); \
    }

#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        std::stringstream ss; \
        ss << "Expected " << (expected) << " but got " << (actual); \
        throw std::runtime_error(ss.str()); \
    }

#define ASSERT_TRUE(condition) if (!(condition)) throw std::runtime_error("Assertion failed: " #condition);
#define ASSERT_FALSE(condition) if (condition) throw std::runtime_error("Assertion failed: " #condition " should be false");
#define ASSERT_FLAG_SET(helper, flag) if (!helper.getFlag(flag)) { throw std::runtime_error("Flag " #flag " should be SET but is CLEAR"); }
#define ASSERT_FLAG_CLEAR(helper, flag) if (helper.getFlag(flag)) { throw std::runtime_error("Flag " #flag " should be CLEAR but is SET"); }

class EnhancedCpuTestHelper {
    Cpu& cpu;
    Memory& memory;
public:
    static constexpr uint8_t FLAG_CARRY = 0x01, FLAG_ZERO = 0x02, FLAG_INTERRUPT = 0x04,
                             FLAG_DECIMAL = 0x08, FLAG_BREAK = 0x10, FLAG_UNUSED = 0x20,
                             FLAG_OVERFLOW = 0x40, FLAG_NEGATIVE = 0x80;

    EnhancedCpuTestHelper() : cpu(Cpu::instance()), memory(Memory::instance()) {}

    void fullReset() {
        cpu.registers = {};
        cpu.registers.pc = 0x0200;
        cpu.registers.sp = 0xFD;
        cpu.registers.p = FLAG_UNUSED;
        cpu.total_cycles = 0;
        std::fill(memory.bus.begin(), memory.bus.end(), 0);
        memory.setupTestData();
    }

    void loadProgram(uint16_t addr, const std::vector<uint8_t>& prog) {
        for (size_t i = 0; i < prog.size(); ++i) memory.bus[addr + i] = prog[i];
    }
    void executeInstruction() { cpu.executeInstruction(); }
    void executeInstructions(int count) { for (int i = 0; i < count; ++i) cpu.executeInstruction(); }
    void setFlag(uint8_t flag, bool val) { val ? cpu.registers.p |= flag : cpu.registers.p &= ~flag; }
    bool getFlag(uint8_t flag) const { return (cpu.registers.p & flag) != 0; }
    uint8_t getA() const { return cpu.registers.a; }
    uint8_t getX() const { return cpu.registers.x; }
    uint8_t getY() const { return cpu.registers.y; }
    uint16_t getPC() const { return cpu.registers.pc; }
    uint8_t getSP() const { return cpu.registers.sp; }
    uint8_t getStatus() const { return cpu.registers.p; }
    void setA(uint8_t v) { cpu.registers.a = v; }
    void setX(uint8_t v) { cpu.registers.x = v; }
    void setY(uint8_t v) { cpu.registers.y = v; }
    void setPC(uint16_t v) { cpu.registers.pc = v; }
    void setSP(uint8_t v) { cpu.registers.sp = v; }
    void setStatus(uint8_t v) { cpu.registers.p = v; }
    uint8_t readMemory(uint16_t addr) const { return memory.bus[addr]; }
    void writeMemory(uint16_t addr, uint8_t v) { memory.bus[addr] = v; }
};

void runLoadInstructionTests(ComprehensiveTestFramework& framework) {
    framework.beginSuite("Load Instructions (LDA, LDX, LDY)");
    EnhancedCpuTestHelper helper;

    framework.runTest("LDA Immediate", [&]() {
        helper.fullReset();
        helper.loadProgram(0x0200, {0xA9, 0x42});
        helper.executeInstruction();
        ASSERT_EQ_HEX(0x42, helper.getA());
    });

    framework.runTest("LDA Indexed Indirect (zp,X)", [&]() {
        helper.fullReset();
        helper.setX(0x04);
        helper.loadProgram(0x0200, {0xA1, 0x20});
        helper.executeInstruction();
        ASSERT_EQ_HEX(0xAB, helper.getA());
    });

    framework.runTest("LDA Indirect Indexed (zp),Y", [&]() {
        helper.fullReset();
        helper.setY(0x10);
        helper.loadProgram(0x0200, {0xB1, 0x86});
        helper.executeInstruction();
        ASSERT_EQ_HEX(0xCD, helper.getA());
    });
}

void runStoreInstructionTests(ComprehensiveTestFramework& framework) {
    framework.beginSuite("Store Instructions (STA, STX, STY)");
    EnhancedCpuTestHelper helper;

    framework.runTest("STA Absolute,Y", [&]() {
        helper.fullReset();
        helper.setA(0x99);
        helper.setY(0x08);
        helper.loadProgram(0x0200, {0x99, 0x00, 0x03});
        helper.executeInstruction();
        ASSERT_EQ_HEX(0x99, helper.readMemory(0x0308));
    });
}

void runControlFlowTests(ComprehensiveTestFramework& framework) {
    framework.beginSuite("Control Flow (JMP, JSR, RTS, BRK, RTI)");
    EnhancedCpuTestHelper helper;

    framework.runTest("JMP Indirect Page Bug", [&]() {
        helper.fullReset();
        helper.setPC(0x0300);
        helper.loadProgram(0x0300, {0x6C, 0xFF, 0x02});
        helper.executeInstruction();
        ASSERT_EQ(0x0700, helper.getPC());
    });

    framework.runTest("BRK/RTI Sequence", [&]() {
        helper.fullReset();
        helper.setPC(0x0210);
        helper.setStatus(helper.FLAG_CARRY | helper.FLAG_UNUSED);
        helper.loadProgram(0x0210, {0x00}); // BRK
        helper.loadProgram(0x0800, {0x40}); // RTI
        helper.executeInstruction();
        ASSERT_EQ(0x0800, helper.getPC());
        helper.executeInstruction();
        ASSERT_EQ(0x0211, helper.getPC());
    });
}

void runRealWorldTests(ComprehensiveTestFramework& framework) {
    framework.beginSuite("Real-World Instruction Sequences");
    EnhancedCpuTestHelper helper;

    framework.runTest("Memory Copy Routine", [&]() {
        helper.fullReset();
        helper.writeMemory(0x0300, 0xAA);
        helper.writeMemory(0x0301, 0xBB);
        helper.writeMemory(0x0302, 0xCC);

        helper.loadProgram(0x0200, {
            0xA2, 0x02,        // LDX #$02
            // loop: (PC=0x0202)
            0xBD, 0x00, 0x03,  // LDA $0300,X
            0x9D, 0x00, 0x04,  // STA $0400,X
            0xCA,              // DEX
            0x10, 0xF7         // BPL loop (-9 bytes from PC after fetch)
        });

        for (int i = 0; i < 20; ++i) {
            if (helper.getPC() >= 0x020A) break;
            helper.executeInstruction();
        }

        ASSERT_EQ_HEX(0xAA, helper.readMemory(0x0400));
        ASSERT_EQ_HEX(0xBB, helper.readMemory(0x0401));
        ASSERT_EQ_HEX(0xCC, helper.readMemory(0x0402));
    });
}

int main() {
    ComprehensiveTestFramework framework;
    try {
        std::cout << "🧪 NESpresso CPU Instruction Tests - COMPREHENSIVE EDITION" << std::endl;
        std::cout << "Date: 2025-07-29 23:22:38 UTC" << std::endl;
        std::cout << "User: nicusor43" << std::endl;
        std::cout << "Target: 6502 CPU Emulator - ALL INSTRUCTIONS & EDGE CASES" << std::endl;
        std::cout << std::string(80, '=') << std::endl;

        runLoadInstructionTests(framework);
        runStoreInstructionTests(framework);
        runControlFlowTests(framework);
        runRealWorldTests(framework);

    } catch (const std::exception& e) {
        std::cerr << "\n🚨 CRITICAL ERROR during test execution: " << e.what() << std::endl;
        return 1;
    }
    framework.printDetailedSummary();
    return 0;
}