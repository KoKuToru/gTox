/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2015  Luca Béla Palkovics
    Copyright (C) 2015  Maurice Mohlek

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
#ifndef H_GTOX_BUILDER
#define H_GTOX_BUILDER

#include <gtkmm.h>
#include <glibmm.h>

namespace utils {
    /**
 * @brief Proxy class for Gtk::Builder
 */
    class builder {
        private:
            Glib::RefPtr<Gtk::Builder> m_builder;

        public:
            template<typename T>
            class ref {
                private:
                    Glib::RefPtr<Gtk::Builder> m_builder;
                    T* m_ptr;
                public:
                    ref(utils::builder builder,
                        T* ptr) :
                        m_builder(builder),
                        m_ptr(ptr) {
                    }
                    T& operator *() {
                        return *m_ptr;
                    }
                    const T& operator*() const {
                        return *m_ptr;
                    }
                    T* operator ->() {
                        return m_ptr;
                    }
                    const T* operator ->() const {
                        return m_ptr;
                    }
                    T* raw() {
                        return m_ptr;
                    }
                    const T* raw() const {
                        return m_ptr;
                    }

            };

            builder(Glib::RefPtr<Gtk::Builder> builder);
            Glib::RefPtr<Gtk::Builder> operator->();
            operator Glib::RefPtr<Gtk::Builder>() const {
                return m_builder;
            }

            template <class T_Widget, typename ...  T> inline
            T_Widget* get_widget_derived(const Glib::ustring& name, T&& ... params) {
                T_Widget* widget = nullptr;

                // Get the widget from the GtkBuilder file.
                typedef typename T_Widget::BaseObjectType cwidget_type;
                cwidget_type* pCWidget = (cwidget_type*)get_cwidget(name);

                if(!pCWidget) {
                    throw std::runtime_error("builder - The error was already reported by get_cwidget().");
                }

                //Check whether there is already a C++ wrapper instance associated with this C instance:
                auto pObjectBase = Glib::ObjectBase::_get_current_wrapper((GObject*)pCWidget);

                //If there is already a C++ instance:
                if(pObjectBase) {
                    widget = dynamic_cast<T_Widget*>( Glib::wrap((GtkWidget*)pCWidget) );
                    if (!widget) {
                        throw std::runtime_error("builder - C++ widget already exists, but different type ?");
                    }
                }
                else
                {
                    //Create a new C++ instance to wrap the existing C instance:
                    widget = new T_Widget(pCWidget, m_builder, params ...);
                }

                return widget;
            }

            template <class T_Widget>
            T_Widget* get_widget(const Glib::ustring& name) {
                T_Widget* widget = nullptr;
                m_builder->get_widget(name, widget);
                if (!widget) {
                    throw std::runtime_error("builder - Couldn't find widget");
                }
                return widget;
            }

            template <class T_Widget>
            void get_widget(const Glib::ustring& name, T_Widget*& widget) {
                widget = nullptr;
                m_builder->get_widget(name, widget);
                if (!widget) {
                    throw std::runtime_error("builder - Couldn't find widget");
                }
            }

            template <class T_Widget, typename ...  T> static
            ref<T_Widget> create_ref(const Glib::ustring resource,
                              const Glib::ustring& name,
                              T&& ... params) {
                auto ori_builder = Gtk::Builder::create_from_resource(resource);
                auto builder = utils::builder(ori_builder);
                return { builder, builder
                        .get_widget_derived<T_Widget>(name, params ...) };
            }

        protected:
            GtkWidget* get_cwidget(const Glib::ustring& name);
    };
}
#endif
