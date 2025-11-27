#!/bin/bash
# Layout Migration Script
# Feature: 006-language-resource-optimization (Phase 2)
# Purpose: Migrate from flat structure to hierarchical layout-based organization
#
# Usage:
#   ./migrate-to-layouts.sh [--dry-run] [--rollback]
#
# See: specs/006-language-resource-optimization/quickstart.md

set -euo pipefail

# Error trap for debugging
set -o errtrace
trap 'echo "[ERROR] Script failed at line $LINENO with command: $BASH_COMMAND" >&2' ERR

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

# Configuration
SRC_KEYS="${REPO_ROOT}/src/Keys.cpp"
RES_DATA="${REPO_ROOT}/res/data"
RES_LAYOUTS="${REPO_ROOT}/res/layouts"
BACKUP_DIR="${REPO_ROOT}/.migration-backup"

# Flags
DRY_RUN=false
ROLLBACK=false

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Statistics
LAYOUTS_MOVED=0
AUDIO_COPIED=0
TTS_COPIED=0
SYMLINKS_CREATED=0
ERRORS=0

#######################################
# Print usage information
#######################################
usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Migrate keyboard layouts from flat structure to hierarchical organization.

OPTIONS:
    --dry-run    Show what would be done without making changes
    --rollback   Restore from backup (reverses migration)
    --help       Show this help message

EXAMPLES:
    # Preview migration
    $0 --dry-run
    
    # Execute migration
    $0
    
    # Rollback migration
    $0 --rollback

See specs/006-language-resource-optimization/quickstart.md for details.
EOF
}

#######################################
# Parse command line arguments
#######################################
parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            --dry-run)
                DRY_RUN=true
                shift
                ;;
            --rollback)
                ROLLBACK=true
                shift
                ;;
            --help)
                usage
                exit 0
                ;;
            *)
                echo -e "${RED}Error: Unknown option: $1${NC}"
                usage
                exit 1
                ;;
        esac
    done
}

#######################################
# Log message with prefix
#######################################
log() {
    local level=$1
    shift
    local message="$@"
    
    case ${level} in
        INFO)
            echo -e "${BLUE}[INFO]${NC} ${message}"
            ;;
        SUCCESS)
            echo -e "${GREEN}[SUCCESS]${NC} ${message}"
            ;;
        WARNING)
            echo -e "${YELLOW}[WARNING]${NC} ${message}"
            ;;
        ERROR)
            echo -e "${RED}[ERROR]${NC} ${message}"
            ERRORS=$((ERRORS + 1))
            ;;
        DRY_RUN)
            if ${DRY_RUN}; then
                echo -e "${YELLOW}[DRY RUN]${NC} ${message}"
            fi
            ;;
    esac
}

#######################################
# Check prerequisites
#######################################
check_prerequisites() {
    log INFO "Checking prerequisites..."
    
    # Check if Keys.cpp exists
    if [[ ! -f "${SRC_KEYS}" ]]; then
        log ERROR "Keys.cpp not found at ${SRC_KEYS}"
        return 1
    fi
    
    # Check if res/data exists
    if [[ ! -d "${RES_DATA}" ]]; then
        log ERROR "Resource directory not found at ${RES_DATA}"
        return 1
    fi
    
    # Check if already migrated
    if [[ -d "${RES_LAYOUTS}" ]] && ! ${ROLLBACK}; then
        log WARNING "Layout structure already exists at ${RES_LAYOUTS} (auto-continue non-interactive)"
        # Non-interactive mode: proceed automatically; to force abort set ABORT_ON_EXIST=1
        if [[ "${ABORT_ON_EXIST:-0}" == "1" ]]; then
            log ERROR "ABORT_ON_EXIST set; stopping."
            return 1
        fi
    fi
    
    log SUCCESS "Prerequisites check passed"
    return 0
}

#######################################
# Create backup
#######################################
create_backup() {
    if ${DRY_RUN}; then
        log DRY_RUN "Would create backup at ${BACKUP_DIR}"
        return 0
    fi
    
    log INFO "Creating backup..."
    mkdir -p "${BACKUP_DIR}"
    
    # Backup Keys.cpp
    cp "${SRC_KEYS}" "${BACKUP_DIR}/Keys.cpp.backup"
    
    # Backup res/data structure info
    find "${RES_DATA}" -type f > "${BACKUP_DIR}/original-files.txt"
    
    log SUCCESS "Backup created at ${BACKUP_DIR}"
}

#######################################
# Parse Keys.cpp to identify layouts
#######################################
parse_layouts() {
    log INFO "Parsing Keys.cpp to identify layouts..."
    
    # Known layouts with their preprocessor defines and types
    declare -gA LAYOUT_MAP
    LAYOUT_MAP=(
        ["g_dutchDefault"]="default/nl_nl __LANGUAGE_NL__"
        ["g_flemishDefault"]="default/nl_be __LANGUAGE_NL_BE__"
        ["g_dutchClassic"]="classic/nl_nl __LANGUAGE_NL__"
        ["g_flemishClassic"]="classic/nl_be __LANGUAGE_NL_BE__"
        ["g_dutchKWeC"]="kwec/nl_nl __LANGUAGE_NL__"
    )
    
    log SUCCESS "Found ${#LAYOUT_MAP[@]} layouts"
    for layout in "${!LAYOUT_MAP[@]}"; do
        log INFO "  - ${layout} → ${LAYOUT_MAP[$layout]}"
    done
}

#######################################
# Extract layout code from Keys.cpp
#######################################
extract_layout_code() {
    local layout_var=$1
    local output_file=$2
    
    log INFO "Extracting ${layout_var}..."
    
    # Extract the specific layout definition using sed
    local start_line=$(grep -n "static const std::vector<KeyTranslationEntry> ${layout_var} =" "${SRC_KEYS}" | cut -d: -f1)
    
    if [[ -z "${start_line}" ]]; then
        log ERROR "Could not find layout definition for ${layout_var}"
        return 1
    fi
    
    log INFO "  Found at line ${start_line}, extracting..."
    
    # Extract from start line until closing brace
    if ! sed -n "${start_line},/^};/p" "${SRC_KEYS}" > "${output_file}.tmp" 2>&1; then
        log ERROR "sed command failed for ${layout_var}"
        return 1
    fi
    
    if [[ ! -s "${output_file}.tmp" ]]; then
        log ERROR "Failed to extract layout code for ${layout_var} (empty output)"
        rm -f "${output_file}.tmp"
        return 1
    fi
    
    log INFO "  Extracted $(wc -l < "${output_file}.tmp") lines"
    
    # Create full layout.cpp file with header and type definitions
    {
        echo "// Auto-generated layout module"
        echo "// Feature: 006-language-resource-optimization (Phase 2)"
        echo "// Migrated from src/Keys.cpp"
        echo ""
        echo "#include \"Keys.h\""
        echo "#include \"layouts/LayoutLoader.h\""
        echo "#include \"layouts/LayoutRegistry.h\""
        echo ""
        echo "// Type definitions (copied from Keys.cpp)"
        echo "enum class CapsLock {"
        echo "    Ignore,"
        echo "    Active,"
        echo "    Inactive,"
        echo "};"
        echo ""
        echo "struct KeyTranslationEntry {"
        echo "    Key input;"
        echo "    bool shift;"
        echo "    bool ctrl;"
        echo "    bool alt;"
        echo "    std::vector<KeyStroke> output;"
        echo "    std::string sound;"
        echo "    bool speak_sentence = false;"
        echo "    CapsLock capsLock = CapsLock::Ignore;"
        echo "};"
        echo ""
        echo "namespace {"
        echo ""
        cat "${output_file}.tmp"
        rm "${output_file}.tmp"
    } > "${output_file}"
    
    # Add ILayoutProvider implementation
    local layout_name_raw=$(echo ${layout_var} | sed 's/g_//')
    local layout_class="${layout_name_raw^}Layout"  # Capitalize first letter
    local layout_name=$(echo ${layout_name_raw} | tr '[:upper:]' '[:lower:]')
    local lang_code=$(echo "${LAYOUT_MAP[$layout_var]}" | awk '{print $1}' | cut -d'/' -f2)
    
    log INFO "  Generating class ${layout_class} for language ${lang_code}"
    
    # Append class implementation
    {
        echo ""
        echo "// Layout provider implementation"
        echo "class ${layout_class} : public Dyscover::ILayoutProvider {"
        echo "public:"
        echo "    const std::vector<KeyTranslationEntry>& GetEntries() const override {"
        echo "        return ${layout_var};"
        echo "    }"
        echo "    const char* GetName() const override { return \"${layout_name}\"; }"
        echo "    const char* GetLanguage() const override { return \"${lang_code}\"; }"
        echo "};"
        echo ""
        echo "// Static registration"
        echo "static ${layout_class} s_layout;"
        echo "static bool s_registered = []() {"
        echo "    Dyscover::LayoutRegistry::Instance().Register(\"${layout_name}\", &s_layout);"
        echo "    return true;"
        echo "}();"
        echo ""
        echo "} // namespace"
    } >> "${output_file}"
    
    log INFO "  Layout module created successfully"
    return 0
}

#######################################
# Extract audio references from layout
#######################################
extract_audio_refs() {
    local layout_file=$1
    
    grep -oP '"[^"]+\.wav"' "${layout_file}" | tr -d '"' | sort -u || true
}

#######################################
# Determine TTS files for language
#######################################
get_tts_files() {
    local lang_code=$1
    local tts_base="${RES_DATA}/tts/data"
    
    # Language files
    local lang_exts=("db" "fsa" "fst")
    for ext in "${lang_exts[@]}"; do
        echo "${tts_base}/${lang_code}.${ext}"
    done
    
    # Voice files (based on language)
    local voice_exts=("db" "fon" "udb")
    case ${lang_code} in
        nl_nl)
            for ext in "${voice_exts[@]}"; do
                echo "${tts_base}/Ilse.${ext}"
            done
            ;;
        nl_be)
            for ext in "${voice_exts[@]}"; do
                echo "${tts_base}/Veerle.${ext}"
            done
            ;;
    esac
}

#######################################
# Create layout directory structure
#######################################
create_layout_structure() {
    local layout_path=$1
    
    if ${DRY_RUN}; then
        log DRY_RUN "Would create: ${layout_path}/"
        log DRY_RUN "Would create: ${layout_path}/audio/"
        log DRY_RUN "Would create: ${layout_path}/tts/"
        return 0
    fi
    
    mkdir -p "${layout_path}/audio"
    mkdir -p "${layout_path}/tts"
    log SUCCESS "Created ${layout_path}"
}

#######################################
# Copy audio files
#######################################
copy_audio_files() {
    local layout_file=$1
    local audio_dir=$2
    
    log INFO "  Copying audio files..."
    
    local audio_refs=$(extract_audio_refs "${layout_file}")
    local audio_count=$(echo "${audio_refs}" | wc -w)
    
    log INFO "  Found ${audio_count} audio references"
    
    for audio_file in ${audio_refs}; do
        local src="${RES_DATA}/${audio_file}"
        local dst="${audio_dir}/${audio_file}"
        
        if [[ ! -f "${src}" ]]; then
            log WARNING "Audio file not found: ${audio_file}"
            continue
        fi
        
        if ${DRY_RUN}; then
            log DRY_RUN "Would copy audio file: ${audio_file}"
        else
            cp "${src}" "${dst}" || { log ERROR "Failed to copy ${audio_file}"; return 1; }
            AUDIO_COPIED=$((AUDIO_COPIED + 1))
        fi
    done
    
    log INFO "  Copied ${audio_count} audio files"
}

#######################################
# Copy TTS files
#######################################
copy_tts_files() {
    local lang_code=$1
    local tts_dir=$2
    
    while IFS= read -r src; do
        if [[ -f "${src}" ]]; then
            local filename=$(basename "${src}")
            local dst="${tts_dir}/${filename}"
            
            if ${DRY_RUN}; then
                log DRY_RUN "Would copy TTS file: ${filename}"
            else
                cp "${src}" "${dst}"
                TTS_COPIED=$((TTS_COPIED + 1))
            fi
        else
            log WARNING "TTS file not found: ${src}"
        fi
    done < <(get_tts_files "${lang_code}")
}

#######################################
# Migrate all layouts
#######################################
migrate_layouts() {
    log INFO "Migrating layouts..."
    
    for layout_var in "${!LAYOUT_MAP[@]}"; do
        local layout_info="${LAYOUT_MAP[$layout_var]}"
        local layout_path="${RES_LAYOUTS}/$(echo ${layout_info} | awk '{print $1}')"
        local layout_file="${layout_path}/layout.cpp"
        local lang_code=$(echo ${layout_path} | rev | cut -d'/' -f1 | rev)
        
        log INFO "Processing ${layout_var}..."
        
        create_layout_structure "${layout_path}" || { log ERROR "Dir create failed for ${layout_var}"; ERRORS=$((ERRORS + 1)); continue; }
        
        if ! ${DRY_RUN}; then
            if ! extract_layout_code "${layout_var}" "${layout_file}"; then
                log ERROR "Failed to extract layout code for ${layout_var}"
                ERRORS=$((ERRORS + 1))
                continue
            fi
            LAYOUTS_MOVED=$((LAYOUTS_MOVED + 1))
        else
            log DRY_RUN "Would move layout definition: ${layout_var}"
        fi
        
        log INFO "Copying audio for ${layout_var}..."
        if ! ${DRY_RUN}; then
            if ! copy_audio_files "${layout_file}" "${layout_path}/audio"; then
                log ERROR "Failed audio copy for ${layout_var}"; ERRORS=$((ERRORS + 1)); continue;
            fi
        else
            local temp_file=$(mktemp)
            extract_layout_code "${layout_var}" "${temp_file}"
            local audio_count=$(extract_audio_refs "${temp_file}" | wc -l || true)
            log DRY_RUN "Would copy ${audio_count} audio files"
            rm -f "${temp_file}"
        fi
        
        log INFO "Copying TTS for ${layout_var} (lang=${lang_code})..."
        copy_tts_files "${lang_code}" "${layout_path}/tts" || { log ERROR "Failed TTS copy for ${layout_var}"; ERRORS=$((ERRORS + 1)); continue; }
        
        log INFO "Completed ${layout_var}"
    done
}

#######################################
# Generate migration report
#######################################
generate_report() {
    echo ""
    echo "=========================================="
    echo "          MIGRATION REPORT"
    echo "=========================================="
    echo ""
    echo "Statistics:"
    echo "  Layouts migrated: ${LAYOUTS_MOVED}"
    echo "  Audio files copied: ${AUDIO_COPIED}"
    echo "  TTS files copied: ${TTS_COPIED}"
    echo "  Symlinks created: ${SYMLINKS_CREATED}"
    echo "  Errors: ${ERRORS}"
    echo ""
    
    if ${DRY_RUN}; then
        echo "This was a DRY RUN. No files were modified."
        echo "Run without --dry-run to execute migration."
    else
        echo "Migration complete!"
        echo "Backup saved to: ${BACKUP_DIR}"
        echo ""
        echo "Next steps:"
        echo "  1. Build with USE_LAYOUT_STRUCTURE=ON"
        echo "  2. Run tests to verify equivalence"
        echo "  3. See quickstart.md for Phase 2 instructions"
    fi
    echo ""
}

#######################################
# Rollback migration
#######################################
rollback_migration() {
    log INFO "Rolling back migration..."
    
    if [[ ! -d "${BACKUP_DIR}" ]]; then
        log ERROR "No backup found at ${BACKUP_DIR}"
        return 1
    fi
    
    # Remove migrated structure
    if [[ -d "${RES_LAYOUTS}" ]]; then
        rm -rf "${RES_LAYOUTS}"
        log SUCCESS "Removed ${RES_LAYOUTS}"
    fi
    
    # Restore Keys.cpp if modified
    if [[ -f "${BACKUP_DIR}/Keys.cpp.backup" ]]; then
        cp "${BACKUP_DIR}/Keys.cpp.backup" "${SRC_KEYS}"
        log SUCCESS "Restored ${SRC_KEYS}"
    fi
    
    log SUCCESS "Rollback complete"
    return 0
}

#######################################
# Main execution
#######################################
main() {
    parse_args "$@"
    
    echo "=========================================="
    echo "  Layout Migration Script"
    echo "=========================================="
    echo ""
    
    if ${ROLLBACK}; then
        rollback_migration
        exit $?
    fi
    
    check_prerequisites || exit 1
    create_backup
    parse_layouts
    migrate_layouts
    generate_report
    
    if [[ ${ERRORS} -gt 0 ]]; then
        log ERROR "Migration completed with ${ERRORS} errors"
        exit 1
    fi
    
    exit 0
}

main "$@"
