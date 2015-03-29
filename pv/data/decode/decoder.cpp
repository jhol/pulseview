/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2013 Joel Holdsworth <joel@airwebreathe.org.uk>
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

#include <libsigrokcxx/libsigrokcxx.hpp>
#include <libsigrokdecode/libsigrokdecode.h>

#include "decoder.hpp"

#include <pv/view/logicsignal.hpp>

using std::set;
using std::map;
using std::shared_ptr;
using std::string;

namespace pv {
namespace data {
namespace decode {

Decoder::Decoder(const srd_decoder *const dec) :
	decoder_(dec),
	shown_(true)
{
}

Decoder::~Decoder()
{
	for (auto i = options_.begin(); i != options_.end(); i++)
		g_variant_unref((*i).second);
}

const srd_decoder* Decoder::decoder() const
{
	return decoder_;
}

bool Decoder::shown() const
{
	return shown_;
}

void Decoder::show(bool show)
{
	shown_ = show;
}

const map<const srd_channel*, shared_ptr<view::LogicSignal> >&
Decoder::channels() const
{
	return channels_;
}

void Decoder::set_channels(std::map<const srd_channel*,
	std::shared_ptr<view::LogicSignal> > channels)
{
	channels_ = channels;
}

const std::map<std::string, GVariant*>& Decoder::options() const
{
	return options_;
}

void Decoder::set_option(const char *id, GVariant *value)
{
	assert(value);
	g_variant_ref(value);
	options_[id] = value;
}

bool Decoder::have_required_channels() const
{
	for (GSList *l = decoder_->channels; l; l = l->next) {
		const srd_channel *const pdch = (const srd_channel*)l->data;
		assert(pdch);
		if (channels_.find(pdch) == channels_.end())
			return false;
	}

	return true;
}

set< shared_ptr<pv::data::Logic> > Decoder::get_data()
{
	set< shared_ptr<pv::data::Logic> > data;
	for(auto i = channels_.cbegin(); i != channels_.cend(); i++) {
		shared_ptr<view::LogicSignal> signal((*i).second);
		assert(signal);
		data.insert(signal->logic_data());
	}

	return data;
}

srd_decoder_inst* Decoder::create_decoder_inst(srd_session *session, int unit_size) const
{
	GHashTable *const opt_hash = g_hash_table_new_full(g_str_hash,
		g_str_equal, g_free, (GDestroyNotify)g_variant_unref);

	for (auto i = options_.cbegin(); i != options_.cend(); i++)
	{
		GVariant *const value = (*i).second;
		g_variant_ref(value);
		g_hash_table_replace(opt_hash, (void*)g_strdup(
			(*i).first.c_str()), value);
	}

	srd_decoder_inst *const decoder_inst = srd_inst_new(
		session, decoder_->id, opt_hash);
	g_hash_table_destroy(opt_hash);

	if(!decoder_inst)
		return NULL;

	// Setup the channels
	GHashTable *const channels = g_hash_table_new_full(g_str_hash,
		g_str_equal, g_free, (GDestroyNotify)g_variant_unref);

	for(auto i = channels_.cbegin(); i != channels_.cend(); i++)
	{
		shared_ptr<view::LogicSignal> signal((*i).second);
		GVariant *const gvar = g_variant_new_int32(
			signal->channel()->index());
		g_variant_ref_sink(gvar);
		g_hash_table_insert(channels, (*i).first->id, gvar);
	}

	srd_inst_channel_set_all(decoder_inst, channels, unit_size);

	return decoder_inst;
}

} // decode
} // data
} // pv
