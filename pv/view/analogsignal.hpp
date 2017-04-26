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

#ifndef PULSEVIEW_PV_VIEWS_TRACEVIEW_ANALOGSIGNAL_HPP
#define PULSEVIEW_PV_VIEWS_TRACEVIEW_ANALOGSIGNAL_HPP

#include "signal.hpp"

#include <memory>

#include <QComboBox>

using std::pair;
using std::shared_ptr;

namespace pv {

namespace data {
class Analog;
class AnalogSegment;
class SignalBase;
}

namespace views {
namespace TraceView {

class AnalogSignal : public Signal
{
	Q_OBJECT

private:
	static const QColor SignalColours[4];
	static const QColor GridMajorColor, GridMinorColor;
	static const QColor SamplingPointColour;

	static const int64_t TracePaintBlockSize;
	static const float EnvelopeThreshold;

	static const int MaximumVDivs;
	static const int MaxScaleIndex, MinScaleIndex;
	static const int InfoTextMarginRight, InfoTextMarginBottom;

	enum DisplayType {
		DisplayAnalog = 0,
		DisplayConverted = 1,
		DisplayBoth = 2
	};

public:
	AnalogSignal(pv::Session &session, shared_ptr<data::SignalBase> base);

	virtual ~AnalogSignal() = default;

	shared_ptr<pv::data::SignalData> data() const;

	virtual void save_settings(QSettings &settings) const;

	virtual void restore_settings(QSettings &settings);

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
	 * @copydoc pv::view::Signal::signal_scale_handle_drag_release()
	 */
	void scale_handle_drag_release();

	/**
	 * Paints the background layer of the signal with a QPainter
	 * @param p the QPainter to paint into.
	 * @param pp the painting parameters object to paint with..
	 */
	void paint_back(QPainter &p, ViewItemPaintParams &pp);

	/**
	 * Paints the mid-layer of the signal with a QPainter
	 * @param p the QPainter to paint into.
	 * @param pp the painting parameters object to paint with..
	 */
	void paint_mid(QPainter &p, ViewItemPaintParams &pp);

	/**
	 * Paints the foreground layer of the item with a QPainter
	 * @param p the QPainter to paint into.
	 * @param pp the painting parameters object to paint with.
	 */
	void paint_fore(QPainter &p, ViewItemPaintParams &pp);

private:
	void paint_grid(QPainter &p, int y, int left, int right);

	void paint_trace(QPainter &p,
		const shared_ptr<pv::data::AnalogSegment> &segment,
		int y, int left, const int64_t start, const int64_t end,
		const double pixels_offset, const double samples_per_pixel);

	void paint_envelope(QPainter &p,
		const shared_ptr<pv::data::AnalogSegment> &segment,
		int y, int left, const int64_t start, const int64_t end,
		const double pixels_offset, const double samples_per_pixel);

	void paint_logic_mid(QPainter &p, ViewItemPaintParams &pp);

	void paint_logic_caps(QPainter &p, QLineF *const lines,
		vector< pair<int64_t, bool> > &edges,
		bool level, double samples_per_pixel, double pixels_offset,
		float x_offset, float y_offset);

	/**
	 * Computes the scale factor from the scale index and vdiv settings.
	 */
	float get_resolution(int scale_index);

	void update_scale();

	void update_conversion_type();

	void perform_autoranging(bool force_update = false);

protected:
	void populate_popup_form(QWidget *parent, QFormLayout *form);

private Q_SLOTS:
	void on_samples_added();

	void on_pos_vdivs_changed(int vdivs);
	void on_neg_vdivs_changed(int vdivs);

	void on_resolution_changed(int index);

	void on_autoranging_changed(int state);

	void on_conversion_changed(int index);

	void on_display_type_changed(int index);

private:
	QComboBox *resolution_cb_, *conversion_cb_, *display_type_cb_;

	float scale_;
	int scale_index_;
	int scale_index_drag_offset_;

	int div_height_;
	int pos_vdivs_, neg_vdivs_;  // divs per positive/negative side
	float resolution_; // e.g. 10 for 10 V/div

	data::SignalBase::ConversionType conversion_type_;
	DisplayType display_type_;
	bool autoranging_;
};

} // namespace TraceView
} // namespace views
} // namespace pv

#endif // PULSEVIEW_PV_VIEWS_TRACEVIEW_ANALOGSIGNAL_HPP
