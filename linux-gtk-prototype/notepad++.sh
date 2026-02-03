#!/bin/bash
# Notepad++ Linux launcher script
# Suppresses harmless GLib-GIO warnings about content-type

# Suppress GLib-GIO warnings about GFileInfo content-type
# This is a known issue with GTK3 file chooser dialogs in newer GLib versions
export G_MESSAGES_DEBUG=none

# Run the actual notepad++ binary
exec /usr/local/bin/notepad++.bin "$@"
