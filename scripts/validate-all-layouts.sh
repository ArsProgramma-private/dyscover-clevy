#!/usr/bin/env bash
# Layout Validation Tool - Validates all layouts in res/layouts/

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
LAYOUTS_DIR="$REPO_ROOT/res/layouts"

TOTAL_LAYOUTS=0
VALID_LAYOUTS=0
INVALID_LAYOUTS=0
WARNINGS=0

echo -e "${BLUE}=== Dyscover Layout Validation Tool ===${NC}"
echo "Validating layouts in: $LAYOUTS_DIR"
echo ""

validate_layout() {
    local layout_dir="$1"
    local layout_type=$(basename "$(dirname "$layout_dir")")
    local language=$(basename "$layout_dir")
    local layout_name="$layout_type/$language"
    
    echo -e "${BLUE}Validating:${NC} $layout_name"
    ((TOTAL_LAYOUTS++))
    
    local is_valid=true
    
    # Check layout.cpp
    if [[ ! -f "$layout_dir/layout.cpp" ]]; then
        echo -e "${RED}  ✗ Missing layout.cpp${NC}"
        is_valid=false
    fi
    
    # Check audio directory
    if [[ ! -d "$layout_dir/audio" ]]; then
        echo -e "${RED}  ✗ Missing audio/ directory${NC}"
        is_valid=false
    else
        local wav_count=$(find "$layout_dir/audio" -maxdepth 1 -name "*.wav" 2>/dev/null | wc -l)
        if [[ $wav_count -eq 0 ]]; then
            echo -e "${RED}  ✗ No audio files found${NC}"
            is_valid=false
        else
            echo "  ✓ Found $wav_count audio files"
        fi
    fi
    
    # Check TTS directory
    if [[ ! -d "$layout_dir/tts" ]]; then
        echo -e "${RED}  ✗ Missing tts/ directory${NC}"
        is_valid=false
    else
        # Check language files
        local has_lang_files=true
        [[ ! -f "$layout_dir/tts/${language}.db" ]] && has_lang_files=false
        [[ ! -f "$layout_dir/tts/${language}.fsa" ]] && has_lang_files=false
        [[ ! -f "$layout_dir/tts/${language}.fst" ]] && has_lang_files=false
        
        if ! $has_lang_files; then
            echo -e "${RED}  ✗ Incomplete TTS language files${NC}"
            is_valid=false
        fi
        
        # Check voice files
        local fon_count=$(find "$layout_dir/tts" -maxdepth 1 -name "*.fon" 2>/dev/null | wc -l)
        if [[ $fon_count -eq 0 ]]; then
            echo -e "${RED}  ✗ No TTS voice files found${NC}"
            is_valid=false
        fi
        
        if $has_lang_files && [[ $fon_count -gt 0 ]]; then
            echo "  ✓ TTS files complete"
        fi
    fi
    
    # Check metadata.json (optional)
    if [[ ! -f "$layout_dir/metadata.json" ]]; then
        echo -e "${YELLOW}  ! No metadata.json (optional)${NC}"
        ((WARNINGS++))
    else
        echo "  ✓ metadata.json present"
    fi
    
    # Report result
    if $is_valid; then
        echo -e "${GREEN}  ✓ VALID${NC}"
        ((VALID_LAYOUTS++))
    else
        echo -e "${RED}  ✗ INVALID${NC}"
        ((INVALID_LAYOUTS++))
    fi
    
    echo ""
}

echo -e "${BLUE}Discovering layouts...${NC}"
echo ""

for layout_dir in $(find "$LAYOUTS_DIR" -mindepth 2 -maxdepth 2 -type d | sort); do
    validate_layout "$layout_dir"
done

echo -e "${BLUE}=== Validation Summary ===${NC}"
echo "Total layouts: $TOTAL_LAYOUTS"
echo -e "${GREEN}Valid layouts: $VALID_LAYOUTS${NC}"
[[ $INVALID_LAYOUTS -gt 0 ]] && echo -e "${RED}Invalid layouts: $INVALID_LAYOUTS${NC}"
[[ $WARNINGS -gt 0 ]] && echo -e "${YELLOW}Warnings: $WARNINGS${NC}"
echo ""

if [[ $INVALID_LAYOUTS -gt 0 ]]; then
    echo -e "${RED}VALIDATION FAILED${NC}"
    exit 1
else
    echo -e "${GREEN}VALIDATION PASSED${NC}"
    exit 0
fi
