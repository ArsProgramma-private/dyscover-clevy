// Unit Test: LayoutRegistry
// Feature: 006-language-resource-optimization (Phase 2)
// Purpose: Verify LayoutRegistry registers and retrieves layouts correctly

#include "layouts/LayoutRegistry.h"
#include "Keys.h"
#include <iostream>
#include <cassert>
#include <vector>

using namespace Dyscover;

// Mock layout provider for testing
class TestLayout : public ILayoutProvider {
public:
    TestLayout(const char* name, const char* lang) 
        : name_(name), lang_(lang) {}
    
    const std::vector<KeyTranslationEntry>& GetEntries() const override {
        return entries_;
    }
    
    const char* GetName() const override { return name_; }
    const char* GetLanguage() const override { return lang_; }

private:
    const char* name_;
    const char* lang_;
    std::vector<KeyTranslationEntry> entries_;
};

void test_singleton() {
    std::cout << "Test: Singleton instance..." << std::endl;
    
    LayoutRegistry& instance1 = LayoutRegistry::Instance();
    LayoutRegistry& instance2 = LayoutRegistry::Instance();
    
    assert(&instance1 == &instance2);
    std::cout << "  ✓ Singleton returns same instance" << std::endl;
}

void test_register_layout() {
    std::cout << "Test: Register layout..." << std::endl;
    
    LayoutRegistry& registry = LayoutRegistry::Instance();
    TestLayout layout("test_dutch", "nl");
    
    registry.Register("test_dutch", &layout);
    
    const ILayoutProvider* retrieved = registry.GetLayout("test_dutch");
    assert(retrieved != nullptr);
    assert(retrieved == &layout);
    std::cout << "  ✓ Layout registered and retrieved successfully" << std::endl;
}

void test_get_nonexistent_layout() {
    std::cout << "Test: Get nonexistent layout..." << std::endl;
    
    LayoutRegistry& registry = LayoutRegistry::Instance();
    const ILayoutProvider* result = registry.GetLayout("nonexistent");
    
    assert(result == nullptr);
    std::cout << "  ✓ Returns nullptr for nonexistent layout" << std::endl;
}

void test_get_layout_properties() {
    std::cout << "Test: Layout properties..." << std::endl;
    
    TestLayout layout("dutch_classic", "nl");
    
    assert(std::string(layout.GetName()) == "dutch_classic");
    assert(std::string(layout.GetLanguage()) == "nl");
    std::cout << "  ✓ Layout properties accessible" << std::endl;
}

void test_multiple_layouts() {
    std::cout << "Test: Multiple layouts..." << std::endl;
    
    LayoutRegistry& registry = LayoutRegistry::Instance();
    TestLayout layout1("dutch_default", "nl");
    TestLayout layout2("flemish_classic", "nl_be");
    
    registry.Register("dutch_default", &layout1);
    registry.Register("flemish_classic", &layout2);
    
    const ILayoutProvider* l1 = registry.GetLayout("dutch_default");
    const ILayoutProvider* l2 = registry.GetLayout("flemish_classic");
    
    assert(l1 == &layout1);
    assert(l2 == &layout2);
    assert(std::string(l1->GetLanguage()) == "nl");
    assert(std::string(l2->GetLanguage()) == "nl_be");
    std::cout << "  ✓ Multiple layouts coexist correctly" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "LayoutRegistry Unit Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    try {
        test_singleton();
        test_register_layout();
        test_get_nonexistent_layout();
        test_get_layout_properties();
        test_multiple_layouts();
        
        std::cout << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "All tests PASSED" << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << std::endl;
        std::cerr << "TEST FAILED: " << e.what() << std::endl;
        return 1;
    }
}
