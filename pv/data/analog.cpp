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

#include <cassert>

#include "analog.hpp"
#include "analogsegment.hpp"

using std::deque;
using std::max;
using std::shared_ptr;
using std::vector;

namespace pv {
namespace data {

Analog::Analog() :
	SignalData()
{
}

void Analog::push_segment(shared_ptr<AnalogSegment> &segment)
{
	segments_.push_front(segment);
}

const deque< shared_ptr<AnalogSegment> >& Analog::analog_segments() const
{
	return segments_;
}

vector< shared_ptr<Segment> > Analog::segments() const
{
	return vector< shared_ptr<Segment> >(
		segments_.begin(), segments_.end());
}

void Analog::clear()
{
	segments_.clear();
}

void Analog::remove(double start_time, double end_time)
{
	if (!segments_.size())
		return;

	if (start_time < 0)
		start_time = 0;
	if (end_time < 0)
		end_time = 0;
	if (start_time >= end_time)
		return;

	erase(start_time, end_time);
}

void Analog::crop(double start_time, double end_time)
{
	if (!segments_.size())
		return;

	if (start_time < 0)
		start_time = 0;
	if (end_time < 0)
		end_time = 0;
	if (start_time >= end_time)
		return;

	double total_time = 0;
	for (auto segment : segments_) {
		double segment_time = segment->time();
		total_time += segment_time;
	}

	if (end_time < total_time)
		erase(end_time, total_time);
	erase(0, start_time);
}

void Analog::erase(double start_time, double end_time)
{
	double segment_time = 0;
	auto iter = segments_.begin();

	// find start
	for (; iter != segments_.end(); ++iter) {
		segment_time = (*iter)->time();
		if (start_time < segment_time)
			break;

		start_time -= segment_time;
		end_time -= segment_time;
	}

	// end is also in this segment?
	if (end_time < segment_time) {
		iter = erase(iter, start_time, end_time);
		return;
	}

	// end is not in this segment, remove initial samples
	iter = erase(iter, start_time, segment_time);
	end_time -= segment_time;

	// remove frames till we reach end segment
	for (; iter != segments_.end();) {
		segment_time = (*iter)->time();
		if (end_time < segment_time)
			break;

		iter = segments_.erase(iter);
		end_time -= segment_time;
	}

	// remove remaining samples
	if (iter != segments_.end())
		iter = erase(iter, 0, end_time);
}

Analog::segment_iterator Analog::erase(Analog::segment_iterator iter,
	double start_time, double end_time)
{
	uint64_t sample_count = (*iter)->get_sample_count();
	uint64_t samplerate = (*iter)->samplerate();

	int64_t start_sample = start_time * samplerate;
	int64_t end_sample = end_time * samplerate;
	int64_t length = end_sample - start_sample;

	// erase entire segment?
	if (length == (int64_t) sample_count)
		return segments_.erase(iter);

	(*iter)->remove_samples(start_sample, end_sample);
	return ++iter;
}

uint64_t Analog::get_max_sample_count() const
{
	uint64_t l = 0;
	for (const std::shared_ptr<AnalogSegment> s : segments_) {
		assert(s);
		l = max(l, s->get_sample_count());
	}
	return l;
}

} // namespace data
} // namespace pv
