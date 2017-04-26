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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PULSEVIEW_PV_VIEWS_TRACEVIEW_LOGICSIGNAL_HPP
#define PULSEVIEW_PV_VIEWS_TRACEVIEW_LOGICSIGNAL_HPP

#include <QCache>

#include "signal.hpp"

#include <memory>

using std::pair;
using std::shared_ptr;
using std::vector;

class QIcon;
class QToolBar;

namespace sigrok {
class TriggerMatchType;
}

namespace pv {

namespace devices {
class Device;
}

namespace data {
class Logic;
}

namespace views {
namespace TraceView {

class LogicSignal : public Signal
{
	Q_OBJECT

public:
	static const float Oversampling;

	static const QColor EdgeColour;
	static const QColor HighColour;
	static const QColor LowColour;
	static const QColor SamplingPointColour;

	static const QColor SignalColours[10];

	static QColor TriggerMarkerBackgroundColour;
	static const int TriggerMarkerPadding;
	static const char* TriggerMarkerIcons[8];

	LogicSignal(pv::Session &session,
		shared_ptr<devices::Device> device,
		shared_ptr<data::SignalBase> base);

	virtual ~LogicSignal() = default;

	shared_ptr<pv::data::SignalData> data() const;

	shared_ptr<pv::data::Logic> logic_data() const;

	/**
	 * Computes the vertical extents of the contents of this row item.
	 * @return A pair containing the minimum and maximum y-values.
	 */
	pair<int, int> v_extents() const;

	/**
	 * Returns the offset to show the drag handle.
	 */
	int scale_handle_offset() const;

	/**
	 * Handles the scale handle being dragged to an offset.
	 * @param offset the offset the scale handle was dragged to.
	 */
	void scale_handle_dragged(int offset);

	/**
	 * Paints the mid-layer of the signal with a QPainter
	 * @param p the QPainter to paint into.
	 * @param pp the painting parameters object to paint with..
	 */
	void paint_mid(QPainter &p, ViewItemPaintParams &pp);

	/**
	 * Paints the foreground layer of the signal with a QPainter
	 * @param p the QPainter to paint into.
	 * @param pp the painting parameters object to paint with.
	 */
	virtual void paint_fore(QPainter &p, ViewItemPaintParams &pp);

private:
	void paint_caps(QPainter &p, QLineF *const lines,
		vector< pair<int64_t, bool> > &edges,
		bool level, double samples_per_pixel, double pixels_offset,
		float x_offset, float y_offset);

	void init_trigger_actions(QWidget *parent);

	const vector<int32_t> get_trigger_types() const;
	QAction* action_from_trigger_type(const sigrok::TriggerMatchType *type);
	const sigrok::TriggerMatchType* trigger_type_from_action(
		QAction *action);
	void populate_popup_form(QWidget *parent, QFormLayout *form);
	void modify_trigger();

	static const QIcon* get_icon(const char *path);
	static const QPixmap* get_pixmap(const char *path);

private Q_SLOTS:
	void on_trigger();

private:
	int signal_height_;

	shared_ptr<pv::devices::Device> device_;

	const sigrok::TriggerMatchType *trigger_match_;
	QToolBar *trigger_bar_;
	QAction *trigger_none_;
	QAction *trigger_rising_;
	QAction *trigger_high_;
	QAction *trigger_falling_;
	QAction *trigger_low_;
	QAction *trigger_change_;

	static QCache<QString, const QIcon> icon_cache_;
	static QCache<QString, const QPixmap> pixmap_cache_;
};

} // namespace TraceView
} // namespace views
} // namespace pv

#endif // PULSEVIEW_PV_VIEWS_TRACEVIEW_LOGICSIGNAL_HPP
