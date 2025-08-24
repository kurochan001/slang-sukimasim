#!/bin/bash
# Script to fix covergroup tests using reserved keywords as bin names
# Created: 2025-08-24

echo "Fixing Covergroup tests with reserved keyword bin names..."

# Define the files that need fixing based on grep results
FILES_TO_FIX=(
    "/mnt/c/Work/sukimasim/tests/new_features/test_simple_components.sv"
    "/mnt/c/Work/sukimasim/tests/new_features/test_covergroup_assertions_100_percent.sv"
    "/mnt/c/Work/sukimasim/tests/covergroup/test_simple_options.sv"
    "/mnt/c/Work/sukimasim/tests/covergroup/test_coverage_options.sv"
    "/mnt/c/Work/sukimasim/tests/covergroup/test_covergroup_comprehensive.sv"
    "/mnt/c/Work/sukimasim/tests/covergroup/test_coverage_driven_verification.sv"
)

# Create backup and fix each file
for file in "${FILES_TO_FIX[@]}"; do
    if [ -f "$file" ]; then
        # Create backup
        cp "$file" "${file}.backup"
        
        # Create fixed version with _fixed suffix
        fixed_file="${file%.sv}_fixed.sv"
        cp "$file" "$fixed_file"
        
        # Replace reserved keywords with safe alternatives
        sed -i 's/bins small /bins small_bin /g' "$fixed_file"
        sed -i 's/bins medium /bins mid /g' "$fixed_file"
        sed -i 's/bins large /bins large_bin /g' "$fixed_file"
        sed -i 's/bins weak /bins weak_bin /g' "$fixed_file"
        sed -i 's/bins strong /bins strong_bin /g' "$fixed_file"
        
        # Also fix references to these bins in binsof expressions
        sed -i 's/binsof(.*\.small)/binsof(\1_bin)/g' "$fixed_file"
        sed -i 's/binsof(.*\.medium)/binsof(\1)/g' "$fixed_file"
        sed -i 's/binsof(.*\.large)/binsof(\1_bin)/g' "$fixed_file"
        
        echo "Fixed: $fixed_file"
    fi
done

echo "Testing fixed files with slang..."
for file in "${FILES_TO_FIX[@]}"; do
    fixed_file="${file%.sv}_fixed.sv"
    if [ -f "$fixed_file" ]; then
        echo -n "Testing $(basename $fixed_file): "
        if /mnt/c/Work/slang-sukimasim/build/bin/slang "$fixed_file" 2>&1 | grep -q "Build succeeded"; then
            echo "✅ PASS"
        else
            echo "❌ FAIL (other issues remain)"
        fi
    fi
done

echo "Done!"