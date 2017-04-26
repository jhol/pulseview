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

#include <cassert>

#include <QApplication>
#include <QFontMetrics>

#include "viewitempaintparams.hpp"

namespace pv {
namespace views {
namespace TraceView {

ViewItemPaintParams::ViewItemPaintParams(
	const QRect &rect, double scale, const pv::util::Timestamp& offset) :
	rect_(rect),
	scale_(scale),
	offset_(offset),
	bg_colour_state_(false)
{
	assert(scale > 0.0);
}

QFont ViewItemPaintParams::font()
{
	return QApplication::font();
}

int ViewItemPaintParams::text_height()
{
	return QFontMetrics(font()).height();
}

} // namespace TraceView
} // namespace views
} // namespace pv
