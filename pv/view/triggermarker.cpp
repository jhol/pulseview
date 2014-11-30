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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "triggermarker.hpp"
#include "view.hpp"

#include <QColor>

#include <libsigrok/libsigrok.hpp>

using std::shared_ptr;

namespace pv {
namespace view {

const QColor TriggerMarker::FillColour(0xED, 0xD4, 0x00);

TriggerMarker::TriggerMarker(View &view) :
	TimeMarker(view, FillColour, 0.0)
{
}

bool TriggerMarker::enabled() const
{
	return (bool)view_.session().session()->trigger();
}

QString TriggerMarker::get_text() const
{
	return "T";
}

} // namespace view
} // namespace pv
