#!/bin/bash
# Trust all desktop files
for i in ~/Desktop/*.desktop; do
  [ -f "${i}" ] || break
  gio set "${i}" "metadata::trusted" true
done

# Restart nautilus, so that the changes take effect (otherwise we would have to press F5)
nautilus -q