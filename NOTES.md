#### TODO

1. Disable GTK Inspector shortcuts in the release mode
    - https://docs.gtk.org/gtk4/running.html#interactive-debugging

2. Try to get more informative logging
    - https://docs.gtk.org/glib/logging.html
        - Proper reference is available via Devhelp
    - https://discourse.gnome.org/t/how-to-set-up-glib-log-output/1742

3. Profiling
    - Sysprof
        - https://docs.gtk.org/gtk4/running.html#profiling
        - https://developer.gnome.org/documentation/tools/sysprof.html
    - Tracy

4. Localization
    - https://developer.gnome.org/documentation/guidelines/localization.html

5. Accessibility
    - https://docs.gtk.org/gtk4/section-accessibility.html
    - https://developer.gnome.org/documentation/guidelines/accessibility.html
    - https://developer.gnome.org/hig

#### Known issues

1. Hovering over main menu emits

   ```
   _gtk_css_corner_value_get_y: assertion 'corner->class == &GTK_CSS_VALUE_CORNER' failed
   ```

   Fixed upstream: https://gitlab.gnome.org/GNOME/gtk/-/issues/5892

2. Preferences leak memory after closing the window or after using the search.

   Reported here: https://gitlab.gnome.org/GNOME/libadwaita/-/issues/704

3. Transient dialogs shift on subsequent view calls.

   Reported here: https://gitlab.gnome.org/GNOME/mutter/-/issues/2917
