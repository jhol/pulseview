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

#ifndef PULSEVIEW_PV_VIEW_TRIGGERMARKER_H
#define PULSEVIEW_PV_VIEW_TRIGGERMARKER_H

#include "timemarker.hpp"

namespace pv {
namespace view {

class TriggerMarker : public TimeMarker
{
	Q_OBJECT

public:
	static const QColor FillColour;

public:
	/**
	 * Constructor.
	 * @param view A reference to the view that owns this cursor pair.
	 */
	TriggerMarker(View &view);

	/**
	 * Returns true if the item is visible and enabled.
	 */
	bool enabled() const;

	/**
	 * Gets the text to show in the marker.
	 */
	QString get_text() const;
};

} // namespace view
} // namespace pv

#endif // PULSEVIEW_PV_VIEW_TRIGGERMARKER_H
