#!/bin/bash
# cleanup-ccache.sh
# Automated cleanup script for ccache directories to prevent unbounded growth
# Usage: ./cleanup-ccache.sh [ccache_dir] [max_size_gb] [max_age_days]

set -euo pipefail

# Configuration (defaults)
CCACHE_DIR="${1:-${CCACHE_DIR:-~/.ccache}}"
MAX_SIZE_GB="${2:-10}"  # Maximum cache size in GB
MAX_AGE_DAYS="${3:-30}" # Maximum age of cache files in days

# Validate ccache directory exists
if [ ! -d "$CCACHE_DIR" ]; then
    echo "Error: ccache directory not found: $CCACHE_DIR"
    exit 1
fi

echo "=== ccache cleanup starting ==="
echo "Cache directory: $CCACHE_DIR"
echo "Max size: ${MAX_SIZE_GB}GB"
echo "Max age: ${MAX_AGE_DAYS} days"

# Show current cache stats
if command -v ccache &> /dev/null; then
    echo ""
    echo "Current ccache stats:"
    CCACHE_DIR="$CCACHE_DIR" ccache -s || true
fi

# Remove cache files older than MAX_AGE_DAYS
echo ""
echo "Removing cache files older than ${MAX_AGE_DAYS} days..."
find "$CCACHE_DIR" -type f -name "*.o" -mtime +${MAX_AGE_DAYS} -delete 2>/dev/null || true
find "$CCACHE_DIR" -type f -name "*.manifest" -mtime +${MAX_AGE_DAYS} -delete 2>/dev/null || true
find "$CCACHE_DIR" -type f -name "*.d" -mtime +${MAX_AGE_DAYS} -delete 2>/dev/null || true

# Remove empty directories
echo "Removing empty directories..."
find "$CCACHE_DIR" -type d -empty -delete 2>/dev/null || true

# Check current size and enforce size limit if needed
if command -v ccache &> /dev/null; then
    echo ""
    echo "Setting cache size limit to ${MAX_SIZE_GB}GB..."
    CCACHE_DIR="$CCACHE_DIR" ccache -M "${MAX_SIZE_GB}G"
    
    # Clean up to size limit
    echo "Running ccache cleanup to enforce size limit..."
    CCACHE_DIR="$CCACHE_DIR" ccache -c
    
    # Show final stats
    echo ""
    echo "Final ccache stats:"
    CCACHE_DIR="$CCACHE_DIR" ccache -s
else
    # Manual size check if ccache command not available
    CURRENT_SIZE_MB=$(du -sm "$CCACHE_DIR" | cut -f1)
    MAX_SIZE_MB=$((MAX_SIZE_GB * 1024))
    
    if [ $CURRENT_SIZE_MB -gt $MAX_SIZE_MB ]; then
        echo "Warning: Cache size (${CURRENT_SIZE_MB}MB) exceeds limit (${MAX_SIZE_MB}MB)"
        echo "Consider installing ccache binary for automatic size management"
    else
        echo "Cache size: ${CURRENT_SIZE_MB}MB (within ${MAX_SIZE_GB}GB limit)"
    fi
fi

echo ""
echo "=== ccache cleanup complete ==="
