/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2014 Joel Holdsworth <joel@airwebreathe.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PULSEVIEW_PV_VIEWS_TRACEVIEW_VIEWITEMPAINTPARAMS_HPP
#define PULSEVIEW_PV_VIEWS_TRACEVIEW_VIEWITEMPAINTPARAMS_HPP

#include "pv/util.hpp"

#include <QFont>
#include <QRect>

namespace pv {
namespace views {
namespace TraceView {

class ViewItemPaintParams
{
public:
	ViewItemPaintParams(
		const QRect &rect, double scale, const pv::util::Timestamp& offset);

	QRect rect() const {
		return rect_;
	}

	double scale() const {
		return scale_;
	}

	const pv::util::Timestamp& offset() const {
		return offset_;
	}

	int left() const {
		return rect_.left();
	}

	int right() const {
		return rect_.right();
	}

	int top() const {
		return rect_.top();
	}

	int bottom() const {
		return rect_.bottom();
	}

	int width() const {
		return rect_.width();
	}

	int height() const {
		return rect_.height();
	}

	double pixels_offset() const {
		return (offset_ / scale_).convert_to<double>();
	}

	bool next_bg_colour_state() {
		const bool state = bg_colour_state_;
		bg_colour_state_ = !bg_colour_state_;
		return state;
	}

public:
	static QFont font();

	static int text_height();

private:
	QRect rect_;
	double scale_;
	pv::util::Timestamp offset_;
	bool bg_colour_state_;
};

} // namespace TraceView
} // namespace views
} // namespace pv

#endif // PULSEVIEW_PV_VIEWS_TRACEVIEW_VIEWITEMPAINTPARAMS_HPP
