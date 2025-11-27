#!/usr/bin/env bash
# Layout Creation Tool - Creates a new layout directory with proper structure
# Part of Feature 006: Language Resource Optimization (Phase 9)

set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
LAYOUTS_DIR="$REPO_ROOT/res/layouts"

# Function: Print usage
usage() {
    cat << EOF
${BLUE}Dyscover Layout Creation Tool${NC}

Creates a new keyboard layout with proper directory structure.

Usage:
  $0 --type <layout_type> --language <language> [--copy-from <source>] [--voice <voice_name>]

Options:
  --type <layout_type>      Layout type (e.g., default, classic, kwec)
  --language <language>     Language code (e.g., nl_nl, nl_be, en_us)
  --copy-from <source>      Copy structure from existing layout (format: type/language)
  --voice <voice_name>      TTS voice name (e.g., Ilse, Veerle, David)
  --help                    Show this help message

Examples:
  # Create a new default English layout
  $0 --type default --language en_us --voice David

  # Create a new layout by copying from existing
  $0 --type advanced --language nl_nl --copy-from default/nl_nl --voice Ilse

EOF
    exit 0
}

# Parse arguments
LAYOUT_TYPE=""
LANGUAGE=""
COPY_FROM=""
VOICE_NAME=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --type)
            LAYOUT_TYPE="$2"
            shift 2
            ;;
        --language)
            LANGUAGE="$2"
            shift 2
            ;;
        --copy-from)
            COPY_FROM="$2"
            shift 2
            ;;
        --voice)
            VOICE_NAME="$2"
            shift 2
            ;;
        --help|-h)
            usage
            ;;
        *)
            echo -e "${RED}Error: Unknown option: $1${NC}"
            usage
            ;;
    esac
done

# Validate required arguments
if [[ -z "$LAYOUT_TYPE" ]] || [[ -z "$LANGUAGE" ]]; then
    echo -e "${RED}Error: --type and --language are required${NC}"
    usage
fi

# Validate language code format
if [[ ! "$LANGUAGE" =~ ^[a-z]{2}_[a-z]{2}$ ]]; then
    echo -e "${RED}Error: Invalid language code format. Expected format: xx_yy (e.g., nl_nl, en_us)${NC}"
    exit 1
fi

# Validate layout type format
if [[ ! "$LAYOUT_TYPE" =~ ^[a-z0-9]+$ ]]; then
    echo -e "${RED}Error: Invalid layout type. Use lowercase alphanumeric characters only.${NC}"
    exit 1
fi

# Set default voice if not specified
if [[ -z "$VOICE_NAME" ]]; then
    case "$LANGUAGE" in
        nl_nl)
            VOICE_NAME="Ilse"
            ;;
        nl_be)
            VOICE_NAME="Veerle"
            ;;
        en_*)
            VOICE_NAME="David"
            ;;
        *)
            echo -e "${YELLOW}Warning: No default voice for $LANGUAGE. Please specify --voice${NC}"
            exit 1
            ;;
    esac
fi

# Create target directory
TARGET_DIR="$LAYOUTS_DIR/$LAYOUT_TYPE/$LANGUAGE"

echo -e "${BLUE}=== Dyscover Layout Creation Tool ===${NC}"
echo "Creating new layout:"
echo "  Type: $LAYOUT_TYPE"
echo "  Language: $LANGUAGE"
echo "  Voice: $VOICE_NAME"
echo "  Target: $TARGET_DIR"
if [[ -n "$COPY_FROM" ]]; then
    echo "  Copy from: $COPY_FROM"
fi
echo ""

# Check if target already exists
if [[ -d "$TARGET_DIR" ]]; then
    echo -e "${RED}Error: Layout already exists: $TARGET_DIR${NC}"
    exit 1
fi

# If copying from existing layout
if [[ -n "$COPY_FROM" ]]; then
    SOURCE_DIR="$LAYOUTS_DIR/$COPY_FROM"
    
    if [[ ! -d "$SOURCE_DIR" ]]; then
        echo -e "${RED}Error: Source layout not found: $SOURCE_DIR${NC}"
        exit 1
    fi
    
    echo -e "${BLUE}Copying from existing layout...${NC}"
    
    # Create target directory
    mkdir -p "$TARGET_DIR"
    
    # Copy audio directory
    if [[ -d "$SOURCE_DIR/audio" ]]; then
        cp -r "$SOURCE_DIR/audio" "$TARGET_DIR/"
        echo "  ✓ Copied audio files"
    fi
    
    # Copy TTS directory (we'll update the files later)
    if [[ -d "$SOURCE_DIR/tts" ]]; then
        mkdir -p "$TARGET_DIR/tts"
        echo "  ✓ Created tts directory (you'll need to add language-specific TTS files)"
    fi
    
    # Copy layout.cpp as template
    if [[ -f "$SOURCE_DIR/layout.cpp" ]]; then
        cp "$SOURCE_DIR/layout.cpp" "$TARGET_DIR/layout.cpp"
        echo "  ✓ Copied layout.cpp as template"
    fi
    
else
    # Create from scratch
    echo -e "${BLUE}Creating new layout from scratch...${NC}"
    
    mkdir -p "$TARGET_DIR/audio"
    mkdir -p "$TARGET_DIR/tts"
    
    # Create minimal layout.cpp template
    cat > "$TARGET_DIR/layout.cpp" << 'EOFCPP'
#include "Keys.h"

// TODO: Define your layout here
// Example:
// const std::vector<KeyTranslationEntry> g_yourLayout = {
//     { Key::A, false, false, false, { { Key::A } }, "a.wav" },
//     { Key::B, false, false, false, { { Key::B } }, "b.wav" },
//     // Add more entries...
// };

EOFCPP
    
    echo "  ✓ Created audio/ directory"
    echo "  ✓ Created tts/ directory"
    echo "  ✓ Created layout.cpp template"
fi

# Create metadata.json
LANGUAGE_NAME=$(echo "$LANGUAGE" | tr '_' ' ' | awk '{for(i=1;i<=NF;i++) $i=toupper(substr($i,1,1)) tolower(substr($i,2))}1')
cat > "$TARGET_DIR/metadata.json" << EOFJSON
{
  "layout_type": "$LAYOUT_TYPE",
  "language": "$LANGUAGE",
  "language_name": "$LANGUAGE_NAME",
  "layout_name": "$(echo "$LAYOUT_TYPE" | awk '{print toupper(substr($0,1,1)) tolower(substr($0,2))}') $LANGUAGE_NAME Keyboard",
  "maintainer": "Dyscover Team",
  "last_updated": "$(date +%Y-%m-%d)",
  "audio_count": 0,
  "shared_audio_count": 0,
  "tts_voice": "$VOICE_NAME",
  "notes": "TODO: Add layout description"
}
EOFJSON

echo "  ✓ Created metadata.json"
echo ""

# Print next steps
echo -e "${GREEN}Layout created successfully!${NC}"
echo ""
echo -e "${BLUE}Next steps:${NC}"
echo "1. Add audio files to: $TARGET_DIR/audio/"
echo "2. Add TTS files to: $TARGET_DIR/tts/"
echo "   Required: ${LANGUAGE}.db, ${LANGUAGE}.fsa, ${LANGUAGE}.fst"
echo "   Required: ${VOICE_NAME}.db, ${VOICE_NAME}.fon, ${VOICE_NAME}.udb"
echo "3. Edit layout.cpp: Define your KeyTranslationEntry vector"
echo "4. Update metadata.json: Add audio_count and description"
echo "5. Validate: Run ./scripts/validate-all-layouts.sh"
echo "6. Build: mkdir build && cd build && cmake .. && make"
echo ""
echo -e "${YELLOW}Important:${NC}"
echo "- All audio filenames in layout.cpp must exist in audio/ directory"
echo "- TTS files must match the language code ($LANGUAGE)"
echo "- Use validate-all-layouts.sh to check for issues before building"
echo ""

exit 0
