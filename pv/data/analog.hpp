/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
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

#ifndef PULSEVIEW_PV_DATA_ANALOG_H
#define PULSEVIEW_PV_DATA_ANALOG_H

#include "signaldata.hpp"

#include <deque>
#include <memory>

namespace pv {
namespace data {

class AnalogSegment;

class Analog : public SignalData
{
public:
	Analog();

	void push_segment(
		std::shared_ptr<AnalogSegment> &segment);

	const std::deque< std::shared_ptr<AnalogSegment> >&
		analog_segments() const;

	std::vector< std::shared_ptr<Segment> > segments() const;

	void clear();
	void remove(double start_time, double end_time);
	void crop(double start_time, double end_time);

	uint64_t get_max_sample_count() const;

private:
	typedef std::deque< std::shared_ptr<AnalogSegment> >::iterator segment_iterator;

	void erase(double start_time, double end_time);
	segment_iterator erase(segment_iterator iter,
		double start_time, double end_time);

private:
	std::deque< std::shared_ptr<AnalogSegment> > segments_;
};

} // namespace data
} // namespace pv

#endif // PULSEVIEW_PV_DATA_ANALOG_H
