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

class HeaderTabChild: public /*Gtk::Frame*/Gtk::ToggleButton {
    private:
        Gtk::HBox m_box;
        Gtk::Button m_close;
        Gtk::Image m_close_icon;

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
            Gtk::ToggleButton::add(m_box);
        }

        virtual void add (Widget& widget) {
            m_box.add(widget);
        }
};

//https://developer.gnome.org/gtkmm-tutorial/stable/sec-custom-containers.html.en
class HeaderTab: public Gtk::Container {
    private:
        std::vector<HeaderTabChild*> m_children;

    public:
        HeaderTab(): Glib::ObjectBase(typeid(*this)) {
            set_has_window(false);
            set_redraw_on_allocate(false);
            get_style_context()->add_class("header-tab");
        }

        ~HeaderTab() {
            for (HeaderTabChild* child : m_children) {
                child->unparent();
            }
        }

        virtual void add(Widget& widget) {
            //add a new child
            HeaderTabChild* child = dynamic_cast<HeaderTabChild*>(&widget);
            if (!child) {
                child = new HeaderTabChild();
                child->add(widget);
                child->show();
            }
            child->set_parent(*this);
            m_children.push_back(child);
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
                                  gpointer    callback_data ) {
            for (Gtk::Widget* child : m_children) {
                callback(child->gobj(), callback_data);
            }
            if (include_internals) {
                //todo
            }
        }

        virtual GType child_type_vfunc() const {
            return Gtk::Widget::get_type();
        }

        virtual void on_size_allocate(Gtk::Allocation& allocation) {
            //Use the offered allocation for this container:
            set_allocation(allocation);

            //put all visible children into a row
            Gtk::Allocation child_allocation;
            child_allocation.set_x(allocation.get_x());
            child_allocation.set_y(allocation.get_y());
            child_allocation.set_width(0);
            child_allocation.set_height(allocation.get_height());
            for (HeaderTabChild* child : m_children) {
                if (!child->get_visible()) {
                    continue;
                }
                int child_min_width, child_nat_width;
                child->get_preferred_width_for_height(child_allocation.get_height(),
                                                      child_min_width,
                                                      child_nat_width);
                child_allocation.set_width(std::max(child_min_width,
                                                    child_nat_width));
                child->size_allocate(child_allocation);
                //place for next
                child_allocation.set_x(child_allocation.get_x() +
                                       child_allocation.get_width());
            }
        }

        /*virtual Gtk::SizeRequestMode get_request_mode_vfunc() const {
            return Gtk::SIZE_REQUEST_WIDTH_FOR_HEIGHT;
        }*/

        virtual void get_preferred_width_vfunc(int& min_width, int& nat_width) const {
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
            for (HeaderTabChild* child : m_children) {
                child->get_preferred_width(child_min_width,
                                           child_nat_width);
                nat_width += std::max(child_min_width,
                                      child_nat_width);
            }
            std::clog << "preferred_width: " << min_width << ", " << nat_width << std::endl;
        }

        virtual void get_preferred_height_vfunc(int& min_height, int& nat_height) const {
            int child_min_height, child_nat_height;
            min_height = 0;
            nat_height = 0;
            for (HeaderTabChild* child : m_children) {
                child->get_preferred_height(child_min_height,
                                            child_nat_height);
                min_height = std::max(min_height, child_min_height);
                nat_height = std::max(nat_height, child_nat_height);
            }
            std::clog << "preferred_height: " << min_height << ", " << nat_height << std::endl;
        }

        virtual void get_preferred_width_for_height_vfunc(int height,
                                                          int& min_width,
                                                          int& nat_width) const {
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
            for (HeaderTabChild* child : m_children) {
                child->get_preferred_width_for_height(
                            height,
                            child_min_width,
                            child_nat_width);
                nat_width += std::max(child_min_width,
                                      child_nat_width);
            }
            std::clog << "preferred_width_for_height: " << min_width << ", " << nat_width << std::endl;
        }

        virtual void get_preferred_height_for_width_vfunc(int width,
                                                          int& min_height,
                                                          int& nat_height) const {
            int child_min_height, child_nat_height;
            min_height = 0;
            nat_height = 0;
            for (HeaderTabChild* child : m_children) {
                child->get_preferred_height_for_width(
                            width, //this is hardly correct..
                            child_min_height,
                            child_nat_height);
                min_height = std::max(min_height, child_min_height);
                nat_height = std::max(nat_height, child_nat_height);
            }
            std::clog << "preferred_height_for_width: " << min_height << ", " << nat_height << std::endl;
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
    demo.add(a);
    demo.add(b);

    a.get_style_context()->set_state(Gtk::STATE_FLAG_SELECTED);

    a.show();
    b.show();

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
