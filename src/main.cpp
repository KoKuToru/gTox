/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2014  Luca Béla Palkovics
    Copyright (C) 2014  Maurice Mohlek

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
**/
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <fstream>

#include <gtkmm.h>
#include "dialog/error.h"
#include <glibmm/i18n.h>
#include <glibmm/exception.h>
#include <gstreamermm/init.h>
#include "tox/exception.h"
#include "utils/debug.h"
#include "gtox.h"

void print_copyright() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    std::clog
        << "gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git"
        << std::endl << std::endl << "Copyright (C) 2014  Luca Béla Palkovics"
        << std::endl << "Copyright (C) 2014  Maurice Mohlek" << std::endl
        << std::endl << "This program is free software: you can redistribute "
                        "it and/or modify" << std::endl
        << "it under the terms of the GNU General Public License as published "
           "by" << std::endl
        << "the Free Software Foundation, either version 3 of the License, or"
        << std::endl << "(at your option) any later version." << std::endl
        << std::endl
        << "This program is distributed in the hope that it will be useful,"
        << std::endl
        << "but WITHOUT ANY WARRANTY; without even the implied warranty of"
        << std::endl
        << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
        << std::endl << "GNU General Public License for more details."
        << std::endl << std::endl
        << "You should have received a copy of the GNU General Public License"
        << std::endl << "along with this program.  If not, see "
                        "<http://www.gnu.org/licenses/>" << std::endl
        << std::endl;
}

bool find_translation_domain(std::string path) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});

    // Translation search locations in order of preference
    std::vector<std::string> locations {
        Glib::build_filename(path, "i18n"),
        Glib::build_filename(path, "..", "share", "locale")
    };

    for (auto l : locations) {
        //check for gtox.mo
        auto mo = Glib::build_filename(l, "en", "LC_MESSAGES", "gtox.mo");
        if (Glib::file_test(mo, Glib::FILE_TEST_EXISTS)) {
            bindtextdomain("gtox", l.c_str());
            return true;
        }
    }

    return false;
}

bool setup_translation(std::string path) {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    // Translations returns in UTF-8
    bind_textdomain_codeset("gtox", "UTF-8");
    textdomain("gtox");
    return find_translation_domain(path);
}

void terminate_handler() {
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {});
    std::exception_ptr exptr = std::current_exception();
    try {
        std::rethrow_exception(exptr);
    } catch (const Glib::Exception &ex) {
        dialog::error(true, "Fatal Unexpected Glib Exception", ex.what()).run();
    } catch (const toxmm::exception &ex) {
        dialog::error(true, "Fatal Unexpected Tox Exception", ex.what()).run();
    } catch (const std::exception &ex) {
        dialog::error(true, "Fatal Unexpected Exception", ex.what()).run();
    } catch (const std::string &ex) {
        dialog::error(true, "Fatal Unexpected String Exception", ex).run();
    } catch (...) {
        dialog::error(true, "Fatal Unexpected Exception", "unknow exception !")
            .run();
    }
    abort();
}
#include <assert.h>

class HeaderTabTitle: public Gtk::VBox {
    private:
        Gtk::Label m_title;
        Gtk::Label m_subtitle;
    public:
        HeaderTabTitle(const Glib::ustring& title,
                       const Glib::ustring& subtitle) {
            m_title.set_label(title);
            m_title.get_style_context()->add_class("title");
            m_subtitle.set_label(subtitle);
            m_subtitle.get_style_context()->add_class("subtitle");
            add(m_title);
            add(m_subtitle);
            show_all();
        }
};

class HeaderTabChild: public Gtk::Frame/*Gtk::ToggleButton*/ {
        friend class HeaderTab;
    private:
        Gtk::HBox    m_box;
        Gtk::Button  m_close;
        Gtk::Image   m_close_icon;
        Gtk::EventBox m_eventbox;

    public:
        HeaderTabChild(): Glib::ObjectBase(typeid(*this)) {
            get_style_context()->add_class("header-tab-child");
            m_box.pack_end(m_close);
            m_box.show();
            m_box.property_spacing() = 6;
            m_close.show();
            m_close.get_style_context()->add_class("titlebutton");
            m_close.add(m_close_icon);
            m_close_icon.set_from_icon_name("window-close-symbolic",
                                            Gtk::IconSize(1));
            m_close_icon.show();
            m_close.property_valign() = Gtk::ALIGN_CENTER;
            m_eventbox.add(m_box);
            m_eventbox.show();
            Gtk::Frame::add(m_eventbox);

            //dnd should be done outside for gtox
            std::vector<Gtk::TargetEntry> dnd_targets;
            dnd_targets.push_back( Gtk::TargetEntry("gtox/chat_window")); //TODO: add unique identifier for the toxmm-instance
            m_eventbox.drag_source_set(dnd_targets);
            m_eventbox.signal_drag_data_get().connect([this](
                                           const Glib::RefPtr<Gdk::DragContext>&,
                                           Gtk::SelectionData& selection_data,
                                           guint, guint) {
                Glib::ustring data = "DEMO";
                selection_data.set(selection_data.get_target(),
                                   data.c_str());
            });
            m_eventbox.signal_drag_begin().connect([this](const Glib::RefPtr<Gdk::DragContext>& drag_context) {
                auto surface = Cairo::ImageSurface::create(
                                   Cairo::FORMAT_RGB24,
                                   get_allocation().get_width(),
                                   get_allocation().get_height());
                auto ctx = Cairo::Context::create(surface);
                ctx->set_source_rgb(1.0, 1.0, 1.0);
                ctx->rectangle(0,
                               0,
                               get_allocation().get_width(),
                               get_allocation().get_height());
                ctx->fill();
                //m_eventbox.draw(ctx); protected :(
                gtk_widget_draw(GTK_WIDGET(gobj()), ctx->cobj());
                drag_context->set_icon(surface);
            });
        }

        Glib::PropertyProxy<bool> property_visible_close_btn() {
            return m_close.property_visible();
        }

        virtual void add(Gtk::Widget& widget) override {
            m_box.add(widget);
        }

        virtual bool on_motion_notify_event(GdkEventMotion*) override {
            return true;
        }
};

class HeaderTabChildBox: public Gtk::Box {
        friend class HeaderTab;
    private:
        Gtk::Widget* m_widget = nullptr;

    public:
        virtual void add(Gtk::Widget& widget) {
            m_widget = &widget;
            Gtk::Box::add(widget);
        }
};

//https://developer.gnome.org/gtkmm-tutorial/stable/sec-custom-containers.html.en
class HeaderTab: public Gtk::Container {
    private:
        std::vector<HeaderTabChildBox*> m_children;

        Gtk::Arrow     m_more_arrow;
        HeaderTabChild m_more;

        Gtk::Popover   m_more_popover;
        Gtk::VBox      m_more_popover_box;

        HeaderTabChild* m_prelight_child = nullptr;
        HeaderTabChild* m_selected_child = nullptr;

    public:
        HeaderTab():
            Glib::ObjectBase(typeid(*this)),
            m_more_arrow(Gtk::ARROW_DOWN, Gtk::SHADOW_NONE) {

            set_has_window(true);
            set_redraw_on_allocate(true);
            get_style_context()->add_class("header-tab");

            m_more.add(m_more_arrow);
            m_more.show();
            m_more_arrow.show();
            m_more.property_visible_close_btn() = false;
            m_more.set_parent(*this);

            m_more_popover.get_style_context()->add_class("titlebar");

            m_more_popover.add(m_more_popover_box);
            m_more_popover_box.show();

            add_events(Gdk::ENTER_NOTIFY_MASK
                       | Gdk::LEAVE_NOTIFY_MASK
                       | Gdk::POINTER_MOTION_MASK
                       | Gdk::KEY_PRESS_MASK
                       | Gdk::BUTTON_PRESS_MASK
                       | Gdk::BUTTON_RELEASE_MASK);
        }

        ~HeaderTab() {
            for (Gtk::Widget* child : m_children) {
                child->unparent();
            }
        }

        virtual void add(Widget& widget) override {
            //add a new child
            HeaderTabChild* item = dynamic_cast<HeaderTabChild*>(&widget);
            item = new HeaderTabChild();
            item->add(widget);
            item->show();
            HeaderTabChildBox* box = new HeaderTabChildBox();
            box->add(*item);
            box->set_parent(*this);
            box->show();
            box->get_style_context()->add_class("header-tab-wrapper");
            m_children.push_back(box);
        }

        virtual void remove(Widget& widget) {
            for (size_t i = 0; i < m_children.size(); ++i) {
                if (m_children[i] == &widget ||
                    m_children[i]->m_widget == &widget) {

                    m_children.erase(m_children.begin() + i);
                    break;
                }
            }
        }

        virtual std::vector<Gtk::Widget*> get_children() {
            std::vector<Gtk::Widget*> tmp(m_children.size());
            std::copy(m_children.begin(), m_children.end(), tmp.begin());
            return tmp;
        }

        virtual std::vector<const Gtk::Widget*> get_children() const {
            std::vector<const Gtk::Widget*> tmp(m_children.size());
            std::copy(m_children.begin(), m_children.end(), tmp.begin());
            return tmp;
        }

    protected:
        virtual void forall_vfunc(gboolean    include_internals,
                                  GtkCallback callback,
                                  gpointer    callback_data ) override {
            for (Gtk::Widget* child : m_children) {
                callback(child->gobj(), callback_data);
            }
            if (include_internals) {
                callback(((Gtk::Widget*)&m_more)->gobj(), callback_data);
            }
        }

        virtual GType child_type_vfunc() const override {
            return Gtk::Widget::get_type();
        }

        virtual void on_size_allocate(Gtk::Allocation& allocation) override {
            //Use the offered allocation for this container:
            set_allocation(allocation);

            //update window size
            auto window = get_window();
            if (window) {
                window->move_resize(
                            allocation.get_x(),
                            allocation.get_y(),
                            allocation.get_width(),
                            allocation.get_height());
                allocation.set_x(0);
                allocation.set_y(0);
            }

            int more_min_width, more_nat_width;
            m_more.get_preferred_width(more_min_width,
                                       more_nat_width);

            bool in_hidden_area = false;

            //put all visible children into a row
            Gtk::Allocation child_allocation;
            child_allocation.set_x(allocation.get_x());
            child_allocation.set_y(allocation.get_y());
            child_allocation.set_width(0);
            child_allocation.set_height(allocation.get_height());
            for (HeaderTabChildBox* child : m_children) {
                int child_min_width, child_nat_width;
                child->m_widget->get_preferred_width_for_height(child_allocation.get_height(),
                                                      child_min_width,
                                                      child_nat_width);
                child_allocation.set_width(std::max(child_min_width,
                                                    child_nat_width));

                if (child_allocation.get_x() + child_allocation.get_width()
                        > allocation.get_x() + allocation.get_width() - more_min_width) {
                    //outside
                    if (!in_hidden_area) {
                        in_hidden_area = true;
                        Gtk::Allocation more_allocation = child_allocation;
                        more_allocation.set_width(more_min_width);
                        m_more.size_allocate(more_allocation);
                    }
                }

                if (in_hidden_area) {
                    if (child->m_widget->get_parent() == child) {
                        child->remove(*child->m_widget);
                        m_more_popover_box.add(*child->m_widget);
                    }
                    child->hide();
                } else {
                    if (child->m_widget->get_parent() != child) {
                        m_more_popover_box.remove(*child->m_widget);
                        child->add(*child->m_widget);
                    }
                    child->size_allocate(child_allocation);
                    child->show();
                }
                //place for next
                child_allocation.set_x(child_allocation.get_x() +
                                       child_allocation.get_width() - 1);
            }

            if (!in_hidden_area) {
                Gtk::Allocation more_allocation;
                more_allocation.set_width(1);
                more_allocation.set_height(1);
                more_allocation.set_y(-allocation.get_y()
                                      -allocation.get_height());
                m_more.size_allocate(more_allocation);
            }
        }

        /*virtual Gtk::SizeRequestMode get_request_mode_vfunc() const {
            return Gtk::SIZE_REQUEST_WIDTH_FOR_HEIGHT;
        }*/

        virtual void get_preferred_width_vfunc(int& min_width, int& nat_width) const override {
            int child_min_width, child_nat_width;
            min_width = 0;
            nat_width = 0;
            if (!m_children.empty()) {
                m_children.front()->get_preferred_width(child_min_width,
                                                        child_nat_width);
                min_width = std::max(child_min_width,
                                     child_nat_width);
                nat_width = 0;
            }
            for (HeaderTabChildBox* child : m_children) {
                child->m_widget->get_preferred_width(child_min_width,
                                           child_nat_width);
                nat_width += std::max(child_min_width,
                                      child_nat_width) - 1;
            }
            nat_width += 1;
            m_more.get_preferred_width(child_min_width,
                                       child_nat_width);
            min_width += child_min_width;
            nat_width += child_nat_width;
            nat_width = std::max(nat_width, min_width);
            std::clog << "preferred_width: " << min_width << ", " << nat_width << std::endl;
        }

        virtual void get_preferred_height_vfunc(int& min_height, int& nat_height) const override {
            int child_min_height, child_nat_height;
            min_height = 0;
            nat_height = 0;
            for (HeaderTabChildBox* child : m_children) {
                child->m_widget->get_preferred_height(child_min_height,
                                            child_nat_height);
                min_height = std::max(min_height, child_min_height);
                nat_height = std::max(nat_height, child_nat_height);
            }
            m_more.get_preferred_height(child_min_height,
                                        child_nat_height);
            min_height = std::max(min_height, child_min_height);
            nat_height = std::max(nat_height, child_nat_height);
            nat_height = std::max(nat_height, min_height);
            std::clog << "preferred_height: " << min_height << ", " << nat_height << std::endl;
        }

        virtual void get_preferred_width_for_height_vfunc(int height,
                                                          int& min_width,
                                                          int& nat_width) const override {
            int child_min_width, child_nat_width;
            min_width = 0;
            nat_width = 0;
            if (!m_children.empty()) {
                m_children.front()->get_preferred_width_for_height(
                            height,
                            child_min_width,
                            child_nat_width);
                min_width = std::max(child_min_width,
                                     child_nat_width);
                nat_width = 0;
            }
            for (HeaderTabChildBox* child : m_children) {
                child->m_widget->get_preferred_width_for_height(
                            height,
                            child_min_width,
                            child_nat_width);
                nat_width += std::max(child_min_width,
                                      child_nat_width) - 1;
            }
            nat_width += 1;
            m_more.get_preferred_width(child_min_width,
                                       child_nat_width);
            min_width += child_min_width;
            nat_width += child_nat_width;
            nat_width = std::max(nat_width, min_width);
            std::clog << "preferred_width_for_height: " << min_width << ", " << nat_width << std::endl;
        }

        virtual void get_preferred_height_for_width_vfunc(int width,
                                                          int& min_height,
                                                          int& nat_height) const override {
            int child_min_height, child_nat_height;
            min_height = 0;
            nat_height = 0;
            for (HeaderTabChildBox* child : m_children) {
                child->m_widget->get_preferred_height_for_width(
                            width, //this is hardly correct..
                            child_min_height,
                            child_nat_height);
                min_height = std::max(min_height, child_min_height);
                nat_height = std::max(nat_height, child_nat_height);
            }
            m_more.get_preferred_height_for_width(
                        width,
                        child_min_height,
                        child_nat_height);
            min_height = std::max(min_height, child_min_height);
            nat_height = std::max(nat_height, child_nat_height);
            nat_height = std::max(nat_height, min_height);
            std::clog << "preferred_height_for_width: " << min_height << ", " << nat_height << std::endl;
        }

        virtual bool on_enter_notify_event(GdkEventCrossing* event) override {
            HeaderTabChild* child = find_child_at_pos(event->x, event->y);
            if (child != m_prelight_child) {
                if (m_prelight_child) {
                    m_prelight_child->unset_state_flags(Gtk::STATE_FLAG_PRELIGHT);
                }
                if (child) {
                    child->set_state_flags(Gtk::STATE_FLAG_PRELIGHT, false);
                }
                m_prelight_child = child;
            }
            return Gtk::Container::on_enter_notify_event(event);
        }

        virtual bool on_leave_notify_event(GdkEventCrossing* event) override {
            if (m_prelight_child) {
                m_prelight_child->unset_state_flags(Gtk::STATE_FLAG_PRELIGHT);
                m_prelight_child = nullptr;
            }
            return Gtk::Container::on_leave_notify_event(event);
        }

        virtual bool on_motion_notify_event(GdkEventMotion* event) override {
            double relative_x = event->x;
            double relative_y = event->y;
            fix_coordinates(Glib::wrap(event->window, true),
                            relative_x,
                            relative_y);

            HeaderTabChild* child = find_child_at_pos(relative_x, relative_y);
            if (child != m_prelight_child) {
                if (m_prelight_child) {
                    m_prelight_child->unset_state_flags(Gtk::STATE_FLAG_PRELIGHT);
                }
                if (child) {
                    child->set_state_flags(Gtk::STATE_FLAG_PRELIGHT, false);
                }
                m_prelight_child = child;
            }
            return Gtk::Container::on_motion_notify_event(event);
        }

        virtual bool on_button_release_event(GdkEventButton* event) override {
            double relative_x = event->x;
            double relative_y = event->y;
            fix_coordinates(Glib::wrap(event->window, true),
                            relative_x,
                            relative_y);

            if (event->button == 1) {
                HeaderTabChild* child = find_child_at_pos(relative_x, relative_y);
                if (child == &m_more) {
                    //open popover
                    m_more_popover.set_position(Gtk::POS_BOTTOM);
                    m_more_popover.set_relative_to(m_more);
                    m_more_popover.show();
                } else {
                    if (child != m_selected_child) {
                        if (m_selected_child) {
                            m_selected_child->unset_state_flags(Gtk::STATE_FLAG_SELECTED);
                        }
                        if (child) {
                            child->set_state_flags(Gtk::STATE_FLAG_SELECTED, false);
                        }
                        m_selected_child = child;
                    }
                }
            }
            return Gtk::Container::on_button_release_event(event);
        }

        virtual void on_realize() override {
            Gtk::Allocation alloc = get_allocation();
            Gtk::Widget::set_realized(true);
            GdkWindowAttr attr;
            memset(&attr, 0, sizeof(attr));
            attr.x = alloc.get_x();
            attr.y = alloc.get_y();
            attr.width = alloc.get_width();
            attr.height = alloc.get_height();
            std::clog << attr.x << ", " << attr.y << "," << attr.width << "," << attr.height << std::endl;
            attr.window_type = GDK_WINDOW_CHILD;
            attr.event_mask = get_events();
            attr.wclass = GDK_INPUT_OUTPUT;
            auto window = Gdk::Window::create(get_parent_window(),
                                              &attr,
                                              GDK_WA_X | GDK_WA_Y);
            register_window(window);
            set_window(window);
        }

    private:
        HeaderTabChild* find_child_at_pos(int x, int y) {
            for (HeaderTabChildBox* child : m_children) {
                if (child->m_widget->get_parent() == child) {
                    auto rect = child->get_allocation();
                    if (x >= rect.get_x() &&
                        x < (rect.get_x() + rect.get_width()) &&
                        y >= rect.get_y() &&
                        y < (rect.get_y() + rect.get_height())) {
                        return (HeaderTabChild*)child->m_widget;
                    }
                }
            }
            HeaderTabChild* child = &m_more;
            auto rect = child->get_allocation();
            if (x >= rect.get_x() &&
                x < (rect.get_x() + rect.get_width()) &&
                y >= rect.get_y() &&
                y < (rect.get_y() + rect.get_height())) {
                return (HeaderTabChild*)child;
            }
            return nullptr;
        }

        void fix_coordinates(Glib::RefPtr<Gdk::Window> event_window,
                             double& relative_x,
                             double& relative_y) {
            while (event_window && event_window != get_window()) {
                double parent_x, parent_y;
                event_window->coords_to_parent(relative_x, relative_y,
                                               parent_x, parent_y);
                relative_x = parent_x;
                relative_y = parent_y;
                event_window = event_window->get_effective_parent();
            }
        }

};

int headerbartab_test(int argc, char *argv[]) {
    Glib::RefPtr<Gtk::Application> app =
            Gtk::Application::create(argc, argv,
                                     "org.gtkmm.examples.base");

    auto css = Gtk::CssProvider::create();
    auto screen = Gdk::Screen::get_default();
    Gtk::StyleContext::add_provider_for_screen(
                screen,
                css,
                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    auto update_style = [css]() {
        utils::debug::scope_log a(DBG_LVL_2("gtox"), {});
        bool dark = Gtk::Settings::get_default()
                    ->property_gtk_application_prefer_dark_theme();
        css->load_from_resource(dark?"/org/gtox/style/dark.css":"/org/gtox/style/light.css");
    };

    Gtk::Settings::get_default()
            ->property_gtk_application_prefer_dark_theme().signal_changed().connect(update_style);

    update_style();

    Gtk::Window window;
    window.set_default_size(200, 200);

    HeaderTab demo;
    demo.show();

    HeaderTabTitle a("LabelA", "Sub 1");
    HeaderTabTitle b("LabelB", "Sub 2");
    HeaderTabTitle c("LabelC", "Sub 3");
    HeaderTabTitle d("LabelD", "Sub 4");
    demo.add(a);
    demo.add(b);
    demo.add(c);
    demo.add(d);

    a.show();
    b.show();
    c.show();
    d.show();

    Gtk::HeaderBar bar;
    bar.set_custom_title(demo);
    bar.show();

    window.set_titlebar(bar);
    return app->run(window);
}

int main(int argc, char* argv[]) {
    return headerbartab_test(argc, argv);
    utils::debug::scope_log log(DBG_LVL_1("gtox"), {
                                  argc,
                                  utils::debug::parameter(argv, argc),
                              });

    std::set_terminate(terminate_handler);
    Glib::add_exception_handler(sigc::ptr_fun(&terminate_handler));

    Gtk::Main kit(argc, argv);
    Gst::init(argc, argv);

    setup_translation(
                Glib::path_get_dirname(
                    Glib::find_program_in_path(argv[0])));

    print_copyright();

    bool non_unique = false;
    if (argc > 1) {
        non_unique = std::any_of(argv, argv + argc, [](auto x) {
            return std::string("-non-unique") == x;
        });
    }
    Glib::RefPtr<gTox> application = gTox::create(non_unique);

    const int status = application->run(argc, argv);
    return status;
}
