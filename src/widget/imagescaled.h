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
#ifndef WIDGETIMAGESCALED_H
#define WIDGETIMAGESCALED_H

#include <gtkmm.h>
#include "utils/builder.h"
#include "utils/dispatcher.h"
#include "utils/debug.h"

namespace widget {
    class imagescaled : public Gtk::Image, public utils::debug::track_obj<imagescaled> {
        public:
            imagescaled();
            imagescaled(BaseObjectType* cobject,
                   utils::builder);
            ~imagescaled();

        protected:
            bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
            Gtk::SizeRequestMode get_request_mode_vfunc() const override;
            void get_preferred_width_vfunc(int& minimum_width,
                                           int& natural_width) const override;
            void get_preferred_height_vfunc(int& minimum_height,
                                            int& natural_height) const override;
            void get_preferred_height_for_width_vfunc(
                    int width,
                    int& minimum_height,
                    int& natural_height) const override;
            void get_preferred_width_for_height_vfunc(
                    int height,
                    int& minimum_width,
                    int& natural_width) const override;
    };
}
#endif
