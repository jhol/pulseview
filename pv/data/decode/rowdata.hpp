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

#ifndef PULSEVIEW_PV_DATA_DECODE_ROWDATA_HPP
#define PULSEVIEW_PV_DATA_DECODE_ROWDATA_HPP

#include <vector>

#include "annotation.hpp"

namespace pv {
namespace data {
namespace decode {

class RowData
{
public:
	RowData();

public:
	uint64_t get_max_sample() const;

	/**
	 * Extracts sorted annotations between two period into a vector.
	 */
	void get_annotation_subset(
		std::vector<pv::data::decode::Annotation> &dest,
		uint64_t start_sample, uint64_t end_sample) const;

	void push_annotation(const Annotation &a);

private:
	std::vector<Annotation> annotations_;
};

}
} // data
} // pv

#endif // PULSEVIEW_PV_DATA_DECODE_ROWDATA_HPP
