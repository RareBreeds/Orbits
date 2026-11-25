#pragma once

#include "plugin.hpp"

#include <atomic>

struct OrbitsWidget;

// The part of a module's saved data that belongs to its widget, currently the theme. The module
// owns this and saves and loads it as the "widget" entry of the module's data.
//
// Loaded data isn't applied straight away: Rack loads a module's data before its widget exists, and
// preset tools such as 8FaceMk2 load it from threads where drawing calls fail. So it's kept until
// the widget next draws, on the UI thread. Implemented in OrbitsWidget.cpp.
struct OrbitsWidgetData
{
        ~OrbitsWidgetData();

        // Adds the widget's data to the module's saved data, if the module has a widget.
        void save(json_t *root);

        // Keeps the widget's data from the module's loaded data, if any, for the widget to apply.
        // Safe to call from any thread.
        void load(json_t *root);

        // Used by OrbitsWidget to connect and disconnect the module's widget, and to take loaded
        // data to apply. The caller releases taken data with json_decref().
        void attach(OrbitsWidget *widget);
        void detach();
        json_t *takeLoaded();

      private:
        OrbitsWidget *m_widget = NULL;
        std::atomic<json_t *> m_loaded{nullptr};
};
