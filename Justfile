# Renegade firmware build recipes. Run `just --list` to see them all.

set shell := ["bash", "-euo", "pipefail", "-c"]

# Hardware target: 7 (Flipper Zero) or 18 (Flipper Zero without radio, dev board)
target := env("TARGET_HW", "7")

# Version suffix used in dist file names and as the update package version
suffix := env("DIST_SUFFIX", "local")

release_flags := "COMPACT=1 DEBUG=0"
fbt := "./fbt TARGET_HW=" + target + " DIST_SUFFIX=" + suffix

# Build the full update package
default: update

# Full update package (.tgz): firmware, radio stack, all SD card databases and apps
update:
    {{fbt}} {{release_flags}} updater_package
    @echo "Update package:"
    @ls -1 dist/f{{target}}-*/renegade-f{{target}}-update-{{suffix}}.tgz

# Same as `update`, but with debug firmware (bigger, includes extra logging)
update-debug:
    {{fbt}} COMPACT=1 DEBUG=1 updater_package

# Debug firmware build
build:
    {{fbt}}

# Release firmware build
build-release:
    {{fbt}} {{release_flags}}

# Build all .fap applications, including debug and example ones, into dist/
apps:
    {{fbt}} {{release_flags}} fap_dist

# Flash the full update package to a Flipper connected over USB
flash:
    {{fbt}} {{release_flags}} flash_usb_full

# Copy all built .fap applications to a Flipper connected over USB
deploy-apps:
    {{fbt}} {{release_flags}} fap_deploy

# Format sources, Python scripts and images
format:
    ./fbt format_all

# Check formatting of sources, Python scripts and images
lint:
    ./fbt lint_all

# Print the firmware API version
api-version:
    @{{fbt}} get_apiversion

# Remove build and dist outputs
clean:
    rm -rf build dist
