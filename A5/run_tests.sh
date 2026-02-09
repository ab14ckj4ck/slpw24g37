#!/bin/bash

declare -A COLOR=(
  ["BLACK"]='\033[0;30m'
  ["DARK_GRAY"]='\033[1;30m'
  ["RED"]='\033[0;31m'
  ["LIGHT_RED"]='\033[1;31m'
  ["GREEN"]='\033[0;32m'
  ["LIGHT_GREEN"]='\033[1;32m'
  ["BROWN_ORANGE"]='\033[0;33m'
  ["YELLOW"]='\033[1;33m'
  ["BLUE"]='\033[0;34m'
  ["LIGHT_BLUE"]='\033[1;34m'
  ["PURPLE"]='\033[0;35m'
  ["LIGHT_PURPLE"]='\033[1;35m'
  ["CYAN"]='\033[0;36m'
  ["LIGHT_CYAN"]='\033[1;36m'
  ["LIGHT_GRAY"]='\033[0;37m'
  ["WHITE"]='\033[1;37m'
)
NO_COLOR='\033[0m'

function debug {
  COL=${COLOR[$1]}
  if [[ -z "$COL" ]]; then
    printf "%s\n" "$2"
  else
    printf "${COL}$2${NO_COLOR}\n"
  fi
}

function debug_info {
  debug "LIGHT_GRAY" "[INFO] $1"
}
function debug_highlight {
  debug "WHITE" "[INFO] $1"
}

function debug_fail {
  debug "RED" "[FAIL] $1"
}
function debug_success {
  debug "GREEN" "[SUCCESS] $1"
}



TEST_DIR="build/"
TIMEOUT_LIMIT=5  # Set the timeout limit in seconds
ret=0

debug_highlight "Starting Tests."

for test_file in "${TEST_DIR}"*; do
    if [[ -x "$test_file" ]]; then
        echo ""
        debug_highlight "Test: $test_file"
        if [[ "$test_file" == *FAIL_* ]]; then
            # Binary starts with "FAIL_"; run with timeout and check for failure
            # grab expected return value from testname
            expected_retval=${test_file//[!0-9]/}
            if [ -z "$expected_retval" ]; then
                expected_retval=0
            fi
            timeout "$TIMEOUT_LIMIT" "$test_file"
            exit_status=$?

            debug_info "Test returned: $exit_status"
            if [[ $expected_retval -ne 0 ]]; then
                # the test expects a specific return value
                if [[ $exit_status -eq $expected_retval ]]; then
                    debug_success "Test failed as intended: $test_file"
                else
                    debug_fail "Test returned wrong value: $test_file"
                    ret=1
                fi
            elif [[ $exit_status -ne 0 ]]; then
                # the test should return non-zero
                debug_success "Test failed as intended: $test_file"
            else
                debug_fail "Test was successful even though it should fail: $test_file"
                ret=1
            fi
        else
            # Run the test without timeout for success
            "$test_file"
            exit_status=$?
            debug_info "Test returned: $exit_status"
            if [[ $exit_status -eq 0 ]]; then
                debug_success "Test $test_file"
            else
                debug_fail "Test $test_file"
                ret=1
            fi
        fi
    fi
done

echo ""
echo ""
debug_highlight "Finished Tests."
if [[ $ret -eq 0 ]]; then
    debug_success "All tests passed!"
else
    debug_fail "Some tests failed!"
fi 
echo ""

exit $ret
