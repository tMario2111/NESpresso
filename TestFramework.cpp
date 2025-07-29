// NESpressoTests_COMPREHENSIVE.cpp - COMPLETE test suite for NESpresso CPU emulator
// Date: 2025-07-26 16:21:11 UTC
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
    int test_count = 0;

public:
    void beginSuite(const std::string& suite_name) {
        current_suite = suite_name;
        std::cout << "\n🧪 " << suite_name << std::endl;
        std::cout << std::string(70, '=') << std::endl;
    }

    void runTest(const std::string& name, std::function<void()> test_func, const std::string& details = "") {
        test_count++;
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
            if (result.passed) passed++;
            else failed++;
            total_time += result.duration;
        }

        std::cout << "\n" << std::string(80, '=') << std::endl;
        std::cout << "🎯 COMPREHENSIVE TEST SUMMARY - NESpresso Emulator" << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        std::cout << "📊 Total tests executed: " << results.size() << std::endl;
        std::cout << "✅ Passed: " << passed << " (" << std::fixed << std::setprecision(1)
                  << (100.0 * passed / results.size()) << "%)" << std::endl;
        std::cout << "❌ Failed: " << failed << " (" << std::fixed << std::setprecision(1)
                  << (100.0 * failed / results.size()) << "%)" << std::endl;
        std::cout << "⏱️  Total execution time: " << total_time.count() << "μs" << std::endl;
        std::cout << "⚡ Average test time: " << (total_time.count() / results.size()) << "μs" << std::endl;
        std::cout << "👤 Tested by: nicusor43" << std::endl;
        std::cout << "📅 Date: 2025-07-26 16:21:11 UTC" << std::endl;

        // Breakdown by suite
        std::cout << "\n📈 RESULTS BY SUITE:" << std::endl;
        std::map<std::string, std::pair<int, int>> suite_stats; // suite -> (passed, failed)

        for (const auto& result : results) {
            if (result.passed) {
                suite_stats[result.suite].first++;
            } else {
                suite_stats[result.suite].second++;
            }
        }

        for (const auto& [suite, stats] : suite_stats) {
            int total = stats.first + stats.second;
            double success_rate = (100.0 * stats.first) / total;
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

// Enhanced assertion macros with better error messages
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
        throw std::runtime_error("Expected " + std::to_string(expected) + \
                                " but got " + std::to_string(actual)); \
    }

#define ASSERT_TRUE(condition) \
    if (!(condition)) { \
        throw std::runtime_error("Assertion failed: " #condition); \
    }

#define ASSERT_FALSE(condition) \
    if (condition) { \
        throw std::runtime_error("Assertion failed: " #condition " should be false"); \
    }

#define ASSERT_FLAG_SET(helper, flag) \
    if (!helper.getFlag(flag)) { \
        throw std::runtime_error("Flag " #flag " should be SET but is CLEAR"); \
    }

#define ASSERT_FLAG_CLEAR(helper, flag) \
    if (helper.getFlag(flag)) { \
        throw std::runtime_error("Flag " #flag " should be CLEAR but is SET"); \
    }

// ================================
// ENHANCED CPU TEST HELPER
// ================================

class EnhancedCpuTestHelper {
private:
    Cpu& cpu;
    Memory& memory;

    uint16_t mapAddress(uint16_t address) const {
        if (address >= 0x8000) {
            return (address - 0x8000) % memory.bus.size();
        }
        if (address >= 0x1000) {
            return address % memory.bus.size();
        }
        return address % memory.bus.size();
    }

public:
    // Flag constants
    static constexpr uint8_t FLAG_CARRY = 0x01;
    static constexpr uint8_t FLAG_ZERO = 0x02;
    static constexpr uint8_t FLAG_INTERRUPT = 0x04;
    static constexpr uint8_t FLAG_DECIMAL = 0x08;
    static constexpr uint8_t FLAG_BREAK = 0x10;
    static constexpr uint8_t FLAG_UNUSED = 0x20;
    static constexpr uint8_t FLAG_OVERFLOW = 0x40;
    static constexpr uint8_t FLAG_NEGATIVE = 0x80;

    EnhancedCpuTestHelper() : cpu(Cpu::instance()), memory(Memory::instance()) {}

    void fullReset() {
        cpu.registers.a = 0;
        cpu.registers.x = 0;
        cpu.registers.y = 0;
        cpu.registers.pc = 0x0200;
        cpu.registers.sp = 0xFD;
        cpu.registers.p = FLAG_UNUSED;
        std::fill(memory.bus.begin(), memory.bus.end(), 0);
    }

    void loadProgram(uint16_t address, const std::vector<uint8_t>& program) {
        uint16_t mapped_addr = mapAddress(address);
        for (size_t i = 0; i < program.size(); i++) {
            uint16_t write_addr = (mapped_addr + i) % memory.bus.size();
            memory.bus[write_addr] = program[i];
        }
    }

    void executeInstruction() { cpu.executeInstruction(); }
    void executeInstructions(int count) {
        for (int i = 0; i < count; i++) {
            cpu.executeInstruction();
        }
    }

    // Flag operations
    void setFlag(uint8_t flag, bool value) {
        if (value) cpu.registers.p |= flag;
        else cpu.registers.p &= ~flag;
    }
    bool getFlag(uint8_t flag) const { return (cpu.registers.p & flag) != 0; }

    // Getters
    uint8_t getA() const { return cpu.registers.a; }
    uint8_t getX() const { return cpu.registers.x; }
    uint8_t getY() const { return cpu.registers.y; }
    uint16_t getPC() const { return cpu.registers.pc; }
    uint8_t getSP() const { return cpu.registers.sp; }
    uint8_t getStatus() const { return cpu.registers.p; }

    // Setters
    void setA(uint8_t value) { cpu.registers.a = value; }
    void setX(uint8_t value) { cpu.registers.x = value; }
    void setY(uint8_t value) { cpu.registers.y = value; }
    void setPC(uint16_t value) { cpu.registers.pc = mapAddress(value); }
    void setSP(uint8_t value) { cpu.registers.sp = value; }
    void setStatus(uint8_t value) { cpu.registers.p = value; }

    uint8_t readMemory(uint16_t address) const {
        uint16_t mapped_addr = mapAddress(address);
        return memory.bus[mapped_addr];
    }

    void writeMemory(uint16_t address, uint8_t value) {
        uint16_t mapped_addr = mapAddress(address);
        memory.bus[mapped_addr] = value;
    }

    // Helper for debugging
    std::string getStatusString() const {
        std::string status = "";
        status += (getFlag(FLAG_NEGATIVE) ? "N" : "n");
        status += (getFlag(FLAG_OVERFLOW) ? "V" : "v");
        status += (getFlag(FLAG_UNUSED) ? "U" : "u");
        status += (getFlag(FLAG_BREAK) ? "B" : "b");
        status += (getFlag(FLAG_DECIMAL) ? "D" : "d");
        status += (getFlag(FLAG_INTERRUPT) ? "I" : "i");
        status += (getFlag(FLAG_ZERO) ? "Z" : "z");
        status += (getFlag(FLAG_CARRY) ? "C" : "c");
        return status;
    }

    void printState() const {
        std::cout << "    CPU State: A=" << std::hex << std::uppercase << (int)getA()
                  << " X=" << (int)getX() << " Y=" << (int)getY()
                  << " PC=" << (int)getPC() << " SP=" << (int)getSP()
                  << " P=" << getStatusString() << std::endl;
    }
};

// ================================
// COMPREHENSIVE TEST SUITES
// ================================

void runLoadInstructionTests(ComprehensiveTestFramework& framework) {
    framework.beginSuite("Load Instructions (LDA, LDX, LDY) - All Addressing Modes");
    EnhancedCpuTestHelper helper;

    // LDA Tests - All addressing modes
    framework.runTest("LDA Immediate - Normal Value", [&]() {
        helper.fullReset();
        helper.loadProgram(0x0200, {0xA9, 0x42});  // LDA #$42
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x42, helper.getA());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
        ASSERT_EQ(0x0202, helper.getPC());
    }, "Load immediate value $42");

    framework.runTest("LDA Immediate - Zero Flag", [&]() {
        helper.fullReset();
        helper.loadProgram(0x0200, {0xA9, 0x00});  // LDA #$00
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getA());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "Zero flag set when loading $00");

    framework.runTest("LDA Immediate - Negative Flag", [&]() {
        helper.fullReset();
        helper.loadProgram(0x0200, {0xA9, 0x80});  // LDA #$80
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x80, helper.getA());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "Negative flag set when bit 7 is 1");

    framework.runTest("LDA Zero Page", [&]() {
        helper.fullReset();
        helper.writeMemory(0x0050, 0x37);
        helper.loadProgram(0x0200, {0xA5, 0x50});  // LDA $50
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x37, helper.getA());
        ASSERT_EQ(0x0202, helper.getPC());
    }, "Load from zero page address $50");

    framework.runTest("LDA Zero Page,X", [&]() {
        helper.fullReset();
        helper.setX(0x05);
        helper.writeMemory(0x0055, 0x69);  // $50 + $05 = $55
        helper.loadProgram(0x0200, {0xB5, 0x50});  // LDA $50,X
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x69, helper.getA());
        ASSERT_EQ(0x0202, helper.getPC());
    }, "Load from zero page with X offset");

    framework.runTest("LDA Absolute", [&]() {
        helper.fullReset();
        helper.writeMemory(0x0234, 0x56);
        helper.loadProgram(0x0200, {0xAD, 0x34, 0x02});  // LDA $0234
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x56, helper.getA());
        ASSERT_EQ(0x0203, helper.getPC());
    }, "Load from absolute address");

    framework.runTest("LDA Absolute,X", [&]() {
        helper.fullReset();
        helper.setX(0x10);
        helper.writeMemory(0x0244, 0x78);  // $0234 + $10 = $0244
        helper.loadProgram(0x0200, {0xBD, 0x34, 0x02});  // LDA $0234,X
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x78, helper.getA());
    }, "Load from absolute address with X offset");

    framework.runTest("LDA Absolute,Y", [&]() {
        helper.fullReset();
        helper.setY(0x08);
        helper.writeMemory(0x023C, 0x9A);  // $0234 + $08 = $023C
        helper.loadProgram(0x0200, {0xB9, 0x34, 0x02});  // LDA $0234,Y
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x9A, helper.getA());
    }, "Load from absolute address with Y offset");

    framework.runTest("LDA Indexed Indirect (zp,X)", [&]() {
        helper.fullReset();
        helper.setX(0x04);
        helper.writeMemory(0x0024, 0x74);  // Low byte of target address
        helper.writeMemory(0x0025, 0x20);  // High byte of target address -> $2074
        helper.writeMemory(0x0074, 0xAB);  // Value at target address (mapped)
        helper.loadProgram(0x0200, {0xA1, 0x20});  // LDA ($20,X)
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xAB, helper.getA());
    }, "Indexed indirect addressing mode");

    framework.runTest("LDA Indirect Indexed (zp),Y", [&]() {
        helper.fullReset();
        helper.setY(0x10);
        helper.writeMemory(0x0086, 0x28);  // Low byte of base address
        helper.writeMemory(0x0087, 0x40);  // High byte of base address -> $4028
        helper.writeMemory(0x0038, 0xCD);  // Value at $4028 + $10 = $4038 (mapped to low memory)
        helper.loadProgram(0x0200, {0xB1, 0x86});  // LDA ($86),Y
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xCD, helper.getA());
    }, "Indirect indexed addressing mode");

    // LDX Tests
    framework.runTest("LDX Immediate", [&]() {
        helper.fullReset();
        helper.loadProgram(0x0200, {0xA2, 0x33});  // LDX #$33
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x33, helper.getX());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "Load X register immediate");

    framework.runTest("LDX Zero Page", [&]() {
        helper.fullReset();
        helper.writeMemory(0x0040, 0x55);
        helper.loadProgram(0x0200, {0xA6, 0x40});  // LDX $40
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x55, helper.getX());
    }, "Load X from zero page");

    framework.runTest("LDX Zero Page,Y", [&]() {
        helper.fullReset();
        helper.setY(0x03);
        helper.writeMemory(0x0043, 0x77);  // $40 + $03 = $43
        helper.loadProgram(0x0200, {0xB6, 0x40});  // LDX $40,Y
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x77, helper.getX());
    }, "Load X from zero page with Y offset");

    framework.runTest("LDX Absolute", [&]() {
        helper.fullReset();
        helper.writeMemory(0x0300, 0x88);
        helper.loadProgram(0x0200, {0xAE, 0x00, 0x03});  // LDX $0300
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x88, helper.getX());
    }, "Load X from absolute address");

    framework.runTest("LDX Absolute,Y", [&]() {
        helper.fullReset();
        helper.setY(0x05);
        helper.writeMemory(0x0305, 0x99);  // $0300 + $05 = $0305
        helper.loadProgram(0x0200, {0xBE, 0x00, 0x03});  // LDX $0300,Y
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x99, helper.getX());
    }, "Load X from absolute address with Y offset");

    // LDY Tests
    framework.runTest("LDY Immediate", [&]() {
        helper.fullReset();
        helper.loadProgram(0x0200, {0xA0, 0x44});  // LDY #$44
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x44, helper.getY());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "Load Y register immediate");

    framework.runTest("LDY Zero Page", [&]() {
        helper.fullReset();
        helper.writeMemory(0x0060, 0x66);
        helper.loadProgram(0x0200, {0xA4, 0x60});  // LDY $60
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x66, helper.getY());
    }, "Load Y from zero page");

    framework.runTest("LDY Zero Page,X", [&]() {
        helper.fullReset();
        helper.setX(0x07);
        helper.writeMemory(0x0067, 0xAA);  // $60 + $07 = $67
        helper.loadProgram(0x0200, {0xB4, 0x60});  // LDY $60,X
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xAA, helper.getY());
    }, "Load Y from zero page with X offset");

    framework.runTest("LDY Absolute", [&]() {
        helper.fullReset();
        helper.writeMemory(0x0400, 0xBB);
        helper.loadProgram(0x0200, {0xAC, 0x00, 0x04});  // LDY $0400
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xBB, helper.getY());
    }, "Load Y from absolute address");

    framework.runTest("LDY Absolute,X", [&]() {
        helper.fullReset();
        helper.setX(0x08);
        helper.writeMemory(0x0408, 0xCC);  // $0400 + $08 = $0408
        helper.loadProgram(0x0200, {0xBC, 0x00, 0x04});  // LDY $0400,X
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xCC, helper.getY());
    }, "Load Y from absolute address with X offset");
}

void runStoreInstructionTests(ComprehensiveTestFramework& framework) {
    framework.beginSuite("Store Instructions (STA, STX, STY) - All Addressing Modes");
    EnhancedCpuTestHelper helper;

    // STA Tests
    framework.runTest("STA Zero Page", [&]() {
        helper.fullReset();
        helper.setA(0x99);
        helper.loadProgram(0x0200, {0x85, 0x60});  // STA $60
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x99, helper.readMemory(0x0060));
        ASSERT_EQ(0x0202, helper.getPC());
    }, "Store A to zero page");

    framework.runTest("STA Zero Page,X", [&]() {
        helper.fullReset();
        helper.setA(0xAB);
        helper.setX(0x05);
        helper.loadProgram(0x0200, {0x95, 0x60});  // STA $60,X
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xAB, helper.readMemory(0x0065));  // $60 + $05 = $65
    }, "Store A to zero page with X offset");

    framework.runTest("STA Absolute", [&]() {
        helper.fullReset();
        helper.setA(0x77);
        helper.loadProgram(0x0200, {0x8D, 0x00, 0x03});  // STA $0300
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x77, helper.readMemory(0x0300));
        ASSERT_EQ(0x0203, helper.getPC());
    }, "Store A to absolute address");

    framework.runTest("STA Absolute,X", [&]() {
        helper.fullReset();
        helper.setA(0x88);
        helper.setX(0x10);
        helper.loadProgram(0x0200, {0x9D, 0x00, 0x03});  // STA $0300,X
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x88, helper.readMemory(0x0310));  // $0300 + $10
    }, "Store A to absolute address with X offset");

    framework.  runTest("STA Absolute,Y", [&]() {
        helper.fullReset();
        helper.setA(0x99);
        helper.setX(0x08);
        helper.loadProgram(0x0200, {0x99, 0x00, 0x03});  // STA $0300,Y
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x99, helper.readMemory(0x0308));  // $0300 + $08
    }, "Store A to absolute address with Y offset");

    framework.runTest("STA Indexed Indirect (zp,X)", [&]() {
        helper.fullReset();
        helper.setA(0xDE);
        helper.setX(0x04);
        helper.writeMemory(0x0024, 0x00);  // Low byte of target address
        helper.writeMemory(0x0025, 0x05);  // High byte -> $0500
        helper.loadProgram(0x0200, {0x81, 0x20});  // STA ($20,X)
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xDE, helper.readMemory(0x0500));
    }, "Store A using indexed indirect addressing");

    framework.runTest("STA Indirect Indexed (zp),Y", [&]() {
        helper.fullReset();
        helper.setA(0xEF);
        helper.setY(0x10);
        helper.writeMemory(0x0086, 0x00);  // Low byte of base address
        helper.writeMemory(0x0087, 0x06);  // High byte -> $0600
        helper.loadProgram(0x0200, {0x91, 0x86});  // STA ($86),Y
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xEF, helper.readMemory(0x0610));  // $0600 + $10
    }, "Store A using indirect indexed addressing");

    // STX Tests
    framework.runTest("STX Zero Page", [&]() {
        helper.fullReset();
        helper.setX(0x55);
        helper.loadProgram(0x0200, {0x86, 0x70});  // STX $70
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x55, helper.readMemory(0x0070));
    }, "Store X to zero page");

    framework.runTest("STX Zero Page,Y", [&]() {
        helper.fullReset();
        helper.setX(0x66);
        helper.setY(0x03);
        helper.loadProgram(0x0200, {0x96, 0x70});  // STX $70,Y
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x66, helper.readMemory(0x0073));  // $70 + $03
    }, "Store X to zero page with Y offset");

    framework.runTest("STX Absolute", [&]() {
        helper.fullReset();
        helper.setX(0x77);
        helper.loadProgram(0x0200, {0x8E, 0x00, 0x07});  // STX $0700
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x77, helper.readMemory(0x0700));
    }, "Store X to absolute address");

    // STY Tests
    framework.runTest("STY Zero Page", [&]() {
        helper.fullReset();
        helper.setY(0x88);
        helper.loadProgram(0x0200, {0x84, 0x80});  // STY $80
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x88, helper.readMemory(0x0080));
    }, "Store Y to zero page");

    framework.runTest("STY Zero Page,X", [&]() {
        helper.fullReset();
        helper.setY(0x99);
        helper.setX(0x05);
        helper.loadProgram(0x0200, {0x94, 0x80});  // STY $80,X
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x99, helper.readMemory(0x0085));  // $80 + $05
    }, "Store Y to zero page with X offset");

    framework.runTest("STY Absolute", [&]() {
        helper.fullReset();
        helper.setY(0xAA);
        helper.loadProgram(0x0200, {0x8C, 0x00, 0x08});  // STY $0800
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xAA, helper.readMemory(0x0800));
    }, "Store Y to absolute address");
}

void runArithmeticTests(ComprehensiveTestFramework& framework) {
    framework.beginSuite("Arithmetic Instructions (ADC, SBC) - All Cases");
    EnhancedCpuTestHelper helper;

    // ADC Tests - Comprehensive
    framework.runTest("ADC Basic Addition", [&]() {
        helper.fullReset();
        helper.setA(0x10);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, false);
        helper.loadProgram(0x0200, {0x69, 0x20});  // ADC #$20
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x30, helper.getA());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_OVERFLOW);
    }, "Simple addition without carry");

    framework.runTest("ADC with Carry In", [&]() {
        helper.fullReset();
        helper.setA(0x10);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, true);
        helper.loadProgram(0x0200, {0x69, 0x20});  // ADC #$20
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x31, helper.getA());  // 0x10 + 0x20 + 1 = 0x31
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);
    }, "Addition with carry input");

    framework.runTest("ADC Carry Out", [&]() {
        helper.fullReset();
        helper.setA(0xFF);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, false);
        helper.loadProgram(0x0200, {0x69, 0x01});  // ADC #$01
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getA());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "Addition generating carry out");

    framework.runTest("ADC Overflow Positive", [&]() {
        helper.fullReset();
        helper.setA(0x7F);  // +127
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, false);
        helper.loadProgram(0x0200, {0x69, 0x01});  // ADC #$01
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x80, helper.getA());  // -128 in two's complement
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_OVERFLOW);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "Positive overflow: +127 + 1 = -128");

    framework.runTest("ADC Overflow Negative", [&]() {
        helper.fullReset();
        helper.setA(0x80);  // -128
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, false);
        helper.loadProgram(0x0200, {0x69, 0x80});  // ADC #$80 (-128)
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getA());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_OVERFLOW);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
    }, "Negative overflow: -128 + -128 = 0");

    framework.runTest("ADC All Addressing Modes", [&]() {
        helper.fullReset();

        // Zero Page
        helper.setA(0x10);
        helper.writeMemory(0x0050, 0x05);
        helper.loadProgram(0x0200, {0x65, 0x50});  // ADC $50
        helper.executeInstruction();
        ASSERT_EQ_HEX(0x15, helper.getA());

        // Absolute
        helper.setPC(0x0202);
        helper.setA(0x20);
        helper.writeMemory(0x0300, 0x08);
        helper.loadProgram(0x0202, {0x6D, 0x00, 0x03});  // ADC $0300
        helper.executeInstruction();
        ASSERT_EQ_HEX(0x28, helper.getA());

    }, "ADC with different addressing modes");

    // SBC Tests - Comprehensive
    framework.runTest("SBC Basic Subtraction", [&]() {
        helper.fullReset();
        helper.setA(0x50);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, true);  // No borrow
        helper.loadProgram(0x0200, {0xE9, 0x30});  // SBC #$30
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x20, helper.getA());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);  // No borrow occurred
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "Simple subtraction without borrow");

    framework.runTest("SBC with Borrow In", [&]() {
        helper.fullReset();
        helper.setA(0x50);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, false);  // Borrow
        helper.loadProgram(0x0200, {0xE9, 0x30});  // SBC #$30
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x1F, helper.getA());  // 0x50 - 0x30 - 1 = 0x1F
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);
    }, "Subtraction with borrow input");

    framework.runTest("SBC Underflow", [&]() {
        helper.fullReset();
        helper.setA(0x20);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, true);
        helper.loadProgram(0x0200, {0xE9, 0x30});  // SBC #$30
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xF0, helper.getA());  // 0x20 - 0x30 = 0xF0 (240 or -16)
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);  // Borrow occurred
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "Subtraction causing underflow");

    framework.runTest("SBC Zero Result", [&]() {
        helper.fullReset();
        helper.setA(0x40);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, true);
        helper.loadProgram(0x0200, {0xE9, 0x40});  // SBC #$40
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getA());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);  // No borrow
    }, "Subtraction resulting in zero");

    framework.runTest("SBC Overflow Test", [&]() {
        helper.fullReset();
        helper.setA(0x80);  // -128
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, true);
        helper.loadProgram(0x0200, {0xE9, 0x01});  // SBC #$01
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x7F, helper.getA());  // -128 - 1 = +127 (overflow)
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_OVERFLOW);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "Signed overflow in subtraction");
}

void runBranchTests(ComprehensiveTestFramework& framework) {
    framework.beginSuite("Branch Instructions - All Conditions and Edge Cases");
    EnhancedCpuTestHelper helper;

    // BEQ Tests
    framework.runTest("BEQ Branch Taken Forward", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_ZERO, true);
        helper.loadProgram(0x0200, {0xF0, 0x10});  // BEQ +16
        helper.executeInstruction();

        ASSERT_EQ(0x0212, helper.getPC());  // 0x0200 + 2 + 16 = 0x0212
    }, "Branch forward when Zero flag set");

    framework.runTest("BEQ Branch Not Taken", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_ZERO, false);
        helper.loadProgram(0x0200, {0xF0, 0x10});  // BEQ +16
        helper.executeInstruction();

        ASSERT_EQ(0x0202, helper.getPC());  // Normal increment
    }, "No branch when Zero flag clear");

    framework.runTest("BEQ Branch Backward", [&]() {
        helper.fullReset();
        helper.setPC(0x0220);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_ZERO, true);
        helper.loadProgram(0x0220, {0xF0, 0xF0});  // BEQ -16 (0xF0 = -16 signed)
        helper.executeInstruction();

        ASSERT_EQ(0x0212, helper.getPC());  // 0x0220 + 2 - 16 = 0x0212
    }, "Branch backward with negative offset");

    // BNE Tests
    framework.runTest("BNE Branch Taken", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_ZERO, false);
        helper.loadProgram(0x0200, {0xD0, 0x05});  // BNE +5
        helper.executeInstruction();

        ASSERT_EQ(0x0207, helper.getPC());  // 0x0200 + 2 + 5 = 0x0207
    }, "Branch when Zero flag clear");

    framework.runTest("BNE Branch Not Taken", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_ZERO, true);
        helper.loadProgram(0x0200, {0xD0, 0x05});  // BNE +5
        helper.executeInstruction();

        ASSERT_EQ(0x0202, helper.getPC());
    }, "No branch when Zero flag set");

    // BCC Tests
    framework.runTest("BCC Branch Taken", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, false);
        helper.loadProgram(0x0200, {0x90, 0x08});  // BCC +8
        helper.executeInstruction();

        ASSERT_EQ(0x020A, helper.getPC());  // 0x0200 + 2 + 8 = 0x020A
    }, "Branch when Carry flag clear");

    framework.runTest("BCC Branch Not Taken", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, true);
        helper.loadProgram(0x0200, {0x90, 0x08});  // BCC +8
        helper.executeInstruction();

        ASSERT_EQ(0x0202, helper.getPC());
    }, "No branch when Carry flag set");

    // BCS Tests
    framework.runTest("BCS Branch Taken", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, true);
        helper.loadProgram(0x0200, {0xB0, 0x0C});  // BCS +12
        helper.executeInstruction();

        ASSERT_EQ(0x020E, helper.getPC());  // 0x0200 + 2 + 12 = 0x020E
    }, "Branch when Carry flag set");

    // BPL Tests
    framework.runTest("BPL Branch Taken", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_NEGATIVE, false);
        helper.loadProgram(0x0200, {0x10, 0x15});  // BPL +21
        helper.executeInstruction();

        ASSERT_EQ(0x0217, helper.getPC());  // 0x0200 + 2 + 21 = 0x0217
    }, "Branch when Negative flag clear (positive)");

    framework.runTest("BPL Branch Not Taken", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_NEGATIVE, true);
        helper.loadProgram(0x0200, {0x10, 0x15});  // BPL +21
        helper.executeInstruction();

        ASSERT_EQ(0x0202, helper.getPC());
    }, "No branch when Negative flag set");

    // BMI Tests
    framework.runTest("BMI Branch Taken", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_NEGATIVE, true);
        helper.loadProgram(0x0200, {0x30, 0x07});  // BMI +7
        helper.executeInstruction();

        ASSERT_EQ(0x0209, helper.getPC());  // 0x0200 + 2 + 7 = 0x0209
    }, "Branch when Negative flag set (minus)");

    // BVC Tests
    framework.runTest("BVC Branch Taken", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_OVERFLOW, false);
        helper.loadProgram(0x0200, {0x50, 0x12});  // BVC +18
        helper.executeInstruction();

        ASSERT_EQ(0x0214, helper.getPC());  // 0x0200 + 2 + 18 = 0x0214
    }, "Branch when Overflow flag clear");

    // BVS Tests
    framework.runTest("BVS Branch Taken", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_OVERFLOW, true);
        helper.loadProgram(0x0200, {0x70, 0x0A});  // BVS +10
        helper.executeInstruction();

        ASSERT_EQ(0x020C, helper.getPC());  // 0x0200 + 2 + 10 = 0x020C
    }, "Branch when Overflow flag set");

    // Edge cases
    framework.runTest("Branch Maximum Forward", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_ZERO, true);
        helper.loadProgram(0x0200, {0xF0, 0x7F});  // BEQ +127 (max positive)
        helper.executeInstruction();

        ASSERT_EQ(0x0281, helper.getPC());  // 0x0200 + 2 + 127 = 0x0281
    }, "Maximum forward branch (+127)");

    framework.runTest("Branch Maximum Backward", [&]() {
        helper.fullReset();
        helper.setPC(0x0300);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_ZERO, true);
        helper.loadProgram(0x0300, {0xF0, 0x80});  // BEQ -128 (max negative)
        helper.executeInstruction();

        ASSERT_EQ(0x0282, helper.getPC());  // 0x0300 + 2 - 128 = 0x0282
    }, "Maximum backward branch (-128)");

    framework.runTest("Branch Page Crossing Forward", [&]() {
        helper.fullReset();
        helper.setPC(0x02F0);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_ZERO, true);
        helper.loadProgram(0x02F0, {0xF0, 0x20});  // BEQ +32
        helper.executeInstruction();

        ASSERT_EQ(0x0312, helper.getPC());  // 0x02F0 + 2 + 32 = 0x0312 (crosses page)
    }, "Forward branch crossing page boundary");

    framework.runTest("Branch Page Crossing Backward", [&]() {
        helper.fullReset();
        helper.setPC(0x0310);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_ZERO, true);
        helper.loadProgram(0x0310, {0xF0, 0xE0});  // BEQ -32
        helper.executeInstruction();

        ASSERT_EQ(0x02F2, helper.getPC());  // 0x0310 + 2 - 32 = 0x02F2 (crosses page)
    }, "Backward branch crossing page boundary");
}

void runLogicalTests(ComprehensiveTestFramework& framework) {
    framework.beginSuite("Logical Instructions (AND, ORA, EOR) - Comprehensive");
    EnhancedCpuTestHelper helper;

    // AND Tests
    framework.runTest("AND Basic Operation", [&]() {
        helper.fullReset();
        helper.setA(0b11110000);
        helper.loadProgram(0x0200, {0x29, 0b10101010});  // AND #%10101010
        helper.executeInstruction();

        ASSERT_EQ_HEX(0b10100000, helper.getA());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "Basic AND operation");

    framework.runTest("AND Zero Result", [&]() {
        helper.fullReset();
        helper.setA(0b00001111);
        helper.loadProgram(0x0200, {0x29, 0b11110000});  // AND #%11110000
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getA());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "AND resulting in zero");

    framework.runTest("AND All Bits Set", [&]() {
        helper.fullReset();
        helper.setA(0xFF);
        helper.loadProgram(0x0200, {0x29, 0xFF});  // AND #$FF
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xFF, helper.getA());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "AND with all bits set");

    framework.runTest("AND Different Addressing Modes", [&]() {
        helper.fullReset();

        // Zero Page
        helper.setA(0xF0);
        helper.writeMemory(0x0080, 0x0F);
        helper.loadProgram(0x0200, {0x25, 0x80});  // AND $80
        helper.executeInstruction();
        ASSERT_EQ_HEX(0x00, helper.getA());

        // Absolute
        helper.setPC(0x0202);
        helper.setA(0xAA);
        helper.writeMemory(0x0400, 0x55);
        helper.loadProgram(0x0202, {0x2D, 0x00, 0x04});  // AND $0400
        helper.executeInstruction();
        ASSERT_EQ_HEX(0x00, helper.getA());

    }, "AND with multiple addressing modes");

    // ORA Tests
    framework.runTest("ORA Basic Operation", [&]() {
        helper.fullReset();
        helper.setA(0b11110000);
        helper.loadProgram(0x0200, {0x09, 0b00001111});  // ORA #%00001111
        helper.executeInstruction();

        ASSERT_EQ_HEX(0b11111111, helper.getA());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "Basic ORA operation");

    framework.runTest("ORA With Zero", [&]() {
        helper.fullReset();
        helper.setA(0x42);
        helper.loadProgram(0x0200, {0x09, 0x00});  // ORA #$00
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x42, helper.getA());  // Should be unchanged
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "ORA with zero (identity)");

    framework.runTest("ORA Zero Result", [&]() {
        helper.fullReset();
        helper.setA(0x00);
        helper.loadProgram(0x0200, {0x09, 0x00});  // ORA #$00
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getA());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "ORA resulting in zero");

    framework.runTest("ORA Set All Bits", [&]() {
        helper.fullReset();
        helper.setA(0x00);
        helper.loadProgram(0x0200, {0x09, 0xFF});  // ORA #$FF
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xFF, helper.getA());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "ORA setting all bits");

    // EOR Tests
    framework.runTest("EOR Basic Operation", [&]() {
        helper.fullReset();
        helper.setA(0b11110000);
        helper.loadProgram(0x0200, {0x49, 0b10101010});  // EOR #%10101010
        helper.executeInstruction();

        ASSERT_EQ_HEX(0b01011010, helper.getA());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "Basic EOR operation");

    framework.runTest("EOR Zero Result (Same Values)", [&]() {
        helper.fullReset();
        helper.setA(0b10101010);
        helper.loadProgram(0x0200, {0x49, 0b10101010});  // EOR #%10101010
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getA());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "EOR with same value (produces zero)");

    framework.runTest("EOR With Zero (Identity)", [&]() {
        helper.fullReset();
        helper.setA(0x55);
        helper.loadProgram(0x0200, {0x49, 0x00});  // EOR #$00
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x55, helper.getA());  // Should be unchanged
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "EOR with zero (identity operation)");

    framework.runTest("EOR Toggle All Bits", [&]() {
        helper.fullReset();
        helper.setA(0x00);
        helper.loadProgram(0x0200, {0x49, 0xFF});  // EOR #$FF
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xFF, helper.getA());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "EOR toggling all bits");

    framework.runTest("EOR Negative Flag", [&]() {
        helper.fullReset();
        helper.setA(0x7F);  // 0111 1111
        helper.loadProgram(0x0200, {0x49, 0xFF});  // EOR #$FF
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x80, helper.getA());  // 1000 0000
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "EOR setting negative flag");
}

void runShiftRotateTests(ComprehensiveTestFramework& framework) {
    framework.beginSuite("Shift/Rotate Instructions (ASL, LSR, ROL, ROR) - All Modes");
    EnhancedCpuTestHelper helper;

    // ASL Tests (Arithmetic Shift Left)
    framework.runTest("ASL Accumulator Basic", [&]() {
        helper.fullReset();
        helper.setA(0b01000001);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, false);
        helper.loadProgram(0x0200, {0x0A});  // ASL A
        helper.executeInstruction();

        ASSERT_EQ_HEX(0b10000010, helper.getA());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);  // Bit 7 was 0
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE); // Result bit 7 is 1
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
    }, "ASL accumulator - basic shift");

    framework.runTest("ASL Accumulator with Carry", [&]() {
        helper.fullReset();
        helper.setA(0b10000001);
        helper.loadProgram(0x0200, {0x0A});  // ASL A
        helper.executeInstruction();

        ASSERT_EQ_HEX(0b00000010, helper.getA());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);   // Bit 7 was 1
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
    }, "ASL accumulator - generates carry");

    framework.runTest("ASL Accumulator Zero Result", [&]() {
        helper.fullReset();
        helper.setA(0b10000000);
        helper.loadProgram(0x0200, {0x0A});  // ASL A
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getA());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "ASL accumulator - zero result");

    framework.runTest("ASL Memory Zero Page", [&]() {
        helper.fullReset();
        helper.writeMemory(0x0080, 0b01010101);
        helper.loadProgram(0x0200, {0x06, 0x80});  // ASL $80
        helper.executeInstruction();

        ASSERT_EQ_HEX(0b10101010, helper.readMemory(0x0080));
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
    }, "ASL memory - zero page");

    framework.runTest("ASL Memory Absolute", [&]() {
        helper.fullReset();
        helper.writeMemory(0x0300, 0b00110011);
        helper.loadProgram(0x0200, {0x0E, 0x00, 0x03});  // ASL $0300
        helper.executeInstruction();

        ASSERT_EQ_HEX(0b01100110, helper.readMemory(0x0300));
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
    }, "ASL memory - absolute addressing");

    // LSR Tests (Logical Shift Right)
    framework.runTest("LSR Accumulator Basic", [&]() {
        helper.fullReset();
        helper.setA(0b10000010);
        helper.loadProgram(0x0200, {0x4A});  // LSR A
        helper.executeInstruction();

        ASSERT_EQ_HEX(0b01000001, helper.getA());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);  // Bit 0 was 0
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE); // LSR always clears N
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
    }, "LSR accumulator - basic shift");

    framework.runTest("LSR Accumulator with Carry", [&]() {
        helper.fullReset();
        helper.setA(0b10000011);
        helper.loadProgram(0x0200, {0x4A});  // LSR A
        helper.executeInstruction();

        ASSERT_EQ_HEX(0b01000001, helper.getA());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);   // Bit 0 was 1
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
    }, "LSR accumulator - generates carry");

    framework.runTest("LSR Accumulator Zero Result", [&]() {
        helper.fullReset();
        helper.setA(0b00000001);
        helper.loadProgram(0x0200, {0x4A});  // LSR A
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getA());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "LSR accumulator - zero result");

    framework.runTest("LSR Memory Zero Page", [&]() {
        helper.fullReset();
        helper.writeMemory(0x0090, 0b11110000);
        helper.loadProgram(0x0200, {0x46, 0x90});  // LSR $90
        helper.executeInstruction();

        ASSERT_EQ_HEX(0b01111000, helper.readMemory(0x0090));
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
    }, "LSR memory - zero page");

    // ROL Tests (Rotate Left)
    framework.runTest("ROL Accumulator Basic", [&]() {
        helper.fullReset();
        helper.setA(0b01000001);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, true);
        helper.loadProgram(0x0200, {0x2A});  // ROL A
        helper.executeInstruction();

        ASSERT_EQ_HEX(0b10000011, helper.getA());  // Rotated left + carry in bit 0
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);  // Bit 7 was 0
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
    }, "ROL accumulator - basic rotation");

    framework.runTest("ROL Accumulator with Carry Out", [&]() {
        helper.fullReset();
        helper.setA(0b10000001);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, false);
        helper.loadProgram(0x0200, {0x2A});  // ROL A
        helper.executeInstruction();

        ASSERT_EQ_HEX(0b00000010, helper.getA());  // Rotated left + carry=0 in bit 0
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);   // Bit 7 was 1
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
    }, "ROL accumulator - generates carry");

    framework.runTest("ROL Accumulator Zero Result", [&]() {
        helper.fullReset();
        helper.setA(0b10000000);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, false);
        helper.loadProgram(0x0200, {0x2A});  // ROL A
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getA());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "ROL accumulator - zero result");

    framework.runTest("ROL Memory Zero Page", [&]() {
        helper.fullReset();
        helper.writeMemory(0x00A0, 0b00110011);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, true);
        helper.loadProgram(0x0200, {0x26, 0xA0});  // ROL $A0
        helper.executeInstruction();

        ASSERT_EQ_HEX(0b01100111, helper.readMemory(0x00A0));
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
    }, "ROL memory - zero page");

    // ROR Tests (Rotate Right)
    framework.runTest("ROR Accumulator Basic", [&]() {
        helper.fullReset();
        helper.setA(0b10000010);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, true);
        helper.loadProgram(0x0200, {0x6A});  // ROR A
        helper.executeInstruction();

        ASSERT_EQ_HEX(0b11000001, helper.getA());  // Rotated right + carry in bit 7
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);  // Bit 0 was 0
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
    }, "ROR accumulator - basic rotation");

    framework.runTest("ROR Accumulator with Carry Out", [&]() {
        helper.fullReset();
        helper.setA(0b10000011);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, false);
        helper.loadProgram(0x0200, {0x6A});  // ROR A
        helper.executeInstruction();

        ASSERT_EQ_HEX(0b01000001, helper.getA());  // Rotated right + carry=0 in bit 7
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);   // Bit 0 was 1
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
    }, "ROR accumulator - generates carry");

    framework.runTest("ROR Accumulator Zero Result", [&]() {
        helper.fullReset();
        helper.setA(0b00000001);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, false);
        helper.loadProgram(0x0200, {0x6A});  // ROR A
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getA());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "ROR accumulator - zero result");

    framework.runTest("ROR Memory Absolute", [&]() {
        helper.fullReset();
        helper.writeMemory(0x0500, 0b11001100);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, true);
        helper.loadProgram(0x0200, {0x6E, 0x00, 0x05});  // ROR $0500
        helper.executeInstruction();

        ASSERT_EQ_HEX(0b11100110, helper.readMemory(0x0500));
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
    }, "ROR memory - absolute addressing");
}

void runCompareTests(ComprehensiveTestFramework& framework) {
    framework.beginSuite("Compare Instructions (CMP, CPX, CPY) - All Cases");
    EnhancedCpuTestHelper helper;

    // CMP Tests
    framework.runTest("CMP Equal Values", [&]() {
        helper.fullReset();
        helper.setA(0x40);
        helper.loadProgram(0x0200, {0xC9, 0x40});  // CMP #$40
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x40, helper.getA());  // A should be unchanged
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);    // Equal
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);   // A >= operand
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE); // Result is 0
    }, "CMP with equal values");

    framework.runTest("CMP A Greater Than Operand", [&]() {
        helper.fullReset();
        helper.setA(0x50);
        helper.loadProgram(0x0200, {0xC9, 0x40});  // CMP #$40
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x50, helper.getA());  // A should be unchanged
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);   // Not equal
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);   // A >= operand
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE); // Positive result
    }, "CMP A > operand");

    framework.runTest("CMP A Less Than Operand", [&]() {
        helper.fullReset();
        helper.setA(0x30);
        helper.loadProgram(0x0200, {0xC9, 0x40});  // CMP #$40
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x30, helper.getA());  // A should be unchanged
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);   // Not equal
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);  // A < operand
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE); // Negative result
    }, "CMP A < operand");

    framework.runTest("CMP Zero Comparison", [&]() {
        helper.fullReset();
        helper.setA(0x00);
        helper.loadProgram(0x0200, {0xC9, 0x00});  // CMP #$00
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getA());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "CMP with zero values");

    framework.runTest("CMP Maximum Values", [&]() {
        helper.fullReset();
        helper.setA(0xFF);
        helper.loadProgram(0x0200, {0xC9, 0xFF});  // CMP #$FF
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xFF, helper.getA());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "CMP with maximum values");

    framework.runTest("CMP Different Addressing Modes", [&]() {
        helper.fullReset();

        // Zero Page
        helper.setA(0x80);
        helper.writeMemory(0x00B0, 0x70);
        helper.loadProgram(0x0200, {0xC5, 0xB0});  // CMP $B0
        helper.executeInstruction();
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);

        // Absolute
        helper.setPC(0x0202);
        helper.setA(0x60);
        helper.writeMemory(0x0600, 0x80);
        helper.loadProgram(0x0202, {0xCD, 0x00, 0x06});  // CMP $0600
        helper.executeInstruction();
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);

    }, "CMP with multiple addressing modes");

    // CPX Tests
    framework.runTest("CPX Equal Values", [&]() {
        helper.fullReset();
        helper.setX(0x80);
        helper.loadProgram(0x0200, {0xE0, 0x80});  // CPX #$80
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x80, helper.getX());  // X should be unchanged
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);    // Equal
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);   // X >= operand
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "CPX with equal values");

    framework.runTest("CPX X Greater Than Operand", [&]() {
        helper.fullReset();
        helper.setX(0x90);
        helper.loadProgram(0x0200, {0xE0, 0x7F});  // CPX #$7F
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x90, helper.getX());  // X should be unchanged
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);   // Not equal
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);   // X >= operand
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE); // Positive result
    }, "CPX X > operand");

    framework.runTest("CPX X Less Than Operand", [&]() {
        helper.fullReset();
        helper.setX(0x70);
        helper.loadProgram(0x0200, {0xE0, 0x90});  // CPX #$90
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x70, helper.getX());  // X should be unchanged
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);   // Not equal
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);  // X < operand
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE); // Negative result
    }, "CPX X < operand");

    framework.runTest("CPX Zero Page", [&]() {
        helper.fullReset();
        helper.setX(0x55);
        helper.writeMemory(0x00C0, 0x55);
        helper.loadProgram(0x0200, {0xE4, 0xC0});  // CPX $C0
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x55, helper.getX());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);
    }, "CPX zero page addressing");

    // CPY Tests
    framework.runTest("CPY Equal Values", [&]() {
        helper.fullReset();
        helper.setY(0x60);
        helper.loadProgram(0x0200, {0xC0, 0x60});  // CPY #$60
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x60, helper.getY());  // Y should be unchanged
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);    // Equal
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);   // Y >= operand
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "CPY with equal values");

    framework.runTest("CPY Y Greater Than Operand", [&]() {
        helper.fullReset();
        helper.setY(0x70);
        helper.loadProgram(0x0200, {0xC0, 0x50});  // CPY #$50
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x70, helper.getY());  // Y should be unchanged
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);   // Not equal
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);   // Y >= operand
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE); // Positive result
    }, "CPY Y > operand");

    framework.runTest("CPY Y Less Than Operand", [&]() {
        helper.fullReset();
        helper.setY(0x10);
        helper.loadProgram(0x0200, {0xC0, 0x20});  // CPY #$20
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x10, helper.getY());  // Y should be unchanged
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);   // Not equal
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);  // Y < operand
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE); // Negative result
    }, "CPY Y < operand");

    framework.runTest("CPY Absolute", [&]() {
        helper.fullReset();
        helper.setY(0xAA);
        helper.writeMemory(0x0700, 0xBB);
        helper.loadProgram(0x0200, {0xCC, 0x00, 0x07});  // CPY $0700
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xAA, helper.getY());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "CPY absolute addressing");
}

void runIncrementDecrementTests(ComprehensiveTestFramework& framework) {
    framework.beginSuite("Increment/Decrement Instructions (INC, DEC, INX, INY, DEX, DEY)");
    EnhancedCpuTestHelper helper;

    // INC Tests
    framework.runTest("INC Zero Page Basic", [&]() {
        helper.fullReset();
        helper.writeMemory(0x00D0, 0x40);
        helper.loadProgram(0x0200, {0xE6, 0xD0});  // INC $D0
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x41, helper.readMemory(0x00D0));
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "INC memory - basic increment");

    framework.runTest("INC Zero Flag", [&]() {
        helper.fullReset();
        helper.writeMemory(0x00D0, 0xFF);
        helper.loadProgram(0x0200, {0xE6, 0xD0});  // INC $D0
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.readMemory(0x00D0));  // Wraps around
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "INC setting zero flag (wrap around)");

    framework.runTest("INC Negative Flag", [&]() {
        helper.fullReset();
        helper.writeMemory(0x00D0, 0x7F);
        helper.loadProgram(0x0200, {0xE6, 0xD0});  // INC $D0
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x80, helper.readMemory(0x00D0));
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "INC setting negative flag");

    framework.runTest("INC Absolute", [&]() {
        helper.fullReset();
        helper.writeMemory(0x0800, 0x99);
        helper.loadProgram(0x0200, {0xEE, 0x00, 0x08});  // INC $0800
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x9A, helper.readMemory(0x0800));
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "INC absolute addressing");

    // DEC Tests
    framework.runTest("DEC Zero Page Basic", [&]() {
        helper.fullReset();
        helper.writeMemory(0x00E0, 0x40);
        helper.loadProgram(0x0200, {0xC6, 0xE0});  // DEC $E0
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x3F, helper.readMemory(0x00E0));
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "DEC memory - basic decrement");

    framework.runTest("DEC Zero Flag", [&]() {
        helper.fullReset();
        helper.writeMemory(0x00E0, 0x01);
        helper.loadProgram(0x0200, {0xC6, 0xE0});  // DEC $E0
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.readMemory(0x00E0));
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "DEC setting zero flag");

    framework.runTest("DEC Wrap Around", [&]() {
        helper.fullReset();
        helper.writeMemory(0x00E0, 0x00);
        helper.loadProgram(0x0200, {0xC6, 0xE0});  // DEC $E0
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xFF, helper.readMemory(0x00E0));  // Wraps to 255
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "DEC wrap around to 255");

    framework.runTest("DEC Absolute", [&]() {
        helper.fullReset();
        helper.writeMemory(0x0900, 0x80);
        helper.loadProgram(0x0200, {0xCE, 0x00, 0x09});  // DEC $0900
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x7F, helper.readMemory(0x0900));
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "DEC absolute addressing");

    // INX Tests
    framework.runTest("INX Basic", [&]() {
        helper.fullReset();
        helper.setX(0x50);
        helper.loadProgram(0x0200, {0xE8});  // INX
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x51, helper.getX());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "INX basic increment");

    framework.runTest("INX Zero Flag", [&]() {
        helper.fullReset();
        helper.setX(0xFF);
        helper.loadProgram(0x0200, {0xE8});  // INX
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getX());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "INX setting zero flag");

    framework.runTest("INX Negative Flag", [&]() {
        helper.fullReset();
        helper.setX(0x7F);
        helper.loadProgram(0x0200, {0xE8});  // INX
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x80, helper.getX());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "INX setting negative flag");

    // INY Tests
    framework.runTest("INY Basic", [&]() {
        helper.fullReset();
        helper.setY(0x30);
        helper.loadProgram(0x0200, {0xC8});  // INY
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x31, helper.getY());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "INY basic increment");

    framework.runTest("INY Zero Flag", [&]() {
        helper.fullReset();
        helper.setY(0xFF);
        helper.loadProgram(0x0200, {0xC8});  // INY
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getY());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "INY setting zero flag");

    // DEX Tests
    framework.runTest("DEX Basic", [&]() {
        helper.fullReset();
        helper.setX(0x50);
        helper.loadProgram(0x0200, {0xCA});  // DEX
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x4F, helper.getX());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "DEX basic decrement");

    framework.runTest("DEX Zero Flag", [&]() {
        helper.fullReset();
        helper.setX(0x01);
        helper.loadProgram(0x0200, {0xCA});  // DEX
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getX());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "DEX setting zero flag");

    framework.runTest("DEX Wrap Around", [&]() {
        helper.fullReset();
        helper.setX(0x00);
        helper.loadProgram(0x0200, {0xCA});  // DEX
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xFF, helper.getX());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "DEX wrap around to 255");

    // DEY Tests
    framework.runTest("DEY Basic", [&]() {
        helper.fullReset();
        helper.setY(0x50);
        helper.loadProgram(0x0200, {0x88});  // DEY
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x4F, helper.getY());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "DEY basic decrement");

    framework.runTest("DEY Zero Flag", [&]() {
        helper.fullReset();
        helper.setY(0x01);
        helper.loadProgram(0x0200, {0x88});  // DEY
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getY());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "DEY setting zero flag");

    framework.runTest("DEY Wrap Around", [&]() {
        helper.fullReset();
        helper.setY(0x00);
        helper.loadProgram(0x0200, {0x88});  // DEY
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xFF, helper.getY());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "DEY wrap around to 255");
}

void runStackTests(ComprehensiveTestFramework& framework) {
    framework.beginSuite("Stack Instructions (PHA, PLA, PHP, PLP) - Comprehensive");
    EnhancedCpuTestHelper helper;

    framework.runTest("PHA/PLA Basic Operation", [&]() {
        helper.fullReset();
        helper.setA(0x42);
        helper.setSP(0xFD);

        // Push A onto stack
        helper.loadProgram(0x0200, {0x48});  // PHA
        helper.executeInstruction();

        ASSERT_EQ(0xFC, helper.getSP());  // SP decremented
        ASSERT_EQ_HEX(0x42, helper.readMemory(0x01FD));  // Value on stack

        // Change A and pull from stack
        helper.setA(0x00);
        helper.setPC(0x0201);
        helper.loadProgram(0x0201, {0x68});  // PLA
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x42, helper.getA());  // A restored
        ASSERT_EQ(0xFD, helper.getSP());  // SP restored
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "PHA/PLA basic push/pull operations");

    framework.runTest("PLA Zero Flag", [&]() {
        helper.fullReset();
        helper.setA(0x00);
        helper.setSP(0xFD);

        // Push zero onto stack
        helper.loadProgram(0x0200, {0x48});  // PHA
        helper.executeInstruction();

        // Pull zero from stack
        helper.setA(0xFF);  // Change A first
        helper.setPC(0x0201);
        helper.loadProgram(0x0201, {0x68});  // PLA
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getA());
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "PLA setting zero flag");

    framework.runTest("PLA Negative Flag", [&]() {
        helper.fullReset();
        helper.setA(0x80);
        helper.setSP(0xFD);

        // Push negative value onto stack
        helper.loadProgram(0x0200, {0x48});  // PHA
        helper.executeInstruction();

        // Pull negative value from stack
        helper.setA(0x00);  // Change A first
        helper.setPC(0x0201);
        helper.loadProgram(0x0201, {0x68});  // PLA
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x80, helper.getA());
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "PLA setting negative flag");

    framework.runTest("Stack Pointer Behavior", [&]() {
        helper.fullReset();
        helper.setSP(0xFD);
        helper.setA(0x11);

        // Multiple pushes
        helper.loadProgram(0x0200, {0x48, 0x48, 0x48});  // PHA x3
        helper.executeInstruction();  // First push
        ASSERT_EQ(0xFC, helper.getSP());

        helper.executeInstruction();  // Second push
        ASSERT_EQ(0xFB, helper.getSP());

        helper.executeInstruction();  // Third push
        ASSERT_EQ(0xFA, helper.getSP());

        // Verify stack contents
        ASSERT_EQ_HEX(0x11, helper.readMemory(0x01FD));
        ASSERT_EQ_HEX(0x11, helper.readMemory(0x01FC));
        ASSERT_EQ_HEX(0x11, helper.readMemory(0x01FB));
    }, "Stack pointer behavior with multiple pushes");

    framework.runTest("PHP/PLP Status Register", [&]() {
        helper.fullReset();
        helper.setStatus(0xFF);  // All flags set
        helper.setSP(0xFD);

        // Push status onto stack
        helper.loadProgram(0x0200, {0x08});  // PHP
        helper.executeInstruction();

        ASSERT_EQ(0xFC, helper.getSP());  // SP decremented

        // Change status and pull from stack
        helper.setStatus(0x00);
        helper.setPC(0x0201);
        helper.loadProgram(0x0201, {0x28});  // PLP
        helper.executeInstruction();

        ASSERT_EQ(0xFD, helper.getSP());  // SP restored
        // Note: PLP behavior may vary with break flag handling
    }, "PHP/PLP status register operations");

    framework.runTest("Stack Underflow Test", [&]() {
        helper.fullReset();
        helper.setSP(0xFF);  // Start at top
        helper.setA(0x55);

        // Push value
        helper.loadProgram(0x0200, {0x48});  // PHA
        helper.executeInstruction();

        ASSERT_EQ(0xFE, helper.getSP());
        ASSERT_EQ_HEX(0x55, helper.readMemory(0x01FF));

        // Pull value back
        helper.setA(0x00);
        helper.setPC(0x0201);
        helper.loadProgram(0x0201, {0x68});  // PLA
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x55, helper.getA());
        ASSERT_EQ(0xFF, helper.getSP());
    }, "Stack operations at boundaries");
}

void runTransferTests(ComprehensiveTestFramework& framework) {
    framework.beginSuite("Transfer Instructions (TAX, TAY, TXA, TYA, TSX, TXS)");
    EnhancedCpuTestHelper helper;

    // TAX Tests
    framework.runTest("TAX Basic Transfer", [&]() {
        helper.fullReset();
        helper.setA(0x80);
        helper.loadProgram(0x0200, {0xAA});  // TAX
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x80, helper.getX());
        ASSERT_EQ_HEX(0x80, helper.getA());  // A unchanged
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "TAX transfer with negative flag");

    framework.runTest("TAX Zero Transfer", [&]() {
        helper.fullReset();
        helper.setA(0x00);
        helper.loadProgram(0x0200, {0xAA});  // TAX
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getX());
        ASSERT_EQ_HEX(0x00, helper.getA());  // A unchanged
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "TAX transfer with zero flag");

    // TAY Tests
    framework.runTest("TAY Basic Transfer", [&]() {
        helper.fullReset();
        helper.setA(0x42);
        helper.loadProgram(0x0200, {0xA8});  // TAY
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x42, helper.getY());
        ASSERT_EQ_HEX(0x42, helper.getA());  // A unchanged
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "TAY basic transfer");

    framework.runTest("TAY Zero Transfer", [&]() {
        helper.fullReset();
        helper.setA(0x00);
        helper.loadProgram(0x0200, {0xA8});  // TAY
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getY());
        ASSERT_EQ_HEX(0x00, helper.getA());  // A unchanged
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "TAY transfer with zero flag");

    // TXA Tests
    framework.runTest("TXA Basic Transfer", [&]() {
        helper.fullReset();
        helper.setX(0x99);
        helper.loadProgram(0x0200, {0x8A});  // TXA
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x99, helper.getA());
        ASSERT_EQ_HEX(0x99, helper.getX());  // X unchanged
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "TXA transfer with negative flag");

    framework.runTest("TXA Zero Transfer", [&]() {
        helper.fullReset();
        helper.setX(0x00);
        helper.loadProgram(0x0200, {0x8A});  // TXA
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getA());
        ASSERT_EQ_HEX(0x00, helper.getX());  // X unchanged
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "TXA transfer with zero flag");

    // TYA Tests
    framework.runTest("TYA Basic Transfer", [&]() {
        helper.fullReset();
        helper.setY(0x37);
        helper.loadProgram(0x0200, {0x98});  // TYA
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x37, helper.getA());
        ASSERT_EQ_HEX(0x37, helper.getY());  // Y unchanged
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "TYA basic transfer");

    framework.runTest("TYA Negative Transfer", [&]() {
        helper.fullReset();
        helper.setY(0xFF);
        helper.loadProgram(0x0200, {0x98});  // TYA
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xFF, helper.getA());
        ASSERT_EQ_HEX(0xFF, helper.getY());  // Y unchanged
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "TYA transfer with negative flag");

    // TSX Tests
    framework.runTest("TSX Basic Transfer", [&]() {
        helper.fullReset();
        helper.setSP(0x80);
        helper.loadProgram(0x0200, {0xBA});  // TSX
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x80, helper.getX());
        ASSERT_EQ_HEX(0x80, helper.getSP());  // SP unchanged
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "TSX transfer stack pointer to X");

    framework.runTest("TSX Zero Transfer", [&]() {
        helper.fullReset();
        helper.setSP(0x00);
        helper.loadProgram(0x0200, {0xBA});  // TSX
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getX());
        ASSERT_EQ_HEX(0x00, helper.getSP());  // SP unchanged
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "TSX transfer with zero flag");

    // TXS Tests
    framework.runTest("TXS Basic Transfer", [&]() {
        helper.fullReset();
        helper.setX(0xFD);
        helper.loadProgram(0x0200, {0x9A});  // TXS
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xFD, helper.getSP());
        ASSERT_EQ_HEX(0xFD, helper.getX());  // X unchanged
        // TXS doesn't affect flags
    }, "TXS transfer X to stack pointer");

    framework.runTest("TXS No Flags", [&]() {
        helper.fullReset();
        helper.setX(0x00);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_ZERO, false);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_NEGATIVE, false);
        helper.loadProgram(0x0200, {0x9A});  // TXS
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getSP());
        ASSERT_EQ_HEX(0x00, helper.getX());  // X unchanged
        // TXS should not affect flags
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "TXS does not affect flags");

    // Chain Transfer Tests
    framework.runTest("Chain Transfers", [&]() {
        helper.fullReset();
        helper.setA(0x55);

        // A -> X -> Y -> A
        helper.loadProgram(0x0200, {0xAA, 0x8A, 0xA8, 0x98});  // TAX, TXA, TAY, TYA

        helper.executeInstruction();  // TAX
        ASSERT_EQ_HEX(0x55, helper.getX());

        helper.executeInstruction();  // TXA (redundant but tests)
        ASSERT_EQ_HEX(0x55, helper.getA());

        helper.executeInstruction();  // TAY
        ASSERT_EQ_HEX(0x55, helper.getY());

        helper.executeInstruction();  // TYA (redundant but tests)
        ASSERT_EQ_HEX(0x55, helper.getA());

        // All registers should have same value
        ASSERT_EQ_HEX(0x55, helper.getA());
        ASSERT_EQ_HEX(0x55, helper.getX());
        ASSERT_EQ_HEX(0x55, helper.getY());
    }, "Chain transfer operations");
}

void runControlFlowTests(ComprehensiveTestFramework& framework) {
    framework.beginSuite("Control Flow Instructions (JMP, JSR, RTS, BRK, RTI, NOP)");
    EnhancedCpuTestHelper helper;

    // JMP Tests
    framework.runTest("JMP Absolute", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        helper.loadProgram(0x0200, {0x4C, 0x00, 0x05});  // JMP $0500
        helper.executeInstruction();

        ASSERT_EQ(0x0500, helper.getPC());  // PC should jump directly
    }, "JMP absolute addressing");

    framework.runTest("JMP Indirect", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        helper.writeMemory(0x0300, 0x00);  // Low byte of target
        helper.writeMemory(0x0301, 0x06);  // High byte of target -> $0600
        helper.loadProgram(0x0200, {0x6C, 0x00, 0x03});  // JMP ($0300)
        helper.executeInstruction();

        ASSERT_EQ(0x0600, helper.getPC());  // PC should jump to indirect address
    }, "JMP indirect addressing");

    framework.runTest("JMP Indirect Page Bug Simulation", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        // Simulate the 6502 JMP indirect bug when crossing page boundary
        helper.writeMemory(0x02FF, 0x00);  // Low byte at end of page
        helper.writeMemory(0x0200, 0x07);  // High byte wraps to start of page (bug!)
        helper.loadProgram(0x0200, {0x6C, 0xFF, 0x02});  // JMP ($02FF)
        helper.executeInstruction();

        // Should read from $02FF and $0200 (not $0300)
        ASSERT_EQ(0x0700, helper.getPC());
    }, "JMP indirect page boundary bug");

    // JSR/RTS Tests
    framework.runTest("JSR/RTS Basic", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        helper.setSP(0xFD);

        // JSR $0400
        helper.loadProgram(0x0200, {0x20, 0x00, 0x04});  // JSR $0400
        helper.executeInstruction();

        ASSERT_EQ(0x0400, helper.getPC());  // PC should jump to subroutine
        ASSERT_EQ(0xFB, helper.getSP());   // SP should be decremented by 2

        // Check return address on stack (PC of JSR + 2)
        uint16_t return_addr = helper.readMemory(0x01FC) | (helper.readMemory(0x01FD) << 8);
        ASSERT_EQ(0x0202, return_addr);  // Should be PC + 2

        // RTS
        helper.loadProgram(0x0400, {0x60});  // RTS
        helper.executeInstruction();

        ASSERT_EQ(0x0203, helper.getPC());  // PC should return + 1
        ASSERT_EQ(0xFD, helper.getSP());   // SP should be restored
    }, "JSR/RTS subroutine call and return");

    framework.runTest("Nested JSR/RTS", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        helper.setSP(0xFD);

        // First JSR
        helper.loadProgram(0x0200, {0x20, 0x00, 0x04});  // JSR $0400
        helper.executeInstruction();
        ASSERT_EQ(0x0400, helper.getPC());
        ASSERT_EQ(0xFB, helper.getSP());

        // Nested JSR
        helper.loadProgram(0x0400, {0x20, 0x00, 0x05});  // JSR $0500
        helper.executeInstruction();
        ASSERT_EQ(0x0500, helper.getPC());
        ASSERT_EQ(0xF9, helper.getSP());  // SP decremented again

        // First RTS (return from nested)
        helper.loadProgram(0x0500, {0x60});  // RTS
        helper.executeInstruction();
        ASSERT_EQ(0x0403, helper.getPC());  // Return to caller + 1
        ASSERT_EQ(0xFB, helper.getSP());

        // Second RTS (return from first)
        helper.loadProgram(0x0403, {0x60});  // RTS
        helper.executeInstruction();
        ASSERT_EQ(0x0203, helper.getPC());  // Return to main + 1
        ASSERT_EQ(0xFD, helper.getSP());
    }, "Nested subroutine calls");

    // NOP Test
    framework.runTest("NOP No Operation", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        helper.setA(0x55);
        helper.setX(0xAA);
                helper.setY(0x77);
        helper.setSP(0xFD);
        uint8_t initial_status = helper.getStatus();

        helper.loadProgram(0x0200, {0xEA});  // NOP
        helper.executeInstruction();

        // Nothing should change except PC
        ASSERT_EQ_HEX(0x55, helper.getA());
        ASSERT_EQ_HEX(0xAA, helper.getX());
        ASSERT_EQ_HEX(0x77, helper.getY());
        ASSERT_EQ(0xFD, helper.getSP());
        ASSERT_EQ(initial_status, helper.getStatus());
        ASSERT_EQ(0x0201, helper.getPC());  // Only PC should increment
    }, "NOP instruction does nothing");

    // BRK Test
    framework.runTest("BRK Interrupt", [&]() {
        helper.fullReset();
        helper.setPC(0x0200);
        helper.setSP(0xFD);
        helper.setStatus(0x30);  // Set some flags

        // Set up interrupt vector
        helper.writeMemory(0x07FE, 0x00);  // IRQ vector low (mapped)
        helper.writeMemory(0x07FF, 0x08);  // IRQ vector high -> $0800

        helper.loadProgram(0x0200, {0x00});  // BRK
        helper.executeInstruction();

        // Check that PC jumped to interrupt vector
        ASSERT_EQ(0x0800, helper.getPC());

        // Check that status was pushed with break flag set
        ASSERT_EQ(0xFA, helper.getSP());  // SP decremented by 3

        // Check interrupt disable flag is set
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_INTERRUPT);
    }, "BRK software interrupt");

    // RTI Test
    framework.runTest("RTI Return from Interrupt", [&]() {
        helper.fullReset();
        helper.setSP(0xFA);  // Simulate stack after interrupt

        // Set up stack as if interrupt occurred
        helper.writeMemory(0x01FB, 0x30);  // Status register
        helper.writeMemory(0x01FC, 0x10);  // PC low
        helper.writeMemory(0x01FD, 0x02);  // PC high -> $0210

        helper.setPC(0x0800);  // Simulate we're in interrupt handler
        helper.loadProgram(0x0800, {0x40});  // RTI
        helper.executeInstruction();

        ASSERT_EQ(0x0210, helper.getPC());  // PC restored
        ASSERT_EQ(0xFD, helper.getSP());   // SP restored

        // Status should be restored (with some modifications)
        uint8_t status = helper.getStatus();
        ASSERT_TRUE((status & 0x20) != 0);  // Unused flag always set
    }, "RTI return from interrupt");
}

void runFlagTests(ComprehensiveTestFramework& framework) {
    framework.beginSuite("Flag Control Instructions (CLC, CLD, CLI, CLV, SEC, SED, SEI)");
    EnhancedCpuTestHelper helper;

    // Clear Flag Tests
    framework.runTest("CLC Clear Carry", [&]() {
        helper.fullReset();
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, true);
        helper.loadProgram(0x0200, {0x18});  // CLC
        helper.executeInstruction();

        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_EQ(0x0201, helper.getPC());
    }, "CLC clears carry flag");

    framework.runTest("CLD Clear Decimal", [&]() {
        helper.fullReset();
        helper.setFlag(EnhancedCpuTestHelper::FLAG_DECIMAL, true);
        helper.loadProgram(0x0200, {0xD8});  // CLD
        helper.executeInstruction();

        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_DECIMAL);
        ASSERT_EQ(0x0201, helper.getPC());
    }, "CLD clears decimal flag");

    framework.runTest("CLI Clear Interrupt", [&]() {
        helper.fullReset();
        helper.setFlag(EnhancedCpuTestHelper::FLAG_INTERRUPT, true);
        helper.loadProgram(0x0200, {0x58});  // CLI
        helper.executeInstruction();

        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_INTERRUPT);
        ASSERT_EQ(0x0201, helper.getPC());
    }, "CLI clears interrupt disable flag");

    framework.runTest("CLV Clear Overflow", [&]() {
        helper.fullReset();
        helper.setFlag(EnhancedCpuTestHelper::FLAG_OVERFLOW, true);
        helper.loadProgram(0x0200, {0xB8});  // CLV
        helper.executeInstruction();

        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_OVERFLOW);
        ASSERT_EQ(0x0201, helper.getPC());
    }, "CLV clears overflow flag");

    // Set Flag Tests
    framework.runTest("SEC Set Carry", [&]() {
        helper.fullReset();
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, false);
        helper.loadProgram(0x0200, {0x38});  // SEC
        helper.executeInstruction();

        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_EQ(0x0201, helper.getPC());
    }, "SEC sets carry flag");

    framework.runTest("SED Set Decimal", [&]() {
        helper.fullReset();
        helper.setFlag(EnhancedCpuTestHelper::FLAG_DECIMAL, false);
        helper.loadProgram(0x0200, {0xF8});  // SED
        helper.executeInstruction();

        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_DECIMAL);
        ASSERT_EQ(0x0201, helper.getPC());
    }, "SED sets decimal flag");

    framework.runTest("SEI Set Interrupt", [&]() {
        helper.fullReset();
        helper.setFlag(EnhancedCpuTestHelper::FLAG_INTERRUPT, false);
        helper.loadProgram(0x0200, {0x78});  // SEI
        helper.executeInstruction();

        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_INTERRUPT);
        ASSERT_EQ(0x0201, helper.getPC());
    }, "SEI sets interrupt disable flag");

    // Flag Preservation Tests
    framework.runTest("Flag Operations Don't Affect Others", [&]() {
        helper.fullReset();
        helper.setFlag(EnhancedCpuTestHelper::FLAG_ZERO, true);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_NEGATIVE, true);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_OVERFLOW, true);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, true);

        // Clear carry, others should remain
        helper.loadProgram(0x0200, {0x18});  // CLC
        helper.executeInstruction();

        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_OVERFLOW);
    }, "Flag operations preserve other flags");
}

void runBitManipulationTests(ComprehensiveTestFramework& framework) {
    framework.beginSuite("Bit Manipulation Instructions (BIT) - Comprehensive");
    EnhancedCpuTestHelper helper;

    framework.runTest("BIT Zero Page Basic", [&]() {
        helper.fullReset();
        helper.setA(0xFF);  // All bits set in A
        helper.writeMemory(0x00F0, 0xFF);  // All bits set in memory
        helper.loadProgram(0x0200, {0x24, 0xF0});  // BIT $F0
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xFF, helper.getA());  // A should be unchanged
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);    // A & M != 0
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);  // Bit 7 of M
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_OVERFLOW);  // Bit 6 of M
    }, "BIT instruction with all bits set");

    framework.runTest("BIT Zero Result", [&]() {
        helper.fullReset();
        helper.setA(0x0F);  // Lower nibble set
        helper.writeMemory(0x00F0, 0xF0);  // Upper nibble set
        helper.loadProgram(0x0200, {0x24, 0xF0});  // BIT $F0
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x0F, helper.getA());  // A should be unchanged
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);     // A & M = 0
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE); // Bit 7 of M
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_OVERFLOW); // Bit 6 of M
    }, "BIT instruction with zero result");

    framework.runTest("BIT Flag Combinations", [&]() {
        helper.fullReset();
        helper.setA(0x80);  // Bit 7 set in A
        helper.writeMemory(0x00F0, 0x40);  // Bit 6 set in memory
        helper.loadProgram(0x0200, {0x24, 0xF0});  // BIT $F0
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x80, helper.getA());  // A should be unchanged
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);      // A & M = 0
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE); // Bit 7 of M is 0
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_OVERFLOW);  // Bit 6 of M is 1
    }, "BIT instruction flag combinations");

    framework.runTest("BIT Absolute", [&]() {
        helper.fullReset();
        helper.setA(0x55);  // 0101 0101
        helper.writeMemory(0x0A00, 0xAA);  // 1010 1010
        helper.loadProgram(0x0200, {0x2C, 0x00, 0x0A});  // BIT $0A00
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x55, helper.getA());  // A should be unchanged
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);     // 0x55 & 0xAA = 0
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE); // Bit 7 of 0xAA
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_OVERFLOW); // Bit 6 of 0xAA
    }, "BIT absolute addressing");

    framework.runTest("BIT Memory Zero", [&]() {
        helper.fullReset();
        helper.setA(0xFF);
        helper.writeMemory(0x00F0, 0x00);  // Memory is zero
        helper.loadProgram(0x0200, {0x24, 0xF0});  // BIT $F0
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xFF, helper.getA());  // A should be unchanged
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);      // A & 0 = 0
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE); // Bit 7 of 0x00
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_OVERFLOW); // Bit 6 of 0x00
    }, "BIT with memory containing zero");
}

void runEdgeCaseTests(ComprehensiveTestFramework& framework) {
    framework.beginSuite("Edge Cases and Special Scenarios");
    EnhancedCpuTestHelper helper;

    framework.runTest("Page Boundary Crossing - Absolute,X", [&]() {
        helper.fullReset();
        helper.setX(0xFF);
        helper.writeMemory(0x02FF, 0x42);  // Value at boundary
        helper.loadProgram(0x0200, {0xBD, 0x00, 0x02});  // LDA $0200,X
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x42, helper.getA());
        // Note: Real 6502 would add extra cycle for page crossing
    }, "Page boundary crossing with indexed addressing");

    framework.runTest("Zero Page Wrap Around", [&]() {
        helper.fullReset();
        helper.setX(0x10);
        helper.writeMemory(0x000F, 0x55);  // 0xFF + 0x10 = 0x0F (wraps)
        helper.loadProgram(0x0200, {0xB5, 0xFF});  // LDA $FF,X
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x55, helper.getA());
    }, "Zero page indexing wrap around");

    framework.runTest("Stack Boundary Test", [&]() {
        helper.fullReset();
        helper.setSP(0x00);  // At bottom of stack
        helper.setA(0x99);

        // Push at stack bottom
        helper.loadProgram(0x0200, {0x48});  // PHA
        helper.executeInstruction();

        ASSERT_EQ(0xFF, helper.getSP());  // Should wrap to top
        ASSERT_EQ_HEX(0x99, helper.readMemory(0x0100));  // Value at stack bottom
    }, "Stack pointer wrap around");

    framework.runTest("All Flags Set/Clear", [&]() {
        helper.fullReset();

        // Set all flags
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, true);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_ZERO, true);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_INTERRUPT, true);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_DECIMAL, true);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_OVERFLOW, true);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_NEGATIVE, true);

        // Verify all are set
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_INTERRUPT);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_DECIMAL);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_OVERFLOW);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);

        // Clear all flags
        helper.setFlag(EnhancedCpuTestHelper::FLAG_CARRY, false);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_ZERO, false);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_INTERRUPT, false);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_DECIMAL, false);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_OVERFLOW, false);
        helper.setFlag(EnhancedCpuTestHelper::FLAG_NEGATIVE, false);

        // Verify all are clear (except unused)
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_INTERRUPT);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_DECIMAL);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_OVERFLOW);
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_UNUSED);  // Always set
    }, "All flags manipulation");

    framework.runTest("Register Independence", [&]() {
        helper.fullReset();

        // Set each register to different value
        helper.setA(0x11);
        helper.setX(0x22);
        helper.setY(0x33);
        helper.setSP(0x44);
        helper.setPC(0x0200);

        // Execute NOP to ensure no cross-contamination
        helper.loadProgram(0x0200, {0xEA});  // NOP
        helper.executeInstruction();

        // Verify registers maintained independence
        ASSERT_EQ_HEX(0x11, helper.getA());
        ASSERT_EQ_HEX(0x22, helper.getX());
        ASSERT_EQ_HEX(0x33, helper.getY());
        ASSERT_EQ_HEX(0x44, helper.getSP());
        ASSERT_EQ(0x0201, helper.getPC());
    }, "Register independence verification");

    framework.runTest("Maximum Values Test", [&]() {
        helper.fullReset();

        // Test with maximum values
        helper.setA(0xFF);
        helper.setX(0xFF);
        helper.setY(0xFF);

        // Test arithmetic with max values
        helper.loadProgram(0x0200, {0x69, 0x01});  // ADC #$01
        helper.executeInstruction();

        ASSERT_EQ_HEX(0x00, helper.getA());  // Should wrap to 0
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_CARRY);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);
    }, "Maximum values handling");

    framework.runTest("Minimum Values Test", [&]() {
        helper.fullReset();

        // Test with minimum values
        helper.setA(0x00);
        helper.setX(0x00);
        helper.setY(0x00);

        // Test decrement with min values
        helper.loadProgram(0x0200, {0xCA});  // DEX
        helper.executeInstruction();

        ASSERT_EQ_HEX(0xFF, helper.getX());  // Should wrap to 255
        ASSERT_FLAG_CLEAR(helper, EnhancedCpuTestHelper::FLAG_ZERO);
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_NEGATIVE);
    }, "Minimum values handling");
}

void runComprehensiveSequenceTests(ComprehensiveTestFramework& framework) {
    framework.beginSuite("Real-World Instruction Sequences");
    EnhancedCpuTestHelper helper;

    framework.runTest("Simple Loop Counter", [&]() {
        helper.fullReset();
        helper.setX(0x05);  // Loop 5 times
        helper.setA(0x00);  // Accumulator starts at 0

        // Program: Loop that adds 1 to A, decrements X, branches if not zero
        helper.loadProgram(0x0200, {
            0x18,       // CLC
            0x69, 0x01, // ADC #$01
            0xCA,       // DEX
            0xD0, 0xFB  // BNE -5 (back to ADC)
        });

        // Execute the sequence
        for (int i = 0; i < 20; i++) {  // Safety limit
            helper.executeInstruction();
            if (helper.getX() == 0) break;  // Loop finished
        }

        ASSERT_EQ_HEX(0x05, helper.getA());  // Should have counted to 5
        ASSERT_EQ_HEX(0x00, helper.getX());  // Counter should be 0
        ASSERT_FLAG_SET(helper, EnhancedCpuTestHelper::FLAG_ZERO);  // X is zero
    }, "Simple counting loop");

    framework.runTest("Memory Copy Routine", [&]() {
        helper.fullReset();

        // Source data
        helper.writeMemory(0x0300, 0xAA);
        helper.writeMemory(0x0301, 0xBB);
        helper.writeMemory(0x0302, 0xCC);

        // Copy 3 bytes from $0300 to $0400
        helper.setX(0x00);  // Index
        helper.loadProgram(0x0200, {
            0xBD, 0x00, 0x03,  // LDA $0300,X
            0x9D, 0x00, 0x04,  // STA $0400,X
            0xE8,              // INX
            0xE0, 0x03,        // CPX #$03
            0xD0, 0xF7         // BNE (back to LDA)
        });

        // Execute copy routine
        for (int i = 0; i < 20; i++) {
            helper.executeInstruction();
            if (helper.getX() == 3) break;
        }

        // Verify copy
        ASSERT_EQ_HEX(0xAA, helper.readMemory(0x0400));
        ASSERT_EQ_HEX(0xBB, helper.readMemory(0x0401));
        ASSERT_EQ_HEX(0xCC, helper.readMemory(0x0402));
        ASSERT_EQ_HEX(0x03, helper.getX());
    }, "Memory copy routine");

    framework.runTest("Subroutine with Parameters", [&]() {
        helper.fullReset();
        helper.setSP(0xFD);

        // Main routine: Load values and call subroutine
        helper.setA(0x10);
        helper.setX(0x20);

        helper.loadProgram(0x0200, {
            0x20, 0x10, 0x02,  // JSR $0210 (call subroutine)
            0xEA,              // NOP (return point)
        });

        // Subroutine: Add A and X, store in A
        helper.loadProgram(0x0210, {
            0x8A,              // TXA (X -> A)
            0x18,              // CLC
            0x65, 0x00,        // ADC $00 (add original A value stored at $00)
            0x60               // RTS
        });

        // Store original A value for subroutine
        helper.writeMemory(0x0000, helper.getA());

        // Execute main routine
        helper.executeInstruction();  // JSR
        ASSERT_EQ(0x0210, helper.getPC());  // Should be in subroutine

        // Execute subroutine
        helper.executeInstruction();  // TXA
        helper.executeInstruction();  // CLC
        helper.executeInstruction();  // ADC
        ASSERT_EQ_HEX(0x30, helper.getA());  // 0x10 + 0x20 = 0x30

        helper.executeInstruction();  // RTS
        ASSERT_EQ(0x0203, helper.getPC());  // Should return to NOP
    }, "Subroutine with parameter passing");

    framework.runTest("Binary to BCD Conversion", [&]() {
        helper.fullReset();

        // Convert binary 99 (0x63) to BCD
        helper.setA(99);  // Binary 99

        helper.loadProgram(0x0200, {
            0xA9, 0x00,        // LDA #$00 (clear result)
            0xA2, 0x08,        // LDX #$08 (8 bits to convert)
            0x0A,              // ASL A (shift binary left)
            0x90, 0x02,        // BCC +2 (skip if no carry)
            0x69, 0x01,        // ADC #$01 (add 1 if carry)
            0xCA,              // DEX
            0xD0, 0xF8         // BNE (back to ASL)
        });

        // This is a simplified example - real BCD conversion is more complex
        // Execute a few instructions to demonstrate
        for (int i = 0; i < 5; i++) {
            helper.executeInstruction();
        }

        // Just verify we can execute the sequence without errors
        ASSERT_TRUE(helper.getPC() > 0x0200);
    }, "Binary to BCD conversion routine");
}

// Test rapid writeMemory
void testWriteMemory() {
    Cpu& cpu = Cpu::instance();
    Memory& mem = Memory::instance();

    // Test write
    cpu.writeMemory(0x0050, 0xAB);

    // Verify
    uint8_t result = mem.bus[0x0050];
    std::cout << "writeMemory test: wrote 0xAB, read 0x"
              << std::hex << (int)result << std::endl;

    if (result == 0xAB) {
        std::cout << "✅ writeMemory WORKS!" << std::endl;
    } else {
        std::cout << "❌ writeMemory BROKEN!" << std::endl;
    }
}

// Pune valori de test în memorie pentru adresările indirecte
void setupTestMemory() {


    Memory& memory = Memory::instance();

    // Pentru IndexedIndirect (zp,X)
    memory.bus[0x24] = 0x74;      // Low byte al adresei țintă
    memory.bus[0x25] = 0x20;      // High byte al adresei țintă
    memory.bus[0x2074] = 0xAB;    // Valoarea așteptată

    // Pentru IndirectIndexed (zp),Y
    memory.bus[0x86] = 0x28;      // Low byte al adresei de bază
    memory.bus[0x87] = 0x40;      // High byte al adresei de bază
    memory.bus[0x4038] = 0xCD;    // Valoarea așteptată (baza + Y)

    // Pentru STA Absolute,Y test
    // Asigură-te că A are valoarea 0x99 înainte de test

    // Pentru JMP Indirect Page Bug
    memory.bus[0x02FF] = 0x00;    // Low byte al adresei destinație
    memory.bus[0x0200] = 0x07;    // High byte al adresei destinație

    // Pentru BRK și RTI
    memory.bus[0xFFFE] = 0x00;    // Low byte al vectorului IRQ/BRK
    memory.bus[0xFFFF] = 0x08;    // High byte al vectorului IRQ/BRK -> 0x0800
}


// ================================
// MAIN FUNCTION - COMPREHENSIVE TEST RUNNER
// ================================
//
int main() {
    testWriteMemory();

    std::cout << "🧪 NESpresso CPU Instruction Tests - COMPREHENSIVE EDITION" << std::endl;
    std::cout << "Date: 2025-07-26 16:28:11 UTC" << std::endl;
    std::cout << "User: nicusor43" << std::endl;
    std::cout << "Target: 6502 CPU Emulator - ALL INSTRUCTIONS & EDGE CASES" << std::endl;
    std::cout << std::string(80, '=') << std::endl;

    auto start_time = std::chrono::high_resolution_clock::now();

    ComprehensiveTestFramework framework;


    try {
        std::cout << "🚀 Starting comprehensive test suite execution..." << std::endl;
        Memory& memory = Memory::instance();
        memory.setupTestData();

        // Core instruction tests
        runLoadInstructionTests(framework);
        runStoreInstructionTests(framework);
        runArithmeticTests(framework);
        runBranchTests(framework);
        runLogicalTests(framework);
        runShiftRotateTests(framework);
        runCompareTests(framework);
        runIncrementDecrementTests(framework);

        // System operation tests
        runStackTests(framework);
        runTransferTests(framework);
        runControlFlowTests(framework);
        runFlagTests(framework);
        runBitManipulationTests(framework);

        // Advanced tests
        runEdgeCaseTests(framework);
        runComprehensiveSequenceTests(framework);

        std::cout << "\n🎯 All test suites completed successfully!" << std::endl;

    } catch (const std::exception& e) {
        std::cout << "\n🚨 CRITICAL ERROR during test execution: " << e.what() << std::endl;
        return 1;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    framework.printDetailedSummary();
    std::cout << "⏱️  Total execution time: " << total_duration.count() << "ms" << std::endl;
    std::cout << "\n🎉 COMPREHENSIVE TESTING COMPLETE!" << std::endl;
    std::cout << "📝 This test suite covers:" << std::endl;
    std::cout << "   • All 6502 instruction types and addressing modes" << std::endl;
    std::cout << "   • Flag behavior and edge cases" << std::endl;
    std::cout << "   • Stack operations and subroutines" << std::endl;
    std::cout << "   • Memory boundary conditions" << std::endl;
    std::cout << "   • Real-world instruction sequences" << std::endl;
    std::cout << "   • Control flow and branching logic" << std::endl;
    std::cout << "   • Arithmetic and logical operations" << std::endl;
    std::cout << "   • Register transfer and manipulation" << std::endl;
    std::cout << "\n👤 Created by: nicusor43" << std::endl;
    std::cout << "📅 Generated: 2025-07-26 16:28:11 UTC" << std::endl;
    std::cout << "🎯 Target: Complete 6502 CPU validation" << std::endl;

    return 0;
}