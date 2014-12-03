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

#include <extdef.h>

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "logicsegment.hpp"

#include <libsigrok/libsigrok.hpp>

using std::lock_guard;
using std::recursive_mutex;
using std::max;
using std::min;
using std::pair;
using std::shared_ptr;

using sigrok::Logic;

namespace pv {
namespace data {

const int LogicSegment::MipMapScalePower = 4;
const int LogicSegment::MipMapScaleFactor = 1 << MipMapScalePower;
const float LogicSegment::LogMipMapScaleFactor = logf(MipMapScaleFactor);
const uint64_t LogicSegment::MipMapDataUnit = 64*1024;	// bytes

LogicSegment::LogicSegment(shared_ptr<Logic> logic, uint64_t samplerate,
                             const uint64_t expected_num_samples) :
	Segment(samplerate, logic->unit_size()),
	last_append_sample_(0)
{
	set_capacity(expected_num_samples);

	lock_guard<recursive_mutex> lock(mutex_);
	memset(mip_map_, 0, sizeof(mip_map_));
	append_payload(logic);
}

LogicSegment::~LogicSegment()
{
	lock_guard<recursive_mutex> lock(mutex_);
	for (MipMapLevel &l : mip_map_)
		free(l.data);
}

uint64_t LogicSegment::unpack_sample(const uint8_t *ptr) const
{
#ifdef HAVE_UNALIGNED_LITTLE_ENDIAN_ACCESS
	return *(uint64_t*)ptr;
#else
	uint64_t value = 0;
	switch(unit_size_) {
	default:
		value |= ((uint64_t)ptr[7]) << 56;
		/* FALLTHRU */
	case 7:
		value |= ((uint64_t)ptr[6]) << 48;
		/* FALLTHRU */
	case 6:
		value |= ((uint64_t)ptr[5]) << 40;
		/* FALLTHRU */
	case 5:
		value |= ((uint64_t)ptr[4]) << 32;
		/* FALLTHRU */
	case 4:
		value |= ((uint32_t)ptr[3]) << 24;
		/* FALLTHRU */
	case 3:
		value |= ((uint32_t)ptr[2]) << 16;
		/* FALLTHRU */
	case 2:
		value |= ptr[1] << 8;
		/* FALLTHRU */
	case 1:
		value |= ptr[0];
		/* FALLTHRU */
	case 0:
		break;
	}
	return value;
#endif
}

void LogicSegment::pack_sample(uint8_t *ptr, uint64_t value)
{
#ifdef HAVE_UNALIGNED_LITTLE_ENDIAN_ACCESS
	*(uint64_t*)ptr = value;
#else
	switch(unit_size_) {
	default:
		ptr[7] = value >> 56;
		/* FALLTHRU */
	case 7:
		ptr[6] = value >> 48;
		/* FALLTHRU */
	case 6:
		ptr[5] = value >> 40;
		/* FALLTHRU */
	case 5:
		ptr[4] = value >> 32;
		/* FALLTHRU */
	case 4:
		ptr[3] = value >> 24;
		/* FALLTHRU */
	case 3:
		ptr[2] = value >> 16;
		/* FALLTHRU */
	case 2:
		ptr[1] = value >> 8;
		/* FALLTHRU */
	case 1:
		ptr[0] = value;
		/* FALLTHRU */
	case 0:
		break;
	}
#endif
}

void LogicSegment::append_payload(shared_ptr<Logic> logic)
{
	assert(unit_size_ == logic->unit_size());
	assert((logic->data_length() % unit_size_) == 0);

	lock_guard<recursive_mutex> lock(mutex_);

	append_data(logic->data_pointer(),
		logic->data_length() / unit_size_);

	// Generate the first mip-map from the data
	append_payload_to_mipmap();
}

void LogicSegment::get_samples(uint8_t *const data,
	int64_t start_sample, int64_t end_sample) const
{
	assert(data);
	assert(start_sample >= 0);
	assert(start_sample <= (int64_t)sample_count_);
	assert(end_sample >= 0);
	assert(end_sample <= (int64_t)sample_count_);
	assert(start_sample <= end_sample);

	lock_guard<recursive_mutex> lock(mutex_);

	const size_t size = (end_sample - start_sample) * unit_size_;
	memcpy(data, (const uint8_t*)data_.data() + start_sample * unit_size_, size);
}

void LogicSegment::remove_samples(int64_t start_sample, int64_t end_sample)
{
	assert(start_sample >= 0);
	assert(start_sample <= (int64_t)sample_count_);
	assert(end_sample >= 0);
	assert(end_sample <= (int64_t)sample_count_);

	lock_guard<recursive_mutex> lock(mutex_);

	memset(mip_map_, 0, sizeof(mip_map_));

	auto start_iter = data_.begin() + start_sample * unit_size_;
	auto end_iter = data_.begin() + end_sample * unit_size_;
	data_.erase(start_iter, end_iter);
	sample_count_ -= end_sample - start_sample;

	append_payload_to_mipmap();
}

void LogicSegment::reallocate_mipmap_level(MipMapLevel &m)
{
	const uint64_t new_data_length = ((m.length + MipMapDataUnit - 1) /
		MipMapDataUnit) * MipMapDataUnit;
	if (new_data_length > m.data_length)
	{
		m.data_length = new_data_length;

		// Padding is added to allow for the uint64_t write word
		m.data = realloc(m.data, new_data_length * unit_size_ +
			sizeof(uint64_t));
	}
}

void LogicSegment::append_payload_to_mipmap()
{
	MipMapLevel &m0 = mip_map_[0];
	uint64_t prev_length;
	const uint8_t *src_ptr;
	uint8_t *dest_ptr;
	uint64_t accumulator;
	unsigned int diff_counter;

	// Expand the data buffer to fit the new samples
	prev_length = m0.length;
	m0.length = sample_count_ / MipMapScaleFactor;

	// Break off if there are no new samples to compute
	if (m0.length == prev_length)
		return;

	reallocate_mipmap_level(m0);

	dest_ptr = (uint8_t*)m0.data + prev_length * unit_size_;

	// Iterate through the samples to populate the first level mipmap
	const uint8_t *const end_src_ptr = (uint8_t*)data_.data() +
		m0.length * unit_size_ * MipMapScaleFactor;
	for (src_ptr = (uint8_t*)data_.data() +
		prev_length * unit_size_ * MipMapScaleFactor;
		src_ptr < end_src_ptr;)
	{
		// Accumulate transitions which have occurred in this sample
		accumulator = 0;
		diff_counter = MipMapScaleFactor;
		while (diff_counter-- > 0)
		{
			const uint64_t sample = unpack_sample(src_ptr);
			accumulator |= last_append_sample_ ^ sample;
			last_append_sample_ = sample;
			src_ptr += unit_size_;
		}

		pack_sample(dest_ptr, accumulator);
		dest_ptr += unit_size_;
	}

	// Compute higher level mipmaps
	for (unsigned int level = 1; level < ScaleStepCount; level++)
	{
		MipMapLevel &m = mip_map_[level];
		const MipMapLevel &ml = mip_map_[level-1];

		// Expand the data buffer to fit the new samples
		prev_length = m.length;
		m.length = ml.length / MipMapScaleFactor;

		// Break off if there are no more samples to computed
		if (m.length == prev_length)
			break;

		reallocate_mipmap_level(m);

		// Subsample the level lower level
		src_ptr = (uint8_t*)ml.data +
			unit_size_ * prev_length * MipMapScaleFactor;
		const uint8_t *const end_dest_ptr =
			(uint8_t*)m.data + unit_size_ * m.length;
		for (dest_ptr = (uint8_t*)m.data +
			unit_size_ * prev_length;
			dest_ptr < end_dest_ptr;
			dest_ptr += unit_size_)
		{
			accumulator = 0;
			diff_counter = MipMapScaleFactor;
			while (diff_counter-- > 0)
			{
				accumulator |= unpack_sample(src_ptr);
				src_ptr += unit_size_;
			}

			pack_sample(dest_ptr, accumulator);
		}
	}
}

uint64_t LogicSegment::get_sample(uint64_t index) const
{
	assert(index < sample_count_);

	return unpack_sample((uint8_t*)data_.data() + index * unit_size_);
}

void LogicSegment::get_subsampled_edges(
	std::vector<EdgePair> &edges,
	uint64_t start, uint64_t end,
	float min_length, int sig_index)
{
	uint64_t index = start;
	unsigned int level;
	bool last_sample;
	bool fast_forward;

	assert(end <= get_sample_count());
	assert(start <= end);
	assert(min_length > 0);
	assert(sig_index >= 0);
	assert(sig_index < 64);

	lock_guard<recursive_mutex> lock(mutex_);

	const uint64_t block_length = (uint64_t)max(min_length, 1.0f);
	const unsigned int min_level = max((int)floorf(logf(min_length) /
		LogMipMapScaleFactor) - 1, 0);
	const uint64_t sig_mask = 1ULL << sig_index;

	// Store the initial state
	last_sample = (get_sample(start) & sig_mask) != 0;
	edges.push_back(pair<int64_t, bool>(index++, last_sample));

	while (index + block_length <= end)
	{
		//----- Continue to search -----//
		level = min_level;

		// We cannot fast-forward if there is no mip-map data at
		// at the minimum level.
		fast_forward = (mip_map_[level].data != NULL);

		if (min_length < MipMapScaleFactor)
		{
			// Search individual samples up to the beginning of
			// the next first level mip map block
			const uint64_t final_index = min(end,
				pow2_ceil(index, MipMapScalePower));

			for (; index < final_index &&
				(index & ~(~0 << MipMapScalePower)) != 0;
				index++)
			{
				const bool sample =
					(get_sample(index) & sig_mask) != 0;

				// If there was a change we cannot fast forward
				if (sample != last_sample) {
					fast_forward = false;
					break;
				}
			}
		}
		else
		{
			// If resolution is less than a mip map block,
			// round up to the beginning of the mip-map block
			// for this level of detail
			const int min_level_scale_power =
				(level + 1) * MipMapScalePower;
			index = pow2_ceil(index, min_level_scale_power);
			if (index >= end)
				break;

			// We can fast forward only if there was no change
			const bool sample =
				(get_sample(index) & sig_mask) != 0;
			if (last_sample != sample)
				fast_forward = false;
		}

		if (fast_forward) {

			// Fast forward: This involves zooming out to higher
			// levels of the mip map searching for changes, then
			// zooming in on them to find the point where the edge
			// begins.

			// Slide right and zoom out at the beginnings of mip-map
			// blocks until we encounter a change
			while (1) {
				const int level_scale_power =
					(level + 1) * MipMapScalePower;
				const uint64_t offset =
					index >> level_scale_power;

				// Check if we reached the last block at this
				// level, or if there was a change in this block
				if (offset >= mip_map_[level].length ||
					(get_subsample(level, offset) &
						sig_mask))
					break;

				if ((offset & ~(~0 << MipMapScalePower)) == 0) {
					// If we are now at the beginning of a
					// higher level mip-map block ascend one
					// level
					if (level + 1 >= ScaleStepCount ||
						!mip_map_[level + 1].data)
						break;

					level++;
				} else {
					// Slide right to the beginning of the
					// next mip map block
					index = pow2_ceil(index + 1,
						level_scale_power);
				}
			}

			// Zoom in, and slide right until we encounter a change,
			// and repeat until we reach min_level
			while (1) {
				assert(mip_map_[level].data);

				const int level_scale_power =
					(level + 1) * MipMapScalePower;
				const uint64_t offset =
					index >> level_scale_power;

				// Check if we reached the last block at this
				// level, or if there was a change in this block
				if (offset >= mip_map_[level].length ||
					(get_subsample(level, offset) &
						sig_mask)) {
					// Zoom in unless we reached the minimum
					// zoom
					if (level == min_level)
						break;

					level--;
				} else {
					// Slide right to the beginning of the
					// next mip map block
					index = pow2_ceil(index + 1,
						level_scale_power);
				}
			}

			// If individual samples within the limit of resolution,
			// do a linear search for the next transition within the
			// block
			if (min_length < MipMapScaleFactor) {
				for (; index < end; index++) {
					const bool sample = (get_sample(index) &
						sig_mask) != 0;
					if (sample != last_sample)
						break;
				}
			}
		}

		//----- Store the edge -----//

		// Take the last sample of the quanization block
		const int64_t final_index = index + block_length;
		if (index + block_length > end)
			break;

		// Store the final state
		const bool final_sample =
			(get_sample(final_index - 1) & sig_mask) != 0;
		edges.push_back(pair<int64_t, bool>(index, final_sample));

		index = final_index;
		last_sample = final_sample;
	}

	// Add the final state
	const bool end_sample = get_sample(end) & sig_mask;
	if (last_sample != end_sample)
		edges.push_back(pair<int64_t, bool>(end, end_sample));
	edges.push_back(pair<int64_t, bool>(end + 1, end_sample));
}

uint64_t LogicSegment::get_subsample(int level, uint64_t offset) const
{
	assert(level >= 0);
	assert(mip_map_[level].data);
	return unpack_sample((uint8_t*)mip_map_[level].data +
		unit_size_ * offset);
}

uint64_t LogicSegment::pow2_ceil(uint64_t x, unsigned int power)
{
	const uint64_t p = 1 << power;
	return (x + p - 1) / p * p;
}

} // namespace data
} // namespace pv
